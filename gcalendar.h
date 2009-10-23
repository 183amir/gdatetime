/* gcalendar.h
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

#ifndef __G_CALENDAR_H__
#define __G_CALENDAR_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define G_TYPE_CALENDAR             (g_calendar_get_type ())
#define G_CALENDAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_CALENDAR, GCalendar))
#define G_CALENDAR_CONST(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_CALENDAR, GCalendar const))
#define G_CALENDAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  G_TYPE_CALENDAR, GCalendarClass))
#define G_IS_CALENDAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_CALENDAR))
#define G_IS_CALENDAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  G_TYPE_CALENDAR))
#define G_CALENDAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  G_TYPE_CALENDAR, GCalendarClass))

typedef struct _GCalendar         GCalendar;
typedef struct _GCalendarClass    GCalendarClass;
typedef struct _GCalendarPrivate  GCalendarPrivate;

struct _GCalendar
{
  GObject parent;

  /*< private >*/
  GCalendarPrivate *priv;
};

struct _GCalendarClass
{
  GObjectClass parent_class;

  gint (*get_year)         (GCalendar *calendar, GDateTime *datetime);
  gint (*get_month)        (GCalendar *calendar, GDateTime *datetime);
  gint (*get_day_of_month) (GCalendar *calendar, GDateTime *datetime);
  gint (*get_hour)         (GCalendar *calendar, GDateTime *datetime);
  gint (*get_minute)       (GCalendar *calendar, GDateTime *datetime);
  gint (*get_second)       (GCalendar *calendar, GDateTime *datetime);
};

GType       g_calendar_get_type         (void) G_GNUC_CONST;
GCalendar * g_calendar_current          (void);
gint        g_calendar_get_year         (GCalendar *calendar,
                                         GDateTime *datetime);
gint        g_calendar_get_month        (GCalendar *calendar,
                                         GDateTime *datetime);
gint        g_calendar_get_day_of_month (GCalendar *calendar,
                                         GDateTime *datetime);
gint        g_calendar_get_hour         (GCalendar *calendar,
                                         GDateTime *datetime);
gint        g_calendar_get_minute       (GCalendar *calendar,
                                         GDateTime *datetime);
gint        g_calendar_get_second       (GCalendar *calendar,
                                         GDateTime *datetime);

G_END_DECLS

#endif /* __G_CALENDAR_H__ */
