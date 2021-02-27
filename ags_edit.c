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

#include "ags_edit.h"

#include <epoxy/gl.h>
#include <math.h>

void ags_edit_class_init(AgsEditClass *edit);
void ags_edit_init(AgsEdit *edit);
void ags_edit_finalize(GObject *gobject);

GLuint ags_edit_gl_area_create_shader(int type,
				      const char *src);
void ags_edit_gl_area_init_shaders(const char *vertex_path,
				   const char *fragment_path,
				   GLuint *program_out);

void ags_edit_gl_area_realize(GtkWidget *widget);
void ags_edit_gl_area_unrealize(GtkWidget *widget);

gboolean ags_edit_render_callback(GtkGLArea *gl_area,
				  GdkGLContext *gl_context,
				  AgsEdit *edit);
gboolean ags_edit_configure_event_callback(AgsEdit *edit,
					   GdkEvent *event,
					   gpointer user_data);

gboolean ags_edit_render_timeout(AgsEdit *edit);

static gpointer ags_edit_parent_class = NULL;

GtkGLArea *gl_area_0 = NULL;
GtkGLArea *gl_area_1 = NULL;

gint64 last_render_0 = 0;
gint64 last_render_1 = 0;
gint64 last_configure = 0;

GLuint gl_area_0_program = 0;
GLuint gl_area_1_program = 0;

GLuint gl_area_0_vertex_arrays = 0;
GLuint gl_area_1_vertex_arrays = 0;

GType
ags_edit_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_edit = 0;

    static const GTypeInfo ags_edit_info = {
      sizeof (AgsEditClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_edit_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsEdit),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_edit_init,
    };

    ags_type_edit = g_type_register_static(GTK_TYPE_WINDOW,
					   "AgsEdit", &ags_edit_info,
					   0);
    
    g_once_init_leave(&g_define_type_id__volatile, ags_type_edit);
  }

  return g_define_type_id__volatile;
}

void
ags_edit_class_init(AgsEditClass *edit)
{
  GObjectClass *gobject;
  
  ags_edit_parent_class = g_type_class_peek_parent(edit);

  /* GObjectClass */
  gobject = (GObjectClass *) edit;

  gobject->finalize = ags_edit_finalize;
}

void
ags_edit_init(AgsEdit *edit)
{
  GtkBox *box;

  gtk_widget_set_events((GtkWidget *) edit,
			(GDK_EXPOSURE_MASK
			 | GDK_LEAVE_NOTIFY_MASK
			 | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 | GDK_POINTER_MOTION_MASK
			 | GDK_POINTER_MOTION_HINT_MASK
			 | GDK_CONTROL_MASK
			 | GDK_KEY_PRESS_MASK
			 | GDK_KEY_RELEASE_MASK));

  gtk_window_set_default_size((GtkWindow *) edit,
			      600, 400);
  
  box = (GtkBox *) gtk_box_new(GTK_ORIENTATION_HORIZONTAL,
			       0);

  gtk_widget_set_vexpand((GtkWidget *) box,
			 TRUE);
  gtk_widget_set_hexpand((GtkWidget *) box,
			 TRUE);
  
  gtk_container_add((GtkContainer *) edit,
		    (GtkWidget *) box);
  
  gl_area_0 = 
    edit->gl_area_0 = (GtkGLArea *) gtk_gl_area_new();
  gtk_box_pack_start(box,
		     (GtkWidget *) edit->gl_area_0,
		     TRUE, TRUE,
		     0);

  gl_area_1 = 
    edit->gl_area_1 = (GtkGLArea *) gtk_gl_area_new();
  gtk_box_pack_start(box,
		     (GtkWidget *) edit->gl_area_1,
		     TRUE, TRUE,
		     0);

  g_signal_connect(edit->gl_area_0, "realize",
		   G_CALLBACK(ags_edit_gl_area_realize), NULL);

  g_signal_connect(edit->gl_area_0, "unrealize",
		   G_CALLBACK(ags_edit_gl_area_unrealize), NULL);

  g_signal_connect(edit->gl_area_1, "realize",
		   G_CALLBACK(ags_edit_gl_area_realize), NULL);

  g_signal_connect(edit->gl_area_1, "unrealize",
		   G_CALLBACK(ags_edit_gl_area_unrealize), NULL);
  
  g_signal_connect(edit, "configure_event",
		   G_CALLBACK(ags_edit_configure_event_callback), NULL);

  g_signal_connect(edit->gl_area_0, "render",
		   G_CALLBACK(ags_edit_render_callback), edit);

  g_signal_connect(edit->gl_area_1, "render",
		   G_CALLBACK(ags_edit_render_callback), edit);

  g_timeout_add((guint) (1000.0 / 60.0),
		(GSourceFunc) ags_edit_render_timeout,
		edit);
}

