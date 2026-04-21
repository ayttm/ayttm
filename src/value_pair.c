/*
 * Ayttm 
 *
 * Copyright (C) 2003, the Ayttm team
 * 
 * Ayttm is derivative of Everybuddy
 * Copyright (C) 1999-2002, Torrey Searle <tsearle@uci.edu>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <string.h>
#include <stdlib.h>
#include "value_pair.h"
#include "util.h"

char *value_pair_get_value(GList *pairs, const char *key)
{
	GList *node;
	for (node = pairs; node; node = node->next) {
		value_pair *vp = node->data;
		if (!strcasecmp(key, vp->key)) {
			if (!vp->value)
				return g_strdup("");
			else
				return g_strdup(vp->value);
		}
	}
	return NULL;
}

void value_pair_free(GList *pairs)
{
	GList *node;
	for (node = pairs; node; node = node->next)
		g_free(node->data);
	g_list_free(pairs);
}

void value_pair_print_values(GList *pairs, FILE *file, int indent)
{
	GList *node;
	int i;
	char *tmp;

	for (node = pairs; node; node = node->next) {
		value_pair *vp = node->data;

		for (i = 0; i < indent; i++)
			fprintf(file, "\t");

		tmp = escape_string(vp->value);
		fprintf(file, "%s=\"%s\"\n", vp->key, tmp);
		g_free(tmp);
	}
}

GList *value_pair_add(GList *list, const char *key, const char *value)
{
	value_pair *vp = NULL;
	char *old_value = NULL;

	old_value = value_pair_get_value(list, key);

	if (old_value != NULL) {
		list = value_pair_remove(list, key);
		g_free(old_value);
	}

	vp = g_new0(value_pair, 1);
	g_strlcpy(vp->key, key, MAX_PREF_NAME_LEN);
	g_strlcpy(vp->value, value, MAX_PREF_LEN);

	return g_list_append(list, vp);
}

GList *value_pair_remove(GList *pairs, const char *key)
{
	GList *node;
	for (node = pairs; node; node = node->next) {
		value_pair *vp = node->data;
		if (vp && !strcasecmp(key, vp->key)) {
			pairs = g_list_remove_link(pairs, node);
			g_free(node->data);
			g_list_free_1(node);
			break;
		}
	}
	return pairs;
}

GList *value_pair_update(GList *pairs, GList *new_list)
{

	GList *node;
	for (node = new_list; node; node = node->next) {
		value_pair *vp = node->data;
		pairs = value_pair_remove(pairs, vp->key);
		pairs = value_pair_add(pairs, vp->key, vp->value);
	}
	return pairs;
}
