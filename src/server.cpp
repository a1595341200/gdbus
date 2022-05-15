#include "Hello.h"
#include <gio/gio.h>
#include <stdio.h>

static helloTest *skeleton = NULL;

// setVersion方法的处理函数
static gboolean on_handle_set_version(helloTest *skeleton,
                                      GDBusMethodInvocation *invocation,
                                      const gchar *str, gpointer user_data) {
  printf("Method call: %s\n", str);

  hello_test_complete_set_version(skeleton, invocation);

  return TRUE;
}

/**
 * 连接上bus daemon的回调
 **/
void on_bus_acquired(GDBusConnection *connection, const gchar *name,
                     gpointer user_data) {
  printf("on_bus_acquired has been invoked, name is %s\n", name);
  GError *error = NULL;
  skeleton = hello_test_skeleton_new();
  //注意方法名，需参照Hello.c里面生成的字符串
  g_signal_connect(skeleton, "handle-set-version",
                   G_CALLBACK(on_handle_set_version), NULL);
  //发布服务到总线
  g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton),
                                   connection, "/com/yft/hello", &error);
  if (error != NULL) {
    printf("Error: Failed to export object. Reason: %s.\n", error->message);
    g_error_free(error);
  }
}

/**
 * 成功注册busName的回调
 **/
void on_bus_name_acquired(GDBusConnection *connection, const gchar *name,
                          gpointer user_data) {
  printf("on_bus_name_acquired has been invoked\n");
}

/**
 * busName丢失的回调，一般是server挂了？？
 **/
void on_bus_name_lost(GDBusConnection *connection, const gchar *name,
                      gpointer user_data) {
  printf("on_bus_name_lost !!!!\n");
}

/**
 * 测试发送信号函数，调用Hello.h自动生成的方法
 **/
static gint status_value = 1;
static gboolean emit_test_status_signal(gconstpointer p) {
  printf("emit_test_status_signal invoked\n");
  if (skeleton != NULL) {
    hello_test_emit_test_status(skeleton, status_value);
  }
  status_value++;
}

int main(int argc, char const *argv[]) {
  guint owner_id;
  GMainLoop *loop;
#if !GLIB_CHECK_VERSION(2, 35, 0)
  g_type_init();
#endif
  owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, "com.yft.hello",
                            G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
                            on_bus_name_acquired, on_bus_name_lost, NULL, NULL);

  //测试，每3s发一次signal
  g_timeout_add(3000, (GSourceFunc)emit_test_status_signal, NULL);

  //主线程进入循环
  loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);

  g_bus_unown_name(owner_id);

  return 0;
}
