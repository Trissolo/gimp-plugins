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
 * 
 */

#ifndef __GFIG_SPIRAL_H__
#define __GFIG_SPIRAL_H__

gint spiral_button_press (GtkWidget      *widget,
                          GdkEventButton *event,
                          gpointer        data);

void       d_update_spiral         (GdkPoint *pnt);
void       d_spiral_start          (GdkPoint *pnt, gint shift_down);
void       d_spiral_end            (GdkPoint *pnt, gint shift_down);

Dobject  * d_load_spiral           (FILE *from);

#endif /* __GFIG_SPIRAL_H__ */
