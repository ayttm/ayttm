/*
 * Ayttm 
 *
 * Copyright (C) 2003, the Ayttm team
 * 
 * Ayttm is a derivative of Everybuddy
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

#include "intl.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef __MINGW32__
#include <sys/stat.h>
#endif

#include "util.h"
#include "account.h"
#include "service.h"
#include "value_pair.h"
#include "globals.h"
#include "defaults.h"
#include "libproxy/networking.h"
#include "plugin.h"
#include "prefs.h"
#include "messages.h"
#include "status.h"

#include "ui_prefs_window.h"

enum {
	CORE_PREF,
	PLUGIN_PREF,
	SERVICE_PREF
};

typedef struct _ptr_list {
	char key[MAX_PREF_NAME_LEN + 1];
	void *value;
} ptr_list;

static GList *s_global_prefs = NULL;

void ayttm_prefs_read_file(char *file);

static int s_compare_ptr_key(const void *a, const void *b)
{
	if (!strcmp(((ptr_list *)a)->key, (const char *)b))
		return (0);

	return (1);
}

static void s_add_pref(const char *key, void *data)
{
	ptr_list *pref_data = g_new0(ptr_list, 1);

	g_strlcpy(pref_data->key, key, sizeof(pref_data->key));
	pref_data->value = (void *)data;

	s_global_prefs = g_list_append(s_global_prefs, pref_data);
}

static char *s_strip_whitespace(char *inStr)
{
	char *ptr = inStr;
	int len = strlen(inStr);

	if ((inStr == NULL) || (inStr[0] == '\0'))
		return (inStr);

	while (isspace(*ptr))
		ptr++;

	if (ptr != inStr) {
		const int new_len = len - (ptr - inStr);

		memmove(inStr, ptr, new_len);
		inStr[new_len] = '\0';
	}

	len = strlen(inStr) - 1;
	ptr = inStr + len;

	while (isspace(*ptr))
		ptr--;

	if (ptr != (inStr + len)) {
		ptr++;
		*ptr = '\0';
	}

	return (inStr);
}



AyModulePrefs *ay_prefs_sift_modules(void)
{
	const GList *plugins = GetPref(EB_PLUGIN_LIST);
	AyModulePrefs *out = g_new0(AyModulePrefs, 1);

	for (; plugins; plugins = plugins->next) {
		eb_PLUGIN_INFO *plugin_info = plugins->data;

		if (plugin_info == NULL)
			continue;

		switch (plugin_info->pi.type)
		{
		case PLUGIN_SERVICE:
			out->services = g_list_append(out->services, plugin_info);
			break;
		case PLUGIN_FILTER:
			out->filters = g_list_append(out->filters, plugin_info);
			break;
		case PLUGIN_UTILITY:
		case PLUGIN_UNKNOWN:
			out->utilities = g_list_append(out->utilities, plugin_info);
			break;
		case PLUGIN_SMILEY:
			out->smileys = g_list_append(out->smileys, plugin_info);
			break;
		case PLUGIN_IMPORTER:
			out->importers = g_list_append(out->importers, plugin_info);
			break;
		}
	}

	return out;
}

void ay_prefs_modules_free(AyModulePrefs *modules)
{
	g_list_free(modules->services);
	g_list_free(modules->filters);
	g_list_free(modules->utilities);
	g_list_free(modules->smileys);
	g_list_free(modules->importers);

	g_free(modules);
}

static void s_write_module_prefs(void *inListItem, void *inData)
{
	eb_PLUGIN_INFO *plugin_info = inListItem;
	FILE *fp = (FILE *)inData;
	GList *master_prefs = NULL;
	GList *current_prefs = NULL;

	eb_debug(DBG_CORE, "Writing prefs for %s\n", plugin_info->name);

	fprintf(fp, "\t%s\n", plugin_info->name);

	master_prefs = GetPref(plugin_info->name);
	master_prefs = value_pair_remove(master_prefs, "load");

	current_prefs = eb_input_to_value_pair(plugin_info->pi.prefs);

	if (plugin_info->status == PLUGIN_LOADED)
		current_prefs = value_pair_add(current_prefs, "load", "1");
	else
		current_prefs = value_pair_add(current_prefs, "load", "0");

	master_prefs = value_pair_update(master_prefs, current_prefs);

	SetPref(plugin_info->name, master_prefs);

	value_pair_print_values(master_prefs, fp, 2);

	fprintf(fp, "\tend\n");

	value_pair_free(current_prefs);
}

void ayttm_prefs_init(void)
{
#ifdef __MINGW32__
	const int buff_size = 1024;
	char buff[buff_size];
	const char *base_dir = "\\Program Files\\ayttm\\";
#endif

	/* set default prefs */

	/* window positions, etc */
	iSetLocalPref("length_contact_window", 256);
	iSetLocalPref("width_contact_window", 150);
	iSetLocalPref("status_show_level", 2);

	/* chat */
	iSetLocalPref("do_typing_notify", 1);
	iSetLocalPref("do_send_typing_notify", 1);
	iSetLocalPref("do_escape_close", 1);
	iSetLocalPref("do_convo_timestamp", 0);
	iSetLocalPref("do_enter_send", 1);
	iSetLocalPref("do_ignore_unknown", 0);
	iSetLocalPref("do_multi_line", 1);
	iSetLocalPref("do_raise_window", 0);
	cSetLocalPref("regex_pattern", "");
	iSetLocalPref("do_send_idle_time", 0);
	iSetLocalPref("do_timestamp", 0);
	iSetLocalPref("do_ignore_fore", 1);
	iSetLocalPref("do_ignore_back", 1);
	iSetLocalPref("do_ignore_font", 1);
	iSetLocalPref("do_auto_complete", 0);
	iSetLocalPref("do_smiley", 1);
	iSetLocalPref("do_spell_checking", 0);
	cSetLocalPref("spell_dictionary", "");
	cSetLocalPref("FontFace", "helvetica");
	cSetLocalPref("encodings", "CP1252 ISO-8859-15 ISO-8859-2");

	/* logging */
	iSetLocalPref("do_logging", 1);
	iSetLocalPref("do_strip_html", 0);
	iSetLocalPref("do_restore_last_conv", 0);

	/* tabs */
	iSetLocalPref("do_tabbed_chat", 0);
	iSetLocalPref("do_tabbed_chat_orient", 0);	/* Tab Orientation:  0 => bottom, 1 => top, 2=> left, 3 => right */

	/* NOTE that the way the defaults for accelerators are stored is likely GTK-specific */
	cSetLocalPref("accel_prev_tab", "<Control>Left");
	cSetLocalPref("accel_next_tab", "<Control>Right");

	/* sound */
	iSetLocalPref("do_no_sound_when_away", 0);
	iSetLocalPref("do_no_sound_for_ignore", 1);
	iSetLocalPref("do_online_sound", 1);
	iSetLocalPref("do_play_send", 1);
	iSetLocalPref("do_play_first", 1);
	iSetLocalPref("do_play_receive", 1);

	/* version checking */
	cSetLocalPref("last_warned_version", "0.0.0");

	ayttm_prefs_read_file(AYTTMRC);

	cSetLocalPref("BuddyArriveFilename",
		cGetLocalPref("default_arrive_snd"));
	cSetLocalPref("BuddyAwayFilename", cGetLocalPref("default_leave_snd"));
	cSetLocalPref("BuddyLeaveFilename", cGetLocalPref("default_leave_snd"));
	cSetLocalPref("SendFilename", cGetLocalPref("default_send_snd"));
	cSetLocalPref("ReceiveFilename", cGetLocalPref("default_recv_snd"));
	cSetLocalPref("FirstMsgFilename", cGetLocalPref("default_recv_snd"));

	fSetLocalPref("SoundVolume", 0.0);

	/* misc */
	iSetLocalPref("do_ayttm_debug", 0);
	iSetLocalPref("do_ayttm_debug_html", 0);
	iSetLocalPref("do_plugin_debug", 0);
	iSetLocalPref("do_noautoresize", 0);
	iSetLocalPref("do_show_tooltips", 1);
	iSetLocalPref("use_alternate_browser", 0);
	cSetLocalPref("alternate_browser", "");
	iSetLocalPref("do_version_check", 0);
	iSetLocalPref("usercount_window_seen", 0);

	/* advanced */
	iSetLocalPref("proxy_type", PROXY_NONE);
	cSetLocalPref("proxy_host", "");
	iSetLocalPref("proxy_port", 3128);
	iSetLocalPref("do_proxy_auth", 0);
	cSetLocalPref("proxy_user", "");
	cSetLocalPref("proxy_password", "");

	iSetLocalPref("use_recoding", 0);
	cSetLocalPref("local_encoding", "");
	cSetLocalPref("remote_encoding", "");

	/* modules */
	cSetLocalPref("modules_path", cGetLocalPref("default_module_path"));
}

