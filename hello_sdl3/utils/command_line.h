#pragma once

#include <string>

constexpr char kRenderDriverArg[] = "--render-driver=";
constexpr char kDisableVsyncArg[] = "--disable-vsync";

class CommandLine final
{
public:
    CommandLine(int argc, char* argv[]);
    ~CommandLine();

    const char* GetRenderDriver() const;

    bool IsDisableVsync() const;

private:
    void SelectRenderDriver(const char* driver);

    std::string render_driver_;
    bool disable_vsync_ = false;
};