/*
 * "$Id$"
 *
 *   Despeckle (adaptive median) filter for The GIMP -- an image manipulation
 *   program
 *
 *   Copyright 1997-1998 Michael Sweet (mike@easysw.com)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Contents:
 *
 *   main()                      - Main entry - just call gimp_main()...
 *   query()                     - Respond to a plug-in query...
 *   run()                       - Run the filter...
 *   despeckle()                 - Despeckle an image using a median filter.
 *   despeckle_dialog()          -  Popup a dialog window for the filter box size...
 *   preview_init()              - Initialize the preview window...
 *   preview_scroll_callback()   - Update the preview when a scrollbar is moved.
 *   preview_update()            - Update the preview window.
 *   preview_exit()              - Free all memory used by the preview window...
 *   dialog_iscale_update()      - Update the value field using the scale.
 *   dialog_adaptive_callback()  - Update the filter type...
 *   dialog_recursive_callback() - Update the filter type...
 *   dialog_ok_callback()        - Start the filter...
 *
 * Revision History:
 *
 *   $Log$
 *   Revision 1.26  2000/01/25 23:05:57  mitch
 *   2000-01-25  Michael Natterer  <mitch@gimp.org>
 *
 *   	* app/appenv.h: removed BOUNDS, MINIMUM and MAXIMUM. No need to
 *   	include both <glib.h> and <gtk/gtk.h>.
 *
 *   	* app/*
 *   	* tools/pdbgen/pdb/text_tool.pdb: s/BOUNDS/CLAMP/,
 *   	same for MIN and MAX.
 *
 *   	* app/preferences_dialog.c: the "Check Size" widget was connected
 *   	to the transparency_type variable.
 *
 *   	* plug-ins/common/sobel.c: removed definitions of MIN and ROUND.
 *
 *   	* libgimp/gimp.h: #include "gimplimits.h" and "gimpcolorspace.h".
 *
 *   	* plug-ins/*: don't include the two files.
 *
 *   Revision 1.25  2000/01/15 15:32:28  mitch
 *   2000-01-15  Michael Natterer  <mitch@gimp.org>
 *
 *   	* libgimp/gimpwidgets.[ch]: added a float adjustment callback.
 *
 *   	* plug-ins/common/bumpmap.c
 *   	* plug-ins/common/depthmerge.c
 *   	* plug-ins/common/despeckle.c
 *   	* plug-ins/common/destripe.c
 *   	* plug-ins/common/gpb.c
 *   	* plug-ins/common/iwarp.c
 *   	* plug-ins/common/polar.c
 *   	* plug-ins/common/sharpen.c
 *   	* plug-ins/common/tileit.c
 *   	* plug-ins/common/whirlpinch.c
 *   	* plug-ins/common/wind.c:
 *
 *   	- Some more hscale+spinbutton instead of hscale+entry widgets.
 *   	- Get the CHECK_SIZE constants from libgimp in some plugins.
 *
 *   Revision 1.24  2000/01/14 12:40:59  mitch
 *   2000-01-14  Michael Natterer  <mitch@gimp.org>
 *
 *   	* app/Makefile.am
 *   	* app/tool_options_ui.h: removed.
 *
 *   	* app/tool_options.c
 *   	* libgimp/gimpwidgets.[ch]: moved some more ui utility functions
 *   	to libgimp.
 *
 *   	* app/airbrush.c
 *   	* app/blend.c
 *   	* app/bucket_fill.c
 *   	* app/channel_ops.c
 *   	* app/clone.c
 *   	* app/color_picker.c
 *   	* app/convolve.c
 *   	* app/crop.c
 *   	* app/dodgeburn.c
 *   	* app/eraser.c
 *   	* app/file_new_dialog.[ch]
 *   	* app/flip_tool.c
 *   	* app/image_new.[ch]
 *   	* app/ink.c
 *   	* app/layers_dialog.c
 *   	* app/magnify.c
 *   	* app/measure.c
 *   	* app/paintbrush.c
 *   	* app/pencil.c
 *   	* app/smudge.c
 *   	* app/text_tool.c
 *   	* app/tool_options.c
 *   	* app/transform_tool.c
 *   	* app/xinput_airbrush.c: use the libgimp functions (esp. the radio
 *   	button group constructor), some code cleanup.
 *
 *   	* plug-ins/common/csource.c
 *   	* plug-ins/common/despeckle.c
 *   	* plug-ins/common/diffraction.c
 *   	* plug-ins/common/jpeg.c
 *   	* plug-ins/common/png.c
 *   	* plug-ins/unsharp/unsharp.c: more plugin ui tuning.
 *
 *   	* plug-ins/unsharp/Makefile.am
 *   	* plug-ins/unsharp/dialog_f.[ch]
 *   	* plug-ins/unsharp/dialog_i.[ch]: removed.
 *
 *   Revision 1.23  2000/01/13 15:39:24  mitch
 *   2000-01-13  Michael Natterer  <mitch@gimp.org>
 *
 *   	* app/gimpui.[ch]
 *   	* app/preferences_dialog.c: removed & renamed some functions from
 *   	gimpui.[ch] (see below).
 *
 *   	* libgimp/Makefile.am
 *   	* libgimp/gimpwidgets.[ch]; new files. Functions moved from
 *   	app/gimpui.[ch]. Added a constructor for the label + hscale +
 *   	entry combination used in many plugins (now hscale + spinbutton).
 *
 *   	* libgimp/gimpui.h: include gimpwidgets.h
 *
 *   	* plug-ins/megawidget/megawidget.[ch]: removed all functions
 *   	except the preview stuff (I'm not yet sure how to implement this
 *   	in libgimp because the libgimp preview should be general enough to
 *   	replace all the other plugin previews, too).
 *
 *   	* plug-ins/borderaverage/Makefile.am
 *   	* plug-ins/borderaverage/borderaverage.c
 *   	* plug-ins/common/plugin-defs.pl
 *   	* plug-ins/common/Makefile.am
 *   	* plug-ins/common/aa.c
 *   	* plug-ins/common/align_layers.c
 *   	* plug-ins/common/animationplay.c
 *   	* plug-ins/common/apply_lens.c
 *   	* plug-ins/common/blinds.c
 *   	* plug-ins/common/bumpmap.c
 *   	* plug-ins/common/checkerboard.c
 *   	* plug-ins/common/colorify.c
 *   	* plug-ins/common/convmatrix.c
 *   	* plug-ins/common/cubism.c
 *   	* plug-ins/common/curve_bend.c
 *   	* plug-ins/common/deinterlace.c
 *   	* plug-ins/common/despeckle.c
 *   	* plug-ins/common/destripe.c
 *   	* plug-ins/common/displace.c
 *   	* plug-ins/common/edge.c
 *   	* plug-ins/common/emboss.c
 *   	* plug-ins/common/hot.c
 *   	* plug-ins/common/nlfilt.c
 *   	* plug-ins/common/pixelize.c
 *   	* plug-ins/common/waves.c
 *   	* plug-ins/sgi/sgi.c
 *   	* plug-ins/sinus/sinus.c: ui updates like removing megawidget,
 *   	using the dialog constructor, I18N fixes, indentation, ...
 *
 *   Revision 1.22  2000/01/06 16:40:17  mitch
 *   2000-01-06  Michael Natterer  <mitch@gimp.org>
 *
 *   	* app/[all files using the dialog or action area constructors]
 *   	* libgimp/gimpdialog.[ch]: added a "slot_object" agrument to the
 *   	constructors' va_args lists to allow the action area buttons to be
 *   	connected wich gtk_signal_connect_object().
 *
 *   	* libgimp/gimphelp.c: show the correct help page for plugins.
 *
 *   	* plug-ins/common/CEL.c
 *   	* plug-ins/common/CML_explorer.c
 *   	* plug-ins/common/Makefile.am
 *   	* plug-ins/common/aa.c
 *   	* plug-ins/common/align_layers.c
 *   	* plug-ins/common/animationplay.c
 *   	* plug-ins/common/apply_lens.c
 *   	* plug-ins/common/blinds.c
 *   	* plug-ins/common/blur.c
 *   	* plug-ins/common/bumpmap.c
 *   	* plug-ins/common/checkerboard.c
 *   	* plug-ins/common/colorify.c
 *   	* plug-ins/common/colortoalpha.c
 *   	* plug-ins/common/compose.c
 *   	* plug-ins/common/convmatrix.c
 *   	* plug-ins/common/csource.c
 *   	* plug-ins/common/cubism.c
 *   	* plug-ins/common/curve_bend.c
 *   	* plug-ins/common/decompose.c
 *   	* plug-ins/common/deinterlace.c
 *   	* plug-ins/common/depthmerge.c
 *   	* plug-ins/common/despeckle.c
 *   	* plug-ins/common/destripe.c
 *   	* plug-ins/common/diffraction.c
 *   	* plug-ins/common/displace.c
 *   	* plug-ins/common/grid.c
 *   	* plug-ins/helpbrowser/Makefile.am
 *   	* plug-ins/helpbrowser/helpbrowser.c: use the dialog constructor
 *   	and enable the "F1" help key.
 *
 *   Revision 1.21  1999/12/29 18:07:43  neo
 *   NEVER EVER use sprintf together with _(...) !
 *
 *
 *   --Sven
 *
 *   Revision 1.20  1999/12/27 18:43:09  neo
 *   small dialog changes and german translation update
 *
 *
 *   --Sven
 *
 *   Revision 1.19  1997/01/03 15:15:10  yasuhiro
 *   1999-12-20  Shirasaki Yasuhiro  <yasuhiro@gnome.gr.jp>
 *
 *           * plug-ins/common/blinds.c
 *           * plug-ins/common/curve_bend.c
 *           * plug-ins/common/deinterlace.c
 *           * plug-ins/common/despeckle.c
 *           * po-plug-ins/POTFILES.in: Added gettext support
 *
 *   -- yasuhiro
 *
 *   Revision 1.18  1999/11/23 23:49:42  neo
 *   added dots to all menu entries of interactive plug-ins and did the usual
 *   action area fixes on lots of them
 *
 *
 *
 *   --Sven
 *
 *   Revision 1.17  1999/10/24 20:48:58  pcg
 *   api change #2, fix #1
 *
 *   Revision 1.16  1999/10/17 00:07:38  pcg
 *   API PATCH #2 or so
 *
 *   Revision 1.15  1999/04/23 06:35:14  asbjoer
 *   use MAIN macro
 *
 *   Revision 1.14  1999/01/15 17:33:24  unammx
 *   1999-01-15  Federico Mena Quintero  <federico@nuclecu.unam.mx>
 *
 *   	* Updated gtk_toggle_button_set_state() to
 *   	gtk_toggle_button_set_active() in all the files.
 *
 *   Revision 1.13  1998/06/06 23:22:16  yosh
 *   * adding Lighting plugin
 *
 *   * updated despeckle, png, sgi, and sharpen
 *
 *   -Yosh
 *
 *   Revision 1.21  1998/05/17 15:57:33  mike
 *   Removed extra variables.
 *   Removed signal handlers (used for debugging)
 *
 *   Revision 1.20  1998/04/27  15:59:17  mike
 *   Fixed RGB preview problem...
 *
 *   Revision 1.19  1998/04/27  15:45:27  mike
 *   OK, put the shadow buffer stuff back in - without shadowing the undo stuff
 *   will *not* work...  sigh...
 *   Doubled tile cache to avoid cache thrashing with shadow buffer.
 *
 *   Revision 1.18  1998/04/27  15:39:48  mike
 *   Fixed destination region code - was using a shadow buffer when it wasn't
 *   needed.
 *   Now add 1 to the number of tiles needed in the cache to avoid possible
 *   rounding error and resulting cache thrashing.
 *
 *   Revision 1.17  1998/04/23  14:39:47  mike
 *   Updated preview code to handle images with alpha (preview now shows checker
 *   pattern).
 *   Added call to gtk_window_set_wmclass() to make sure the GIMP icon is used
 *   by default.
 *
 *   Revision 1.16  1998/01/22  14:35:03  mike
 *   Added black & white level controls.
 *   Fixed bug in despeckle code that caused the borders to darken.
 *
 *   Revision 1.15  1998/01/21  21:33:47  mike
 *   Fixed malloc buffer overflow bug - wasn't realloc'ing buffers
 *   when the filter radius changed.
 *
 *   Revision 1.14  1997/11/14  17:17:59  mike
 *   Updated to dynamically allocate return params in the run() function.
 *
 *   Revision 1.13  1997/11/12  15:53:34  mike
 *   Added <string.h> header file for Digital UNIX...
 *
 *   Revision 1.12  1997/10/17  13:56:54  mike
 *   Updated author/contact information.
 *
 *   Revision 1.11  1997/06/12  16:58:11  mike
 *   Optimized final despeckle - now grab gimp_tile_height() rows at a time
 *   for faster filtering.
 *
 *   Revision 1.10  1997/06/08  23:30:29  mike
 *   Improved the preview update speed significantly by loading the entire
 *   source (preview) image first.
 *
 *   Revision 1.9  1997/06/08  16:48:21  mike
 *   Renamed "adaptive" argument to "type" (filter type).
 *
 *   Revision 1.8  1997/06/08  12:45:09  mike
 *   Added recursive filter option.
 *   Cleaned up UI.
 *
 *   Revision 1.7  1997/06/08  04:27:19  mike
 *   Updated documentation.
 *   Moved plug-in back to original location in menu tree.
 *
 *   Revision 1.6  1997/06/08  04:24:56  mike
 *   Added filter type argument & control.
 *
 *   Revision 1.5  1997/06/08  04:12:36  mike
 *   Added preview window.
 *
 *   Revision 1.4  1997/06/08  02:18:22  mike
 *   Updated to adjust the despeckling radius based upon the window's
 *   histogram.  This improves filter quality significantly as surface
 *   details are preserved and not blurred...
 *
 *   Revision 1.3  1997/06/07  01:29:47  mike
 *   Added some minor optimizations.
 *   Updated version to 1.01.
 *   Fixed minor bug in dialog_ientry_update() - was using gdouble instead
 *   of gint for new_value...
 *
 *   Revision 1.2  1997/06/07  01:03:07  mike
 *   Updated docos, changed maximum radius to 20.
 *
 *   Revision 1.1  1997/06/07  00:01:15  mike
 *   Initial Revision.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "libgimp/stdplugins-intl.h"

/*
 * Constants...
 */

