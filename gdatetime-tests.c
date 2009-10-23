/* gdatetime-tests.c
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
#include <string.h>
#include <time.h>

#include "gdatetime.h"

#define ASSERT_DATE(dt,y,m,d) G_STMT_START { \
  g_assert_cmpint ((y), ==, g_date_time_get_year ((dt))); \
  g_assert_cmpint ((m), ==, g_date_time_get_month ((dt))); \
  g_assert_cmpint ((d), ==, g_date_time_get_day_of_month ((dt))); \
} G_STMT_END

static void
test_g_date_time_now (void)
{
  GDateTime *dt;
  time_t     t;
  struct tm  tm;

  memset (&tm, 0, sizeof (tm));
  t = time (NULL);
  localtime_r (&t, &tm);
  dt = g_date_time_now ();
  g_assert_cmpint (g_date_time_get_year (dt), ==, 1900 + tm.tm_year);
  g_assert_cmpint (g_date_time_get_month (dt), ==, 1 + tm.tm_mon);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, tm.tm_mday);
  g_assert_cmpint (g_date_time_get_hour (dt), ==, tm.tm_hour);
  g_assert_cmpint (g_date_time_get_minute (dt), ==, tm.tm_min);
  g_assert_cmpint (g_date_time_get_second (dt), ==, tm.tm_sec);
  g_date_time_unref (dt);
}

static void
test_g_date_time_today (void)
{
  GDateTime *dt;
  struct tm  tm;
  time_t     t;

  memset (&tm, 0, sizeof (tm));
  dt = g_date_time_today ();
  t = time (NULL);
  localtime_r (&t, &tm);
  g_assert_cmpint (g_date_time_get_year (dt), ==, 1900 + tm.tm_year);
  g_assert_cmpint (g_date_time_get_month (dt), ==, 1 + tm.tm_mon);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, tm.tm_mday);
  g_assert_cmpint (g_date_time_get_hour (dt), ==, 0);
  g_assert_cmpint (g_date_time_get_minute (dt), ==, 0);
  g_assert_cmpint (g_date_time_get_second (dt), ==, 0);
  g_assert_cmpint (g_date_time_get_millisecond (dt), ==, 0);
  g_date_time_unref (dt);
}

static void
test_g_date_time_new_from_time_t (void)
{
  GDateTime *dt;
  struct tm  tm;
  time_t     t;

  memset (&tm, 0, sizeof (tm));
  t = time (NULL);
  localtime_r (&t, &tm);
  dt = g_date_time_new_from_time_t (t);
  g_assert_cmpint (g_date_time_get_year (dt), ==, 1900 + tm.tm_year);
  g_assert_cmpint (g_date_time_get_month (dt), ==, 1 + tm.tm_mon);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, tm.tm_mday);
  g_assert_cmpint (g_date_time_get_hour (dt), ==, tm.tm_hour);
  g_assert_cmpint (g_date_time_get_minute (dt), ==, tm.tm_min);
  g_assert_cmpint (g_date_time_get_second (dt), ==, tm.tm_sec);
  g_date_time_unref (dt);

  memset (&tm, 0, sizeof (tm));
  tm.tm_year = 70;
  tm.tm_mday = 1;
  tm.tm_mon = 0;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  t = mktime (&tm);
  dt = g_date_time_new_from_time_t (t);
  g_assert_cmpint (g_date_time_get_year (dt), ==, 1970);
  g_assert_cmpint (g_date_time_get_month (dt), ==, 1);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 1);
  g_assert_cmpint (g_date_time_get_hour (dt), ==, 0);
  g_assert_cmpint (g_date_time_get_minute (dt), ==, 0);
  g_assert_cmpint (g_date_time_get_second (dt), ==, 0);
  g_date_time_unref (dt);
}

static void
test_g_date_time_is_leap_year (void)
{
  GDateTime *dt;
  gint       i;

  for (i = 1; i <= 3000; i++)
    {
      dt = g_date_time_new_from_date (i, 1, 1);
      if ((((i % 4) == 0) && ((i % 100) != 0)) || ((i % 400) == 0))
        g_assert (g_date_time_is_leap_year (dt));
      else
        g_assert (!g_date_time_is_leap_year (dt));
      g_date_time_unref (dt);
    }
}

static void
test_g_date_time_compare (void)
{
  GDateTime *dt1, *dt2;
  gint       i;

  dt1 = g_date_time_new_from_date (2000, 1, 1);

  for (i = 1; i < 2000; i++)
    {
      dt2 = g_date_time_new_from_date (i, 12, 31);
      g_assert_cmpint (1, ==, g_date_time_compare (dt1, dt2));
      g_date_time_unref (dt2);
    }

  dt2 = g_date_time_new_full (1999, 12, 31, 23, 59, 59);
  g_assert_cmpint (1, ==, g_date_time_compare (dt1, dt2));
  g_date_time_unref (dt2);

  dt2 = g_date_time_new_full (2000, 1, 1, 0, 0, 1);
  g_assert_cmpint (-1, ==, g_date_time_compare (dt1, dt2));
  g_date_time_unref (dt2);

  dt2 = g_date_time_new_full (2000, 1, 1, 0, 0, 0);
  g_assert_cmpint (0, ==, g_date_time_compare (dt1, dt2));
  g_date_time_unref (dt2);

  g_date_time_unref (dt1);
}

static void
test_g_date_time_copy (void)
{
  GDateTime *dt1, *dt2;

  dt1 = g_date_time_now ();
  dt2 = g_date_time_copy (dt1);
  g_assert (dt1 != NULL);
  g_assert (dt2 != NULL);
  g_assert_cmpint (0, ==, g_date_time_compare (dt1, dt2));
  g_date_time_unref (dt1);
  g_date_time_unref (dt2);
}

static void
test_g_date_time_date (void)
{
  GDateTime *dt1, *dt2;

  dt1 = g_date_time_new_full (2009, 10, 19, 13, 0, 55);
  dt2 = g_date_time_date (dt1);
  g_assert (dt1 != NULL);
  g_assert (dt2 != NULL);
  g_assert_cmpint (2009, ==, g_date_time_get_year (dt2));
  g_assert_cmpint (10, ==, g_date_time_get_month (dt2));
  g_assert_cmpint (19, ==, g_date_time_get_day_of_month (dt2));
  g_assert_cmpint (0, ==, g_date_time_get_hour (dt2));
  g_assert_cmpint (0, ==, g_date_time_get_minute (dt2));
  g_assert_cmpint (0, ==, g_date_time_get_second (dt2));
  g_date_time_unref (dt1);
  g_date_time_unref (dt2);
}

static void
test_g_date_time_equal (void)
{
  GDateTime *dt1, *dt2;

  dt1 = g_date_time_new_from_date (2009, 10, 19);
  dt2 = g_date_time_new_from_date (2009, 10, 19);
  g_assert (g_date_time_equal (dt1, dt2));
  g_date_time_unref (dt1);
  g_date_time_unref (dt2);

  dt1 = g_date_time_new_from_date (2009, 10, 18);
  dt2 = g_date_time_new_from_date (2009, 10, 19);
  g_assert (!g_date_time_equal (dt1, dt2));
  g_date_time_unref (dt1);
  g_date_time_unref (dt2);
}

static void
test_g_date_time_get_day_of_week (void)
{
  GDateTime *dt;

  dt = g_date_time_new_from_date (2009, 10, 19);
  g_assert_cmpint (1, ==, g_date_time_get_day_of_week (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_from_date (2000, 10, 1);
  g_assert_cmpint (7, ==, g_date_time_get_day_of_week (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_day_of_month (void)
{
  GDateTime *dt;

  dt = g_date_time_new_from_date (2009, 10, 19);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 19);
  g_date_time_unref (dt);

  dt = g_date_time_new_from_date (1400, 3, 12);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 12);
  g_date_time_unref (dt);

  dt = g_date_time_new_from_date (1800, 12, 31);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 31);
  g_date_time_unref (dt);

  dt = g_date_time_new_from_date (1000, 1, 1);
  g_assert_cmpint (g_date_time_get_day_of_month (dt), ==, 1);
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_hour (void)
{
  GDateTime *dt;

  dt = g_date_time_new_full (2009, 10, 19, 15, 13, 11);
  g_assert_cmpint (15, ==, g_date_time_get_hour (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_full (100, 10, 19, 1, 0, 0);
  g_assert_cmpint (1, ==, g_date_time_get_hour (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_full (100, 10, 19, 0, 0, 0);
  g_assert_cmpint (0, ==, g_date_time_get_hour (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_full (100, 10, 1, 23, 59, 59);
  g_assert_cmpint (23, ==, g_date_time_get_hour (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_julian (void)
{
  GDateTime *dt;
  gint period, julian, hour = 1, min = 1, sec = 1;

  dt = g_date_time_new_from_date (1984, 8, 16);
  g_date_time_get_julian (dt, &period, &julian, &hour, &min, &sec);
  g_assert_cmpint (period, ==, 0);
  g_assert_cmpint (julian, ==, 2445929);
  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_microsecond (void)
{
  GTimeVal   tv;
  GDateTime *dt;

  g_get_current_time (&tv);
  dt = g_date_time_new_from_timeval (&tv);
  g_assert_cmpint (tv.tv_usec, ==, g_date_time_get_microsecond (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_millisecond (void)
{
  GTimeVal   tv;
  GDateTime *dt;

  g_get_current_time (&tv);
  dt = g_date_time_new_from_timeval (&tv);
  g_assert_cmpint ((tv.tv_usec/1000), ==, g_date_time_get_millisecond (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_year (void)
{
  GDateTime *dt;

  dt = g_date_time_new_from_date (2009, 1, 1);
  g_assert_cmpint (2009, ==, g_date_time_get_year (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_from_date (1, 1, 1);
  g_assert_cmpint (1, ==, g_date_time_get_year (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_from_date (13, 1, 1);
  g_assert_cmpint (13, ==, g_date_time_get_year (dt));
  g_date_time_unref (dt);

  dt = g_date_time_new_from_date (3000, 1, 1);
  g_assert_cmpint (3000, ==, g_date_time_get_year (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_hash (void)
{
  GHashTable *h;

  h = g_hash_table_new_full (g_date_time_hash, g_date_time_equal,
                             (GDestroyNotify)g_date_time_unref,
                             NULL);
  g_hash_table_insert (h, g_date_time_now (), NULL);
  g_hash_table_remove_all (h);
  g_hash_table_destroy (h);
}

static void
test_g_date_time_new_from_timeval (void)
{
  GDateTime *dt;
  GTimeVal   tv, tv2;

  g_get_current_time (&tv);
  dt = g_date_time_new_from_timeval (&tv);
  g_date_time_to_timeval (dt, &tv2);
  g_assert_cmpint (tv.tv_sec, ==, tv2.tv_sec);
  g_assert_cmpint (tv.tv_usec, ==, tv2.tv_usec);
  g_date_time_unref (dt);
}

static void
test_g_date_time_to_time_t (void)
{
  GDateTime *dt;
  time_t     t;

  t = time (NULL);
  dt = g_date_time_new_from_time_t (t);
  g_assert_cmpint (g_date_time_to_time_t (dt), ==, t);
  g_date_time_unref (dt);
}

static void
test_g_date_time_ref (void)
{
  GDateTime *dt;
  struct {
    gint64 t;
    gint ref_count;
  } *t;

  dt = g_date_time_now ();
  t = (void*)dt;
  g_assert_cmpint (t->ref_count, ==, 1);
  g_date_time_ref (dt);
  g_assert_cmpint (t->ref_count, ==, 2);
  g_date_time_unref (dt);
  g_assert_cmpint (t->ref_count, ==, 1);
  g_date_time_unref (dt);
  g_assert_cmpint (t->ref_count, ==, 0);
}

static void
test_g_date_time_add_years (void)
{
  GDateTime *dt, *dt2;

  dt = g_date_time_new_from_date (2009, 10, 19);
  dt2 = g_date_time_add_years (dt, 1);
  g_assert_cmpint (2010, ==, g_date_time_get_year (dt2));
  g_date_time_unref (dt);
  g_date_time_unref (dt2);
}

static void
test_g_date_time_add_months (void)
{
#define TEST_ADD_MONTHS(y,m,d,a,ny,nm,nd) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_from_date (y, m, d); \
  dt2 = g_date_time_add_months (dt, a); \
  ASSERT_DATE (dt2, ny, nm, nd); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_MONTHS (2009, 12, 31, 1, 2010, 1, 31);
  TEST_ADD_MONTHS (2009, 12, 31, 1, 2010, 1, 31);
  TEST_ADD_MONTHS (2009, 6, 15, 1, 2009, 7, 15);
  TEST_ADD_MONTHS (1400, 3, 1, 1, 1400, 4, 1);
  TEST_ADD_MONTHS (1400, 1, 31, 1, 1400, 2, 28);
  TEST_ADD_MONTHS (1400, 1, 31, 7200, 2000, 1, 31);
  TEST_ADD_MONTHS (2008, 2, 29, 12, 2009, 2, 28);
  TEST_ADD_MONTHS (2000, 8, 16, -5, 2000, 3, 16);
  TEST_ADD_MONTHS (2000, 8, 16, -12, 1999, 8, 16);
  TEST_ADD_MONTHS (2011, 2, 1, -13, 2010, 1, 1);
  TEST_ADD_MONTHS (1776, 7, 4, 1200, 1876, 7, 4);
}

static void
test_g_date_time_add_days (void)
{
#define TEST_ADD_DAYS(y,m,d,a,ny,nm,nd) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_from_date (y, m, d); \
  dt2 = g_date_time_add_days (dt, a); \
  g_assert_cmpint (ny, ==, g_date_time_get_year (dt2)); \
  g_assert_cmpint (nm, ==, g_date_time_get_month (dt2)); \
  g_assert_cmpint (nd, ==, g_date_time_get_day_of_month (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_DAYS (2009, 1, 31, 1, 2009, 2, 1);
  TEST_ADD_DAYS (2009, 2, 1, -1, 2009, 1, 31);
  TEST_ADD_DAYS (2008, 2, 28, 1, 2008, 2, 29);
  TEST_ADD_DAYS (2008, 12, 31, 1, 2009, 1, 1);
  TEST_ADD_DAYS (1, 1, 1, 1, 1, 1, 2);
  TEST_ADD_DAYS (1955, 5, 24, 10, 1955, 6, 3);
  TEST_ADD_DAYS (1955, 5, 24, -10, 1955, 5, 14);
}

static void
test_g_date_time_add_weeks (void)
{
#define TEST_ADD_WEEKS(y,m,d,a,ny,nm,nd) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_from_date (y, m, d); \
  dt2 = g_date_time_add_weeks (dt, a); \
  g_assert_cmpint (ny, ==, g_date_time_get_year (dt2)); \
  g_assert_cmpint (nm, ==, g_date_time_get_month (dt2)); \
  g_assert_cmpint (nd, ==, g_date_time_get_day_of_month (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_WEEKS (2009, 1, 1, 1, 2009, 1, 8);
  TEST_ADD_WEEKS (2009, 8, 30, 1, 2009, 9, 6);
  TEST_ADD_WEEKS (2009, 12, 31, 1, 2010, 1, 7);
  TEST_ADD_WEEKS (2009, 1, 1, -1, 2008, 12, 25);
}

static void
test_g_date_time_add_hours (void)
{
#define TEST_ADD_HOURS(y,m,d,h,mi,s,a,ny,nm,nd,nh,nmi,ns) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_full (y, m, d, h, mi, s); \
  dt2 = g_date_time_add_hours (dt, a); \
  g_assert_cmpint (ny, ==, g_date_time_get_year (dt2)); \
  g_assert_cmpint (nm, ==, g_date_time_get_month (dt2)); \
  g_assert_cmpint (nd, ==, g_date_time_get_day_of_month (dt2)); \
  g_assert_cmpint (nh, ==, g_date_time_get_hour (dt2)); \
  g_assert_cmpint (nmi, ==, g_date_time_get_minute (dt2)); \
  g_assert_cmpint (ns, ==, g_date_time_get_second (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_HOURS (2009,  1,  1,  0, 0, 0, 1, 2009, 1, 1, 1, 0, 0);
  TEST_ADD_HOURS (2008, 12, 31, 23, 0, 0, 1, 2009, 1, 1, 0, 0, 0);
}

static void
test_g_date_time_add_full (void)
{
#define TEST_ADD_FULL(y,m,d,h,mi,s,ay,am,ad,ah,ami,as,ny,nm,nd,nh,nmi,ns) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_full (y, m, d, h, mi, s); \
  dt2 = g_date_time_add_full (dt, ay, am, ad, ah, ami, as); \
  g_assert_cmpint (ny, ==, g_date_time_get_year (dt2)); \
  g_assert_cmpint (nm, ==, g_date_time_get_month (dt2)); \
  g_assert_cmpint (nd, ==, g_date_time_get_day_of_month (dt2)); \
  g_assert_cmpint (nh, ==, g_date_time_get_hour (dt2)); \
  g_assert_cmpint (nmi, ==, g_date_time_get_minute (dt2)); \
  g_assert_cmpint (ns, ==, g_date_time_get_second (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_FULL (2009, 10, 21, 0, 0, 0,
                    1,  1,  1, 1, 1, 1,
                 2010, 11, 22, 1, 1, 1);
  TEST_ADD_FULL (2000,  1,  1, 1, 1, 1,
                    0,  1,  0, 0, 0, 0,
                 2000,  2,  1, 1, 1, 1);
  TEST_ADD_FULL (2000,  1,  1, 0, 0, 0,
                   -1,  1,  0, 0, 0, 0,
                 1999,  2,  1, 0, 0, 0);
}

static void
test_g_date_time_add_milliseconds (void)
{
#define TEST_ADD_MILLISECOND(i,o) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_from_date (2000, 1, 1); \
  dt2 = g_date_time_add_milliseconds (dt, i); \
  g_assert_cmpint (o, ==, g_date_time_get_millisecond (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_MILLISECOND (199, 199);
  TEST_ADD_MILLISECOND (10001, 1);
  TEST_ADD_MILLISECOND (22201, 201);
  TEST_ADD_MILLISECOND (-1000, 0);
}

static void
test_g_date_time_add_minutes (void)
{
#define TEST_ADD_MINUTES(i,o) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_from_date (2000, 1, 1); \
  dt2 = g_date_time_add_minutes (dt, i); \
  g_assert_cmpint (o, ==, g_date_time_get_minute (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_MINUTES (60, 0);
  TEST_ADD_MINUTES (100, 40);
  TEST_ADD_MINUTES (5, 5);
  TEST_ADD_MINUTES (86401, 1);
  TEST_ADD_MINUTES (-86401, 59);
}

static void
test_g_date_time_add_seconds (void)
{
#define TEST_ADD_SECONDS(i,o) G_STMT_START { \
  GDateTime *dt, *dt2; \
  dt = g_date_time_new_from_date (2000, 1, 1); \
  dt2 = g_date_time_add_seconds (dt, i); \
  g_assert_cmpint (o, ==, g_date_time_get_second (dt2)); \
  g_date_time_unref (dt); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_ADD_SECONDS (1, 1);
  TEST_ADD_SECONDS (60, 0);
  TEST_ADD_SECONDS (61, 1);
  TEST_ADD_SECONDS (120, 0);
  TEST_ADD_SECONDS (-61, 59);
  TEST_ADD_SECONDS (86401, 1);
  TEST_ADD_SECONDS (-86401, 59);
  TEST_ADD_SECONDS (-31, 29);
  TEST_ADD_SECONDS (13, 13);
}

static void
test_g_date_time_diff (void)
{
#define TEST_DIFF(y,m,d,y2,m2,d2,u) G_STMT_START { \
  GDateTime *dt1, *dt2; \
  GTimeSpan  ts = 0; \
  dt1 = g_date_time_new_from_date (y, m, d); \
  dt2 = g_date_time_new_from_date (y2, m2, d2); \
  g_date_time_diff (dt1, dt2, &ts); \
  g_assert_cmpint (ts, ==, u); \
  g_date_time_unref (dt1); \
  g_date_time_unref (dt2); \
} G_STMT_END

  TEST_DIFF (2009, 1, 1, 2009, 2, 1, G_TIME_SPAN_DAY * 31);
  TEST_DIFF (2009, 1, 1, 2010, 1, 1, G_TIME_SPAN_DAY * 365);
  TEST_DIFF (2008, 2, 28, 2008, 2, 29, G_TIME_SPAN_DAY);
  TEST_DIFF (2008, 2, 29, 2008, 2, 28, -G_TIME_SPAN_DAY);

  /* TODO: Add usec tests */
}

