/* gdatetime.c
 *
 * Copyright (C) 2009 Christian Hergert <chris@dronelabs.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "gdatetime.h"

#define GREGORIAN_LEAP(y)    (((y%4)==0)&&(!(((y%100)==0)&&((y%400)!=0))))
#define JULIAN_YEAR(d)       (d->julian/365.25)
#define DAYS_PER_PERIOD      (2914695)
#define USEC_PER_SECOND      (G_GINT64_CONSTANT (1000000))
#define USEC_PER_MINUTE      (G_GINT64_CONSTANT (60000000))
#define USEC_PER_HOUR        (G_GINT64_CONSTANT (3600000000))
#define USEC_PER_MILLISECOND (G_GINT64_CONSTANT (1000))
#define USEC_PER_DAY         (G_GINT64_CONSTANT (86400000000))
#define ADD_DAYS(d,n) G_STMT_START {                                        \
  gint __day = d->julian + (n);                                             \
  if (__day < 1)                                                            \
    {                                                                       \
      d->period += -1 + (__day / DAYS_PER_PERIOD);                          \
      d->period += DAYS_PER_PERIOD + (__day % DAYS_PER_PERIOD);             \
    }                                                                       \
  else if (__day > DAYS_PER_PERIOD)                                         \
    {                                                                       \
      d->period += (d->julian + (n)) / DAYS_PER_PERIOD;                     \
      d->julian = (d->julian + (n)) % DAYS_PER_PERIOD;                      \
    }                                                                       \
  else                                                                      \
    d->julian += n;                                                         \
} G_STMT_END
#define ADD_USEC(d,n) G_STMT_START {                                        \
  gint64 __usec = (d)->usec + (n);                                          \
  gint __days = __usec / USEC_PER_DAY;                                      \
  if (__days != 0)                                                          \
    ADD_DAYS ((d), __days);                                                 \
  if (__usec < 0)                                                           \
    d->usec = USEC_PER_DAY + (__usec % USEC_PER_DAY);                       \
  else                                                                      \
    d->usec = __usec % USEC_PER_DAY;                                        \
} G_STMT_END
#define PRINT_DATE_TIME(d) G_STMT_START {                                   \
  guint64 __usec_p = d->usec;                                               \
  g_print ("GDateTime {\n"                                                  \
           "  Period..: %d\n"                                               \
           "  Julian..: %d\n"                                               \
           "  Usec....: %llu\n"                                             \
           "}\n", d->period, d->julian, __usec_p);                          \
} G_STMT_END
#define TO_JULIAN(year,month,day,julian) G_STMT_START {                     \
  gint a = (14 - month) / 12;                                               \
  gint y = year + 4800 - a;                                                 \
  gint m = month + (12 * a) - 3;                                            \
                                                                            \
  *(julian) = day                                                           \
             + (((153 * m) + 2) / 5)                                        \
             + (y * 365)                                                    \
             + (y / 4)                                                      \
             - (y / 100)                                                    \
             + (y / 400)                                                    \
             - 32045;                                                       \
} G_STMT_END

typedef struct _GTimeZone GTimeZone;

static const guint16 days_in_months[2][13] =
{
  { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const guint16 days_in_year[2][13] = 
{
  {  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 }, 
  {  0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

struct _GDateTime
{
  gint           period   :  3; /* Julian Period, 0 is Initial Epoch */
  guint          julian   : 22; /* Day within Julian Period */
  guint64        usec     : 37; /* Microsecond timekeeping within Day */
  gint           reserved :  2;

  volatile gint  ref_count;

  GTimeZone     *tz;            /* TimeZone information */
};

struct _GTimeZone
{
  gint    year;
  gchar  *std_name;
  gint    std_gmtoff;
  gchar  *dst_name;
  gint    dst_gmtoff;

  struct {
    gint  julian;
    gint  seconds;
  } dst_begin, dst_end;
};

static gint
gmt_offset (struct tm *tm,
            time_t     t)
{
#if defined (HAVE_TM_GMTOFF)
  return tm->tm_gmtoff;
#else
  struct tm g;
  time_t t2;
  g = *gmtime (&t);
  g.tm_isdst = tm->tm_isdst;
  t2 = mktime (&g);
  return (int)difftime (t, t2);
#endif
}

static GHashTable*
g_time_zone_get_cache (void)
{
  static GHashTable *hash = NULL;

  if (g_once_init_enter ((gsize*)&hash))
    {
      GHashTable *h;

      h = g_hash_table_new (g_int64_hash, g_int64_equal);
      g_once_init_leave ((gsize*)&hash, (gsize)h);
    }

  return hash;
}

