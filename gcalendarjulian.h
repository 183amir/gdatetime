/* gcalendarjulian.h
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

#ifndef __G_CALENDAR_JULIAN_H__
#define __G_CALENDAR_JULIAN_H__

#include <glib-object.h>

#include "gcalendar.h"

G_BEGIN_DECLS

#define G_TYPE_CALENDAR_JULIAN             (g_calendar_julian_get_type ())
#define G_CALENDAR_JULIAN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_CALENDAR_JULIAN, GCalendarJulian))
#define G_CALENDAR_JULIAN_CONST(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_CALENDAR_JULIAN, GCalendarJulian const))
#define G_CALENDAR_JULIAN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  G_TYPE_CALENDAR_JULIAN, GCalendarJulianClass))
#define G_IS_CALENDAR_JULIAN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_CALENDAR_JULIAN))
#define G_IS_CALENDAR_JULIAN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  G_TYPE_CALENDAR_JULIAN))
#define G_CALENDAR_JULIAN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  G_TYPE_CALENDAR_JULIAN, GCalendarJulianClass))

typedef struct _GCalendarJulian         GCalendarJulian;
typedef struct _GCalendarJulianClass    GCalendarJulianClass;
typedef struct _GCalendarJulianPrivate  GCalendarJulianPrivate;

struct _GCalendarJulian
{
  GCalendar parent;

  /*< private >*/
  GCalendarJulianPrivate *priv;
};

struct _GCalendarJulianClass
{
  GCalendarClass parent_class;
};

GType      g_calendar_julian_get_type (void) G_GNUC_CONST;
GCalendar* g_calendar_julian_new      (void);

G_END_DECLS

#endif /* __G_CALENDAR_JULIAN_H__ */