#define PLUG_IN_NAME		"plug_in_despeckle"
#define PLUG_IN_VERSION		"1.3.2 - 17 May 1998"
#define PREVIEW_SIZE		128
#define SCALE_WIDTH		80
#define ENTRY_WIDTH		40
#define MAX_RADIUS		20

#define FILTER_ADAPTIVE		0x01
#define FILTER_RECURSIVE	0x02

#define despeckle_radius	(despeckle_vals[0])	/* Radius of filter */
#define filter_type		(despeckle_vals[1])	/* Type of filter */
#define black_level		(despeckle_vals[2])	/* Black level */
#define white_level		(despeckle_vals[3])	/* White level */

/*
 * Local functions...
 */

static void	query (void);
static void	run   (gchar   *name,
		       gint     nparams,
		       GParam  *param,
		       gint    *nreturn_vals,
		       GParam **return_vals);

static void	despeckle (void);

static gint	despeckle_dialog          (void);

static void	dialog_iscale_update      (GtkAdjustment *, gint *);
static void	dialog_adaptive_callback  (GtkWidget *, gpointer);
static void	dialog_recursive_callback (GtkWidget *, gpointer);
static void	dialog_ok_callback        (GtkWidget *, gpointer);

static void	preview_init              (void);
static void	preview_exit              (void);
static void	preview_update            (void);
static void	preview_scroll_callback   (void);