void ayttm_prefs_read_file(char *file)
{
	const int buffer_size = 1024;
	char buff[buffer_size];
	char *const param = buff;	/* just another name for buff... */
	FILE *fp = NULL;

	snprintf(buff, buffer_size, "%s", file);

	fp = fopen(buff, "r");

	if (fp == NULL) {
		char tmp[1024];
		snprintf(tmp, 1024, "%sprefs", config_dir);
		if (!strcmp(file, tmp)) {
			printf("Creating prefs file [%s]\n", buff);
			ayttm_prefs_write();
		}
		return;
	}

	(void)fgets(param, buffer_size, fp);

	while (!feof(fp)) {
		int pref_type = CORE_PREF;
		char *val = buff;

		s_strip_whitespace(param);

		if (!strcasecmp(param, "plugins"))
			pref_type = PLUGIN_PREF;
		else if (!strcasecmp(param, "connections"))
			pref_type = SERVICE_PREF;

		if (pref_type != CORE_PREF) {
			for (;;) {
				int id = -1;
				char *plugin_name = NULL;
				GList *session_prefs = NULL;

				(void)fgets(param, buffer_size, fp);

				s_strip_whitespace(param);

				if (!strcasecmp(param, "end"))
					break;

				switch (pref_type) {
				case PLUGIN_PREF:
					plugin_name = g_strdup(param);
					break;

				case SERVICE_PREF:
					id = get_service_id(param);
					break;

				default:
					assert(FALSE);
					break;
				}

				for (;;) {
					GList *old_session_prefs = NULL;

					(void)fgets(param, buffer_size, fp);

					s_strip_whitespace(param);

					if (!strcasecmp(param, "end")) {
						switch (pref_type) {
						case PLUGIN_PREF:
							old_session_prefs =
								SetPref
								(plugin_name,
								session_prefs);

							g_free(plugin_name);
							break;

						case SERVICE_PREF:
							old_session_prefs =
								SetPref
								(get_service_name
								(id),
								session_prefs);
							break;

						default:
							assert(FALSE);
							break;
						}

						if (old_session_prefs != NULL) {
							eb_debug(DBG_CORE,
								"Freeing old_session_prefs\n");
							value_pair_free
								(old_session_prefs);
						}
						break;
					} else {
						val = param;

						while (*val != 0 && *val != '=')
							val++;

						if (*val == '=') {
							*val = '\0';
							val++;
						}

						/* strip off quotes */
						if (*val == '"') {
							val++;
							val[strlen(val) - 1] =
								'\0';
						}

						eb_debug(DBG_CORE,
							"Adding %s:%s to session_prefs\n",
							param, val);
						session_prefs =
							value_pair_add
							(session_prefs, param,
							val);
					}
				}
			}
			continue;
		}
		/* if(pref_type != CORE_PREF) */
		val = param;

		while (*val != 0 && *val != '=')
			val++;

		if (*val == '=') {
			*val = '\0';
			val++;
		}

		cSetLocalPref(param, val);

		(void)fgets(param, buffer_size, fp);
	}

	fclose(fp);

	if (iGetLocalPref("do_proxy_auth") != 0)
		ay_proxy_set_default(iGetLocalPref("proxy_type"),
			cGetLocalPref("proxy_host"),
			iGetLocalPref("proxy_port"),
			cGetLocalPref("proxy_user"),
			cGetLocalPref("proxy_password"));
	else
		ay_proxy_set_default(iGetLocalPref("proxy_type"),
			cGetLocalPref("proxy_host"),
			iGetLocalPref("proxy_port"), NULL, NULL);
}

