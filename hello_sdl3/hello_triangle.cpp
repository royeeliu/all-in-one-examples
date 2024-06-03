#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <array>
#include <map>
#include <string>
#include <vector>

constexpr char kRenderDriverArg[] = "--render-driver=";

static const std::map<std::string, const char*> kRenderDriverMap = {
    {"d3d9", "direct3d"},      //
    {"d3d11", "direct3d11"},   //
    {"d3d12", "direct3d12"},   //
    {"gl", "opengl"},          //
    {"gles", "opengles2"},     //
    {"vulkan", "vulkan"},      //
    {"metal", "metal"},        //
    {"software", "software"},  //
};

static std::string GetRenderDriver(const char* arg)
{
    if (!arg) {
        return "";
    }

    auto it = kRenderDriverMap.find(arg);
    if (it == kRenderDriverMap.end()) {
        return "";
    }

    const char* driver = it->second;
    std::string result;

    int count = SDL_GetNumRenderDrivers();
    for (int i = 0; i < count; ++i) {
        const char* name = SDL_GetRenderDriver(i);
        SDL_Log("Enumerate render driver[%d]: %s", i, name);
        if (result.empty() && strcmp(name, driver) == 0) {
            result = name;
        }
    }

    return result;
}

int main(int argc, char* argv[])
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    SDL_Init(SDL_INIT_VIDEO);

    std::string render_driver;
    bool disable_vsync = false;
    for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], kRenderDriverArg, strlen(kRenderDriverArg)) == 0) {
            render_driver = GetRenderDriver(argv[i] + strlen(kRenderDriverArg));
            SDL_Log("Select render driver: %s", render_driver.c_str());
        } else if (strcmp(argv[i], "--disable-vsync") == 0) {
            disable_vsync = true;
        }
    }

    SDL_Window* window = SDL_CreateWindow("Hello, Triangle!", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Could not create a window: %s", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, render_driver.empty() ? nullptr : render_driver.c_str());
    if (!renderer) {
        SDL_Log("Create renderer failed: %s", SDL_GetError());
        return -1;
    }

    int vsync = disable_vsync ? 0 : 1;
    SDL_SetRenderVSync(renderer, vsync);
    SDL_Log("VSync: %d", vsync);

    std::array<SDL_Vertex, 3> origin_vertices = {
        SDL_Vertex{{150, 100}, {1.0f, 0.0f, 0.0f, 1.0f}},  // top
        SDL_Vertex{{000, 300}, {0.0f, 1.0f, 0.0f, 1.0f}},  // left bottom
        SDL_Vertex{{300, 300}, {0.0f, 0.0f, 1.0f, 1.0f}}   // right bottom
    };

    uint64_t last_tickets = SDL_GetTicks();
    float position = 0.0f;
    float direction = 1.0f;

    while (true) {
        SDL_Event event{};
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            break;
        }

        uint64_t current_ticks = SDL_GetTicks();
        float delta_time = (current_ticks - last_tickets) / 1000.0f;
        last_tickets = current_ticks;

        position += 200.0f * delta_time * direction;

        int width = 0;
        SDL_GetWindowSize(window, &width, nullptr);
        float max_position = static_cast<float>(width) -
                             (origin_vertices[2].position.x - origin_vertices[1].position.x);

        if (position > max_position) {
            direction = -1.0f;
        } else if (position < 0.0f) {
            position = 0.0f;
            direction = 1.0f;
        }

        std::vector<SDL_Vertex> vertices;
        for (const auto& vertex : origin_vertices) {
            vertices.push_back(vertex);
            vertices.back().position.x += position;
        }

        SDL_SetRenderDrawColor(renderer, 16, 0, 16, 255);
        SDL_RenderClear(renderer);
        SDL_RenderGeometry(renderer, nullptr, vertices.data(), vertices.size(), nullptr, 0);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
