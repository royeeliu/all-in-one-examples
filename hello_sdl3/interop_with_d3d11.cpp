#include "constants.h"
#include "utils/command_line.h"
#include "utils/performance.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

ComPtr<ID3D11Texture2D> CreateTexture2D(ID3D11Device* d3d11_device, int width, int height)
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = d3d11_device->CreateTexture2D(&desc, nullptr, &texture);
    if (FAILED(hr)) {
        SDL_Log("Create texture failed: 0x%08X", hr);
        return nullptr;
    }

    return texture;
}

int main(int argc, char* argv[])
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    SDL_Init(SDL_INIT_VIDEO);

    CommandLine command_line(argc, argv);

    SDL_Window* window =
        SDL_CreateWindow("Interop with D3D11", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Could not create a window: %s", SDL_GetError());
        return -1;
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "direct3d11");
    if (!renderer) {
        SDL_Log("Create renderer failed: %s", SDL_GetError());
        return -1;
    }

    int vsync = command_line.IsDisableVsync() ? 0 : 1;
    SDL_SetRenderVSync(renderer, vsync);
    SDL_Log("VSync: %d", vsync);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    for (int i = 0; i < info.num_texture_formats; ++i) {
        SDL_Log("Texture format[%d]: %s", i, SDL_GetPixelFormatName(info.texture_formats[i]));
    }

    ComPtr<ID3D11Device> d3d11_device = static_cast<ID3D11Device*>(SDL_GetProperty(
        SDL_GetRendererProperties(renderer), SDL_PROP_RENDERER_D3D11_DEVICE_POINTER, NULL));
    if (!d3d11_device) {
        SDL_Log("Get D3D11 device failed: %s", SDL_GetError());
        return -1;
    }

    ComPtr<ID3D11DeviceContext> d3d11_context;
    d3d11_device->GetImmediateContext(&d3d11_context);

    ComPtr<ID3D11Texture2D> d3d11_texture = CreateTexture2D(d3d11_device.Get(), 2, 2);
    if (!d3d11_texture) {
        SDL_Log("Create D3D11 texture failed");
        return -1;
    }

    uint8_t pixels[4 * 2 * 2] = {
        0,   0,   255, 255,  // b, g, r, a
        0,   255, 0,   255,  //
        255, 0,   0,   255,  //
        0,   255, 255, 255   //
    };

    d3d11_context->UpdateSubresource(d3d11_texture.Get(), 0, nullptr, pixels, 4 * 2, 0);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER, 2);
    SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER, 2);
    SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER, SDL_PIXELFORMAT_ARGB8888);
    SDL_SetProperty(props, SDL_PROP_TEXTURE_CREATE_D3D11_TEXTURE_POINTER, d3d11_texture.Get());
    SDL_Texture* texture = SDL_CreateTextureWithProperties(renderer, props);
    SDL_DestroyProperties(props);

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
