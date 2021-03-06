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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"
#include "intl.h"
#include "status.h"
#include "service.h"
#include "globals.h"
#include "value_pair.h"
#include "util.h"
#include "add_contact_window.h"
#include "edit_local_accounts.h"
#include "plugin_api.h"
#include "messages.h"

#ifndef NAME_MAX
#define NAME_MAX 4096
#endif

enum { NONE, ROT13, XOR13, MAX_ENC };
const char *decode_password(const char *pass_in, int enc_type)
{
	static char pwd[1024];
	int i;
	for (i = 0; pass_in[i] && i < 1023; i++) {
		pwd[i] = pass_in[i];
		if (enc_type == ROT13) {
			if (pwd[i] >= 'a' && pwd[i] <= 'z') {
				pwd[i] = (pwd[i] - 'a' + 13) % 26 + 'a';
			} else if (pwd[i] >= 'A' && pwd[i] <= 'Z') {
				pwd[i] = (pwd[i] - 'A' + 13) % 26 + 'A';
			} else if (pwd[i] >= '0' && pwd[i] <= '9') {
				pwd[i] = (pwd[i] - '0' + 5) % 10 + '0';
			}
		} else if (enc_type == XOR13) {
			if (pwd[i] >= 'a' && pwd[i] <= 'x') {
				pwd[i] = ((pwd[i] - 'a') ^ 2) + 'a';
			} else if (pwd[i] >= 'x' && pwd[i] <= 'z') {
				pwd[i] = ((pwd[i] - 'a') ^ 1) + 'a';
			} else if (pwd[i] >= 'A' && pwd[i] <= 'X') {
				pwd[i] = ((pwd[i] - 'A') ^ 2) + 'A';
			} else if (pwd[i] >= 'X' && pwd[i] <= 'Z') {
				pwd[i] = ((pwd[i] - 'A') ^ 1) + 'A';
			} else if (pwd[i] >= '0' && pwd[i] <= '7') {
				pwd[i] = ((pwd[i] - '0') ^ 2) + '0';
			} else if (pwd[i] >= '8' && pwd[i] <= '9') {
				pwd[i] = ((pwd[i] - '0') ^ 1) + '0';
			}
		}
	}

	pwd[i] = '\0';

	return pwd;
}

void write_account_list()
{
	FILE *fp;
	char buff2[1024];
	LList *l1;

	snprintf(buff2, 1024, "%saccounts", config_dir);
	if (!(fp = fdopen(creat(buff2, S_IRWXU), "w"))) {
		ay_do_error(_("Account creation problem"),
			_("Cannot open account file !"));
		return;
	}
	fprintf(fp, "# Ayttm's Local Account file\n"
		"# Edit only if you know what you're doing\n"
		"# Passwords are obfuscated, not encrypted\n" "\n");

	for (l1 = accounts; l1; l1 = l1->next) {
		eb_local_account *ela = l1->data;

		/*
		 * This is the deal, what a protocol stores as account data is 
		 * protocol specific, so you just query for the values you need to write
		 * cool stuff :-)
		 */

		LList *config = RUN_SERVICE(ela)->write_local_config(ela);
		char *pwd = value_pair_get_value(config, "PASSWORD");
		int enc_type =
			1 + (int)(rand() / (RAND_MAX + 1.0) * (MAX_ENC - 1));
		char e[2];
		snprintf(e, sizeof(e), "%d", enc_type);
		config = value_pair_add(config, "enc_type", e);
		config = value_pair_add(config, "password_encoded",
			decode_password(pwd, enc_type));
		config = value_pair_remove(config, "PASSWORD");
		free(pwd);
		fprintf(fp, "<ACCOUNT %s>\n",
			eb_services[ela->service_id].name);
		value_pair_print_values(config, fp, 1);
		fprintf(fp, "</ACCOUNT>\n");
		value_pair_free(config);
	}

	fclose(fp);
}

/*
 * Saves the contact list to (the list of people you see go on and off
 * line)
 */

