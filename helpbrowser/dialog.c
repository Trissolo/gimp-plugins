/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * The GIMP Help Browser
 * Copyright (C) 1999-2004 Sven Neumann <sven@gimp.org>
 *                         Michael Natterer <mitch@gimp.org>
 *
 * dialog.c
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtk/gtk.h>
#include <libgtkhtml/gtkhtml.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "libgimp/gimp.h"
#include "libgimp/gimpui.h"

#include "dialog.h"
#include "queue.h"
#include "uri.h"

#include "libgimp/stdplugins-intl.h"


enum
{
  BUTTON_HOME,
  BUTTON_INDEX,
  BUTTON_BACK,
  BUTTON_FORWARD
};

enum
{
  HISTORY_TITLE,
  HISTORY_REF
};

/*  local function prototypes  */

static void       button_callback  (GtkWidget        *widget,
                                    gpointer          data);
static void       update_toolbar   (void);
static void       combo_changed    (GtkWidget        *widget,
                                    gpointer          data);
static void       drag_begin       (GtkWidget        *widget,
                                    GdkDragContext   *context,
                                    gpointer          data);
static void       drag_data_get    (GtkWidget        *widget,
                                    GdkDragContext   *context,
                                    GtkSelectionData *selection_data,
                                    guint             info,
                                    guint             time,
                                    gpointer          data);

static void       title_changed    (HtmlDocument     *doc,
                                    const gchar      *new_title,
                                    gpointer          data);
static void       link_clicked     (HtmlDocument     *doc,
                                    const gchar      *url,
                                    gpointer          data);
static void       request_url      (HtmlDocument     *doc,
                                    const gchar      *url,
                                    HtmlStream       *stream,
                                    gpointer          data);
static gboolean   io_handler       (GIOChannel       *io,
                                    GIOCondition      condition,
                                    gpointer          data);
static void       load_remote_page (const gchar      *ref);

static void       history_add      (GtkComboBox      *combo,
                                    const gchar      *ref,
                                    const gchar      *title);

static gboolean   has_case_prefix  (const gchar      *haystack,
                                    const gchar      *needle);


/*  private variables  */

static const gchar  *eek_png_tag    = "<h1>Eeek!</h1>";

static Queue        *queue          = NULL;
static gchar        *current_ref    = NULL;

static GtkWidget    *back_button    = NULL;
static GtkWidget    *forward_button = NULL;
static GtkWidget    *html           = NULL;

static GtkTargetEntry help_dnd_target_table[] =
{
  { "_NETSCAPE_URL", 0, 0 },
};


/*  public functions  */

void
browser_dialog_open (void)
{
  GtkWidget       *window;
  GtkWidget       *vbox;
  GtkWidget       *hbox;
  GtkWidget       *bbox;
  GtkWidget       *scroll;
  GtkWidget       *button;
  GtkWidget       *drag_source;
  GtkWidget       *image;
  GtkWidget       *combo;
  GtkListStore    *history;
  GtkCellRenderer *cell;
  gchar           *eek_png_path;

  gimp_ui_init ("helpbrowser", TRUE);

  eek_png_path = g_build_filename (gimp_data_directory (),
                                   "themes", "Default", "images",
                                   "stock-wilber-eek-64.png", NULL);

  if (g_file_test (eek_png_path, G_FILE_TEST_EXISTS))
    eek_png_tag = g_strdup_printf ("<img src=\"%s\">", eek_png_path);

  g_free (eek_png_path);

  /*  the dialog window  */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("GIMP Help Browser"));
  gtk_window_set_role (GTK_WINDOW (window), "helpbrowser");

  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_main_quit),
                    NULL);

  gimp_help_connect (window, gimp_standard_help_func,
                     "gimp-help", NULL);

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  /*  buttons  */
  bbox = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
  gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
  gtk_widget_show (bbox);

  button = gtk_button_new_from_stock (GTK_STOCK_HOME);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_callback),
                    GINT_TO_POINTER (BUTTON_HOME));

