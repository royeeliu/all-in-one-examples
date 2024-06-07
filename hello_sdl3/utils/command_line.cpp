#include "command_line.h"

#include <SDL3/SDL.h>

#include <map>
#include <string>

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

CommandLine::CommandLine(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], kRenderDriverArg, strlen(kRenderDriverArg)) == 0) {
            SelectRenderDriver(argv[i] + strlen(kRenderDriverArg));
        } else if (strcmp(argv[i], kDisableVsyncArg) == 0) {
            disable_vsync_ = true;
        }
    }
}

CommandLine::~CommandLine() {}

const char* CommandLine::GetRenderDriver() const
{
    return render_driver_.empty() ? nullptr : render_driver_.c_str();
}

bool CommandLine::IsDisableVsync() const
{
    return disable_vsync_;
}

void CommandLine::SelectRenderDriver(const char* driver)
{
    if (!driver) {
        return;
    }

    auto it = kRenderDriverMap.find(driver);
    if (it == kRenderDriverMap.end()) {
        return;
    }

    const char* driver_name = it->second;
    int count = SDL_GetNumRenderDrivers();
    for (int i = 0; i < count; ++i) {
        const char* name = SDL_GetRenderDriver(i);
        SDL_Log("Enumerate render driver[%d]: %s", i, name);
        if (render_driver_.empty() && strcmp(name, driver_name) == 0) {
            render_driver_ = name;
        }
    }
}