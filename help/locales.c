/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * The GIMP Help plug-in
 * Copyright (C) 1999-2004 Sven Neumann <sven@gimp.org>
 *                         Michael Natterer <mitch@gimp.org>
 *                         Henrik Brix Andersen <brix@gimp.org>
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

#include <glib.h>

#include "locales.h"


#define GIMP_HELP_DEFAULT_LOCALE  "en"


GList *
locales_parse (const gchar *help_locales)
{
  GList        *locales = NULL;
  GList        *list;
  const gchar  *s;
  const gchar  *p;

  g_return_val_if_fail (help_locales != NULL, NULL);

  /*  split the string at colons, building a list  */
  s = help_locales;
  for (p = strchr (s, ':'); p; p = strchr (s, ':'))
    {
      gchar *new = g_strndup (s, p - s);

      locales = g_list_prepend (locales, new);
      s = p + 1;
    }

  if (*s)
    locales = g_list_prepend (locales, g_strdup (s));

  /*  append locales w/o variants unless they exist already */
  for (list = locales; list; list = list->next)
    {
      s = (const gchar *) list->data;
      p = strchr (s, '_');

      if (p)
        {
          GList *iter;
          gchar *new = g_strndup (s, p - s);

          for (iter = locales; iter; iter = iter->next)
            if (strcmp ((const gchar *) iter->data, new) == 0)
              break;

          if (iter)
            g_free (new);
          else
            locales = g_list_append (locales, new);
        }
    }

  /*  if the list doesn't contain the default domain yet, append it  */
  for (list = locales; list; list = list->next)
    if (strcmp ((const gchar *) list->data, GIMP_HELP_DEFAULT_LOCALE) == 0)
      break;

  if (!list)
    locales = g_list_append (locales, GIMP_HELP_DEFAULT_LOCALE);

#ifdef GIMP_HELP_DEBUG
  g_printerr ("help: locales: ");
  for (list = locales; list; list = list->next)
    g_printerr ("%s ", (const gchar *) list->data);
  g_printerr ("\n");
#endif

  return locales;
}