#if 0
  button = gtk_button_new_from_stock (GTK_STOCK_INDEX);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_callback),
                    GINT_TO_POINTER (BUTTON_INDEX));
#endif

  back_button = button = gtk_button_new_from_stock (GTK_STOCK_GO_BACK);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_callback),
                    GINT_TO_POINTER (BUTTON_BACK));
  gtk_widget_show (button);

  forward_button = button = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (button_callback),
                    GINT_TO_POINTER (BUTTON_FORWARD));
  gtk_widget_show (button);

  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  the drag source  */
  drag_source = gtk_event_box_new ();
  gtk_box_pack_start (GTK_BOX (hbox), drag_source, FALSE, FALSE, 4);
  gtk_widget_show (drag_source);

  gtk_drag_source_set (GTK_WIDGET (drag_source),
                       GDK_BUTTON1_MASK,
                       help_dnd_target_table,
                       G_N_ELEMENTS (help_dnd_target_table),
                       GDK_ACTION_MOVE | GDK_ACTION_COPY);

  g_signal_connect (drag_source, "drag_begin",
                    G_CALLBACK (drag_begin),
                    NULL);
  g_signal_connect (drag_source, "drag_data_get",
                    G_CALLBACK (drag_data_get),
                    NULL);

  image = gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_BUTTON);
  gtk_container_add (GTK_CONTAINER (drag_source), image);
  gtk_widget_show (image);

  /*  the title combo  */
  history = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (history));
  g_object_unref (history);

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
                                  "text", HISTORY_TITLE,
                                  NULL);

  gtk_widget_set_size_request (GTK_WIDGET (combo), 320, -1);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  g_signal_connect (combo, "changed",
                    G_CALLBACK (combo_changed),
                    NULL);

  /*  HTML view  */
  html  = html_view_new ();
  queue = queue_new ();

  gtk_widget_set_size_request (GTK_WIDGET (html), -1, 240);

  scroll =
    gtk_scrolled_window_new (gtk_layout_get_hadjustment (GTK_LAYOUT (html)),
                             gtk_layout_get_vadjustment (GTK_LAYOUT (html)));

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
  gtk_widget_show (scroll);

  gtk_container_add (GTK_CONTAINER (scroll), html);
  gtk_widget_show (html);

  html_view_set_document (HTML_VIEW (html), html_document_new ());

  g_signal_connect (HTML_VIEW (html)->document, "title_changed",
                    G_CALLBACK (title_changed),
                    combo);
  g_signal_connect (HTML_VIEW (html)->document, "link_clicked",
                    G_CALLBACK (link_clicked),
                    NULL);
  g_signal_connect (HTML_VIEW (html)->document, "request_url",
                    G_CALLBACK (request_url),
                    NULL);

  gtk_widget_show (window);
}

void
browser_dialog_load (const gchar *ref,
                     gboolean     add_to_queue)
{
  HtmlDocument *doc = HTML_VIEW (html)->document;
  gchar        *abs;
  gchar        *new_ref;
  gchar        *anchor;
  gchar        *tmp;

  g_return_if_fail (ref != NULL);

  if (! current_ref)
    {
      gchar *slash;

      current_ref = g_strdup (ref);

      slash = strrchr (current_ref, '/');

      if (slash)
        *slash = '\0';
    }

  abs = uri_to_abs (ref, current_ref);

  g_return_if_fail (abs != NULL);

  anchor = strchr (ref, '#');
  if (anchor && anchor[0] && anchor[1])
    {
      new_ref = g_strconcat (abs, anchor, NULL);
      anchor += 1;
    }
  else
    {
      new_ref = g_strdup (abs);
      anchor = NULL;
    }

  if (! has_case_prefix (abs, "file:/"))
    {
      load_remote_page (ref);
      return;
    }

  tmp = uri_to_abs (current_ref, current_ref);
  if (!tmp || strcmp (tmp, abs))
    {
      html_document_clear (doc);
      html_document_open_stream (doc, "text/html");
      gtk_adjustment_set_value (gtk_layout_get_vadjustment (GTK_LAYOUT (html)),
                                0);

      request_url (doc, abs, doc->current_stream, NULL);
    }
  g_free (tmp);

  if (anchor)
    html_view_jump_to_anchor (HTML_VIEW (html), anchor);
  else
    gtk_adjustment_set_value (GTK_LAYOUT (html)->vadjustment, 0.0);

  g_free (current_ref);
  current_ref = new_ref;

  if (add_to_queue)
    queue_add (queue, new_ref);

  update_toolbar ();

  gtk_window_present (GTK_WINDOW (gtk_widget_get_toplevel (html)));
}


