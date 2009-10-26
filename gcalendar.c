/* gcalendar.c
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

#include "gcalendar.h"
#include "gcalendargregorian.h"

G_DEFINE_ABSTRACT_TYPE (GCalendar, g_calendar, G_TYPE_OBJECT)

static void
g_calendar_class_init (GCalendarClass *klass)
{
}

static void
g_calendar_init (GCalendar *calendar)
{
}

gint
g_calendar_get_year (GCalendar *calendar,
                     GDateTime *datetime)
{
  return G_CALENDAR_GET_CLASS (calendar)->get_year (calendar, datetime);
}

gint
g_calendar_get_month (GCalendar *calendar,
                      GDateTime *datetime)
{
  return G_CALENDAR_GET_CLASS (calendar)->get_month (calendar, datetime);
}

gint
g_calendar_get_day_of_month (GCalendar *calendar,
                             GDateTime *datetime)
{
  return G_CALENDAR_GET_CLASS (calendar)->get_day_of_month (calendar, datetime);
}

gint
g_calendar_get_hour (GCalendar *calendar,
                     GDateTime *datetime)
{
  return G_CALENDAR_GET_CLASS (calendar)->get_hour (calendar, datetime);
}

gint
g_calendar_get_minute (GCalendar *calendar,
                       GDateTime *datetime)
{
  return G_CALENDAR_GET_CLASS (calendar)->get_minute (calendar, datetime);
}

gint
g_calendar_get_second (GCalendar *calendar,
                       GDateTime *datetime)
{
  return G_CALENDAR_GET_CLASS (calendar)->get_second (calendar, datetime);
}

GCalendar*
g_calendar_from_locale (void)
{
  static GCalendar *current = NULL;

  if (g_once_init_enter ((gsize*)&current))
    {
      GCalendar *calendar = NULL;

      /* TODO: Support other locales */
      calendar = g_calendar_gregorian_new ();

      g_once_init_leave ((gsize*)&current, (gsize)calendar);
    }

  return current;
}
