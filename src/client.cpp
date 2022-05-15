#include "Hello.h"
#include <gio/gio.h>
#include <stdio.h>

helloTest *proxy;

static gboolean test_status_handler(helloTest *object, gint value,
                                    gpointer userdata) {
  printf("test_status_handler invoked! %d \n", value);
  return TRUE;
}

int main(int argc, char const *argv[]) {
  GError *connerror = NULL;
  GError *proxyerror = NULL;
  GDBusConnection *conn;
  GMainLoop *loop;

#if !GLIB_CHECK_VERSION(2, 35, 0)
  g_type_init();
#endif

  conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &connerror);

  if (connerror != NULL) {
    printf("g_bus_get_sync connect error! %s \n", connerror->message);
    g_error_free(connerror);
    return 0;
  }

  proxy =
      hello_test_proxy_new_sync(conn, G_DBUS_PROXY_FLAGS_NONE, "com.yft.hello",
                                "/com/yft/hello", NULL, &proxyerror);

  if (proxy == NULL) {
    printf("hello_test_proxy_new_sync error! %s \n", proxyerror->message);
    g_error_free(proxyerror);
    return 0;
  }
  //注册signal处理函数
  g_signal_connect(proxy, "test-status", G_CALLBACK(test_status_handler), NULL);

  GError *callError = NULL;
  //调用server方法
  hello_test_call_set_version_sync(proxy, "v1.0", NULL, &callError);
  if (callError == NULL) {
    printf("call_set_version_sync success\n");
  } else {
    printf("call_set_version_sync error, %s\n", callError->message);
  }

  loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);

  g_object_unref(proxy);

  return 0;
}