static GTimeZone*
g_time_zone_new_from_year (gint year)
{
  static GStaticRWLock  hash_lock = G_STATIC_RW_LOCK_INIT;
  GHashTable           *hash;
  GTimeZone            *tz = NULL;
  gboolean              limited     = FALSE,
                        is_daylight = FALSE;
  gint64                key,
                       *mkey,
                        julian;
  gint                  gmtoff,
                        day;
  time_t                t;
  struct tm             tt, start;
  gchar                 tzone [64];

  if ((year < 1970) || (year > 2037))
    {
      limited = TRUE;
      year = 1970;
    }

  memset (&start, 0, sizeof (start));

  start.tm_mday = 1;
  start.tm_year = year - 1900;

  t = mktime (&start);

  gmtoff = gmt_offset (&start, t);
  key = year;
  key = (key << 32) | gmtoff;

  hash = g_time_zone_get_cache ();

  g_static_rw_lock_reader_lock (&hash_lock);
  tz = g_hash_table_lookup (hash, &key);
  g_static_rw_lock_reader_unlock (&hash_lock);

  if (!tz)
    {
      g_static_rw_lock_writer_lock (&hash_lock);
      if (!(tz = g_hash_table_lookup (hash, &key)))
        {
          tz = g_slice_new0 (GTimeZone);
          mkey = g_malloc (sizeof (gint64));
          *mkey = key;

          if (limited)
            {
              localtime_r (&t, &tt);
              strftime (tzone, sizeof (tzone), "%Z", &tt);
              tz->std_name = g_strdup (tzone);
              tz->dst_name = g_strdup (tzone);
            }
          else
            {
              gmtoff = gmt_offset (&start, t);

              /* For each day of the year, calculate the tm_gmtoff */
              for (day = 0; day < 365; day++)
                {    
                  t += 86400;
                  localtime_r (&t, &tt);

                  /* Check if daylight savings starts or ends here */
                  if (gmt_offset (&tt, t) != gmtoff)
                    {    
                      struct tm tt1; 
                      time_t    t1;  

                      /* Try to find the exact hour when daylight saving starts/ends. */
                      t1 = t; 
                      do { 
                        t1 -= 3600;
                        localtime_r (&t1, &tt1);
                      } while (gmt_offset (&tt1, t1) != gmtoff);

                      /* Try to find the exact minute when daylight saving starts/ends. */
                      do { 
                        t1 += 60;
                        localtime_r (&t1, &tt1);
                      } while (gmt_offset (&tt1, t1) == gmtoff);
                      t1 += gmtoff;
                      strftime (tzone, sizeof (tzone), "%Z", &tt);
          
                      /* Write data, if we're already in daylight saving, we're done. */
                      if (is_daylight)
                        {
                          tz->std_name = g_strdup (tzone);
                          TO_JULIAN (tt1.tm_year, tt1.tm_mon + 1, tt1.tm_mday, &julian);
                          tz->dst_end.julian = julian;
                          tz->dst_end.seconds = ((tt1.tm_hour * 60 * 60) +
                                                 (tt1.tm_min * 60) +
                                                 (tt1.tm_sec));
                          goto finished;
                        }
                      else
                        {
                          tz->dst_name = g_strdup (tzone);
                          TO_JULIAN (tt1.tm_year, tt1.tm_mon + 1, tt1.tm_mday, &julian);
                          tz->dst_begin.julian = julian;
                          tz->dst_begin.seconds = ((tt1.tm_hour * 60 * 60) +
                                                   (tt1.tm_min * 60) +
                                                   (tt1.tm_sec));
                          is_daylight = 1; 
                        }    

                      /* This is only set once when we enter daylight saving. */
                      tz->std_gmtoff = (gint64)gmtoff;
                      tz->dst_gmtoff = (gint64)(gmt_offset (&tt, t) - gmtoff);

                      g_debug ("StdOffset: %d", tz->std_gmtoff);
                      g_debug ("DstOffset: %d", tz->dst_gmtoff);

                      gmtoff = gmt_offset (&tt, t);
                    }
                }

              if (!is_daylight)
                {
                  strftime (tzone, sizeof (tzone), "%Z", &tt);
                  tz->std_name = g_strdup (tzone);
                  tz->dst_name = g_strdup (tzone);
                  tz->std_gmtoff = gmtoff;
                }
            }

finished:
          g_hash_table_insert (hash, mkey, tz);
          g_static_rw_lock_writer_unlock (&hash_lock);
        }
    }

  return tz;
}

static GDateTime*
g_date_time_new (void)
{
  GDateTime *datetime;

  datetime = g_slice_new0 (GDateTime);
  datetime->ref_count = 1;

  return datetime;
}

static void
g_date_time_free (GDateTime *datetime)
{
  g_slice_free (GDateTime, datetime);
}

static void
g_date_time_get_dmy (GDateTime *datetime,
                     gint      *day,
                     gint      *month,
                     gint      *year)
{
  gint a, b, c, d, e, m;

  a = datetime->julian + 32044;
  b = ((4 * a) + 3) / 146097;
  c = a - ((b * 146097) / 4);
  d = ((4 * c) + 3) / 1461;
  e = c - (1461 * d) / 4;
  m = (5 * e + 2) / 153;

  if (day)
    *day = e - (((153 * m) + 2) / 5) + 1;

  if (month)
    *month = m + 3 - (12 * (m / 10));

  if (year)
    *year  = (b * 100) + d - 4800 + (m / 10);
}

