/*
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This is a plug-in for the GIMP.
 *
 * Generates images containing vector type drawings.
 *
 * Copyright (C) 1997 Andy Thomas  alt@picnic.demon.co.uk
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

#ifndef __GFIG_DOBJECT_H__
#define __GFIG_DOBJECT_H__

#include "gfig-types.h"
#include "gfig-style.h"

struct Dobject; /* fwd declaration for DobjFunc */

typedef void            (*DobjFunc)     (struct Dobject *);
typedef struct Dobject *(*DobjGenFunc)  (struct Dobject *);
typedef struct Dobject *(*DobjLoadFunc) (FILE *);
typedef void            (*DobjSaveFunc) (struct Dobject *, GString *);
typedef struct Dobject *(*DobjCreateFunc) (gint, gint);

typedef struct DobjPoints
{
  struct DobjPoints *next;
  GdkPoint           pnt;
  gint               found_me;
} DobjPoints;

typedef struct
{
  DobjType      type;       /* the object type for this class */
  gchar        *name;
  DobjFunc      drawfunc;   /* How do I draw myself */
  DobjFunc      paintfunc;  /* Draw me on canvas */
  DobjGenFunc   copyfunc;   /* copy */
} DobjClass;

DobjClass dobj_class[10];

/* The object itself */
typedef struct Dobject
{
  DobjType      type;       /* What is the type? */
  DobjClass    *class;      /* What class does it belong to? */
  gint          type_data;  /* Extra data needed by the object */
  DobjPoints   *points;     /* List of points */
  Style         style;      /* this object's individual style settings */
  gint          style_no;   /* style index of this specific object */
} Dobject;

typedef struct DAllObjs
{
  struct DAllObjs *next;
  Dobject         *obj; /* Object on list */
} DAllObjs;

/* States of the object */
#define GFIG_OK       0x0
#define GFIG_MODIFIED 0x1
#define GFIG_READONLY 0x2

extern Dobject *obj_creating;
extern Dobject      *tmp_line;

void d_pnt_add_line (Dobject *obj,
                     gint     x,
                     gint     y,
                     gint     pos);

DobjPoints     *new_dobjpoint           (gint x, gint y);
void            do_save_obj             (Dobject *obj,
                                         GString *to);

DobjPoints     *d_copy_dobjpoints       (DobjPoints * pnts);
void            free_one_obj            (Dobject *obj);
void            d_delete_dobjpoints     (DobjPoints * pnts);
void            object_update           (GdkPoint *pnt);
DAllObjs       *copy_all_objs           (DAllObjs *objs);
void            draw_objects            (DAllObjs *objs, gint show_single);

Dobject *d_load_object            (gchar      *desc,
                                   FILE       *fp);

Dobject *d_new_object             (DobjType    type,
                                   gint        x,
                                   gint        y);

void     d_save_object            (Dobject    *obj,
                                   GString    *string);

void     free_all_objs            (DAllObjs   *objs);

void     clear_undo               (void);

void     new_obj_2edit            (GFigObj    *obj);

gint     gfig_obj_counts          (DAllObjs   *objs);

void     gfig_init_object_classes (void);

#endif /* __GFIG_DOBJECT_H__ */


