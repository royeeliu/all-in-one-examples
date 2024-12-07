#include "utils.h"

void PrintSupportedRenderDrivers()
{
    int count = SDL_GetNumRenderDrivers();
    for (int i = 0; i < count; ++i) {
        const char* name = SDL_GetRenderDriver(i);
        SDL_Log("Render driver[%d]: %s", i, name);
    }
}

void PrintSupportedTextureFormats(SDL_Renderer* renderer)
{
    SDL_PixelFormat* texture_format = static_cast<SDL_PixelFormat*>(SDL_GetPointerProperty(
        SDL_GetRendererProperties(renderer), SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER, nullptr));
    int index = 0;
    while (texture_format && *texture_format != SDL_PIXELFORMAT_UNKNOWN) {
        SDL_Log("Texture format[%d]: %s", index, SDL_GetPixelFormatName(*texture_format));
        ++texture_format;
        ++index;
    }
}