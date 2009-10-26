/* gcalendargregorian.h
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

#ifndef __G_CALENDAR_GREGORIAN_H__
#define __G_CALENDAR_GREGORIAN_H__

#include <glib-object.h>

#include "gcalendar.h"

G_BEGIN_DECLS

#define G_TYPE_CALENDAR_GREGORIAN             (g_calendar_gregorian_get_type ())
#define G_CALENDAR_GREGORIAN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_CALENDAR_GREGORIAN, GCalendarGregorian))
#define G_CALENDAR_GREGORIAN_CONST(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_CALENDAR_GREGORIAN, GCalendarGregorian const))
#define G_CALENDAR_GREGORIAN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  G_TYPE_CALENDAR_GREGORIAN, GCalendarGregorianClass))
#define G_IS_CALENDAR_GREGORIAN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_CALENDAR_GREGORIAN))
#define G_IS_CALENDAR_GREGORIAN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  G_TYPE_CALENDAR_GREGORIAN))
#define G_CALENDAR_GREGORIAN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  G_TYPE_CALENDAR_GREGORIAN, GCalendarGregorianClass))

typedef struct _GCalendarGregorian         GCalendarGregorian;
typedef struct _GCalendarGregorianClass    GCalendarGregorianClass;
typedef struct _GCalendarGregorianPrivate  GCalendarGregorianPrivate;

struct _GCalendarGregorian
{
  GCalendar parent;

  /*< private >*/
  GCalendarGregorianPrivate *priv;
};

struct _GCalendarGregorianClass
{
  GCalendarClass parent_class;
};

GType      g_calendar_gregorian_get_type (void) G_GNUC_CONST;
GCalendar* g_calendar_gregorian_new      (void);

G_END_DECLS

#endif /* __G_CALENDAR_GREGORIAN_H__ */
