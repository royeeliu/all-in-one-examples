#include <wayland-client.h>

int main()
{
    wl_display* display = wl_display_connect(NULL);
    if (display == NULL) {
        return -1;
    }

    wl_registry* registry = wl_display_get_registry(display);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    // 在这里，我们可以使用从注册表获取的全局对象来创建窗口和表面。
    // 添加消息循环
    while (wl_display_dispatch(display) != -1) {
        // 处理事件
    }

    wl_display_disconnect(display);
    return 0;
}