/*  private functions  */

static void
button_callback (GtkWidget *widget,
                 gpointer   data)
{
  const gchar *ref;

  switch (GPOINTER_TO_INT (data))
    {
    case BUTTON_HOME:
    case BUTTON_INDEX:
      browser_dialog_load ("index.html", TRUE);
      break;

    case BUTTON_BACK:
      if (!(ref = queue_prev (queue)))
        return;
      browser_dialog_load (ref, FALSE);
      queue_move_prev (queue);
      break;

    case BUTTON_FORWARD:
      if (!(ref = queue_next (queue)))
        return;
      browser_dialog_load (ref, FALSE);
      queue_move_next (queue);
      break;

    default:
      return;
    }

  update_toolbar ();
}

static void
update_toolbar (void)
{
  if (back_button)
    gtk_widget_set_sensitive (back_button, queue_has_prev (queue));
  if (forward_button)
    gtk_widget_set_sensitive (forward_button, queue_has_next (queue));
}

static void
combo_changed (GtkWidget *widget,
               gpointer   data)
{
  GtkComboBox *combo = GTK_COMBO_BOX (widget);
  GtkTreeIter  iter;

  if (gtk_combo_box_get_active_iter (combo, &iter))
    {
      GValue  value = { 0, };

      gtk_tree_model_get_value (gtk_combo_box_get_model (combo),
                                &iter, HISTORY_REF, &value);

      browser_dialog_load (g_value_get_string (&value), TRUE);

      g_value_unset (&value);
    }
}

static void
drag_begin (GtkWidget      *widget,
            GdkDragContext *context,
            gpointer        data)
{
  gtk_drag_set_icon_stock (context, GTK_STOCK_JUMP_TO, -8, -8);
}

static void
drag_data_get (GtkWidget        *widget,
               GdkDragContext   *context,
               GtkSelectionData *selection_data,
               guint             info,
               guint             time,
               gpointer          data)
{
  if (! current_ref)
    return;

  gtk_selection_data_set (selection_data,
                          selection_data->target,
                          8,
                          current_ref,
                          strlen (current_ref));
}

static void
title_changed (HtmlDocument *doc,
               const gchar  *new_title,
               gpointer      data)
{
  if (new_title)
    {
      gchar *title = g_strstrip (g_strdup (new_title));

      history_add (GTK_COMBO_BOX (data), current_ref, title);

      g_free (title);
    }
  else
    {
      history_add (GTK_COMBO_BOX (data), current_ref, _("Untitled"));
    }
}

static void
link_clicked (HtmlDocument *doc,
              const gchar  *url,
              gpointer      data)
{
  browser_dialog_load (url, TRUE);
}