void
ags_edit_finalize(GObject *gobject)
{
  AgsEdit *edit;

  edit = (AgsEdit *) gobject;
}

/* Create and compile a shader */
GLuint
ags_edit_gl_area_create_shader(int type,
			       const char *src)
{
  GLuint shader;
  int status;
  
  shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  
  if(status == GL_FALSE){
    int log_len;
    char *buffer;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

    buffer = g_malloc(log_len + 1);
    glGetShaderInfoLog(shader, log_len, NULL, buffer);

    g_warning("Compile failure in %s shader:\n%s",
	      type == GL_VERTEX_SHADER ? "vertex" : "fragment",
	      buffer);

    g_free(buffer);

    glDeleteShader(shader);

    return(0);
  }
  
  return(shader);
}

/* Initialize the shaders and link them into a program */
void
ags_edit_gl_area_init_shaders(const char *vertex_path,
			      const char *fragment_path,
			      GLuint *program_out)
{
  GLuint vertex, fragment;
  GLuint program = 0;
  int status;
  GBytes *source;

  source = g_mapped_file_get_bytes(g_mapped_file_new(vertex_path,
						     FALSE,
						     NULL));
//  source = g_resources_lookup_data(vertex_path, 0, NULL);
  vertex = ags_edit_gl_area_create_shader(GL_VERTEX_SHADER, g_bytes_get_data(source, NULL));
  g_bytes_unref(source);

  if(vertex == 0){
    program_out[0] = 0;
    
    return;
  }

  source = g_mapped_file_get_bytes(g_mapped_file_new(fragment_path,
						     FALSE,
						     NULL));
//  source = g_resources_lookup_data(fragment_path, 0, NULL);
  fragment = ags_edit_gl_area_create_shader(GL_FRAGMENT_SHADER, g_bytes_get_data(source, NULL));
  g_bytes_unref(source);

  if(fragment == 0){
    glDeleteShader(vertex);
    program_out[0] = 0;
      
    return;
  }

  program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);

  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &status);
  
  if(status == GL_FALSE){
    int log_len;
    char *buffer;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

    buffer = g_malloc(log_len + 1);
    glGetProgramInfoLog(program, log_len, NULL, buffer);

    g_warning("Linking failure:\n%s", buffer);

    g_free(buffer);

    glDeleteProgram(program);
    program = 0;

    goto out;
  }

  glDetachShader(program, vertex);
  glDetachShader(program, fragment);

out:
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  if(program_out != NULL){
    program_out[0] = program;
  }
}

void
ags_edit_gl_area_realize(GtkWidget *widget)
{
  GdkGLContext *context;

  char *vertex_path, *fragment_path;
  
  gtk_gl_area_make_current(GTK_GL_AREA(widget));

  if(gtk_gl_area_get_error(GTK_GL_AREA (widget)) != NULL){
    return;
  }

  context = gtk_gl_area_get_context(GTK_GL_AREA(widget));

  vertex_path = NULL;
  fragment_path = NULL;
  
  if(widget == gl_area_0){
    vertex_path = "/home/joelkraehemann/opengl/ags_edit_gl_area_0_vertex_shader.glsl";
    fragment_path = "/home/joelkraehemann/opengl/ags_edit_gl_area_0_fragment_shader.glsl";

    ags_edit_gl_area_init_shaders(vertex_path, fragment_path, &gl_area_0_program);

    glCreateVertexArrays(1, &gl_area_0_vertex_arrays);
    glBindVertexArray(gl_area_0_vertex_arrays);
  }else if(widget == gl_area_1){
    vertex_path = "/home/joelkraehemann/opengl/ags_edit_gl_area_1_vertex_shader.glsl";
    fragment_path = "/home/joelkraehemann/opengl/ags_edit_gl_area_1_fragment_shader.glsl";

    ags_edit_gl_area_init_shaders(vertex_path, fragment_path, &gl_area_1_program);

    glCreateVertexArrays(1, &gl_area_1_vertex_arrays);
    glBindVertexArray(gl_area_1_vertex_arrays);
  }
}

