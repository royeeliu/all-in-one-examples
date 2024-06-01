#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

#define WIDTH 200
#define HEIGHT 100

static wl_compositor* compositor = nullptr;
static wl_shell* shell = nullptr;
static wl_shm* shm = nullptr;
static void* shm_data = nullptr;
static struct wl_buffer* buffer = nullptr;

static void global_registry_handler(
    void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version);
static void global_registry_remover(void* data, wl_registry* registry, uint32_t id);
static void shm_format(void* data, struct wl_shm* wl_shm, uint32_t format);

static wl_shm_listener shm_listener = {.format = shm_format};

static const wl_registry_listener registry_listener = {
    .global = global_registry_handler, .global_remove = global_registry_remover};

void global_registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
    std::cout << "Got a registry event for " << interface << "id: " << id << std::endl;

    if (strcmp(interface, "wl_compositor") == 0)
    {
        compositor = (wl_compositor*)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shell") == 0)
    {
        shell = (wl_shell*)wl_registry_bind(registry, id, &wl_shell_interface, 1);
    }
    else if (strcmp(interface, "wl_shm") == 0)
    {
        shm = (wl_shm*)wl_registry_bind(registry, id, &wl_shm_interface, 1);
        wl_shm_add_listener(shm, &shm_listener, nullptr);
    }
}

void global_registry_remover(void* data, wl_registry* registry, uint32_t id)
{
    printf("Got a registry losing event for %d\n", id);
}

void shm_format(void* data, struct wl_shm* wl_shm, uint32_t format)
{
    // struct display *d = data;
    //   d->formats |= (1 << format);
    fprintf(stderr, "Format %d\n", format);
}

static int set_cloexec_or_close(int fd)
{
    long flags;

    if (fd == -1)
        return -1;

    flags = fcntl(fd, F_GETFD);
    if (flags == -1)
        goto err;

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
        goto err;

    return fd;

err:
    close(fd);
    return -1;
}

static int create_tmpfile_cloexec(char* tmpname)
{
    int fd;

#ifdef HAVE_MKOSTEMP
    fd = mkostemp(tmpname, O_CLOEXEC);
    if (fd >= 0)
        unlink(tmpname);
#else
    fd = mkstemp(tmpname);
    if (fd >= 0)
    {
        fd = set_cloexec_or_close(fd);
        unlink(tmpname);
    }
#endif
    return fd;
}

int os_create_anonymous_file(off_t size)
{
    static const char file_template[] = "/weston-shared-XXXXXX";
    int fd;

    const char* path = getenv("XDG_RUNTIME_DIR");
    if (!path)
    {
        errno = ENOENT;
        return -1;
    }

    char* name = reinterpret_cast<char*>(malloc(strlen(path) + sizeof(file_template)));
    if (!name)
    {
        return -1;
    }

    strcpy(name, path);
    strcat(name, file_template);
    printf("%s\n", name);

    fd = create_tmpfile_cloexec(name);

    free(name);

    if (fd < 0)
        return -1;

    if (ftruncate(fd, size) < 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}

static struct wl_buffer* create_buffer()
{
    struct wl_shm_pool* pool;
    int stride = WIDTH * 4; // 4 bytes per pixel
    int size = stride * HEIGHT;
    int fd;
    struct wl_buffer* buff;

    fd = os_create_anonymous_file(size);
    if (fd < 0)
    {
        fprintf(stderr, "creating a buffer file for %d B failed: %m\n", size);
        exit(1);
    }

    shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_data == MAP_FAILED)
    {
        fprintf(stderr, "mmap failed: %m\n");
        close(fd);
        exit(1);
    }

    pool = wl_shm_create_pool(shm, fd, size);
    buff = wl_shm_pool_create_buffer(pool, 0, WIDTH, HEIGHT, stride, WL_SHM_FORMAT_XRGB8888);
    //wl_buffer_add_listener(buffer, &buffer_listener, buffer);
    wl_shm_pool_destroy(pool);
    return buff;
}
static void paint_pixels()
{
    uint32_t* pixel = reinterpret_cast<uint32_t*>(shm_data);
    fprintf(stderr, "Painting pixels\n");

    for (int n = 0; n < WIDTH * HEIGHT; n++)
    {
        *pixel++ = 0xff0000; //红色
    }
}

int main()
{
    wl_display* display = wl_display_connect(nullptr);
    if (!display)
    {
        return -1;
    }

    wl_registry* registry = wl_display_get_registry(display);
    if (!registry)
    {
        wl_display_disconnect(display);
        return -1;
    }

    wl_registry_add_listener(registry, &registry_listener, nullptr);
    wl_display_dispatch(display);

    if (compositor == NULL)
    {
        fprintf(stderr, "Can't find compositor\n");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Found compositor\n");
    }

    if (!shm)
    {
        fprintf(stderr, "Can't create shm\n");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Created shm\n");
    }

    wl_surface* surface = wl_compositor_create_surface(compositor);
    if (!surface)
    {
        fprintf(stderr, "Can't create surface\n");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Created surface\n");
    }

    wl_shell_surface* shell_surface = wl_shell_get_shell_surface(shell, surface);
    if (!shell_surface)
    {
        fprintf(stderr, "Can't create shell surface\n");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Created shell surface\n");
    }
    wl_shell_surface_set_toplevel(shell_surface);

    buffer = create_buffer();
    paint_pixels();

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_commit(surface);

    while (wl_display_dispatch(display) != -1)
    {
    }

    munmap(shm_data, WIDTH * HEIGHT * 4);
    wl_shell_surface_destroy(shell_surface);
    wl_surface_destroy(surface);
    wl_shm_destroy(shm);
    wl_shell_destroy(shell);
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
    return 0;
}