static void
g_date_time_get_week_number (GDateTime *datetime,
                             gint      *week_number,
                             gint      *day_of_week,
                             gint      *day_of_year)
{
  gint a, b, c, d, e, f, g, n, s, month, day, year;

  g_date_time_get_dmy (datetime, &day, &month, &year);

  if (month <= 2)
    {
      a = g_date_time_get_year (datetime) - 1;
      b = (a / 4) - (a / 100) + (a / 400);
      c = ((a - 1) / 4) - ((a - 1) / 100) + ((a - 1) / 400);
      s = b - c;
      e = 0;
      f = day - 1 + (31 * (month - 1));
    }
  else
    {
      a = year;
      b = (a / 4) - (a / 100) + (a / 400);
      c = ((a - 1) / 4) - ((a - 1) / 100) + ((a - 1) / 400);
      s = b - c;
      e = s + 1;
      f = day + (((153 * (month - 3)) + 2) / 5) + 58 + s;
    }

  g = (a + b) % 7;
  d = (f + g - e) % 7;
  n = f + 3 - d;

  if (week_number)
    {
      if (n < 0)
        *week_number = 53 - ((g - s) / 5);
      else if (n > 364 + s)
        *week_number = 1;
      else
        *week_number = (n / 7) + 1;
    }

  if (day_of_week)
    *day_of_week = d + 1;

  if (day_of_year)
    *day_of_year = f + 1;
}

/**
 * g_date_time_add:
 * @datetime: a #GDateTime
 * @timespan: a #GTimeSpan
 *
 * Creates a copy of @datetime and adds the specified timespan to the copy.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add (GDateTime *datetime,
                 GTimeSpan *timespan)
{
  GDateTime *dt;

  g_return_val_if_fail (datetime != NULL, NULL);
  g_return_val_if_fail (timespan != NULL, NULL);

  dt = g_date_time_copy (datetime);

  /* TODO: Add julians by order of days, etc */
  g_warn_if_reached ();

  return dt;
}