static void
request_url (HtmlDocument *doc,
             const gchar  *url,
             HtmlStream   *stream,
             gpointer      data)
{
  gchar *abs;
  gchar *filename;

  g_return_if_fail (url != NULL);
  g_return_if_fail (stream != NULL);

  abs = uri_to_abs (url, current_ref);
  if (! abs)
    return;

  filename = g_filename_from_uri (abs, NULL, NULL);

  if (filename)
    {
      gint fd;

      fd = open (filename, O_RDONLY);

      if (fd == -1)
        {
          gchar *name;
          gchar *msg;

          name = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

          msg = g_strdup_printf
            ("<html>"
             "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />"
             "<head><title>%s</title></head>"
             "<body bgcolor=\"white\">"
             "<div align=\"center\">"
             "<div>%s</div>"
             "<h3>%s</h3>"
             "<tt>%s</tt>"
             "</div>"
             "<br /><br />"
             "<div align=\"justify\">%s</div>"
             "</body>"
             "</html>",
             _("Document Not Found"),
             eek_png_tag,
             _("Could not locate help document"),
             name,
             _("The requested document could not be found in your GIMP help "
               "path as shown above. This means that the topic has not yet "
               "been written or your installation is not complete. Ensure "
               "that your installation is complete before reporting this "
               "error as a bug."));

          html_document_write_stream (doc, msg, strlen (msg));

          g_free (msg);
          g_free (name);
        }
      else
        {
          GIOChannel *io = g_io_channel_unix_new (fd);

          g_io_channel_set_close_on_unref (io, TRUE);
          g_io_channel_set_encoding (io, NULL, NULL);

          g_io_add_watch (io, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
                          io_handler, stream);
        }

      g_free (filename);
    }

  g_free (abs);
}

static gboolean
io_handler (GIOChannel   *io,
            GIOCondition  condition,
            gpointer      data)
{
  HtmlStream *stream;
  gchar       buffer[8192];
  gsize       bytes;

  stream = (HtmlStream *) data;

  if (condition & G_IO_IN)
    {
      if (g_io_channel_read_chars (io, buffer, sizeof (buffer),
                                   &bytes, NULL) != G_IO_STATUS_ERROR
          && bytes > 0)
        {
          html_stream_write (stream, buffer, bytes);
        }
      else
	{
          html_stream_close (stream);
          g_io_channel_unref (io);

	  return FALSE;
	}

      if (condition & G_IO_HUP)
        {
          while (g_io_channel_read_chars (io, buffer, sizeof (buffer),
                                          &bytes, NULL) != G_IO_STATUS_ERROR
                 && bytes > 0)
            {
              html_stream_write (stream, buffer, bytes);
            }
        }
    }

  if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
    {
      html_stream_close (stream);
      g_io_channel_unref (io);

      return FALSE;
    }

  return TRUE;
}

static void
load_remote_page (const gchar *ref)
{
  GimpParam *return_vals;
  gint       nreturn_vals;

  /*  try to call the user specified web browser */
  return_vals = gimp_run_procedure ("plug_in_web_browser",
                                    &nreturn_vals,
                                    GIMP_PDB_STRING, ref,
                                    GIMP_PDB_END);
  gimp_destroy_params (return_vals, nreturn_vals);
}

static void
history_add (GtkComboBox *combo,
             const gchar *ref,
	     const gchar *title)
{
  GtkTreeModel *model = gtk_combo_box_get_model (combo);
  GtkTreeIter   iter;
  gboolean      iter_valid;
  GValue        value = { 0, };

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter))
    {
      gtk_tree_model_get_value (model, &iter, HISTORY_REF, &value);

      if (strcmp (g_value_get_string (&value), ref) == 0)
        {
          gtk_list_store_move_after (GTK_LIST_STORE (model), &iter, NULL);
          g_value_unset (&value);
          break;
        }

      g_value_unset (&value);
    }

  if (! iter_valid)
    {
      gtk_list_store_prepend (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          HISTORY_TITLE, title,
                          HISTORY_REF,   ref,
                          -1);
    }

  g_signal_handlers_block_by_func (combo, combo_changed, NULL);
  gtk_combo_box_set_active_iter (combo, &iter);
  g_signal_handlers_unblock_by_func (combo, combo_changed, NULL);
}

/* Taken from glib/gconvert.c:
 * Test of haystack has the needle prefix, comparing case
 * insensitive. haystack may be UTF-8, but needle must
 * contain only ascii.
 */
static gboolean
has_case_prefix (const gchar *haystack, const gchar *needle)
{
  const gchar *h = haystack;
  const gchar *n = needle;

  while (*n && *h && g_ascii_tolower (*n) == g_ascii_tolower (*h))
    {
      n++;
      h++;
    }

  return (*n == '\0');
}
