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

#include "imap_cmd_select.h"
#include "imap_cmd_select_next.h"
#include "imap_cmd_unselect.h"

#include "imap_main.h"

#include "libgimp/stdplugins-intl.h"

static CmdExecuteValue_t select_next_command_execute(Command_t *parent);
static void select_next_command_undo(Command_t *command);
static void select_next_command_redo(Command_t *command);

static CommandClass_t select_next_command_class = {
   NULL,			/* select_next_command_destruct */
   select_next_command_execute,
   select_next_command_undo,
   select_next_command_redo
};

typedef struct {
   Command_t parent;
   ObjectList_t *list;
} SelectNextCommand_t;

Command_t* 
select_next_command_new(ObjectList_t *list)
{
   SelectNextCommand_t *command = g_new(SelectNextCommand_t, 1);
   command->list = list;
   return command_init(&command->parent, _("Select Next"), 
		       &select_next_command_class);
}

static void
select_one_object(Object_t *obj, gpointer data)
{
   SelectNextCommand_t *command = (SelectNextCommand_t*) data;
   Command_t *sub_command;

   sub_command = (obj->selected) 
      ? select_command_new(obj) : unselect_command_new(obj);
   command_add_subcommand(&command->parent, sub_command);
}

static CmdExecuteValue_t
select_next_command_execute(Command_t *parent)
{
   SelectNextCommand_t *command = (SelectNextCommand_t*) parent;
   ObjectList_t *list = command->list;
   gpointer id;

   id = object_list_add_select_cb(list, select_one_object, command);
   object_list_select_next(list);
   object_list_remove_select_cb(list, id);
   return CMD_APPEND;
}

static void
select_next_command_undo(Command_t *command)
{
   redraw_preview();		/* Fix me! */
}

static void
select_next_command_redo(Command_t *command)
{
   redraw_preview();		/* Fix me! */
}
