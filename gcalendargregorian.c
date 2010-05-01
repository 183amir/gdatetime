/* gcalendargregorian.c
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

#include "gcalendargregorian.h"

/**
 * SECTION:gcalendargregorian
 * @title: GCalendarGregorian
 * @short_description: The Gregorian calendar
 *
 * The Gregorian calendar is the currently the most geographically used
 * calendar.  It supports Jan 1, 1 Common Era to _INSERT_MAX_HERE_.  Howerver,
 * using the proleptic Gregorian calendar dates can be supported as far back
 * as -23,000 BC.
 *
 * TODO: Add some more information from the Calendar FAQ.
 * TODO: Update date/time boundries.
 */

G_DEFINE_TYPE (GCalendarGregorian, g_calendar_gregorian, G_TYPE_CALENDAR)

static gint
g_calendar_gregorian_real_get_year (GCalendar *calendar, /* IN */
                                    GDateTime *datetime) /* IN */
{
  return g_date_time_get_year (datetime);
}

static gint
g_calendar_gregorian_real_get_month (GCalendar *calendar, /* IN */
                                     GDateTime *datetime) /* IN */
{
  return g_date_time_get_month (datetime);
}

static gint
g_calendar_gregorian_real_get_day_of_month (GCalendar *calendar, /* IN */
                                            GDateTime *datetime) /* IN */
{
  return g_date_time_get_day_of_month (datetime);
}

gint
g_calendar_gregorian_real_get_day_of_week (GCalendar *calendar, /* IN */
                                           GDateTime *datetime) /* IN */
{
  return g_date_time_get_day_of_week (datetime);
}

static gint
g_calendar_gregorian_real_get_day_of_year (GCalendar *calendar, /* IN */
                                           GDateTime *datetime) /* IN */
{
  return g_date_time_get_day_of_year (datetime);
}

static gint
g_calendar_gregorian_real_get_hour (GCalendar *calendar, /* IN */
                                    GDateTime *datetime) /* IN */
{
  return g_date_time_get_hour (datetime);
}

static gint
g_calendar_gregorian_real_get_minute (GCalendar *calendar, /* IN */
                                      GDateTime *datetime) /* IN */
{
  return g_date_time_get_minute (datetime);
}

static gint
g_calendar_gregorian_real_get_second (GCalendar *calendar, /* IN */
                                      GDateTime *datetime) /* IN */
{
  return g_date_time_get_second (datetime);
}

static gboolean
g_calendar_gregorian_real_is_leap_year (GCalendar *calendar, /* IN */
                                        GDateTime *datetime) /* IN */
{
  return g_date_time_is_leap_year (datetime);
}

static void
g_calendar_gregorian_class_init (GCalendarGregorianClass *klass) /* IN */
{
  GCalendarClass *calendar_class;

  calendar_class                   = G_CALENDAR_CLASS (klass);
  calendar_class->get_year         = g_calendar_gregorian_real_get_year;
  calendar_class->get_month        = g_calendar_gregorian_real_get_month;
  calendar_class->get_day_of_month = g_calendar_gregorian_real_get_day_of_month;
  calendar_class->get_day_of_week  = g_calendar_gregorian_real_get_day_of_week;
  calendar_class->get_day_of_year  = g_calendar_gregorian_real_get_day_of_year;
  calendar_class->get_hour         = g_calendar_gregorian_real_get_hour;
  calendar_class->get_minute       = g_calendar_gregorian_real_get_minute;
  calendar_class->get_second       = g_calendar_gregorian_real_get_second;
  calendar_class->is_leap_year     = g_calendar_gregorian_real_is_leap_year;
}

static void
g_calendar_gregorian_init (GCalendarGregorian *self) /* IN */
{
}

/**
 * g_calendar_gregorian_new:
 *
 * Creates a new instance of #GCalendarGregorian which can be used to translate
 * GDateTime<!-- -->'s into the Gregorian Calendar.  Since #GDateTime uses the
 * Gregorian Calendar by default, this is mostly just a wrapper conforming to
 * the #GCalendar API.
 *
 * Return value: the newly created #GCalendarGregorian which should be freed with
 *   g_object_unref().
 *
 * Since: 2.26
 */
GCalendar*
g_calendar_gregorian_new (void)
{
  return g_object_new (G_TYPE_CALENDAR_GREGORIAN, 0);
}
