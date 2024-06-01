#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <array>
#include <vector>

int main(int argc, char* argv[])
{
    bool disable_hwaccel = false;
    bool disable_vsync = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--disable-hwaccel") == 0) {
            disable_hwaccel = true;
        } else if (strcmp(argv[i], "--disable-vsync") == 0) {
            disable_vsync = true;
        }
    }

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window =
        SDL_CreateWindow("Hello, Triangle!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800,
                         600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Could not create a window: %s", SDL_GetError());
        return -1;
    }

    uint32_t flags = disable_hwaccel ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_ACCELERATED;
    if (!disable_vsync) {
        flags |= SDL_RENDERER_PRESENTVSYNC;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, flags);

    std::array<SDL_Vertex, 3> origin_vertices = {
        SDL_Vertex{{150, 100}, {255, 0, 0, 255}},   // top
        SDL_Vertex{{0, 300}, {0, 255, 0, 255}},     // left bottom
        SDL_Vertex{{300, 300}, {0, 0, 255, 255}}};  // right bottom

    uint64_t last_tickets = SDL_GetTicks64();
    float position = 0.0f;
    float direction = 1.0f;

    while (true) {
        SDL_Event event{};
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

        uint64_t current_ticks = SDL_GetTicks64();
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