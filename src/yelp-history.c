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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include "yelp-history.h"

static void    history_init              (YelpHistory      *history);
static void    history_class_init        (YelpHistoryClass *klass);
static void    history_finalize          (GObject          *object);
static void    history_free_history_list (GList            *history_list);

static void    history_maybe_emit        (YelpHistory      *history);
					
enum { 
	FORWARD_EXISTS_CHANGED,
	BACK_EXISTS_CHANGED, 
	LAST_SIGNAL 
}; 

static gint signals[LAST_SIGNAL] = { 0 };

struct _YelpHistoryPriv {
	GList    *history_list;
	GList    *current;

	gboolean  last_emit_forward;
	gboolean  last_emit_back;
};

GType
yelp_history_get_type (void)
{
        static GType history_type = 0;

        if (!history_type) {
                static const GTypeInfo history_info = {
                        sizeof (YelpHistoryClass),
                        NULL,
                        NULL,
                        (GClassInitFunc) history_class_init,
                        NULL,
                        NULL,
                        sizeof (YelpHistory),
                        0,
                        (GInstanceInitFunc) history_init,
                };
                
                history_type = g_type_register_static (G_TYPE_OBJECT,
                                                      "YelpHistory", 
                                                      &history_info, 0);
        }
        
        return history_type;
}

static void
history_init (YelpHistory *history)
{
	YelpHistoryPriv *priv;

	priv = g_new0 (YelpHistoryPriv, 1);
        
	priv->history_list      = NULL;
	priv->current           = NULL;
	priv->last_emit_forward = FALSE;
	priv->last_emit_back    = FALSE;
	history->priv           = priv;
}

static void
history_class_init (YelpHistoryClass *klass)
{
        GObjectClass *object_class;
        
        object_class = (GObjectClass *) klass;

        object_class->finalize = history_finalize;

	signals[FORWARD_EXISTS_CHANGED] =
                g_signal_new ("forward_exists_changed",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (YelpHistoryClass, 
                                               forward_exists_changed),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__BOOLEAN,
                              G_TYPE_NONE,
                              1, G_TYPE_BOOLEAN);
        
	signals[BACK_EXISTS_CHANGED] =
		g_signal_new ("back_exists_changed",
                              G_TYPE_FROM_CLASS (object_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (YelpHistoryClass,
                                               back_exists_changed),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__BOOLEAN,
                              G_TYPE_NONE,
                              1, G_TYPE_BOOLEAN);
}

static void
history_finalize (GObject *object)
{
	YelpHistory     *history;
	YelpHistoryPriv *priv;
	GList           *node;
        
	g_return_if_fail (object != NULL);
	g_return_if_fail (YELP_IS_HISTORY (object));
        
	history = YELP_HISTORY (object);
	priv    = history->priv;
        
	for (node = priv->history_list; node; node = node->next) {
		yelp_uri_unref (YELP_URI (node->data));
	}

	g_list_free (priv->history_list);

	g_free (priv);

	history->priv = NULL;
}

static void
history_free_history_list (GList *history_list)
{
	GList *node;
        
	for (node = history_list; node; node = node->next) {
		yelp_uri_unref (YELP_URI (node->data));
	}

	g_list_free (history_list);
}

static void
history_maybe_emit (YelpHistory *history)
{
	YelpHistoryPriv *priv;
		
	g_return_if_fail (history != NULL);
	g_return_if_fail (YELP_IS_HISTORY (history));
	
	priv = history->priv;

	if (priv->last_emit_forward != yelp_history_exist_forward (history)) {
		priv->last_emit_forward = yelp_history_exist_forward (history);
		
		g_signal_emit (history,
                               signals[FORWARD_EXISTS_CHANGED],
			       0, priv->last_emit_forward);
	}

	if (priv->last_emit_back != yelp_history_exist_back (history)) {
		priv->last_emit_back = yelp_history_exist_back (history);
		
		g_signal_emit (history,
                               signals[BACK_EXISTS_CHANGED],
                               0, priv->last_emit_back);
	}
}

void
yelp_history_goto (YelpHistory *history, YelpURI *uri)
{
	YelpHistoryPriv *priv;
	GList           *forward_list;
	
	g_return_if_fail (history != NULL);
	g_return_if_fail (YELP_IS_HISTORY (history));

	priv = history->priv;

	if (priv->current && priv->current->data &&
	    yelp_uri_equal (YELP_URI (priv->current->data), uri)) {
		return;
	}

	if (yelp_history_exist_forward (history)) {
		forward_list = priv->current->next;
		priv->current->next = NULL;
			
		history_free_history_list (forward_list);
	}

 	priv->history_list = g_list_append (priv->history_list, 
					    yelp_uri_ref (uri));
	
	priv->current = g_list_last (priv->history_list);
	
	history_maybe_emit (history);
}

YelpURI *
yelp_history_go_forward (YelpHistory *history)
{
	YelpHistoryPriv *priv;
        
	g_return_val_if_fail (history != NULL, NULL);
	g_return_val_if_fail (YELP_IS_HISTORY (history), NULL);

	priv = history->priv;
        
	if (priv->current->next) {
		priv->current = priv->current->next;

		history_maybe_emit (history);
		
		return YELP_URI (priv->current->data);
	}

	return NULL;
}

YelpURI *
yelp_history_go_back (YelpHistory *history)
{
	YelpHistoryPriv *priv;
	
	g_return_val_if_fail (history != NULL, NULL);
	g_return_val_if_fail (YELP_IS_HISTORY (history), NULL);

	priv = history->priv;
        
	if (priv->current->prev) {
		priv->current = priv->current->prev;

		history_maybe_emit (history);

		return YELP_URI (priv->current->data);
	}
        
	return NULL;
}

YelpURI *
yelp_history_get_current (YelpHistory *history)
{
	YelpHistoryPriv *priv;
	
	g_return_val_if_fail (history != NULL, NULL);
	g_return_val_if_fail (YELP_IS_HISTORY (history), NULL);

	priv = history->priv;
	
	if (!priv->current) {
		return NULL;
	}

	return YELP_URI (priv->current->data);
}

gboolean
yelp_history_exist_forward (YelpHistory *history)
{
	YelpHistoryPriv *priv;
        
	g_return_val_if_fail (history != NULL, FALSE);
	g_return_val_if_fail (YELP_IS_HISTORY (history), FALSE);
        
	priv = history->priv;
        
	if (!priv->current) {
		return FALSE;
	}

	if (priv->current->next) {
		return TRUE;
	}

	return FALSE;
}

gboolean
yelp_history_exist_back (YelpHistory *history)
{
	YelpHistoryPriv *priv;
        
	g_return_val_if_fail (history != NULL, FALSE);
	g_return_val_if_fail (YELP_IS_HISTORY (history), FALSE);

	priv = history->priv;
        
	if (!priv->current) {
		return FALSE;
	}
        
	if (priv->current->prev) {
		return TRUE;
	}
        
	return FALSE;
}

YelpHistory *
yelp_history_new ()
{
	return g_object_new (YELP_TYPE_HISTORY, NULL);
}