void write_contact_list()
{
	FILE *fp;
	char buff2[1024];
	LList *l1;
	LList *l2;
	LList *l3;

	/*
	 * The contact list is a 3 dimensional linked list, at the top
	 * level you have a list of groups, each group can have several
	 * contacts, each contact can have several accounts.  Please see
	 * the Nomenclature file in the docs directory for details
	 */

	snprintf(buff2, 1024, "%scontacts", config_dir);
	if (!(fp = fdopen(creat(buff2, 0700), "w")))
		return;

	for (l1 = groups; l1; l1 = l1->next) {
		fprintf(fp, "<GROUP>\n\tNAME=\"%s\"\n",
			((grouplist *)l1->data)->name);
		for (l2 = ((grouplist *)l1->data)->members; l2; l2 = l2->next) {
			struct contact *c = (struct contact *)l2->data;
			char *strbuf = NULL;
			fprintf(fp,
				"\t<CONTACT>\n\t\tNAME=\"%s\"\n\t\tDEFAULT_PROTOCOL=\"%s\"\n\t\tLANGUAGE=\"%s\"\n",
				c->nick, eb_services[c->default_chatb].name,
				c->language);
			strbuf = escape_string(c->trigger.param);
			fprintf(fp,
				"\t\tTRIGGER_TYPE=\"%s\"\n\t\tTRIGGER_ACTION=\"%s\"\n\t\tTRIGGER_PARAM=\"%s\"\n",
				get_trigger_type_text(c->trigger.type),
				get_trigger_action_text(c->trigger.action),
				strbuf);
			g_free(strbuf);

			fprintf(fp, "\t\tGPG_KEY=\"%s\"\n",
				(c->gpg_key != NULL) ? c->gpg_key : "");
			fprintf(fp, "\t\tGPG_CRYPT=\"%d\"\n",
				c->gpg_do_encryption);
			fprintf(fp, "\t\tGPG_SIGN=\"%d\"\n",
				c->gpg_do_signature);

			for (l3 = ((struct contact *)l2->data)->accounts; l3;
				l3 = l3->next) {
				eb_account *account = (eb_account *)l3->data;
				fprintf(fp, "\t\t<ACCOUNT %s>\n"
					"\t\t\tNAME=\"%s\"\n"
					"\t\t\tLOCAL_ACCOUNT=\"%s\"\n"
					"\t\t</ACCOUNT>\n",
					eb_services[account->service_id].name,
					account->handle,
					account->ela ? account->ela->
					handle : "");

			}
			fprintf(fp, "\t</CONTACT>\n");
		}
		fprintf(fp, "</GROUP>\n");
	}

	fclose(fp);
}

/*
 * To load the accounts file we currently use flex and bison for
 * speed and robustness.... so all that we have to do is just
 * call account parse after opening the file and we are home free
 */

int load_accounts_from_file(const char *file)
{
	FILE *fp;
	extern int accountparse();
	extern FILE *accountin;
	/*char buff[1024];
	   LList *accounts_old = accounts; */

	if (!(fp = fopen(file, "r")))
		return 0;

	accountin = fp;
	accountparse();
	eb_debug(DBG_CORE, "closing fp\n");
	fclose(fp);
	/*if (accounts_old) {
	   LList *walk = accounts_old;
	   for (; walk; walk = walk->next) 
	   accounts = l_list_append(accounts, walk->data);
	   }
	   fp = NULL; */
/*	g_snprintf(buff, 1024, "%saccounts", eb_config_dir());
	fp = fdopen(creat(buff, 0700), "w");
	while (fp && naccounts) {
		LList *config = NULL;
		eb_local_account *ela = (eb_local_account *)(naccounts->data);
		config = RUN_SERVICE(ela)->write_local_config(ela);

		fprintf(fp, "<ACCOUNT %s>\n", get_service_name(ela->service_id));
		value_pair_print_values(config, fp, 1);
		fprintf(fp, "</ACCOUNT>\n");

		naccounts = naccounts->next;
	}
	fclose(fp);
*/
	ay_set_submenus();
	return accounts != NULL;
}

int load_accounts()
{
	char buff2[1024];
	snprintf(buff2, 1024, "%saccounts", config_dir);
	accounts = NULL;
	return load_accounts_from_file(buff2);
}