/**
 * g_date_time_add_years:
 * @datetime: a #GDateTime
 * @years: the number of years
 *
 * Creates a copy of @datetime and adds the specified number of years to the
 * copy.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_years (GDateTime *datetime,
                       gint       years)
{
  GDateTime *dt;
  gint       day;

  g_return_val_if_fail (datetime != NULL, NULL);

  day = g_date_time_get_day_of_month (datetime);
  if (g_date_time_is_leap_year (datetime) &&
      g_date_time_get_month (datetime) == 2)
    if (day == 29)
      day--;

  dt = g_date_time_new_from_date (
    g_date_time_get_year (datetime) + years,
    g_date_time_get_month (datetime),
    day);
  dt->usec = datetime->usec;

  return dt;
}

/**
 * g_date_time_add_months:
 * @datetime: a #GDateTime
 * @months: the number of months
 *
 * Creates a copy of @datetime and adds the specified number of months to the
 * copy.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_months (GDateTime *datetime,
                        gint       months)
{
  GDateTime     *dt;
  gint           year,
                 month,
                 day,
                 i,
                 a;
  const guint16 *days;

  g_return_val_if_fail (datetime != NULL, NULL);
  g_return_val_if_fail (months != 0, NULL);

  month = g_date_time_get_month (datetime);
  year = g_date_time_get_year (datetime);
  a = months > 0 ? 1 : -1;

  for (i = 0; i < ABS (months); i++)
    {
      month += a;
      if (month < 1)
        {
          year--;
          month = 12;
        }
      else if (month > 12)
        {
          year++;
          month = 1;
        }
    }

  day = g_date_time_get_day_of_month (datetime);
  days = days_in_months [GREGORIAN_LEAP (year) ? 1 : 0];

  if (days [month] < day)
    day = days [month];

  dt = g_date_time_new_from_date (year, month, day);
  dt->usec = datetime->usec;

  return dt;
}

/**
 * g_date_time_add_weeks:
 * @datetime: a #GDateTime
 * @weeks: the number of weeks
 *
 * Creates a copy of @datetime and adds the specified number of weeks to the
 * copy.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_weeks (GDateTime *datetime,
                       gint       weeks)
{
  g_return_val_if_fail (datetime != NULL, NULL);
  return g_date_time_add_days (datetime, weeks * 7);
}

/**
 * g_date_time_add_days:
 * @datetime: a #GDateTime
 * @days: the number of days
 *
 * Creates a copy of @datetime and adds the specified number of days to the
 * copy.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_days (GDateTime *datetime,
                      gint       days)
{
  GDateTime *dt;

  g_return_val_if_fail (datetime != NULL, NULL);

  dt = g_date_time_copy (datetime);
  ADD_DAYS (dt, days);

  return dt;
}

/**
 * g_date_time_add_hours:
 * @datetime: a #GDateTime
 * @hours: the number of hours
 *
 * Creates a copy of @datetime and adds the specified number of hours to the
 * copy.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_hours (GDateTime *datetime,
                       gint       hours)
{
  GDateTime *dt;
  gint64     usec;

  g_return_val_if_fail (datetime != NULL, NULL);

  usec = hours * USEC_PER_HOUR;
  dt = g_date_time_copy (datetime);
  ADD_USEC (dt, usec);

  return dt;
}

/**
 * g_date_time_add_seconds:
 * @datetime: a #GDateTime
 * @seconds: the number of seconds
 *
 * Creates a copy of @datetime and adds the specified number of seconds
 * to the copy.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_seconds (GDateTime *datetime,
                         gint       seconds)
{
  GDateTime *dt;
  gint64     usec;

  g_return_val_if_fail (datetime != NULL, NULL);

  dt = g_date_time_copy (datetime);
  usec = seconds * USEC_PER_SECOND;  
  ADD_USEC (dt, usec);

  return dt;
}

/**
 * g_date_time_add_milliseconds:
 * @datetime: a #GDateTime
 * @milliseconds: the number of milliseconds
 *
 * Creates a new #GDateTime adding the specified milliseconds @milliseconds to
 * the current date and time.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_milliseconds (GDateTime *datetime,
                              gint       milliseconds)
{
  GDateTime *dt;
  guint64    usec;

  g_return_val_if_fail (datetime != NULL, NULL);

  dt = g_date_time_copy (datetime);
  usec = milliseconds * USEC_PER_MILLISECOND;
  ADD_USEC (dt, usec);

  return dt;
}

/**
 * g_date_time_add_minutes:
 * @datetime: a #GDateTime
 * @minutes: the number of minutes to add
 *
 * Creates a new #GDateTime adding the specified number of minutes.
 *
 * Return value: the newly created #GDateTime which should be freed with
 * g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_minutes (GDateTime *datetime,
                         gint       minutes)
{
  GDateTime *dt;

  g_return_val_if_fail (datetime != NULL, NULL);

  dt = g_date_time_copy (datetime);
  ADD_USEC (dt, minutes * USEC_PER_MINUTE);

  return dt;
}

/**
 * g_date_time_add_full:
 * @datetime: a #GDateTime
 * @years: the number of years to add
 * @months: the number of months to add
 * @days: the number of days to add
 * @hours: the number of hours to add
 * @minutes: the number of minutes to add
 * @seconds: the number of seconds to add
 *
 * Creates a new #GDateTime adding the specified values to the current date and
 * time in @datetime.
 *
 * Return value: the newly created #GDateTime that should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_add_full (GDateTime      *datetime,
                      gint            years,
                      gint            months,
                      gint            days,
                      gint            hours,
                      gint            minutes,
                      gint            seconds)
{
  GDateTime *tmp, *dt;

  g_return_val_if_fail (datetime != NULL, NULL);

  dt = g_date_time_add_years (datetime, years);
  tmp = dt;

  dt = g_date_time_add_months (tmp, months);
  g_date_time_unref (tmp);
  tmp = dt;

  dt = g_date_time_add_days (tmp, days);
  g_date_time_unref (tmp);
  tmp = dt;

  dt = g_date_time_add_hours (tmp, hours);
  g_date_time_unref (tmp);
  tmp = dt;

  dt = g_date_time_add_minutes (tmp, minutes);
  g_date_time_unref (tmp);
  tmp = dt;

  dt = g_date_time_add_seconds (tmp, seconds);
  g_date_time_unref (tmp);

  return dt;
}

/**
 * g_date_time_compare:
 * @dt1: first #GDateTime to compare
 * @dt2: second #GDateTime to compare
 *
 * qsort()-style comparison for #GDateTime<!-- -->'s. Both #GDateTime<-- -->'s
 * must be non-%NULL.
 *
 * Return value: 0 for equal, less than zero if dt1 is less than dt2, greater
 *   than zero if dt2 is greator than dt1.
 *
 * Since: 2.24
 */
gint
g_date_time_compare (gconstpointer dt1,
                     gconstpointer dt2)
{
  const GDateTime *a, *b;

  a = dt1;
  b = dt2;

  if ((a->period == b->period) &&
      (a->julian == b->julian) &&
      (a->usec == b->usec))
    return 0;
  else if ((a->period > b->period) ||
           ((a->period == b->period) && (a->julian > b->julian)) ||
           ((a->period == b->period) && (a->julian == b->julian) && a->usec > b->usec))
    return 1;
  else
    return -1;
}

/**
 * g_date_time_copy:
 * @datetime: a #GDateTime
 *
 * Creates a copy of @datetime.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_copy (GDateTime *datetime)
{
  GDateTime *copied;

  g_return_val_if_fail (datetime != NULL, NULL);

  copied = g_date_time_new ();
  copied->period = datetime->period;
  copied->julian = datetime->julian;
  copied->usec = datetime->usec;

  /* TODO: Copy GTimeZone information */

  return copied;
}

/**
 * g_date_time_date:
 * @datetime: a #GDateTime
 *
 * Creates a new #GDateTime at Midnight on the date represented by @datetime.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_date (GDateTime *datetime)
{
  GDateTime *date;

  g_return_val_if_fail (datetime != NULL, NULL);

  date = g_date_time_copy (datetime);
  date->usec = 0;

  return date;
}

/**
 * g_date_time_diff:
 * @begin: a #GDateTime
 * @end: a #GDateTime
 * @timespan: a #GTimeSpan
 *
 * Calculates the known difference in time between @begin and @end.  Since the
 * exact precision cannot always be known due to incomplete historic
 * information, a best attempt is made to calculate the difference.
 *
 * Since: 2.24
 */
