
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         gm-cell-renderer-bitext.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt, but very directly
 *                          inspired by the code of GossipCellRendererText in
 *                          imendio's gossip instant messaging client
 *   copyright            : (c) 2004 by Imendio AB
 *                          (c) 2006-2007 by Julien Puydt
 *   description          : implementation of a cell renderer showing two texts
 *
 */

#include <string.h>

#include "gm-cell-renderer-bitext.h"

struct _GmCellRendererBitextPrivate
{

  gchar *primary_text;
  gchar *secondary_text;

  gboolean is_valid;
  gboolean is_selected;
};

enum
{

  GM_CELL_RENDERER_BITEXT_PROP_PRIMARY_TEXT = 1,
  GM_CELL_RENDERER_BITEXT_PROP_SECONDARY_TEXT
};

G_DEFINE_TYPE (GmCellRendererBitext, gm_cell_renderer_bitext, GTK_TYPE_CELL_RENDERER_TEXT);

/* helper function */

static void
gm_cell_renderer_bitext_update_text (GmCellRendererBitext *renderer,
				     GtkWidget *widget,
				     gboolean is_selected)
{
  GtkStyle *style = NULL;
  PangoAttrList *attr_list = NULL;
  GdkColor color;
  PangoAttribute *attr_color = NULL;
  PangoAttribute *attr_size = NULL;
  gchar *str = NULL;

  if (renderer->priv->is_valid && renderer->priv->is_selected == is_selected)
    return;

  style = gtk_widget_get_style (widget);

  attr_list = pango_attr_list_new ();
  if (!is_selected) {

    color = style->text_aa[GTK_STATE_NORMAL];

    attr_color = pango_attr_foreground_new (color.red, color.green, color.blue);
    attr_color->start_index = strlen (renderer->priv->primary_text) + 1;
    attr_color->end_index = (guint) - 1;
    pango_attr_list_insert (attr_list, attr_color);
  }

  /* we want the secondary text smaller */
  attr_size = pango_attr_size_new ((int) (pango_font_description_get_size (style->font_desc) * 0.8));
  attr_size->start_index = strlen (renderer->priv->primary_text) + 1;
  attr_size->end_index = (guint) - 1;
  pango_attr_list_insert (attr_list, attr_size);

  if (renderer->priv->secondary_text && g_strcmp0 (renderer->priv->secondary_text, ""))
    str = g_strdup_printf ("%s\n%s",
                           renderer->priv->primary_text,
                           renderer->priv->secondary_text);
  else
    str = g_strdup_printf ("%s",
                           renderer->priv->primary_text);

  g_object_set (renderer,
		"visible", TRUE,
		"weight", PANGO_WEIGHT_NORMAL,
		"text", str,
		"attributes", attr_list,
		NULL);
  g_free (str);
  pango_attr_list_unref (attr_list);

  renderer->priv->is_selected = is_selected;
  renderer->priv->is_valid = TRUE;
}

/* overriden inherited functions, so we make sure the text is right before
 * we compute size or draw */

static void
gm_cell_renderer_bitext_get_size (GtkCellRenderer *cell,
				  GtkWidget *widget,
				  GdkRectangle *cell_area,
				  gint *x_offset,
				  gint *y_offset,
				  gint *width,
				  gint *height)
{
  GmCellRendererBitext *renderer = NULL;
  GtkCellRendererClass* parent_class = NULL;

  renderer = (GmCellRendererBitext *)cell;
  parent_class = GTK_CELL_RENDERER_CLASS (gm_cell_renderer_bitext_parent_class);

  gm_cell_renderer_bitext_update_text (renderer, widget,
				       renderer->priv->is_selected);

  parent_class->get_size (cell, widget, cell_area,
			  x_offset, y_offset,
			  width, height);
}