/*
 * Globals...
 */

GPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init  */
  NULL,  /* quit  */
  query, /* query */
  run    /* run   */
};

GtkWidget      *preview;		/* Preview widget */
gint		preview_width,		/* Width of preview widget */
		preview_height,		/* Height of preview widget */
		preview_x1,		/* Upper-left X of preview */
		preview_y1,		/* Upper-left Y of preview */
		preview_x2,		/* Lower-right X of preview */
		preview_y2;		/* Lower-right Y of preview */
guchar	       *preview_src = NULL,	/* Source pixel rows */
	       *preview_dst,		/* Destination pixel row */
	       *preview_sort;		/* Pixel value sort array */
GtkObject      *hscroll_data,		/* Horizontal scrollbar data */
	       *vscroll_data;		/* Vertical scrollbar data */

GDrawable      *drawable = NULL;	/* Current image */
gint		sel_x1,			/* Selection bounds */
		sel_y1,
		sel_x2,
		sel_y2;
gint		sel_width,		/* Selection width */
		sel_height;		/* Selection height */
gint		img_bpp;		/* Bytes-per-pixel in image */

gint run_filter = FALSE;	/* True if we should run the filter */

gint despeckle_vals[4] =
{
  3,
  FILTER_ADAPTIVE,
  7,
  248
};