static void
test_g_date_time_get_minute (void)
{
  GDateTime *dt;

  dt = g_date_time_new_full (2009, 12, 1, 1, 31, 0);
  g_assert_cmpint (31, ==, g_date_time_get_minute (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_month (void)
{
  GDateTime *dt;

  dt = g_date_time_new_full (2009, 12, 1, 1, 31, 0);
  g_assert_cmpint (12, ==, g_date_time_get_month (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_second (void)
{
  GDateTime *dt;

  dt = g_date_time_new_full (2009, 12, 1, 1, 31, 44);
  g_assert_cmpint (44, ==, g_date_time_get_second (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_new_from_date (void)
{
  GDateTime *dt;

  dt = g_date_time_new_from_date (2009, 12, 11);
  g_assert_cmpint (2009, ==, g_date_time_get_year (dt));
  g_assert_cmpint (12, ==, g_date_time_get_month (dt));
  g_assert_cmpint (11, ==, g_date_time_get_day_of_month (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_new_full (void)
{
  GDateTime *dt;

  dt = g_date_time_new_full (2009, 12, 11, 12, 11, 10);
  g_assert_cmpint (2009, ==, g_date_time_get_year (dt));
  g_assert_cmpint (12, ==, g_date_time_get_month (dt));
  g_assert_cmpint (11, ==, g_date_time_get_day_of_month (dt));
  g_assert_cmpint (12, ==, g_date_time_get_hour (dt));
  g_assert_cmpint (11, ==, g_date_time_get_minute (dt));
  g_assert_cmpint (10, ==, g_date_time_get_second (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_unref (void)
{
  GDateTime *dt;
  struct gdt {
    gint64 a;
    gint   ref_count;
  } *d;

  dt = g_date_time_now ();
  d = (void*) dt;
  g_assert_cmpint (d->ref_count, ==, 1);
  dt = g_date_time_ref (dt);
  g_assert_cmpint (d->ref_count, ==, 2);
  g_date_time_unref (dt);
  g_assert_cmpint (d->ref_count, ==, 1);
  g_date_time_unref (dt);
}

static void
test_g_date_time_utc_now (void)
{
  GDateTime *dt;
  time_t     t;
  struct tm  tm;

  t = time (NULL);
  gmtime_r (&t, &tm);
  dt = g_date_time_new_from_time_t (t);
  g_assert_cmpint (1900 + tm.tm_year, ==, g_date_time_get_year (dt));
  g_assert_cmpint (tm.tm_mon + 1, ==, g_date_time_get_month (dt));
  g_assert_cmpint (tm.tm_mday, ==, g_date_time_get_day_of_month (dt));
  g_assert_cmpint (tm.tm_hour, ==, g_date_time_get_hour (dt));
  g_assert_cmpint (tm.tm_min, ==, g_date_time_get_minute (dt));
  g_assert_cmpint (tm.tm_sec, ==, g_date_time_get_second (dt));
  g_date_time_unref (dt);
}

static void
test_g_date_time_get_utc_offset (void)
{
  GDateTime *dt;
  GTimeSpan  ts;

  dt = g_date_time_new_from_date (2009, 10, 22);
  g_date_time_get_utc_offset (dt, &ts);
  g_assert_cmpint (ts, ==, -8 * G_TIME_SPAN_HOUR);
  g_date_time_unref (dt);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/GDateTime/add_days",
                   test_g_date_time_add_days);
  g_test_add_func ("/GDateTime/add_full",
                   test_g_date_time_add_full);
  g_test_add_func ("/GDateTime/add_hours",
                   test_g_date_time_add_hours);
  g_test_add_func ("/GDateTime/add_milliseconds",
                   test_g_date_time_add_milliseconds);
  g_test_add_func ("/GDateTime/add_minutes",
                   test_g_date_time_add_minutes);
  g_test_add_func ("/GDateTime/add_months",
                   test_g_date_time_add_months);
  g_test_add_func ("/GDateTime/add_seconds",
                   test_g_date_time_add_seconds);
  g_test_add_func ("/GDateTime/add_weeks",
                   test_g_date_time_add_weeks);
  g_test_add_func ("/GDateTime/add_years",
                   test_g_date_time_add_years);
  g_test_add_func ("/GDateTime/compare",
                   test_g_date_time_compare);
  g_test_add_func ("/GDateTime/copy",
                   test_g_date_time_copy);
  g_test_add_func ("/GDateTime/date",
                   test_g_date_time_date);
  g_test_add_func ("/GDateTime/diff",
                   test_g_date_time_diff);
  g_test_add_func ("/GDateTime/equal",
                   test_g_date_time_equal);
  /*
  g_test_add_func ("/GDateTime/format_for_display",
                   test_g_date_time_format_for_display);
  */
  g_test_add_func ("/GDateTime/get_day_of_week",
                   test_g_date_time_get_day_of_week);
  g_test_add_func ("/GDateTime/get_day_of_month",
                   test_g_date_time_get_day_of_month);
  g_test_add_func ("/GDateTime/get_hour",
                   test_g_date_time_get_hour);
  g_test_add_func ("/GDateTime/get_julian",
                   test_g_date_time_get_julian);
  g_test_add_func ("/GDateTime/get_microsecond",
                   test_g_date_time_get_microsecond);
  g_test_add_func ("/GDateTime/get_millisecond",
                   test_g_date_time_get_millisecond);
  g_test_add_func ("/GDateTime/get_minute",
                   test_g_date_time_get_minute);
  g_test_add_func ("/GDateTime/get_month",
                   test_g_date_time_get_month);
  g_test_add_func ("/GDateTime/get_second",
                   test_g_date_time_get_second);
  g_test_add_func ("/GDateTime/get_utc_offset",
                   test_g_date_time_get_utc_offset);
  g_test_add_func ("/GDateTime/get_year",
                   test_g_date_time_get_year);
  g_test_add_func ("/GDateTime/hash",
                   test_g_date_time_hash);
  g_test_add_func ("/GDateTime/is_leap_year",
                   test_g_date_time_is_leap_year);
  g_test_add_func ("/GDateTime/new_from_date",
                   test_g_date_time_new_from_date);
  g_test_add_func ("/GDateTime/new_from_time_t",
                   test_g_date_time_new_from_time_t);
  g_test_add_func ("/GDateTime/new_from_timeval",
                   test_g_date_time_new_from_timeval);
  g_test_add_func ("/GDateTime/new_full",
                   test_g_date_time_new_full);
  g_test_add_func ("/GDateTime/now",
                   test_g_date_time_now);
  /*
  g_test_add_func ("/GDateTime/parse",
                   test_g_date_time_parse);
  g_test_add_func ("/GDateTime/parse_with_format",
                   test_g_date_time_parse_with_format);
  g_test_add_func ("/GDateTime/printf",
                   test_g_date_time_printf);
  */
  g_test_add_func ("/GDateTime/ref",
                   test_g_date_time_ref);
  /*
  g_test_add_func ("/GDateTime/to_local",
                   test_g_date_time_to_local);
  */
  g_test_add_func ("/GDateTime/to_time_t",
                   test_g_date_time_to_time_t);
  /*
  g_test_add_func ("/GDateTime/to_timeval",
                   test_g_date_time_to_timeval);
  g_test_add_func ("/GDateTime/to_utc",
                   test_g_date_time_to_utc);
  */
  g_test_add_func ("/GDateTime/today",
                   test_g_date_time_today);
  g_test_add_func ("/GDateTime/unref",
                   test_g_date_time_unref);
  g_test_add_func ("/GDateTime/utc_now",
                   test_g_date_time_utc_now);

  return g_test_run ();
}