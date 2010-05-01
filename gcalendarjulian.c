/* gcalendarjulian.c
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

#include "gcalendarjulian.h"

/**
 * SECTION:gcalendarjulian
 * @title: GCalendarJulian
 * @short_description: The Julian calendar
 *
 * TODO: Add some more information from the Calendar FAQ.
 * TODO: Update date/time boundries.
 */

G_DEFINE_TYPE (GCalendarJulian, g_calendar_julian, G_TYPE_CALENDAR)

static void
get_julian_dmy (GDateTime *datetime, /* IN */
                gint      *day,      /* OUT */
                gint      *month,    /* OUT */
                gint      *year)     /* OUT */
{
  gint b, c, d, e, m, jd;

  g_date_time_get_julian (datetime, NULL, &jd, NULL, NULL, NULL);

  b = 0;
  c = jd + 32082;
  d = ((4 * c) + 3) / 1461;
  e = c - ((1461 * d) / 4);
  m = ((5 * e) + 2) / 153;

  if (day)
    *day = e - (((153 * m) + 2) / 5) + 1;

  if (month)
    *month = m + 3 - (12 * (m / 10));

  if (year)
    *year = (b * 100) + d - 4800 + (m / 10);
}

static gint
g_calendar_julian_real_get_year (GCalendar *calendar, /* IN */
                                 GDateTime *datetime) /* IN */
{
  gint year;
  get_julian_dmy (datetime, NULL, NULL, &year);
  return year;
}

static gint
g_calendar_julian_real_get_month (GCalendar *calendar, /* IN */
                                  GDateTime *datetime) /* IN */
{
  gint month;
  get_julian_dmy (datetime, NULL, &month, NULL);
  return month;
}

static gint
g_calendar_julian_real_get_day_of_month (GCalendar *calendar, /* IN */
                                         GDateTime *datetime) /* IN */
{
  gint day;
  get_julian_dmy (datetime, &day, NULL, NULL);
  return day;
}

static gint
g_calendar_julian_real_get_day_of_week (GCalendar *calendar, /* IN */
                                        GDateTime *datetime) /* IN */
{
  gint year,
       month,
       day,
       a,
       y,
       m;

  year = g_date_time_get_year (datetime);
  month = g_date_time_get_month (datetime);
  day = g_date_time_get_day_of_month (datetime);

  a = (14 - month);
  y = year - a;
  m = month + (12 * a) - 2;

  return ((5 + day + y + (y / 4) + ((31 * m) / 12)) % 7) + 1;
}

static gint
g_calendar_julian_real_get_day_of_year (GCalendar *calendar, /* IN */
                                        GDateTime *datetime) /* IN */
{
  return g_date_time_get_day_of_year (datetime);
}

static gint
g_calendar_julian_real_get_hour (GCalendar *calendar, /* IN */
                                 GDateTime *datetime) /* IN */
{
  return g_date_time_get_hour (datetime);
}

static gint
g_calendar_julian_real_get_minute (GCalendar *calendar, /* IN */
                                   GDateTime *datetime) /* IN */
{
  return g_date_time_get_minute (datetime);
}

static gint
g_calendar_julian_real_get_second (GCalendar *calendar, /* IN */
                                   GDateTime *datetime) /* IN */
{
  return g_date_time_get_second (datetime);
}

static gboolean
g_calendar_julian_real_is_leap_year (GCalendar *calendar, /* IN */
                                     GDateTime *datetime) /* IN */
{
  gint year;

  year = g_calendar_get_year (calendar, datetime);
  return (ABS (year % 4) == (year > 0 ? 0 : 3));
}

static void
g_calendar_julian_class_init (GCalendarJulianClass *klass) /* IN */
{
  GCalendarClass *calendar_class;

  calendar_class                   = G_CALENDAR_CLASS (klass);
  calendar_class->get_year         = g_calendar_julian_real_get_year;
  calendar_class->get_month        = g_calendar_julian_real_get_month;
  calendar_class->get_day_of_month = g_calendar_julian_real_get_day_of_month;
  calendar_class->get_day_of_week  = g_calendar_julian_real_get_day_of_week;
  calendar_class->get_day_of_year  = g_calendar_julian_real_get_day_of_year;
  calendar_class->get_hour         = g_calendar_julian_real_get_hour;
  calendar_class->get_minute       = g_calendar_julian_real_get_minute;
  calendar_class->get_second       = g_calendar_julian_real_get_second;
  calendar_class->is_leap_year     = g_calendar_julian_real_is_leap_year;
}

static void
g_calendar_julian_init (GCalendarJulian *self) /* IN */
{
}

/**
 * g_calendar_julian_new:
 *
 * Creates a new instance of #GCalendarJulian which can be used to translate
 * GDateTime<!-- -->'s into the Julian Calendar.  Since #GDateTime uses the
 * Julian Calendar by default, this is mostly just a wrapper conforming to
 * the #GCalendar API.
 *
 * Return value: the newly created #GCalendarJulian which should be freed with
 *   g_object_unref().
 *
 * Since: 2.24
 */
GCalendar*
g_calendar_julian_new (void)
{
  return g_object_new (G_TYPE_CALENDAR_JULIAN, 0);
}