/*
 * 'main()' - Main entry - just call gimp_main()...
 */

MAIN ()

/*
 * 'query()' - Respond to a plug-in query...
 */

static void
query (void)
{
  static GParamDef	args[] =
  {
    { PARAM_INT32,	"run_mode",	"Interactive, non-interactive" },
    { PARAM_IMAGE,	"image",	"Input image" },
    { PARAM_DRAWABLE,	"drawable",	"Input drawable" },
    { PARAM_INT32,	"radius",	"Filter box radius (default = 3)" },
    { PARAM_INT32,	"type",		"Filter type (0 = median, 1 = adaptive, 2 = recursive-median, 3 = recursive-adaptive)" },
    { PARAM_INT32,	"black",	"Black level (0 to 255)" },
    { PARAM_INT32,	"white",	"White level (0 to 255)" }
  };
  static GParamDef	*return_vals = NULL;
  static int		nargs        = sizeof(args) / sizeof(args[0]),
			nreturn_vals = 0;

  INIT_I18N();

  gimp_install_procedure (PLUG_IN_NAME,
			  _("Despeckle filter, typically used to \'despeckle\' a photographic image."),
			  _("This plug-in selectively performs a median or adaptive box filter on an image."),
			  "Michael Sweet <mike@easysw.com>",
			  "Copyright 1997-1998 by Michael Sweet",
			  PLUG_IN_VERSION,
			  N_("<Image>/Filters/Enhance/Despeckle..."),
			  "RGB*, GRAY*",
			  PROC_PLUG_IN,
			  nargs,
			  nreturn_vals,
			  args,
			  return_vals);
}


/*
 * 'run()' - Run the filter...
 */

static void
run (gchar   *name,		/* I - Name of filter program. */
     gint     nparams,		/* I - Number of parameters passed in */
     GParam  *param,		/* I - Parameter values */
     gint    *nreturn_vals,	/* O - Number of return values */
     GParam **return_vals)	/* O - Return values */
{
  GRunModeType	run_mode;	/* Current run mode */
  GStatusType	status;		/* Return status */
  GParam	*values;	/* Return values */

  /*
   * Initialize parameter data...
   */

  status   = STATUS_SUCCESS;
  run_mode = param[0].data.d_int32;

  values = g_new (GParam, 1);

  values[0].type          = PARAM_STATUS;
  values[0].data.d_status = status;

  *nreturn_vals = 1;
  *return_vals  = values;

  /*
   * Get drawable information...
   */

  drawable = gimp_drawable_get (param[2].data.d_drawable);

  gimp_drawable_mask_bounds (drawable->id, &sel_x1, &sel_y1, &sel_x2, &sel_y2);

  sel_width  = sel_x2 - sel_x1;
  sel_height = sel_y2 - sel_y1;
  img_bpp    = gimp_drawable_bpp (drawable->id);

  /*
   * See how we will run
   */

  switch (run_mode)
    {
    case RUN_INTERACTIVE :
      INIT_I18N_UI();
      /*
       * Possibly retrieve data...
       */

      gimp_get_data (PLUG_IN_NAME, &despeckle_radius);

      /*
       * Get information from the dialog...
       */

      if (!despeckle_dialog())
	return;
      break;

    case RUN_NONINTERACTIVE:
      /*
       * Make sure all the arguments are present...
       */

      INIT_I18N();
      if (nparams < 4 || nparams > 7)
	status = STATUS_CALLING_ERROR;
      else if (nparams == 4)
	{
	  despeckle_radius = param[3].data.d_int32;
	  filter_type      = FILTER_ADAPTIVE;
	  black_level      = 7;
	  white_level      = 248;
	}
      else if (nparams == 5)
	{
	  despeckle_radius = param[3].data.d_int32;
	  filter_type      = param[4].data.d_int32;
	  black_level      = 7;
	  white_level      = 248;
	}
      else if (nparams == 6)
	{
	  despeckle_radius = param[3].data.d_int32;
	  filter_type      = param[4].data.d_int32;
	  black_level      = param[5].data.d_int32;
	  white_level      = 248;
	}
      else
	{
	  despeckle_radius = param[3].data.d_int32;
	  filter_type      = param[4].data.d_int32;
	  black_level      = param[5].data.d_int32;
	  white_level      = param[6].data.d_int32;
	};
      break;

    case RUN_WITH_LAST_VALS:
      /*
       * Possibly retrieve data...
       */

      INIT_I18N();
      gimp_get_data (PLUG_IN_NAME, despeckle_vals);
	break;
	
    default:
      status = STATUS_CALLING_ERROR;
      break;;
    };

  /*
   * Despeckle the image...
   */

  if (status == STATUS_SUCCESS)
    {
      if ((gimp_drawable_is_rgb(drawable->id) ||
	   gimp_drawable_is_gray(drawable->id)))
	{
	  /*
	   * Set the tile cache size...
	   */

	  gimp_tile_cache_ntiles (2 * (drawable->width + gimp_tile_width() - 1) /
				  gimp_tile_width() + 1);

	  /*
	   * Run!
	   */

	  despeckle ();

	  /*
	   * If run mode is interactive, flush displays...
	   */

	  if (run_mode != RUN_NONINTERACTIVE)
	    gimp_displays_flush ();

	  /*
	   * Store data...
	   */

	  if (run_mode == RUN_INTERACTIVE)
	    gimp_set_data (PLUG_IN_NAME,
			   despeckle_vals, sizeof (despeckle_vals));
	}
      else
	status = STATUS_EXECUTION_ERROR;
    };

  /*
   * Reset the current run status...
   */

  values[0].data.d_status = status;

  /*
   * Detach from the drawable...
   */

  gimp_drawable_detach (drawable);
}


