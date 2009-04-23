/*
 * adopted from finch's gntaccount.c
 */

/* finch
 *
 * Finch is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <libpurple/account.h>
#include <libpurple/conversation.h>
#include <libpurple/core.h>
#include <libpurple/debug.h>
#include <libpurple/cipher.h>
#include <libpurple/eventloop.h>
#include <libpurple/ft.h>
#include <libpurple/log.h>
#include <libpurple/notify.h>
#include <libpurple/prefs.h>
#include <libpurple/prpl.h>
#include <libpurple/pounce.h>
#include <libpurple/request.h>
#include <libpurple/savedstatuses.h>
#include <libpurple/sound.h>
#include <libpurple/status.h>
#include <libpurple/util.h>
#include <libpurple/whiteboard.h>
#include <libpurple/network.h>

#include <ruby.h>

extern ID CALL;
extern VALUE cAccount;
extern VALUE new_buddy_handler;

extern VALUE check_callback(VALUE, const char*);

static char *
make_info(PurpleAccount *account, PurpleConnection *gc, const char *remote_user,
          const char *id, const char *alias, const char *msg)
{
	if (msg != NULL && *msg == '\0')
		msg = NULL;

	return g_strdup_printf(_("%s%s%s%s has made %s his or her buddy%s%s"),
	                       remote_user,
	                       (alias != NULL ? " ("  : ""),
	                       (alias != NULL ? alias : ""),
	                       (alias != NULL ? ")"   : ""),
	                       (id != NULL
	                        ? id
	                        : (purple_connection_get_display_name(gc) != NULL
	                           ? purple_connection_get_display_name(gc)
	                           : purple_account_get_username(account))),
	                       (msg != NULL ? ": " : "."),
	                       (msg != NULL ? msg  : ""));
}

static void
notify_added(PurpleAccount *account, const char *remote_user,
			const char *id, const char *alias,
			const char *msg)
{
	char *buffer;
	PurpleConnection *gc;

	gc = purple_account_get_connection(account);

	buffer = make_info(account, gc, remote_user, id, alias, msg);

	purple_notify_info(NULL, NULL, buffer, NULL);

	g_free(buffer);
}

static void
request_add(PurpleAccount *account, const char *remote_user,
		  const char *id, const char *alias,
		  const char *message)
{
	if (new_buddy_handler != Qnil) {
    VALUE args[3];
    args[0] = Data_Wrap_Struct(cAccount, NULL, NULL, account);
    args[1] = rb_str_new2(NULL == remote_user ? "" : remote_user);
    args[2] = rb_str_new2(NULL == message ? "" : message);
    check_callback(new_buddy_handler, "new_buddy_handler");
    VALUE v = rb_funcall2(new_buddy_handler, CALL, 3, args);
    
    if (v != Qnil && v != Qfalse) {
      PurpleConnection *gc = purple_account_get_connection(account);
	    if (g_list_find(purple_connections_get_all(), gc))
	    {
		    purple_blist_request_add_buddy(account, remote_user,
									     NULL, alias);
	    }
    }
  }
}

static void *request_authorize(PurpleAccount *account,
                        const char *remote_user,
                        const char *id,
                        const char *alias,
                        const char *message,
                        gboolean on_list,
                        PurpleAccountRequestAuthorizationCb auth_cb,
                        PurpleAccountRequestAuthorizationCb deny_cb,
                        void *user_data)
{
  if (new_buddy_handler != Qnil) {
    VALUE args[3];
    args[0] = Data_Wrap_Struct(cAccount, NULL, NULL, account);
    args[1] = rb_str_new2(NULL == remote_user ? "" : remote_user);
    args[2] = rb_str_new2(NULL == message ? "" : message);
    VALUE v = rb_funcall2((VALUE)new_buddy_handler, CALL, 3, args);
    
    if (v != Qnil && v != Qfalse) {
      auth_cb(user_data);
	    purple_blist_request_add_buddy(account, remote_user, NULL, alias);
    } else {
      deny_cb(user_data);
    }
  }
  
  return NULL;
}

static void
request_close(void *uihandle)
{
	purple_request_close(PURPLE_REQUEST_ACTION, uihandle);
}

PurpleAccountUiOps account_ops =
{
	notify_added,
	NULL,
	request_add,
	request_authorize,
	request_close,
	NULL,
	NULL,
	NULL,
	NULL
};