void ayttm_prefs_read(void)
{
	const int buffer_size = 1024;
	char buff[buffer_size];
	snprintf(buff, buffer_size, "%sprefs", config_dir);
	ayttm_prefs_read_file(buff);
}

void ayttm_prefs_write(void)
{
	const int bufferLen = 1024;
	char buff[bufferLen];
	char file[bufferLen];
	FILE *fp = NULL;

	snprintf(buff, bufferLen, "%sprefs.tmp", config_dir);
	snprintf(file, bufferLen, "%sprefs", config_dir);

	fp = fopen(buff, "w");

	/* window positions, etc. */
	fprintf(fp, "x_contact_window=%d\n", iGetLocalPref("x_contact_window"));
	fprintf(fp, "y_contact_window=%d\n", iGetLocalPref("y_contact_window"));
	fprintf(fp, "status_show_level=%d\n",
		iGetLocalPref("status_show_level"));

	/* chat */
	fprintf(fp, "do_typing_notify=%d\n", iGetLocalPref("do_typing_notify"));
	fprintf(fp, "do_send_typing_notify=%d\n",
		iGetLocalPref("do_send_typing_notify"));
	fprintf(fp, "do_escape_close=%d\n", iGetLocalPref("do_escape_close"));
	fprintf(fp, "do_convo_timestamp=%d\n",
		iGetLocalPref("do_convo_timestamp"));
	fprintf(fp, "do_enter_send=%d\n", iGetLocalPref("do_enter_send"));
	fprintf(fp, "do_ignore_unknown=%d\n",
		iGetLocalPref("do_ignore_unknown"));
	fprintf(fp, "do_multi_line=%d\n", iGetLocalPref("do_multi_line"));
	fprintf(fp, "do_raise_window=%d\n", iGetLocalPref("do_raise_window"));
	fprintf(fp, "regex_pattern=%s\n", cGetLocalPref("regex_pattern"));
	fprintf(fp, "do_smiley=%d\n", iGetLocalPref("do_smiley"));
	fprintf(fp, "do_send_idle_time=%d\n",
		iGetLocalPref("do_send_idle_time"));
	fprintf(fp, "do_timestamp=%d\n", iGetLocalPref("do_timestamp"));
	fprintf(fp, "do_ignore_fore=%d\n", iGetLocalPref("do_ignore_fore"));
	fprintf(fp, "do_ignore_back=%d\n", iGetLocalPref("do_ignore_back"));
	fprintf(fp, "do_ignore_font=%d\n", iGetLocalPref("do_ignore_font"));
	fprintf(fp, "do_auto_complete=%d\n", iGetLocalPref("do_auto_complete"));
	fprintf(fp, "FontFace=%s\n", cGetLocalPref("FontFace"));
	fprintf(fp, "encodings=%s\n", cGetLocalPref("encodings"));
#ifdef HAVE_LIBENCHANT
	fprintf(fp, "do_spell_checking=%d\n",
		iGetLocalPref("do_spell_checking"));
	fprintf(fp, "spell_dictionary=%s\n", cGetLocalPref("spell_dictionary"));
#endif

	/* logging */
	fprintf(fp, "do_logging=%d\n", iGetLocalPref("do_logging"));
	fprintf(fp, "do_strip_html=%d\n", iGetLocalPref("do_strip_html"));
	fprintf(fp, "do_restore_last_conv=%d\n",
		iGetLocalPref("do_restore_last_conv"));

	/* tabs */
	fprintf(fp, "do_tabbed_chat=%d\n", iGetLocalPref("do_tabbed_chat"));
	fprintf(fp, "do_tabbed_chat_orient=%d\n",
		iGetLocalPref("do_tabbed_chat_orient"));
	fprintf(fp, "accel_next_tab=%s\n", cGetLocalPref("accel_next_tab"));
	fprintf(fp, "accel_prev_tab=%s\n", cGetLocalPref("accel_prev_tab"));

	/* sound */
	fprintf(fp, "do_no_sound_when_away=%d\n",
		iGetLocalPref("do_no_sound_when_away"));
	fprintf(fp, "do_no_sound_for_ignore=%d\n",
		iGetLocalPref("do_no_sound_for_ignore"));
	fprintf(fp, "do_online_sound=%d\n", iGetLocalPref("do_online_sound"));
	fprintf(fp, "do_play_send=%d\n", iGetLocalPref("do_play_send"));
	fprintf(fp, "do_play_first=%d\n", iGetLocalPref("do_play_first"));
	fprintf(fp, "do_play_receive=%d\n", iGetLocalPref("do_play_receive"));
	fprintf(fp, "BuddyArriveFilename=%s\n",
		cGetLocalPref("BuddyArriveFilename"));
	fprintf(fp, "BuddyAwayFilename=%s\n",
		cGetLocalPref("BuddyAwayFilename"));
	fprintf(fp, "BuddyLeaveFilename=%s\n",
		cGetLocalPref("BuddyLeaveFilename"));
	fprintf(fp, "SendFilename=%s\n", cGetLocalPref("SendFilename"));
	fprintf(fp, "ReceiveFilename=%s\n", cGetLocalPref("ReceiveFilename"));
	fprintf(fp, "FirstMsgFilename=%s\n", cGetLocalPref("FirstMsgFilename"));
	fprintf(fp, "SoundVolume=%f\n", fGetLocalPref("SoundVolume"));

	/* misc */
	fprintf(fp, "do_ayttm_debug=%d\n", iGetLocalPref("do_ayttm_debug"));
	fprintf(fp, "do_ayttm_debug_html=%d\n",
		iGetLocalPref("do_ayttm_debug_html"));
	fprintf(fp, "do_plugin_debug=%d\n", iGetLocalPref("do_ayttm_debug"));
	fprintf(fp, "do_version_check=%d\n", iGetLocalPref("do_version_check"));
	fprintf(fp, "usercount_window_seen=%d\n",
		iGetLocalPref("usercount_window_seen"));
	fprintf(fp, "length_contact_window=%d\n",
		iGetLocalPref("length_contact_window"));
	fprintf(fp, "width_contact_window=%d\n",
		iGetLocalPref("width_contact_window"));
	fprintf(fp, "do_noautoresize=%d\n", iGetLocalPref("do_noautoresize"));
	fprintf(fp, "do_show_tooltips=%d\n", iGetLocalPref("do_show_tooltips"));
	fprintf(fp, "use_alternate_browser=%d\n",
		iGetLocalPref("use_alternate_browser"));
	fprintf(fp, "alternate_browser=%s\n",
		cGetLocalPref("alternate_browser"));

	/* advanced */
	fprintf(fp, "proxy_type=%d\n", iGetLocalPref("proxy_type"));
	fprintf(fp, "proxy_host=%s\n", cGetLocalPref("proxy_host"));
	fprintf(fp, "proxy_port=%d\n", iGetLocalPref("proxy_port"));
	fprintf(fp, "do_proxy_auth=%d\n", iGetLocalPref("do_proxy_auth"));
	fprintf(fp, "proxy_user=%s\n", cGetLocalPref("proxy_user"));
	fprintf(fp, "proxy_password=%s\n", cGetLocalPref("proxy_password"));

	fprintf(fp, "use_recoding=%d\n", iGetLocalPref("use_recoding"));
	fprintf(fp, "local_encoding=%s\n", cGetLocalPref("local_encoding"));
	fprintf(fp, "remote_encoding=%s\n", cGetLocalPref("remote_encoding"));

	/* version checking */
	fprintf(fp, "last_warned_version=%s\n",
		cGetLocalPref("last_warned_version"));

	/* modules */
	fprintf(fp, "modules_path=%s\n", cGetLocalPref("modules_path"));
	fprintf(fp, "plugins\n");
	g_list_foreach(GetPref(EB_PLUGIN_LIST), (GFunc)s_write_module_prefs, fp);
	fprintf(fp, "end\n");

	fclose(fp);

#ifdef __MINGW32__
	{
		struct stat stb;
		if (stat(file, &stb) == 0)
			unlink(file);
	}
#endif

	rename(buff, file);
}