/*
 * 'despeckle()' - Despeckle an image using a median filter.
 *
 * A median filter basically collects pixel values in a region around the
 * target pixel, sorts them, and uses the median value. This code uses a
 * circular row buffer to improve performance.
 *
 * The adaptive filter is based on the median filter but analizes the histogram
 * of the region around the target pixel and adjusts the despeckle radius
 * accordingly.
 */

static void
despeckle (void)
{
  GPixelRgn	src_rgn,	/* Source image region */
		dst_rgn;	/* Destination image region */
  guchar      **src_rows,	/* Source pixel rows */
	       *dst_row,	/* Destination pixel row */
	       *src_ptr,	/* Source pixel pointer */
	       *sort,		/* Pixel value sort array */
	       *sort_ptr;	/* Current sort value */
  gint		sort_count,	/* Number of soft values */
		i, j, t, d,	/* Looping vars */
		x, y,		/* Current location in image */
		row,		/* Current row in src_rows */
		rowcount,	/* Number of rows loaded */
		lasty,		/* Last row loaded in src_rows */
		trow,		/* Looping var */
		startrow,	/* Starting row for loop */
		endrow,		/* Ending row for loop */
		max_row,	/* Maximum number of filled src_rows */
		size,		/* Width/height of the filter box */
		width,		/* Byte width of the image */
		xmin, xmax, tx,	/* Looping vars */
		radius,		/* Current radius */
		hist0,		/* Histogram count for 0 values */
		hist255;	/* Histogram count for 255 values */

  /*
   * Let the user know what we're doing...
   */

  gimp_progress_init (_("Despeckling..."));

  /*
   * Setup for filter...
   */

  gimp_pixel_rgn_init (&src_rgn, drawable, sel_x1, sel_y1, sel_width, sel_height,
		       FALSE, FALSE);
  gimp_pixel_rgn_init (&dst_rgn, drawable, sel_x1, sel_y1, sel_width, sel_height,
		       TRUE, TRUE);

  size     = despeckle_radius * 2 + 1;
  max_row  = 2 * gimp_tile_height ();
  width    = sel_width * img_bpp;

  src_rows    = g_new (guchar *, max_row);
  src_rows[0] = g_new (guchar, max_row * width);

  for (row = 1; row < max_row; row ++)
    src_rows[row] = src_rows[0] + row * width;

  dst_row = g_new (guchar, width),
  sort    = g_new (guchar, size * size);

  /*
   * Pre-load the first "size" rows for the filter...
   */

  if (sel_height < gimp_tile_height())
    rowcount = sel_height;
  else
    rowcount = gimp_tile_height ();

  gimp_pixel_rgn_get_rect (&src_rgn, src_rows[0], sel_x1, sel_y1, sel_width,
			   rowcount);

  row   = rowcount;
  lasty = sel_y1 + rowcount;

  /*
   * Despeckle...
   */

  for (y = sel_y1 ; y < sel_y2; y ++)
    {
      if ((y + despeckle_radius) >= lasty &&
	  lasty < sel_y2)
	{
	  /*
	   * Load the next block of rows...
	   */

	  rowcount -= gimp_tile_height();
	  if ((i = sel_y2 - lasty) > gimp_tile_height())
	    i = gimp_tile_height();

	  gimp_pixel_rgn_get_rect (&src_rgn, src_rows[row],
				   sel_x1, lasty, sel_width, i);

	  rowcount += i;
	  lasty    += i;
	  row      = (row + i) % max_row;
	};

      /*
       * Now find the median pixels and save the results...
       */

      radius = despeckle_radius;

      memcpy (dst_row, src_rows[(row + y - lasty + max_row) % max_row], width);

      if (y >= (sel_y1 + radius) && y < (sel_y2 - radius))
	{
	  for (x = 0; x < width; x ++)
	    {
	      hist0   = 0;
	      hist255 = 0;
	      xmin    = x - radius * img_bpp;
	      xmax    = x + (radius + 1) * img_bpp;

	      if (xmin < 0)
		xmin = x % img_bpp;

	      if (xmax > width)
		xmax = width;

	      startrow = (row + y - lasty - radius + max_row) % max_row;
	      endrow   = (row + y - lasty + radius + 1 + max_row) % max_row;

	      for (sort_ptr = sort, trow = startrow;
		   trow != endrow;
		   trow = (trow + 1) % max_row)
		for (tx = xmin, src_ptr = src_rows[trow] + xmin;
		     tx < xmax;
		     tx += img_bpp, src_ptr += img_bpp)
		  {
		    if ((*sort_ptr = *src_ptr) <= black_level)
		      hist0 ++;
		    else if (*sort_ptr >= white_level)
		      hist255 ++;

		    if (*sort_ptr < white_level)
		      sort_ptr ++;
		  };

	      /*
	       * Shell sort the color values...
	       */

	      sort_count = sort_ptr - sort;

	      if (sort_count > 1)
		{
		  for (d = sort_count / 2; d > 0; d = d / 2)
		    for (i = d; i < sort_count; i ++)
		      for (j = i - d, sort_ptr = sort + j;
			   j >= 0 && sort_ptr[0] > sort_ptr[d];
			   j -= d, sort_ptr -= d)
			{
			  t           = sort_ptr[0];
			  sort_ptr[0] = sort_ptr[d];
			  sort_ptr[d] = t;
			};

		  /*
		   * Assign the median value...
		   */

		  t = sort_count / 2;

		  if (sort_count & 1)
		    dst_row[x] = (sort[t] + sort[t + 1]) / 2;
		  else
		    dst_row[x] = sort[t];

		  /*
		   * Save the change to the source image too if the user
		   * wants the recursive method...
		   */

		  if (filter_type & FILTER_RECURSIVE)
		    src_rows[(row + y - lasty + max_row) % max_row][x] =
		      dst_row[x];
		};

	      /*
	       * Check the histogram and adjust the radius accordingly...
	       */

	      if (filter_type & FILTER_ADAPTIVE)
		{
		  if (hist0 >= radius || hist255 >= radius)
		    {
		      if (radius < despeckle_radius)
			radius ++;
		    }
		  else if (radius > 1)
		    radius --;
		};
	    };
	};

      gimp_pixel_rgn_set_row (&dst_rgn, dst_row, sel_x1, y, sel_width);

      if ((y & 15) == 0)
	gimp_progress_update ((double) (y - sel_y1) / (double) sel_height);
    };

  /*
   * OK, we're done.  Free all memory used...
   */

  g_free (src_rows[0]);
  g_free (src_rows);
  g_free (dst_row);
  g_free (sort);

  /*
   * Update the screen...
   */

  gimp_drawable_flush (drawable);
  gimp_drawable_merge_shadow (drawable->id, TRUE);
  gimp_drawable_update (drawable->id, sel_x1, sel_y1, sel_width, sel_height);
}


