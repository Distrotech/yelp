/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@codefactory.se>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Mikael Hallendal <micke@codefactory.se>
 */

#ifndef __YELP_SCROLLKEEPER_H__
#define __YELP_SCROLLKEEPER_H__

#include <glib.h>
#include <gtk/gtktreestore.h>

gboolean       yelp_scrollkeeper_init               (GNode          *tree,
						     GList         **index);
GNode *        yelp_scrollkeeper_lookup_seriesid    (const gchar    *seriesid);

GNode *        yelp_scrollkeeper_get_toc_tree       (const gchar    *docpath);

#endif /* __YELP_SCROLLKEEPER_H__ */
