/*
 * This is a plug-in for the GIMP.
 *
 * Generates clickable image maps.
 *
 * Copyright (C) 1998-1999 Maurits Rijk  lpeek.mrijk@consunet.nl
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

#include "config.h"

#include <gtk/gtk.h>

#include "imap_cmd_copy_object.h"
#include "imap_cmd_cut_object.h"
#include "imap_cmd_delete.h"
#include "imap_main.h"

#include "libgimp/stdplugins-intl.h"

static CmdExecuteValue_t cut_object_command_execute(Command_t *parent);
static void cut_object_command_undo(Command_t *parent);
static void cut_object_command_redo(Command_t *parent);

static CommandClass_t cut_object_command_class = {
   NULL,			/* cut_object_command_destruct */
   cut_object_command_execute,
   cut_object_command_undo,
   cut_object_command_redo
};

typedef struct {
   Command_t parent;
} CutObjectCommand_t;

Command_t*
cut_object_command_new(Object_t *obj)
{
   CutObjectCommand_t *command = g_new(CutObjectCommand_t, 1);
   Command_t *parent;

   parent = command_init(&command->parent, _("Cut"), 
			 &cut_object_command_class);
   command_add_subcommand(parent, copy_object_command_new(obj));
   command_add_subcommand(parent, delete_command_new(obj->list, obj));

   return parent;
}

static CmdExecuteValue_t
cut_object_command_execute(Command_t *parent)
{
   redraw_preview();		/* fix me! */
   return CMD_APPEND;
}

static void
cut_object_command_undo(Command_t *parent)
{
   redraw_preview();		/* fix me! */
}

static void
cut_object_command_redo(Command_t *parent)
{
   redraw_preview();		/* fix me! */
}