/*
 * to load the contact list we also use flex and bison, so we just
 * call the parser,  (isn't life so simple now :) )
 */

int load_contacts_from_file(const char *file)
{
	FILE *fp;
	extern int contactparse();
	extern FILE *contactin;
	LList *cts = NULL;

	if (!(fp = fopen(file, "r")))
		return 0;
	contactin = fp;

	contactparse();

	fclose(fp);

	/* rename logs from old format (contact->nick) to new 
	   (contact->nick "-" contact->group->name) */

	if (temp_groups && groups) {
		while (temp_groups) {
			grouplist *grp = (grouplist *)temp_groups->data;
			grouplist *oldgrp = NULL;
			if ((oldgrp = find_grouplist_by_name(grp->name)) ==
				NULL) {
				eb_debug(DBG_CORE, "adding group %s\n",
					grp->name);
				add_group(grp->name);
				oldgrp = find_grouplist_by_name(grp->name);
			}
			while (grp->members) {
				struct contact *con =
					(struct contact *)grp->members->data;
				if (!find_contact_in_group_by_nick(con->nick,
						oldgrp)) {
					LList *w = con->accounts;
					int sid = 0;
					while (w) {
						eb_account *ea =
							(eb_account *)w->data;
						if (find_account_by_handle(ea->
								handle,
								ea->
								service_id)) {
							con->accounts =
								l_list_remove
								(con->accounts,
								ea);
							w = con->accounts;
						} else {
							sid = ea->service_id;
							w = w->next;
						}
					}
					if (!l_list_empty(con->accounts)
						&& con->accounts->data) {
						eb_debug(DBG_CORE,
							" adding contact %s\n",
							con->nick);
						add_new_contact(grp->name,
							con->nick, sid);
						w = con->accounts;
						while (w) {
							add_account_silent(con->
								nick,
								(eb_account *)
								w->data);
							eb_debug(DBG_CORE,
								"  adding account %s\n",
								((eb_account *)
									w->
									data)->
								handle);
							w = w->next;
						}
					}
				} else {
					while (con->accounts) {
						eb_account *ea =
							(eb_account *)con->
							accounts->data;
						if (!find_account_by_handle(ea->
								handle,
								ea->
								service_id)) {
							add_account_silent(con->
								nick, ea);
							eb_debug(DBG_CORE,
								"  adding account to ex.ct %s\n",
								ea->handle);
						}
						con->accounts =
							con->accounts->next;
					}
				}
				grp->members = grp->members->next;
			}

			temp_groups = temp_groups->next;
		}
	} else if (temp_groups) {
		eb_debug(DBG_CORE, "First pass\n");
		groups = temp_groups;
	}

	if (groups) {
		update_contact_list();
		write_contact_list();
	}

	cts = get_all_contacts();
	for (; cts && cts->data; cts = cts->next) {
		struct contact *c = (struct contact *)cts->data;
		FILE *test = NULL;
		char buff[NAME_MAX];
		eb_debug(DBG_CORE, "contact:%s\n", c->nick);
		make_safe_filename(buff, c->nick, c->group->name);
		if ((test = fopen(buff, "r")) != NULL)
			fclose(test);
		else
			rename_nick_log(NULL, c->nick, c->group->name, c->nick);
	}
	return 1;
}

int load_contacts()
{
	char buff2[1024];
	int result;
	snprintf(buff2, 1024, "%scontacts", config_dir);

	result = load_contacts_from_file(buff2);
	if (!groups)
		groups = temp_groups;

	return result;
}

eb_account *dummy_account(char *handle, char *group, eb_local_account *ela)
{
	eb_account *dum = g_new0(eb_account, 1);
	dum->account_contact = g_new0(struct contact, 1);
	dum->account_contact->group = g_new0(grouplist, 1);

	strncpy(dum->handle, handle, sizeof(dum->handle));
	strncpy(dum->account_contact->group->name, group,
		sizeof(dum->account_contact->group->name));
	dum->service_id = ela->service_id;
	dum->ela = ela;

	return dum;
}