void
g_date_time_diff (GDateTime *begin,
                  GDateTime *end,
                  GTimeSpan *timespan)
{
  gint64 usec;
  gint   days;

  g_return_if_fail (begin != NULL);
  g_return_if_fail (end != NULL);
  g_return_if_fail (timespan != NULL);

  if (begin->period != 0 || end->period != 0)
    {
      g_warning ("%s only supports current julian epoch", G_STRFUNC);
      *timespan = 0;
      return;
    }

  days = end->julian - begin->julian;
  usec = end->usec - begin->usec;

  *timespan = (days * USEC_PER_DAY) + (end->usec - begin->usec);
}

/**
 * g_date_time_equal:
 * @dt1: a #GDateTime
 * @dt2: a #GDateTime
 *
 * Checks to see if @dt1 and @dt2 are equal
 *
 * Return value: %TRUE if @dt1 and @dt2 are equal
 *
 * Since: 2.24
 */
gboolean
g_date_time_equal (gconstpointer dt1,
                   gconstpointer dt2)
{
  const GDateTime *a, *b;

  a = dt1;
  b = dt2;

  /* TODO: Check timezone offset */

  return ((a->period == b->period) &&
          (a->julian == b->julian) &&
          (a->usec == b->usec));
}

/**
 * g_date_time_get_day_of_week:
 * @datetime: a #GDateTime
 *
 * Retrieves the day of the week represented by @datetime within the gregorian
 * calendar. 1 is Sunday, 2 is Monday, etc.
 *
 * Return value: the day of the week
 *
 * Since: 2.24
 */
gint
g_date_time_get_day_of_week (GDateTime *datetime)
{
  gint a, y, m,
       year  = 0,
       month = 0,
       day   = 0,
       dow;

  g_return_val_if_fail (datetime != NULL, 0);

  /* See Calendar FAQ Section 2.6 for algorithm information */

  g_date_time_get_dmy (datetime, &day, &month, &year);
  a = (14 - month) / 12;
  y = year - a;
  m = month + (12 * a) - 2;
  dow = ((day + y + (y / 4) - (y / 100) + (y / 400) + (31 * m) / 12) % 7);

  /* 1 is Monday and 7 is Sunday */
  return (dow == 0) ? 7 : dow;
}

/**
 * g_date_time_get_day_of_month:
 * @datetime: a #GDateTime
 *
 * Retrieves the day of the month represented by @datetime in the gregorian
 * calendar.
 *
 * Return value: the day of the month
 *
 * Since: 2.24
 */
gint
g_date_time_get_day_of_month (GDateTime *datetime)
{
  gint           day_of_year,
                 i;
  const guint16 *days;
  guint16        last = 0;

  g_return_val_if_fail (datetime != NULL, 0);

  days = days_in_year [g_date_time_is_leap_year (datetime)? 1 : 0];
  g_date_time_get_week_number (datetime, NULL, NULL, &day_of_year);

  for (i = 1; i <= 12; i++)
    {
      if (days [i] >= day_of_year)
        return day_of_year - last;
      last = days [i];
    }

  g_warn_if_reached ();
  return 0;
}

/**
 * g_date_time_get_hour:
 * @datetime: a #GDateTime
 *
 * Retrieves the hour of the day represented by @datetime in the gregorian
 * calendar.
 *
 * Return value: the hour of the day
 *
 * Since: 2.24
 */
gint
g_date_time_get_hour (GDateTime *datetime)
{
  g_return_val_if_fail (datetime != NULL, 0);
  return (datetime->usec / USEC_PER_HOUR);
}

/**
 * g_date_time_get_julian:
 * @datetime: a #GDateTime
 * @period: a location for the julian period
 * @julian: a location for the day in the julian period
 * @hour: a location for the hour of the day
 * @minute: a location for the minute of the hour
 * @second: a location for hte second of the minute
 *
 * Retrieves the julian period, day, hour, mintute, and second which @datetime
 * represents in the Julian calendar.
 *
 * Since: 2.24
 */
void
g_date_time_get_julian (GDateTime *datetime,
                        gint      *period,
                        gint      *julian,
                        gint      *hour,
                        gint      *minute,
                        gint      *second)
{
  g_return_if_fail (datetime != NULL);

  if (period)
    *period = datetime->period;

  if (julian)
    *julian = datetime->julian;

  if (hour)
    *hour = (datetime->usec / USEC_PER_HOUR);

  if (minute)
    *minute = (datetime->usec % USEC_PER_HOUR) / USEC_PER_MINUTE;

  if (second)
    *second = (datetime->usec % USEC_PER_MINUTE) / USEC_PER_SECOND;
}

