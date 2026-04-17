/*
 * Ayttm 
 *
 * Copyright (C) 2003, the Ayttm team
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "spellcheck.h"
#include <stddef.h>

#ifdef HAVE_LIBENCHANT

#include <enchant.h>
#include <stdlib.h>
#include <string.h>
#include "prefs.h"
#include "globals.h"	// for DBG_CORE
#include "debug.h"

class AySpellChecker {
	EnchantBroker *broker;
	EnchantDict *dict;
	const char *language;

	public:

	AySpellChecker();
	void reload();
	int check(const char * word);
	GList * suggest(const char * word);
	~AySpellChecker();
};


/*
 * Returns the selected language from the first of the following that succeed:
 *  - prefs file
 *  - LC_LANG environment variable
 *  - LC_ALL environment variable
 *  - LANG environment variable
 *  - en_GB
 * Do not free this memory
 */
static const char * get_language()
{
	const char * lang = cGetLocalPref("spell_dictionary");
	const char * env_lang[] = {"LC_LANG", "LC_ALL", "LANG", NULL};
	int i=0;

	while(env_lang[i] && (!lang || !lang[0])) {
		lang = getenv(env_lang[i]); 
		i++;
	}

	if(!lang || !lang[0])
		lang = "en_GB";

	return lang;
}

AySpellChecker::AySpellChecker() : broker(NULL), dict(NULL)
{
	broker = enchant_broker_init();
	reload();
}

void AySpellChecker::reload()
{
	language = get_language();

	if (dict && broker) {
		enchant_broker_free_dict(broker, dict);
		dict = NULL;
	}

	if (broker) {
		dict = enchant_broker_request_dict(broker, language);
		if (!dict)
			eb_debug(DBG_CORE, "Error while loading enchant dictionary for language: %s\n", language);
	}
}

int AySpellChecker::check(const char * word)
{
	if(!word || !dict)
		return 1;
	return enchant_dict_check(dict, word, -1) == 0 ? 0 : 1;
}

GList * AySpellChecker::suggest(const char * word)
{
	if(!word || !dict)
		return NULL;

	size_t n_suggestions = 0;
	char **suggestions = enchant_dict_suggest(dict, word, -1, &n_suggestions);

	GList *words = NULL;
	for (size_t i = 0; i < n_suggestions; i++)
		words = g_list_prepend(words, g_strdup(suggestions[i]));
	words = g_list_reverse(words);

	if (suggestions)
		enchant_dict_free_string_list(dict, suggestions);

	return words;
}

AySpellChecker::~AySpellChecker()
{
	if (dict && broker)
		enchant_broker_free_dict(broker, dict);
	if (broker)
		enchant_broker_free(broker);
}


static AySpellChecker speller;

int ay_spell_check(const char * word)
{
	return speller.check(word);
}

GList * ay_spell_check_suggest(const char * word)
{
	return speller.suggest(word);
}

void ay_spell_check_reload()
{
	speller.reload();
}

#else

int ay_spell_check(const char * word)
{
	return 1;
}

GList * ay_spell_check_suggest(const char * word)
{
	return NULL;
}

void ay_spell_check_reload()
{
}

#endif
