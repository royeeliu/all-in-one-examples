#include "utils/performance.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <map>
#include <string>

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

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, render_driver.empty() ? nullptr : render_driver.c_str());
    if (!renderer) {
        SDL_Log("Create renderer failed: %s", SDL_GetError());
        return -1;
    }

    int vsync = disable_vsync ? 0 : 1;
    SDL_SetRenderVSync(renderer, vsync);
    SDL_Log("VSync: %d", vsync);

    uint8_t pixels[4 * 2 * 2] = {
        255, 0,   0,   255,  // r, g, b, a
        0,   255, 0,   255,  //
        0,   0,   255, 255,  //
        255, 255, 0,   255   //
    };
    SDL_Surface* surface = SDL_CreateSurfaceFrom(pixels, 2, 2, 4 * 2, SDL_PIXELFORMAT_ABGR8888);
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
        SDL_GetWindowSize(window, &width, &height);
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