/* Find old pref data, and replace with new, returning old data */
void *SetPref(const char *key, void *data)
{
	ptr_list *pref_data = NULL;
	void *old_data = NULL;
	GList *list_data =
		g_list_find_custom(s_global_prefs, key, (GCompareFunc)s_compare_ptr_key);

	if (!list_data) {
		s_add_pref(key, data);

		return (NULL);
	}

	pref_data = (ptr_list *)list_data->data;
	old_data = pref_data->value;
	pref_data->value = data;

	return (old_data);
}

void *GetPref(const char *key)
{
	ptr_list *pref_data = NULL;
	GList *list_data =
		g_list_find_custom(s_global_prefs, key, (GCompareFunc)s_compare_ptr_key);

	if (!list_data)
		return (NULL);

	pref_data = list_data->data;

	if (!pref_data)
		return (NULL);

	return (pref_data->value);
}

void cSetLocalPref(const char *key, const char *data)
{
	char newkey[MAX_PREF_NAME_LEN];
	char *oldvalue = NULL;

	snprintf(newkey, MAX_PREF_NAME_LEN, "Local::%s", key);

	oldvalue = SetPref(newkey, g_strdup(data));

	if (oldvalue)
		g_free(oldvalue);
}

void iSetLocalPref(const char *key, int data)
{
	char value[MAX_PREF_LEN];

	snprintf(value, MAX_PREF_LEN, "%i", data);
	cSetLocalPref(key, value);
}

void fSetLocalPref(const char *key, float data)
{
	char value[MAX_PREF_LEN];

	snprintf(value, MAX_PREF_LEN, "%f", data);
	cSetLocalPref(key, value);
}

char *cGetLocalPref(const char *key)
{
	char newkey[MAX_PREF_NAME_LEN];
	char *value = NULL;

	snprintf(newkey, MAX_PREF_NAME_LEN, "Local::%s", key);

	value = (char *)GetPref(newkey);

	if (!value)
		value = "";

	return (value);
}

float fGetLocalPref(const char *key)
{
	float value = 0.0;

	value = atof(cGetLocalPref(key));

	return (value);
}

int iGetLocalPref(const char *key)
{
	int value = 0;

	value = atoi(cGetLocalPref(key));

	return (value);
}

/* Used when loading service modules later, so passwords and user names are still available
 * as service:username */
void save_account_info(const char *service, GList *pairs)
{
	const int buffer_size = 256;
	char buff[buffer_size];
	char *val = value_pair_get_value(pairs, "SCREEN_NAME");

	assert(val != NULL);

	snprintf(buff, buffer_size, "%s:%s", service, val);

	g_free(val);

	SetPref(buff, pairs);
}
