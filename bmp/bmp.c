/* bmp.c                                          */
/* Version 0.51	                                  */
/* This is a File input and output filter for the */
/* Gimp. It loads and saves images in windows(TM) */
/* bitmap format.                                 */
/* Some Parts that deal with the interaction with */
/* the Gimp are taken from the GIF plugin by      */
/* Peter Mattis & Spencer Kimball and from the    */
/* PCX plugin by Francisco Bustamante.            */
/*                                                */
/* Alexander.Schulz@stud.uni-karlsruhe.de         */

/* Changes:   28.11.1997 Noninteractive operation */
/*            16.03.1998 Endian-independent!!     */
/*	      21.03.1998 Little Bug-fix		  */
/*            06.04.1998 Bugfix in Padding        */
/*            11.04.1998 Arch. cleanup (-Wall)    */
/*                       Parses gtkrc             */
/*            14.04.1998 Another Bug in Padding   */
/*            28.04.1998 RLE-Encoding rewritten   */
/*            29.10.1998 Changes by Tor Lillqvist */
/*                       <tml@iki.fi> to support  */
/*                       16 and 32 bit images     */
/*            28.11.1998 Bug in RLE-read-padding  */
/*                       fixed.                   */
/*            19.12.1999 Resolution support added */

/* 
 * The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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
 * ----------------------------------------------------------------------------
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "bmp.h"

#include "libgimp/stdplugins-intl.h"

FILE  *errorfile;
gchar *prog_name = "bmp";
gchar *filename;
gint   interactive_bmp;

struct Bitmap_File_Head_Struct Bitmap_File_Head;
struct Bitmap_Head_Struct Bitmap_Head;
struct Bitmap_OS2_Head_Struct Bitmap_OS2_Head;

/* Declare some local functions.
 */
static void   query      (void);
static void   run        (gchar   *name,
                          gint     nparams,
                          GParam  *param,
                          gint    *nreturn_vals,
                          GParam **return_vals);
static void   init_gtk   (void);

GPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

MAIN ()

static void
query (void)
{
  static GParamDef load_args[] =
  {
    { PARAM_INT32,    "run_mode",     "Interactive, non-interactive" },
    { PARAM_STRING,   "filename",     "The name of the file to load" },
    { PARAM_STRING,   "raw_filename", "The name entered" },
  };
  static GParamDef load_return_vals[] =
  {
    { PARAM_IMAGE, "image", "Output image" },
  };
  static gint nload_args = sizeof (load_args) / sizeof (load_args[0]);
  static gint nload_return_vals = (sizeof (load_return_vals) /
				   sizeof (load_return_vals[0]));

  static GParamDef save_args[] =
  {
    { PARAM_INT32,    "run_mode",     "Interactive, non-interactive" },
    { PARAM_IMAGE,    "image",        "Input image" },
    { PARAM_DRAWABLE, "drawable",     "Drawable to save" },
    { PARAM_STRING,   "filename",     "The name of the file to save the image in" },
    { PARAM_STRING,   "raw_filename", "The name entered" },
  };
  static gint nsave_args = sizeof (save_args) / sizeof (save_args[0]);
  
  INIT_I18N();

  gimp_install_procedure ("file_bmp_load",
                          "Loads files of Windows BMP file format",
                          "Loads files of Windows BMP file format",
                          "Alexander Schulz",
                          "Alexander Schulz",
                          "1997",
                          "<Load>/BMP",
                          NULL,
                          PROC_PLUG_IN,
                          nload_args, nload_return_vals,
                          load_args, load_return_vals);

  gimp_install_procedure ("file_bmp_save",
                          "Saves files in Windows BMP file format",
                          "Saves files in Windows BMP file format",
                          "Alexander Schulz",
                          "Alexander Schulz",
                          "1997",
                          "<Save>/BMP",
                          "INDEXED, GRAY, RGB",
                          PROC_PLUG_IN,
                          nsave_args, 0,
                          save_args, NULL);

  gimp_register_magic_load_handler ("file_bmp_load",
				    "bmp",
				    "",
				    "0,string,BM");
  gimp_register_save_handler       ("file_bmp_save",
				    "bmp",
				    "");
}

