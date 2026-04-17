/*
 * externs.h
 */

#ifndef __externs_h__
#define __externs_h__

#include <glib.h>

#if defined(__MINGW32__) && defined(__IN_PLUGIN__)
__declspec(dllimport)
GList *outgoing_message_filters;
__declspec(dllimport)
GList *incoming_message_filters;
__declspec(dllimport)
GList *nick_modify_utility;
#else
extern GList *outgoing_message_filters_local;
extern GList *outgoing_message_filters_remote;
extern GList *incoming_message_filters;
extern GList *nick_modify_utility;
#endif

#endif
