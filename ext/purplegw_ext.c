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

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ruby.h>
#include <stdarg.h>

#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

typedef struct _PurpleGLibIOClosure {
	PurpleInputFunction function;
	guint result;
	gpointer data;
} PurpleGLibIOClosure;

static void purple_glib_io_destroy(gpointer data)
{
	g_free(data);
}

static gboolean purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data)
{
	PurpleGLibIOClosure *closure = data;
	PurpleInputCondition purple_cond = 0;

	if (condition & PURPLE_GLIB_READ_COND)
		purple_cond |= PURPLE_INPUT_READ;
	if (condition & PURPLE_GLIB_WRITE_COND)
		purple_cond |= PURPLE_INPUT_WRITE;

	closure->function(closure->data, g_io_channel_unix_get_fd(source),
			  purple_cond);

	return TRUE;
}

static guint glib_input_add(gint fd, PurpleInputCondition condition, PurpleInputFunction function,
							   gpointer data)
{
	PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
	GIOChannel *channel;
	GIOCondition cond = 0;

	closure->function = function;
	closure->data = data;

	if (condition & PURPLE_INPUT_READ)
		cond |= PURPLE_GLIB_READ_COND;
	if (condition & PURPLE_INPUT_WRITE)
		cond |= PURPLE_GLIB_WRITE_COND;

	channel = g_io_channel_unix_new(fd);
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond,
					      purple_glib_io_invoke, closure, purple_glib_io_destroy);

	g_io_channel_unref(channel);
	return closure->result;
}

static PurpleEventLoopUiOps glib_eventloops = 
{
	g_timeout_add,
	g_source_remove,
	glib_input_add,
	g_source_remove,
	NULL,
#if GLIB_CHECK_VERSION(2,14,0)
	g_timeout_add_seconds,
#else
	NULL,
#endif

	/* padding */
	NULL,
	NULL,
	NULL
};

static VALUE cPurpleGW;
static char* UI_ID = "purplegw";
static GMainLoop *main_loop;

static void write_conv(PurpleConversation *conv, const char *who, const char *alias,
			const char *message, PurpleMessageFlags flags, time_t mtime)
{
	const char *name;
	if (alias && *alias)
		name = alias;
	else if (who && *who)
		name = who;
	else
		name = NULL;

	printf("(%s) %s %s: %s\n", purple_conversation_get_name(conv),
			purple_utf8_strftime("(%H:%M:%S)", localtime(&mtime)),
			name, message);
}

static PurpleConversationUiOps conv_uiops = 
{
	NULL,                      /* create_conversation  */
	NULL,                      /* destroy_conversation */
	NULL,                      /* write_chat           */
	NULL,                      /* write_im             */
	write_conv,           /* write_conv           */
	NULL,                      /* chat_add_users       */
	NULL,                      /* chat_rename_user     */
	NULL,                      /* chat_remove_users    */
	NULL,                      /* chat_update_user     */
	NULL,                      /* present              */
	NULL,                      /* has_focus            */
	NULL,                      /* custom_smiley_add    */
	NULL,                      /* custom_smiley_write  */
	NULL,                      /* custom_smiley_close  */
	NULL,                      /* send_confirm         */
	NULL,
	NULL,
	NULL,
	NULL
};

static void ui_init(void)
{
	/**
	 * This should initialize the UI components for all the modules. Here we
	 * just initialize the UI for conversations.
	 */
	purple_conversations_set_ui_ops(&conv_uiops);
}

static PurpleCoreUiOps core_uiops = 
{
	NULL,
	NULL,
	ui_init,
	NULL,

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static VALUE init(VALUE self, VALUE debug)
{
  purple_debug_set_enabled((debug == Qnil || debug == Qfalse) ? FALSE : TRUE);
  purple_core_set_ui_ops(&core_uiops);
  purple_eventloop_set_ui_ops(&glib_eventloops);
  
  if (!purple_core_init(UI_ID)) {
		/* Initializing the core failed. Terminate. */
		fprintf(stderr,
				"libpurple initialization failed. Dumping core.\n"
				"Please report this!\n");
		abort();
	}

  /* Create and load the buddylist. */
  purple_set_blist(purple_blist_new());
  purple_blist_load();

  /* Load the preferences. */
  purple_prefs_load();

  /* Load the pounces. */
  purple_pounces_load();

  return Qnil;
}

static void callback(va_list args, void *proc)
{
  printf("$$$$$$$$$$$$$$$ %d\n", proc);
  rb_funcall2(proc, rb_intern("call"), 0, NULL);
}

static VALUE subscribe(VALUE self, VALUE signal)
{
  static int handle;
  VALUE proc = rb_block_proc();
	purple_signal_connect_vargs(purple_connections_get_handle(), RSTRING(signal)->ptr, &handle,
				PURPLE_CALLBACK(callback), proc);
  printf("$$$$$$$$$$$$$$$ %d\n", proc);
  return proc;
}

static VALUE login(VALUE self, VALUE protocol, VALUE username, VALUE password)
{
  PurpleAccount* account = purple_account_new(RSTRING(username)->ptr, RSTRING(protocol)->ptr);
  purple_account_set_password(account, RSTRING(password)->ptr);
  purple_account_set_enabled(account, UI_ID, TRUE);
  PurpleSavedStatus *status = purple_savedstatus_new(NULL, PURPLE_STATUS_AVAILABLE);
	purple_savedstatus_activate(status);
	
	return Qnil;
}

static VALUE main_loop_run(VALUE self)
{
  main_loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(main_loop);
  return Qnil;
}

void Init_purplegw_ext() 
{
  cPurpleGW = rb_define_class("PurpleGW", rb_cObject);
  rb_define_singleton_method(cPurpleGW, "init", init, 1);
  rb_define_singleton_method(cPurpleGW, "subscribe", subscribe, 1);
  rb_define_singleton_method(cPurpleGW, "login", login, 3);
  rb_define_singleton_method(cPurpleGW, "main_loop_run", main_loop_run, 0);
}