static void
run (gchar   *name,
     gint     nparams,
     GParam  *param,
     gint    *nreturn_vals,
     GParam **return_vals)
{
  static GParam values[2];
  GRunModeType  run_mode;
  GStatusType   status = STATUS_SUCCESS;
  gint32        image_ID;
  gint32        drawable_ID;
  GimpExportReturnType export = EXPORT_CANCEL;
  
  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;
  values[0].type          = PARAM_STATUS;
  values[0].data.d_status = STATUS_EXECUTION_ERROR;

  if (strcmp (name, "file_bmp_load") == 0)
    {
       INIT_I18N();

       switch (run_mode)
        {
        case RUN_INTERACTIVE:
	  interactive_bmp = TRUE;
          break;

        case RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          interactive_bmp = FALSE;
	  if (nparams != 3)
            status = STATUS_CALLING_ERROR;
          break;

        default:
          break;
        }

       if (status == STATUS_SUCCESS)
	 {
	   image_ID = ReadBMP (param[1].data.d_string);

	   if (image_ID != -1)
	     {
	       *nreturn_vals = 2;
	       values[1].type         = PARAM_IMAGE;
	       values[1].data.d_image = image_ID;
	     }
	   else
	     {
	       status = STATUS_EXECUTION_ERROR;
	     }
	 }
    }
  else if (strcmp (name, "file_bmp_save") == 0)
    {
      INIT_I18N();

      image_ID    = param[1].data.d_int32;
      drawable_ID = param[2].data.d_int32;

      /*  eventually export the image */ 
      switch (run_mode)
	{
	case RUN_INTERACTIVE:
	case RUN_WITH_LAST_VALS:
	  init_gtk ();
	  export = gimp_export_image (&image_ID, &drawable_ID, "BMP", 
				      (CAN_HANDLE_RGB |
				       CAN_HANDLE_GRAY |
				       CAN_HANDLE_INDEXED));
	  if (export == EXPORT_CANCEL)
	    {
	      values[0].data.d_status = STATUS_CANCEL;
	      return;
	    }
	  break;
	default:
	  break;
	}

      switch (run_mode)
        {
        case RUN_INTERACTIVE:
	  interactive_bmp = TRUE;
          break;

        case RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          interactive_bmp = FALSE;
	  if (nparams != 5)
            status = STATUS_CALLING_ERROR;
          break;

        case RUN_WITH_LAST_VALS:
          interactive_bmp = FALSE;
          break;

        default:
          break;
        }

      if (status == STATUS_SUCCESS)
	{
	  status = WriteBMP (param[3].data.d_string, image_ID, drawable_ID);
	}

      if (export == EXPORT_EXPORT)
	gimp_image_delete (image_ID);
    }
  else
    {
      status = STATUS_CALLING_ERROR;
    }

  values[0].data.d_status = status;
}

gint32 
ToL (guchar *puffer)
{
  return (puffer[0] | puffer[1]<<8 | puffer[2]<<16 | puffer[3]<<24);
}

gint16 
ToS (guchar *puffer)
{
  return (puffer[0] | puffer[1]<<8);
}

void 
FromL (gint32  wert, 
       guchar *bopuffer)
{
  bopuffer[0] = (wert & 0x000000ff)>>0x00;
  bopuffer[1] = (wert & 0x0000ff00)>>0x08;
  bopuffer[2] = (wert & 0x00ff0000)>>0x10;
  bopuffer[3] = (wert & 0xff000000)>>0x18;
}

void  
FromS (gint16  wert, 
       guchar *bopuffer)
{
  bopuffer[0] = (wert & 0x00ff)>>0x00;
  bopuffer[1] = (wert & 0xff00)>>0x08;
}

static void
init_gtk (void)
{
  gchar **argv;
  gint    argc;

  argc    = 1;
  argv    = g_new (gchar *, 1);
  argv[0] = g_strdup ("bmp");
  
  gtk_init (&argc, &argv);
  gtk_rc_parse (gimp_gtkrc ());
}