/* We should tear down the state when unrealizing */
void
ags_edit_gl_area_unrealize(GtkWidget *widget)
{
  gtk_gl_area_make_current(GTK_GL_AREA(widget));

  if(gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL){
    return;
  }

  glDeleteVertexArrays(1, gl_area_0_vertex_arrays);
  glDeleteProgram(gl_area_0_program);

  glDeleteVertexArrays(1, gl_area_1_vertex_arrays);
  glDeleteProgram(gl_area_1_program);
}

gboolean
ags_edit_render_callback(GtkGLArea *gl_area,
			 GdkGLContext *gl_context,
			 AgsEdit *edit)
{
  GLfloat *bg_color;

  gint64 current_render;
  
  static const GLfloat red[] = { 1.0, 0.0, 0.0, 1.0 };
  static const GLfloat blue[] = { 0.0, 0.0, 1.0, 1.0 };

  if(gtk_gl_area_get_error(gl_area) != NULL){
    return(FALSE);
  }

  current_render = g_get_monotonic_time();

  if(gl_area == edit->gl_area_0){
    if(current_render < last_render_0 + (G_USEC_PER_SEC / 25)){
      return(FALSE);
    }

    last_render_0 = current_render;
  
    bg_color = red;
  }else{
    if(current_render < last_render_1 + (G_USEC_PER_SEC / 25)){
      return(FALSE);
    }

    last_render_1 = current_render;
  
    bg_color = blue;
  }
  
  glClearBufferfv(GL_COLOR,
		  0,
		  bg_color);


  if(gl_area == edit->gl_area_0){
    /* Use our shaders */
    glUseProgram(gl_area_0_program);
  }else{
    /* Use our shaders */
    glUseProgram(gl_area_1_program);
  }
  
  /* Draw the three vertices as a triangle */
  glDrawArrays(GL_TRIANGLES, 0, 3);

  /* We finished using the buffers and program */
//  glUseProgram(0);
  
//  glFlush();

  return(TRUE);
}

gboolean
ags_edit_configure_event_callback(AgsEdit *edit,
				  GdkEvent *event,
				  gpointer user_data)
{
  last_configure = g_get_monotonic_time() + (G_USEC_PER_SEC / 30);
  
  return(FALSE);
}

gboolean
ags_edit_render_timeout(AgsEdit *edit)
{
  gint64 current_time;

  current_time = g_get_monotonic_time();
  
  if(last_configure > last_render_0){
    if(last_render_0 + (G_USEC_PER_SEC / 30) < current_time){
      gtk_gl_area_queue_render((GtkGLArea *) edit->gl_area_0);
    }
  }

  if(last_configure > last_render_1){
    if(last_render_1 + (G_USEC_PER_SEC / 30) < current_time){
      gtk_gl_area_queue_render((GtkGLArea *) edit->gl_area_1);
    }
  }

  return(G_SOURCE_CONTINUE);
}

/**
 * ags_edit_new:
 *
 * Creates an #AgsEdit
 *
 * Returns: a new #AgsEdit
 *
 * Since: 3.8.0
 */
AgsEdit*
ags_edit_new()
{
  AgsEdit *edit;

  edit = (AgsEdit *) g_object_new(AGS_TYPE_EDIT,
				  NULL);

  return(edit);
}

int
main(int argc, char **argv)
{
  AgsEdit *edit;
  
  gtk_init(&argc, &argv);

  edit = ags_edit_new();
  
  gtk_widget_show_all((GtkWidget *) edit);

  gtk_main();

  return(0);
}
