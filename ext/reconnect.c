/*
 * adopted from finch's gntconn.c
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

extern const char* UI_ID;

#define INITIAL_RECON_DELAY_MIN  8000
#define INITIAL_RECON_DELAY_MAX 60000

#define MAX_RECON_DELAY 600000

typedef struct {
	int delay;
	guint timeout;
} FinchAutoRecon;

/**
 * Contains accounts that are auto-reconnecting.
 * The key is a pointer to the PurpleAccount and the
 * value is a pointer to a FinchAutoRecon.
 */
static GHashTable *hash = NULL;

static void
free_auto_recon(gpointer data)
{
	FinchAutoRecon *info = data;

	if (info->timeout != 0)
		g_source_remove(info->timeout);

	g_free(info);
}

static gboolean
do_signon(gpointer data)
{
	PurpleAccount *account = data;
	FinchAutoRecon *info;
	PurpleStatus *status;

	purple_debug_info("autorecon", "do_signon called\n");
	g_return_val_if_fail(account != NULL, FALSE);
	info = g_hash_table_lookup(hash, account);

	if (info)
		info->timeout = 0;

	status = purple_account_get_active_status(account);
	if (purple_status_is_online(status))
	{
		purple_debug_info("autorecon", "calling purple_account_connect\n");
		purple_account_connect(account);
		purple_debug_info("autorecon", "done calling purple_account_connect\n");
	}

	return FALSE;
}

static gboolean
enable_account(gpointer data)
{
  PurpleAccount *account = data;
  FinchAutoRecon *info;
  
  purple_debug_info("autorecon", "enable_account called\n");
  g_return_val_if_fail(account != NULL, FALSE);
	info = g_hash_table_lookup(hash, account);

	if (info)
		info->timeout = 0;
  
	purple_account_set_enabled(account, UI_ID, TRUE);
	
	return FALSE;
}

void
finch_connection_report_disconnect(PurpleConnection *gc, PurpleConnectionError reason,
		const char *text)
{
	PurpleAccount *account = purple_connection_get_account(gc);
	FinchAutoRecon *info = g_hash_table_lookup(hash, account);

	if (info == NULL) {
		info = g_new0(FinchAutoRecon, 1);
		g_hash_table_insert(hash, account, info);
		info->delay = g_random_int_range(INITIAL_RECON_DELAY_MIN, INITIAL_RECON_DELAY_MAX);
	} else {
		info->delay = MIN(2 * info->delay, MAX_RECON_DELAY);
		if (info->timeout != 0)
			g_source_remove(info->timeout);
	}
	
	if (!purple_connection_error_is_fatal(reason)) {
		info->timeout = g_timeout_add(info->delay, do_signon, account);
	} else {
		info->timeout = g_timeout_add(info->delay, enable_account, account);
	}
}

static void
account_removed_cb(PurpleAccount *account, gpointer user_data)
{
	g_hash_table_remove(hash, account);
}

static void *
finch_connection_get_handle(void)
{
	static int handle;

	return &handle;
}

void finch_connections_init()
{
	hash = g_hash_table_new_full(
							g_direct_hash, g_direct_equal,
							NULL, free_auto_recon);

	purple_signal_connect(purple_accounts_get_handle(), "account-removed",
						finch_connection_get_handle(),
						PURPLE_CALLBACK(account_removed_cb), NULL);
}