/**
 * g_date_time_get_microsecond:
 * @datetime: a #GDateTime
 *
 * Retrieves the microsecond of the current second represented by @datetime in
 * the gregorian calendar.
 *
 * Return value: the microsecond of the second
 *
 * Since: 2.24
 */
gint
g_date_time_get_microsecond (GDateTime *datetime)
{
  g_return_val_if_fail (datetime != NULL, 0);
  return (datetime->usec % USEC_PER_SECOND);
}

/**
 * g_date_time_get_millisecond:
 * @datetime: a #GDateTime
 *
 * Retrieves the millisecond of the current second represented by @datetime in
 * the gregorian calendar.
 *
 * Return value: the millisecond of the second
 *
 * Since: 2.24
 */
gint
g_date_time_get_millisecond (GDateTime *datetime)
{
  g_return_val_if_fail (datetime != NULL, 0);
  return (datetime->usec % USEC_PER_SECOND) / USEC_PER_MILLISECOND;
}

/**
 * g_date_time_get_minute:
 * @datetime: a #GDateTime
 *
 * Retrieves the minute of the hour represented by @datetime in the gregorian
 * calendar.
 *
 * Return value: the minute of the hour
 *
 * Since: 2.24
 */
gint
g_date_time_get_minute (GDateTime *datetime)
{
  g_return_val_if_fail (datetime != NULL, 0);
  return (datetime->usec % USEC_PER_HOUR) / USEC_PER_MINUTE;
}

/**
 * g_date_time_get_month:
 * @datetime: a #GDateTime
 *
 * Retrieves the month of the year represented by @datetime in the gregorian
 * calendar.
 *
 * Return value: the month represented by @datetime
 *
 * Since: 2.24
 */
gint
g_date_time_get_month (GDateTime *datetime)
{
  gint month;

  g_return_val_if_fail (datetime != NULL, 0);

  g_date_time_get_dmy (datetime, NULL, &month, NULL);

  return month;
}

/**
 * g_date_time_get_second:
 * @datetime: a #GDateTime
 *
 * Retrieves the second of the minute represented by @datetime in the gregorian
 * calendar.
 *
 * Return value: the second represented by @datetime
 *
 * Since: 2.24
 */
gint
g_date_time_get_second (GDateTime *datetime)
{
  g_return_val_if_fail (datetime != NULL, 0);
  return (datetime->usec % USEC_PER_MINUTE) / USEC_PER_SECOND;
}

/**
 * g_date_time_get_utc_offset:
 * @datetime: a #GDateTime
 * @timespan: a #GTimeSpan
 *
 * Retrieves the offset from UTC that the local timezone specified by @datetime
 * represents.  If @datetime represents UTC time, then the offset is zero.
 *
 * Since: 2.24
 */
void
g_date_time_get_utc_offset (GDateTime *datetime,
                            GTimeSpan *timespan)
{
  g_return_if_fail (datetime != NULL);
  g_return_if_fail (timespan != NULL);

  if (datetime->tz)
    {
      if (g_date_time_is_daylight_savings (datetime))
        *timespan = datetime->tz->dst_gmtoff;
      else
        *timespan = datetime->tz->std_gmtoff;
    }
  else
    *timespan = 0;
}

/**
 * g_date_time_get_year:
 * @datetime: A #GDateTime
 *
 * Retrieves the year in the gregorian calendar that @datetime represents.
 *
 * Return value: the year of the gregorian calendar
 *
 * Since: 2.24
 */
gint
g_date_time_get_year (GDateTime *datetime)
{
  gint year;

  g_return_val_if_fail (datetime != NULL, 0);

  g_date_time_get_dmy (datetime, NULL, NULL, &year);

  return year;
}

/**
 * g_date_time_hash:
 * @datetime: a #GDateTime
 *
 * Hashes @datetime into a #guint suitable for use within #GHashTable.
 *
 * Return value: a #guint containing the hash
 *
 * Since: 2.24
 */
guint
g_date_time_hash (gconstpointer datetime)
{
  return (guint)(*((guint64*)datetime));
}

/**
 * g_date_time_format_for_display:
 * @datetime: a #GDateTime
 *
 * Formats @datetime into a string suitable for display in a user interface such
 * as a file-browser.
 *
 * An example would be "Yesterday, 4:35 PM".
 *
 * Return value: the newly allocated string that should be freed using g_free().
 *
 * Since: 2.24
 */
gchar*
g_date_time_format_for_display (GDateTime *datetime)
{
  GDateTime *today;
  gint       julian;

  g_return_val_if_fail (datetime != NULL, NULL);

  today = g_date_time_today ();
  julian = today->julian;
  g_date_time_unref (today);

  if (!datetime->period)
    {
      if (julian == datetime->julian)
        return g_date_time_printf (datetime, _("Today, %l:%M %p"));
      else if (julian == (datetime->julian + 1))
        return g_date_time_printf (datetime, _("Yesterday, %l:%M %p"));
      else if (julian == (datetime->julian - 1))
        return g_date_time_printf (datetime, _("Tomorrow, %l:%M %p"));
    }

  return g_date_time_printf (datetime, "%b %d, %Y, %l:%M %p");
}

