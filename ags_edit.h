/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2021 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AGS_EDIT_H__
#define __AGS_EDIT_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define AGS_TYPE_EDIT                (ags_edit_get_type())
#define AGS_EDIT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_EDIT, AgsEdit))
#define AGS_EDIT_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_EDIT, AgsEditClass))
#define AGS_IS_EDIT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_EDIT))
#define AGS_IS_EDIT_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_EDIT))
#define AGS_EDIT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_EDIT, AgsEditClass))

typedef struct _AgsEdit AgsEdit;
typedef struct _AgsEditClass AgsEditClass;

struct _AgsEdit
{
  GtkWindow window;

  guint flags;

  GtkGLArea *gl_area_0;
  GtkGLArea *gl_area_1;
};

struct _AgsEditClass
{
  GtkWindowClass window;
};

GType ags_edit_get_type(void);

AgsEdit* ags_edit_new();

G_END_DECLS

#endif /*__AGS_EDIT_H__*/