/*
 * 'despeckle_dialog()' - Popup a dialog window for the filter box size...
 */

static gint
despeckle_dialog (void)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *ptable;
  GtkWidget *frame;
  GtkWidget *scrollbar;
  GtkWidget *button;
  GtkObject *adj;
  gint     argc;
  gchar	 **argv;
  guchar  *color_cube;
  gchar   *plugin_name;

  argc    = 1;
  argv    = g_new (gchar *, 1);
  argv[0] = g_strdup ("despeckle");

  gtk_init (&argc, &argv);
  gtk_rc_parse (gimp_gtkrc ());
  gdk_set_use_xshm (gimp_use_xshm ());

  gtk_preview_set_gamma (gimp_gamma ());
  gtk_preview_set_install_cmap (gimp_install_cmap ());
  color_cube = gimp_color_cube ();
  gtk_preview_set_color_cube (color_cube[0], color_cube[1],
			      color_cube[2], color_cube[3]);

  gtk_widget_set_default_visual (gtk_preview_get_visual ());
  gtk_widget_set_default_colormap (gtk_preview_get_cmap ());

  plugin_name = g_strdup_printf ("%s%s", _("Despeckle "), PLUG_IN_VERSION);

  dialog = gimp_dialog_new (plugin_name, "despeckle",
			    gimp_plugin_help_func, "filters/despeckle.html",
			    GTK_WIN_POS_MOUSE,
			    FALSE, TRUE, FALSE,

			    _("OK"), dialog_ok_callback,
			    NULL, NULL, NULL, TRUE, FALSE,
			    _("Cancel"), gtk_widget_destroy,
			    NULL, 1, NULL, FALSE, TRUE,

			    NULL);

  g_free (plugin_name);

  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      GTK_SIGNAL_FUNC (gtk_main_quit),
		      NULL);

  main_vbox = gtk_vbox_new (FALSE, 4);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), main_vbox,
		      TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  hbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*
   * Preview window...
   */

  ptable = gtk_table_new (2, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), ptable, FALSE, FALSE, 0);
  gtk_widget_show(ptable);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_table_attach (GTK_TABLE (ptable), frame, 0, 1, 0, 1, 0, 0, 0, 0);
  gtk_widget_show (frame);

  preview_width  = MIN (sel_width, PREVIEW_SIZE);
  preview_height = MIN (sel_height, PREVIEW_SIZE);

  preview = gtk_preview_new (GTK_PREVIEW_COLOR);
  gtk_preview_size (GTK_PREVIEW (preview), preview_width, preview_height);
  gtk_container_add (GTK_CONTAINER (frame), preview);
  gtk_widget_show (preview);

  hscroll_data = gtk_adjustment_new (0, 0, sel_width - 1, 1.0,
				     MIN (preview_width, sel_width),
				     MIN (preview_width, sel_width));

  gtk_signal_connect (hscroll_data, "value_changed",
		      GTK_SIGNAL_FUNC (preview_scroll_callback),
		      NULL);

  scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT(hscroll_data));
  gtk_range_set_update_policy (GTK_RANGE (scrollbar), GTK_UPDATE_CONTINUOUS);
  gtk_table_attach (GTK_TABLE(ptable), scrollbar, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
  gtk_widget_show (scrollbar);

  vscroll_data = gtk_adjustment_new (0, 0, sel_height - 1, 1.0,
				     MIN (preview_height, sel_height),
				     MIN (preview_height, sel_height));

  gtk_signal_connect (vscroll_data, "value_changed",
		      GTK_SIGNAL_FUNC (preview_scroll_callback),
		      NULL);

  scrollbar = gtk_vscrollbar_new (GTK_ADJUSTMENT (vscroll_data));
  gtk_range_set_update_policy (GTK_RANGE (scrollbar), GTK_UPDATE_CONTINUOUS);
  gtk_table_attach (GTK_TABLE (ptable), scrollbar, 1, 2, 0, 1, 0,
		    GTK_FILL, 0, 0);
  gtk_widget_show (scrollbar);

  preview_init ();

  preview_x1 = sel_x1;
  preview_y1 = sel_y1;
  preview_x2 = preview_x1 + MIN (preview_width, sel_width);
  preview_y2 = preview_y1 + MIN (preview_height, sel_height);

  /*
   * Filter type controls...
   */

  frame = gtk_frame_new (_("Type"));
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  button = gtk_check_button_new_with_label (_("Adaptive"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				(filter_type & FILTER_ADAPTIVE) ? TRUE : FALSE);
  gtk_signal_connect (GTK_OBJECT (button), "toggled",
		      GTK_SIGNAL_FUNC (dialog_adaptive_callback),
		      NULL);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_label (_("Recursive"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				(filter_type & FILTER_RECURSIVE) ? TRUE : FALSE);
  gtk_signal_connect (GTK_OBJECT (button), "toggled",
		      GTK_SIGNAL_FUNC (dialog_recursive_callback),
		      NULL);
  gtk_widget_show (button);

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  /*
   * Box size (radius) control...
   */

  adj = gimp_scale_entry_new (GTK_TABLE (table), 0, 0,
			      _("Radius:"), 100, 0,
			      despeckle_radius, 1, MAX_RADIUS, 1, 5, 0,
			      NULL, NULL);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (dialog_iscale_update),
		      &despeckle_radius);

  /*
   * Black level control...
   */

  adj = gimp_scale_entry_new (GTK_TABLE (table), 0, 1,
			      _("Black Level:"), 100, 0,
			      black_level, 0, 256, 1, 8, 0,
			      NULL, NULL);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (dialog_iscale_update),
		      &black_level);

  /*
   * White level control...
   */

  adj = gimp_scale_entry_new (GTK_TABLE (table), 0, 2,
			      _("White Level:"), 100, 0,
			      white_level, 0, 256, 1, 8, 0,
			      NULL, NULL);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (dialog_iscale_update),
		      &white_level);

  /*
   * Show it and wait for the user to do something...
   */

  gtk_widget_show (dialog);

  preview_update ();

  gtk_main ();
  gdk_flush ();

  /*
   * Free the preview data...
   */

  preview_exit ();

  /*
   * Return ok/cancel...
   */

  return run_filter;
}