/**
 * g_date_time_is_leap_year:
 * @datetime: a #GDateTime
 *
 * Determines if @datetime represents a date known to fall within a leap year in
 * the gregorian calendar.
 *
 * Return value: %TRUE if @datetime is a leap year.
 *
 * Since: 2.24
 */
gboolean
g_date_time_is_leap_year (GDateTime *datetime)
{
  gint year;

  g_return_val_if_fail (datetime != NULL, FALSE);

  year = g_date_time_get_year (datetime);
  return GREGORIAN_LEAP (year);
}

/**
 * g_date_time_is_daylight_savings:
 * @datetime: a #GDateTime
 *
 * Determines if @datetime represents a date known to fall within daylight
 * savings time in the gregorian calendar.
 *
 * Return value: %TRUE if @datetime falls within daylight savings time.
 *
 * Since: 2.24
 */
gboolean
g_date_time_is_daylight_savings (GDateTime *datetime)
{
  g_return_val_if_fail (datetime != NULL, FALSE);

  // tmp
  return TRUE;

  /* TODO: Get GTimeZone to determine daylight savings range */
  g_warn_if_reached ();

  return FALSE;
}

/**
 * g_date_time_new_from_date:
 * @year: the gregorian year
 * @month: the gregorian month
 * @day: the day in the gregorian month
 *
 * Creates a new #GDateTime using the specified date within the gregorian
 * calendar.
 *
 * Return value: the newly created #GDateTime or %NULL if it is outside of
 *   the representable range.
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_new_from_date (gint year,
                           gint month,
                           gint day)
{
  GDateTime *dt;
  gint       julian;

  g_return_val_if_fail (year > -4712 && year <= 3268, NULL);
  g_return_val_if_fail (month > 0 && month <= 12, NULL);
  g_return_val_if_fail (day > 0 && day <= 31, NULL);

  dt = g_date_time_new ();
  TO_JULIAN (year, month, day, &julian);
  dt->julian = julian;
  dt->tz = g_time_zone_new_from_year (year);

  return dt;
}

/**
 * g_date_time_new_from_time_t:
 * @t: a time_t
 *
 * Creates a new #GDateTime using the time since Jan 1, 1970 specified by @t.
 *
 * Return value: the newly created #GDateTime
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_new_from_time_t (time_t t)
{
  struct tm tm;

  memset (&tm, 0, sizeof (tm));
  localtime_r (&t, &tm);

  return g_date_time_new_full (tm.tm_year + 1900,
                               tm.tm_mon  + 1,
                               tm.tm_mday,
                               tm.tm_hour,
                               tm.tm_min,
                               tm.tm_sec);
}

/**
 * g_date_time_new_from_timeval:
 * @tv: #GTimeVal
 *
 * Creates a new #GDateTime using the date and time specified by #GTimeVal.
 *
 * Return value: the newly created #GDateTime
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_new_from_timeval (GTimeVal *tv)
{
  GDateTime *datetime;
  gint       year;

  g_return_val_if_fail (tv != NULL, NULL);

  datetime = g_date_time_new_from_time_t ((time_t)tv->tv_sec);
  datetime->usec += tv->tv_usec;
  g_date_time_get_dmy (datetime, NULL, NULL, &year);
  datetime->tz = g_time_zone_new_from_year (year);

  return datetime;
}

/**
 * g_date_time_new_full:
 * @year: the gregorian year
 * @month: the gregorian month
 * @day: the day of the gregorian month
 * @hour: the hour of the day
 * @minute: the minute of the hour
 * @second: the second of the minute
 *
 * Creates a new #GDateTime using the date and times in the gregorian calendar.
 *
 * Return value: the newly created #GDateTime
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_new_full (gint year,
                      gint month,
                      gint day,
                      gint hour,
                      gint minute,
                      gint second)
{
  GDateTime *dt;
  
  g_return_val_if_fail (hour >= 0 && hour < 24, NULL);
  g_return_val_if_fail (minute >= 0 && minute < 60, NULL);
  g_return_val_if_fail (second >= 0 && second <= 60, NULL);

  if (!(dt = g_date_time_new_from_date (year, month, day)))
    return NULL;

  dt->usec = (hour   * USEC_PER_HOUR)
           + (minute * USEC_PER_MINUTE)
           + (second * USEC_PER_SECOND);
  dt->tz = g_time_zone_new_from_year (year);

  return dt;
}

/**
 * g_date_time_now:
 *
 * Creates a new #GDateTime representing the current date and time.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_now (void)
{
  GTimeVal tv;
  g_get_current_time (&tv);
  return g_date_time_new_from_timeval (&tv);
}

/**
 * g_date_time_parse:
 * @input: the string to parse
 *
 * Parses @input using a the set of known formats for date and times.  The
 * parsed date and time is stored in a new #GDateTime and returned.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref() or %NULL upon error.
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_parse (const gchar *input)
{
  /* TODO: Implement parsing with locale support */
  g_warn_if_reached ();
  return NULL;
}

