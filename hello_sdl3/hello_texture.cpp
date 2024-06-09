#include "constants.h"
#include "utils/command_line.h"
#include "utils/performance.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[])
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    SDL_Init(SDL_INIT_VIDEO);

    CommandLine command_line(argc, argv);

    SDL_Window* window = SDL_CreateWindow("Hello, Texture!", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Could not create a window: %s", SDL_GetError());
        return -1;
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, command_line.GetRenderDriver());
    if (!renderer) {
        SDL_Log("Create renderer failed: %s", SDL_GetError());
        return -1;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    for (int i = 0; i < info.num_texture_formats; ++i) {
        SDL_Log("Texture format[%d]: %s", i, SDL_GetPixelFormatName(info.texture_formats[i]));
    }

    int vsync = command_line.IsDisableVsync() ? 0 : 1;
    SDL_SetRenderVSync(renderer, vsync);
    SDL_Log("VSync: %d", vsync);

    uint8_t pixels[4 * 2 * 2] = {
        0,   0,   255, 255,  // b, g, r, a
        0,   255, 0,   255,  //
        255, 0,   0,   255,  //
        0,   255, 255, 255   //
    };
    SDL_Surface* surface = SDL_CreateSurfaceFrom(pixels, 2, 2, 4 * 2, SDL_PIXELFORMAT_ARGB8888);
    if (!surface) {
        SDL_Log("Create surface failed: %s", SDL_GetError());
        return -1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Create texture failed: %s", SDL_GetError());
        return -1;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    Performance performance;

    while (true) {
        SDL_Event event{};
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            break;
        }

        int width = 0;
        int height = 0;
        SDL_GetRenderOutputSize(renderer, &width, &height);
        SDL_FRect dst_rect{};
        dst_rect.w = width * 0.5f;
        dst_rect.h = height * 0.5f;
        dst_rect.x = (width - dst_rect.w) * 0.5f;
        dst_rect.y = (height - dst_rect.h) * 0.5f;

        SDL_SetRenderDrawColor(renderer, 16, 0, 16, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, &dst_rect);
        SDL_RenderPresent(renderer);

        performance.IncreaseFrameCount();
        performance.PrintEverySecond();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
