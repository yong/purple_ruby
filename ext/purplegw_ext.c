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

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ruby.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
static VALUE cAccount;
static VALUE cConversation;
static char* UI_ID = "purplegw";
static GMainLoop *main_loop;
static VALUE im_hanlder;
static VALUE signed_on_hanlder;

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
		
  VALUE *args = g_new(VALUE, 3);
  args[0] = rb_str_new2(purple_account_get_username(purple_conversation_get_account(conv)));
  args[1] = rb_str_new2(name);
  args[2] = rb_str_new2(message);
  rb_funcall2(im_hanlder, rb_intern("call"), 3, args);
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

static VALUE watch_incoming_im(VALUE self)
{
  im_hanlder = rb_block_proc();
  return im_hanlder;
}

static void signed_on(PurpleConnection* connection)
{
  VALUE *args = g_new(VALUE, 1);
  args[0] = Data_Wrap_Struct(cAccount, NULL, NULL, purple_connection_get_account(connection));
  rb_funcall2((VALUE)signed_on_hanlder, rb_intern("call"), 1, args);
}

static VALUE watch_signed_on_event(VALUE self)
{
  signed_on_hanlder = rb_block_proc();
  int handle;
	purple_signal_connect(purple_connections_get_handle(), "signed-on", &handle,
				PURPLE_CALLBACK(signed_on), NULL);
  return signed_on_hanlder;
}

static void _server_socket_handler(gpointer data, int server_socket, PurpleInputCondition condition)
{
  /* Check that it is a read condition */
	if (condition != PURPLE_INPUT_READ)
		return;
		
	struct sockaddr_in their_addr; /* connector's address information */
	socklen_t sin_size = sizeof(struct sockaddr);
	int client_socket;
  if ((client_socket = accept(server_socket, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
		return;
	}
		
  char message[40960];
  if (recv(client_socket, message, sizeof(message) - 1, 0) <= 0) {
    close(client_socket);
    return;
  }

  VALUE *args = g_new(VALUE, 1);
  args[0] = rb_str_new2(message);
  rb_funcall2((VALUE)data, rb_intern("call"), 1, args);
  
  close(client_socket);
}

static VALUE watch_incoming_ipc(VALUE self, VALUE port)
{
	struct sockaddr_in my_addr;
  int soc;

	/* Open a listening socket for incoming conversations */
	if ((soc = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		purple_debug_error("bonjour", "Cannot open socket: %s\n", g_strerror(errno));
		return Qnil;
	}

	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	my_addr.sin_port = htons(FIX2INT(port));
	if (bind(soc, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) != 0)
	{
		purple_debug_info("bonjour", "Unable to bind to port %d: %s\n", (int)FIX2INT(port), g_strerror(errno));
		return Qnil;
	}

	/* Attempt to listen on the bound socket */
	if (listen(soc, 10) != 0)
	{
		purple_debug_error("bonjour", "Cannot listen on socket: %s\n", g_strerror(errno));
		return Qnil;
	}

  VALUE proc = rb_block_proc();
  
	/* Open a watcher in the socket we have just opened */
	purple_input_add(soc, PURPLE_INPUT_READ, _server_socket_handler, (gpointer)proc);

	return port;
}

static VALUE login(VALUE self, VALUE protocol, VALUE username, VALUE password)
{
  PurpleAccount* account = purple_account_new(RSTRING(username)->ptr, RSTRING(protocol)->ptr);
  purple_account_set_password(account, RSTRING(password)->ptr);
  purple_account_set_enabled(account, UI_ID, TRUE);
  PurpleSavedStatus *status = purple_savedstatus_new(NULL, PURPLE_STATUS_AVAILABLE);
	purple_savedstatus_activate(status);
	
	return Data_Wrap_Struct(cAccount, NULL, NULL, account);
}

static VALUE main_loop_run(VALUE self)
{
  main_loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(main_loop);
  return Qnil;
}

static VALUE send_im(VALUE self, VALUE name, VALUE message)
{
  PurpleAccount *account;
  Data_Get_Struct(self, PurpleAccount, account);
  
  if (purple_account_is_connected(account)) {
    int i = serv_send_im(purple_account_get_connection(account), RSTRING(name)->ptr, RSTRING(message)->ptr, 0);
    return INT2FIX(i);
  } else {
    return Qnil;
  }
}

static VALUE username(VALUE self)
{
  PurpleAccount *account;
  Data_Get_Struct(self, PurpleAccount, account);
  return rb_str_new2(purple_account_get_username(account));
}

void Init_purplegw_ext() 
{
  cPurpleGW = rb_define_class("PurpleGW", rb_cObject);
  rb_define_singleton_method(cPurpleGW, "init", init, 1);
  rb_define_singleton_method(cPurpleGW, "watch_signed_on_event", watch_signed_on_event, 0);
  rb_define_singleton_method(cPurpleGW, "watch_incoming_im", watch_incoming_im, 0);
  rb_define_singleton_method(cPurpleGW, "login", login, 3);
  rb_define_singleton_method(cPurpleGW, "watch_incoming_ipc", watch_incoming_ipc, 1);
  rb_define_singleton_method(cPurpleGW, "main_loop_run", main_loop_run, 0);
  
  cAccount = rb_define_class_under(cPurpleGW, "Account", rb_cObject);
  rb_define_method(cAccount, "send_im", send_im, 2);
  rb_define_method(cAccount, "username", username, 0);
}