/**
 * g_date_time_parse_with_format:
 * @format: the format string
 * @input: the string to parse
 *
 * Parses @input using the @format specified.  The parsed date and time is
 * stored in a new #GDateTime and returned.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref() or %NULL upon error.
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_parse_with_format (const gchar *format,
                               const gchar *input)
{
  /* TODO: Implement parsing with locale support */
  g_warn_if_reached ();
  return NULL;
}

/**
 * g_date_time_printf:
 * @datetime: a #GDateTime
 * @format: the format 
 *
 * Creates a new string formatted to the specification @format.
 *
 * Return value: the formatted string which should be freed with g_free() or
 *   %NULL.
 *
 * Since: 2.24
 */
gchar*
g_date_time_printf (GDateTime   *datetime,
                    const gchar *format)
{
  /* TODO: Implement printf with locale support */
  g_warn_if_reached ();
  return NULL;
}

/**
 * g_date_time_ref:
 * @datetime: a #GDateTime
 *
 * Atomically increments the reference count of @datetime by one.
 *
 * Return value: the reference @datetime
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_ref (GDateTime *datetime)
{
  g_return_val_if_fail (datetime != NULL, NULL);
  g_return_val_if_fail (datetime->ref_count > 0, NULL);
  g_atomic_int_inc (&datetime->ref_count);
  return datetime;
}

/**
 * g_date_time_to_local:
 * @datetime: a #GDateTime
 *
 * Creates a new #GDateTime with @datetime converted to local time.
 *
 * Return value: the newly created #GDateTime
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_to_local (GDateTime *datetime)
{
  /* TODO: Convert to local time using current timezone infor for year */
  g_warn_if_reached ();
  return NULL;
}

/**
 * g_date_time_to_time_t:
 * @datetime: a #GDateTime
 *
 * Converts @datetime into a #time_t
 *
 * Return value: @datetime as a #time_t
 *
 * Since: 2.24
 */
time_t
g_date_time_to_time_t (GDateTime *datetime)
{
  struct tm tm;

  g_return_val_if_fail (datetime != NULL, (time_t)0);
  g_return_val_if_fail (datetime->period == 0, (time_t)0);

  memset (&tm, 0, sizeof (tm));

  tm.tm_year = g_date_time_get_year (datetime) - 1900;
  tm.tm_mon = g_date_time_get_month (datetime) - 1;
  tm.tm_mday = g_date_time_get_day_of_month (datetime);
  tm.tm_hour = g_date_time_get_hour (datetime);
  tm.tm_min = g_date_time_get_minute (datetime);
  tm.tm_sec = g_date_time_get_second (datetime);

  if (datetime->tz)
    {
      /* TODO: adjust based on offset */
      g_warn_if_reached ();
    }

  return mktime (&tm);
}

/**
 * g_date_time_to_timeval:
 * @datetime: a #GDateTime
 * @tv: A #GTimeVal
 *
 * Converts @datetime into a #GTimeVal and stores the result into @timeval.
 *
 * Since: 2.24
 */
void
g_date_time_to_timeval (GDateTime *datetime,
                        GTimeVal  *tv)
{
  g_return_if_fail (datetime != NULL);

  tv->tv_sec = 0;
  tv->tv_usec = 0;

  if (datetime->period == 0)
    {
      tv->tv_sec = g_date_time_to_time_t (datetime);
      tv->tv_usec = datetime->usec % USEC_PER_SECOND;
    }
}

/**
 * g_date_time_to_utc:
 * @datetime: a #GDateTime
 *
 * Creates a new #GDateTime that reprents @datetime in Universal coordinated
 * time.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_to_utc (GDateTime *datetime)
{
  /* TODO: Convert to UTC */
  g_warn_if_reached ();
  return NULL;
}

/**
 * g_date_time_today:
 *
 * Createsa new #GDateTime that represents Midnight on the current day.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_today (void)
{
  GDateTime *dt;

  dt = g_date_time_now ();
  dt->usec = 0;

  return dt;
}

/**
 * g_date_time_unref:
 * @datetime: a #GDateTime
 *
 * Atomically decrements the reference count of @datetime by one.  When the
 * reference count reaches zero, the structure is freed.
 *
 * Since: 2.24
 */
void
g_date_time_unref (GDateTime *datetime)
{
  g_return_if_fail (datetime != NULL);
  g_return_if_fail (datetime->ref_count > 0);

  if (g_atomic_int_dec_and_test (&datetime->ref_count))
    g_date_time_free (datetime);
}

/**
 * g_date_time_utc_now:
 *
 * Creates a new #GDateTime that represents the current instant at Universal
 * coordinated time.
 *
 * Return value: the newly created #GDateTime which should be freed with
 *   g_date_time_unref().
 *
 * Since: 2.24
 */
GDateTime*
g_date_time_utc_now (void)
{
  GDateTime *datetime,
            *now;
  GTimeSpan  offset;

  now = g_date_time_now ();
  g_date_time_get_utc_offset (now, &offset);
  datetime = g_date_time_add (now, &offset);
  datetime->tz = NULL;
  g_date_time_unref (now);

  return datetime;
}