static void
gm_cell_renderer_bitext_render (GtkCellRenderer *cell,
				GdkWindow *window,
				GtkWidget *widget,
				GdkRectangle *background_area,
				GdkRectangle *cell_area,
				GdkRectangle *expose_area,
				GtkCellRendererState flags)
{
  GmCellRendererBitext *renderer = NULL;
  GtkCellRendererClass* parent_class = NULL;

  renderer = (GmCellRendererBitext *)cell;
  parent_class = GTK_CELL_RENDERER_CLASS (gm_cell_renderer_bitext_parent_class);

  gm_cell_renderer_bitext_update_text (renderer, widget,
				       (flags & GTK_CELL_RENDERER_SELECTED));
  parent_class->render (cell, window, widget,
			background_area, cell_area,
			expose_area, flags);
}

/* GObject code */

static void
gm_cell_renderer_bitext_finalize (GObject *obj)
{
  GmCellRendererBitext *self = NULL;

  self = (GmCellRendererBitext *)obj;

  g_free (self->priv->primary_text);
  g_free (self->priv->secondary_text);

  G_OBJECT_CLASS (gm_cell_renderer_bitext_parent_class)->finalize (obj);
}

static void
gm_cell_renderer_bitext_get_property (GObject *obj,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *spec)
{
  GmCellRendererBitext *renderer = NULL;

  renderer = (GmCellRendererBitext *)obj;

  switch (prop_id) {

  case GM_CELL_RENDERER_BITEXT_PROP_PRIMARY_TEXT:
    g_value_set_string (value, renderer->priv->primary_text);
    break;

  case GM_CELL_RENDERER_BITEXT_PROP_SECONDARY_TEXT:
    g_value_set_string (value, renderer->priv->secondary_text);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
gm_cell_renderer_bitext_set_property (GObject *obj,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *spec)
{
  GmCellRendererBitext *renderer = NULL;
  const gchar *str = NULL;

  renderer = (GmCellRendererBitext *)obj;

  switch (prop_id) {

  case GM_CELL_RENDERER_BITEXT_PROP_PRIMARY_TEXT:
    g_free (renderer->priv->primary_text);
    str = g_value_get_string (value);
    renderer->priv->primary_text = g_strdup (str ? str : "");
    (void) g_strdelimit (renderer->priv->primary_text, "\n\r\t", ' ');
    renderer->priv->is_valid = FALSE;
    break;

  case GM_CELL_RENDERER_BITEXT_PROP_SECONDARY_TEXT:
    g_free (renderer->priv->secondary_text);
    str = g_value_get_string (value);
    renderer->priv->secondary_text = g_strdup (str ? str : "");
    (void) g_strdelimit (renderer->priv->secondary_text, "\n\r\t", ' ');
    renderer->priv->is_valid = FALSE;
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
gm_cell_renderer_bitext_init (GmCellRendererBitext* self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
					    GM_TYPE_CELL_RENDERER_BITEXT,
					    GmCellRendererBitextPrivate);
  self->priv->primary_text = g_strdup ("");
  self->priv->secondary_text = g_strdup ("");
}

static void
gm_cell_renderer_bitext_class_init (GmCellRendererBitextClass* klass)
{
  GObjectClass *gobject_class = NULL;
  GtkCellRendererClass *renderer_class = NULL;
  GParamSpec *spec = NULL;

  g_type_class_add_private (klass, sizeof (GmCellRendererBitextPrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = gm_cell_renderer_bitext_finalize;
  gobject_class->get_property = gm_cell_renderer_bitext_get_property;
  gobject_class->set_property = gm_cell_renderer_bitext_set_property;

  spec = g_param_spec_string ("primary-text",
			      "Primary text",
			      "Primary text",
			      NULL, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
				   GM_CELL_RENDERER_BITEXT_PROP_PRIMARY_TEXT,
				   spec);

  spec = g_param_spec_string ("secondary-text",
			      "Secondary text",
			      "Secondary text",
			      NULL, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
				   GM_CELL_RENDERER_BITEXT_PROP_SECONDARY_TEXT,
				   spec);

  renderer_class = GTK_CELL_RENDERER_CLASS (klass);
  renderer_class->get_size = gm_cell_renderer_bitext_get_size;
  renderer_class->render = gm_cell_renderer_bitext_render;
}

/* public api */

GtkCellRenderer *
gm_cell_renderer_bitext_new ()
{
  return GTK_CELL_RENDERER (g_object_new (GM_TYPE_CELL_RENDERER_BITEXT, NULL));
}