/*
 * 'preview_init()' - Initialize the preview window...
 */

static void
preview_init (void)
{
  gint	size,		/* Size of filter box */
	width;		/* Byte width of the image */

  /*
   * Setup for preview filter...
   */

  size  = despeckle_radius * 2 + 1;
  width = preview_width * img_bpp;

  if (preview_src != NULL)
    {
      g_free (preview_src);
      g_free (preview_dst);
      g_free (preview_sort);
    }

  preview_src  = (guchar *) g_new (guchar *, width * preview_height);
  preview_dst  = g_new (guchar, width);
  preview_sort = g_new (guchar, size * size);
}


/*
 * 'preview_scroll_callback()' - Update the preview when a scrollbar is moved.
 */

static void
preview_scroll_callback (void)
{
  preview_x1 = sel_x1 + GTK_ADJUSTMENT (hscroll_data)->value;
  preview_y1 = sel_y1 + GTK_ADJUSTMENT (vscroll_data)->value;
  preview_x2 = preview_x1 + MIN (preview_width, sel_width);
  preview_y2 = preview_y1 + MIN (preview_height, sel_height);

  preview_update ();
}


/*
 * 'preview_update()' - Update the preview window.
 */

static void
preview_update (void)
{
  GPixelRgn	src_rgn;	/* Source image region */
  guchar       *sort_ptr,	/* Current preview_sort value */
	       *src_ptr,	/* Current source pixel */
	       *dst_ptr;	/* Current destination pixel */
  gint		sort_count,	/* Number of soft values */
		i, j, t, d,	/* Looping vars */
		x, y,		/* Current location in image */
		size,		/* Width/height of the filter box */
		width,		/* Byte width of the image */
		xmin, xmax, tx,	/* Looping vars */
		radius,		/* Current radius */
		hist0,		/* Histogram count for 0 values */
		hist255;	/* Histogram count for 255 values */
  guchar	check,		/* Checkerboard pattern */
		rgb[PREVIEW_SIZE * PREVIEW_SIZE * 3],
				/* Output image */
	       *rgb_ptr;	/* Pixel pointer for output */

 /*
  * Setup for filter...
  */

  gimp_pixel_rgn_init (&src_rgn, drawable,
		       preview_x1, preview_y1, preview_width, preview_height,
		       FALSE, FALSE);

  /*
   * Pre-load the preview rectangle...
   */

  size  = despeckle_radius * 2 + 1;
  width = preview_width * img_bpp;

  gimp_pixel_rgn_get_rect (&src_rgn, preview_src, preview_x1, preview_y1,
			   preview_width, preview_height);

  /*
   * Despeckle...
   */

  for (y = 0; y < preview_height; y ++)
    {
      /*
       * Now find the median pixels and save the results...
       */

      radius = despeckle_radius;

      memcpy (preview_dst, preview_src + y * width, width);

      if (y >= radius && y < (preview_height - radius))
	{
	  for (x = 0, dst_ptr = preview_dst; x < width; x ++, dst_ptr ++)
	    {
	      hist0   = 0;
	      hist255 = 0;
	      xmin    = x - radius * img_bpp;
	      xmax    = x + (radius + 1) * img_bpp;

	      if (xmin < 0)
		xmin = x % img_bpp;

	      if (xmax > width)
		xmax = width;

	      for (i = -radius, sort_ptr = preview_sort,
		     src_ptr = preview_src + width * (y - radius);
		   i <= radius;
		   i ++, src_ptr += width)
		for (tx = xmin; tx < xmax; tx += img_bpp)
		  {
		    if ((*sort_ptr = src_ptr[tx]) <= black_level)
		      hist0 ++;
		    else if (*sort_ptr >= white_level)
		      hist255 ++;

		    if (*sort_ptr < white_level)
		      sort_ptr ++;
		  };

	      /*
	       * Shell preview_sort the color values...
	       */

	      sort_count = sort_ptr - preview_sort;

	      if (sort_count > 1)
		{
		  for (d = sort_count / 2; d > 0; d = d / 2)
		    for (i = d; i < sort_count; i ++)
		      for (j = i - d, sort_ptr = preview_sort + j;
			   j >= 0 && sort_ptr[0] > sort_ptr[d];
			   j -= d, sort_ptr -= d)
			{
			  t           = sort_ptr[0];
			  sort_ptr[0] = sort_ptr[d];
			  sort_ptr[d] = t;
			};

		  /*
		   * Assign the median value...
		   */

		  t = sort_count / 2;

		  if (sort_count & 1)
		    *dst_ptr = (preview_sort[t] + preview_sort[t + 1]) / 2;
		  else
		    *dst_ptr = preview_sort[t];

		  /*
		   * Save the change to the source image too if the user
		   * wants the recursive method...
		   */

		  if (filter_type & FILTER_RECURSIVE)
		    preview_src[y * width + x] = *dst_ptr;
		};

	      /*
	       * Check the histogram and adjust the radius accordingly...
	       */

	      if (filter_type & FILTER_ADAPTIVE)
		{
		  if (hist0 >= radius || hist255 >= radius)
		    {
		      if (radius < despeckle_radius)
			radius ++;
		    }
		  else if (radius > 1)
		    radius --;
		};
	    };
	};

      /*
       * Draw this row...
       */

      rgb_ptr = rgb + y * preview_width * 3;

      switch (img_bpp)
	{
	case 1:
          for (x = preview_width, dst_ptr = preview_dst;
               x > 0;
               x --, dst_ptr ++, rgb_ptr += 3)
            rgb_ptr[0] = rgb_ptr[1] = rgb_ptr[2] = *dst_ptr;
          break;

	case 2:
	  for (x = preview_width, dst_ptr = preview_dst;
	       x > 0;
	       x --, dst_ptr += 2, rgb_ptr += 3)
	    if (dst_ptr[1] == 255)
	      rgb_ptr[0] = rgb_ptr[1] = rgb_ptr[2] = *dst_ptr;
	    else
	      {
		if ((y & GIMP_CHECK_SIZE) ^ (x & GIMP_CHECK_SIZE))
		  check = GIMP_CHECK_LIGHT * 255;
		else
		  check = GIMP_CHECK_DARK * 255;

		if (dst_ptr[1] == 0)
		  rgb_ptr[0] = rgb_ptr[1] = rgb_ptr[2] = check;
		else
		  rgb_ptr[0] = rgb_ptr[1] = rgb_ptr[2] =
                    check + ((dst_ptr[0] - check) * dst_ptr[1]) / 255;
	      };
          break;

	case 3:
          memcpy (rgb_ptr, preview_dst, preview_width * 3);
          break;

	case 4:
	  for (x = preview_width, dst_ptr = preview_dst;
	       x > 0;
	       x --, dst_ptr += 4, rgb_ptr += 3)
	    if (dst_ptr[3] == 255)
	      {
		rgb_ptr[0] = dst_ptr[0];
		rgb_ptr[1] = dst_ptr[1];
		rgb_ptr[2] = dst_ptr[2];
	      }
	    else
	      {
		if ((y & GIMP_CHECK_SIZE) ^ (x & GIMP_CHECK_SIZE))
		  check = GIMP_CHECK_LIGHT * 255;
		else
		  check = GIMP_CHECK_DARK * 255;

		if (dst_ptr[3] == 0)
		  rgb_ptr[0] = rgb_ptr[1] = rgb_ptr[2] = check;
		else
		  {
		    rgb_ptr[0] =
		      check + ((dst_ptr[0] - check) * dst_ptr[3]) / 255;
		    rgb_ptr[1] =
		      check + ((dst_ptr[1] - check) * dst_ptr[3]) / 255;
		    rgb_ptr[2] =
		      check + ((dst_ptr[2] - check) * dst_ptr[3]) / 255;
		  };
	      };
          break;
	};
    };

  /*
   * Update the screen...
   */

  for (y = 0, rgb_ptr = rgb;
       y < preview_height;
       y ++, rgb_ptr += preview_width * 3)
    gtk_preview_draw_row (GTK_PREVIEW (preview), rgb_ptr, 0, y, preview_width);

  gtk_widget_draw (preview, NULL);
  gdk_flush ();
}

static void
preview_exit (void)
{
  g_free (preview_src);
  g_free (preview_dst);
  g_free (preview_sort);
}

static void
dialog_iscale_update (GtkAdjustment *adjustment,
		      gint          *value)
{
  *value = adjustment->value;

  if (value == &despeckle_radius)
    preview_init ();

  preview_update ();
}

static void
dialog_adaptive_callback (GtkWidget *widget,
			  gpointer   data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active)
    filter_type |= FILTER_ADAPTIVE;
  else
    filter_type &= ~FILTER_ADAPTIVE;

  preview_update ();
}

static void
dialog_recursive_callback (GtkWidget *widget,
			   gpointer  data)
{
  if (GTK_TOGGLE_BUTTON (widget)->active)
    filter_type |= FILTER_RECURSIVE;
  else
    filter_type &= ~FILTER_RECURSIVE;

  preview_update ();
}

static void
dialog_ok_callback (GtkWidget *widget,
		    gpointer  data)
{
  run_filter = TRUE;

  gtk_widget_destroy (GTK_WIDGET (data));
}
