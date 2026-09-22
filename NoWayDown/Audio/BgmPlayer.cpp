#include "BgmPlayer.h"

#include <Windows.h>
#include <mmsystem.h>
#include <filesystem>

#pragma comment(lib, "winmm.lib")

namespace
{
    bool isPlaying = false;

    std::filesystem::path MainBgmPath()
    {
        wchar_t buffer[32768] = {};
        const DWORD length = GetModuleFileNameW(
            nullptr, buffer, static_cast<DWORD>(std::size(buffer)));

        if (length == 0 || length >= std::size(buffer))
        {
            return {};
        }

        const std::filesystem::path executablePath(
            buffer, buffer + length);
        const std::filesystem::path executableDirectory =
            executablePath.parent_path();

        // The pre-build step copies root Assets to Bin/x64/Assets.
        const std::filesystem::path besideArchitecture =
            executableDirectory.parent_path() /
            L"Assets" / L"Audio" / L"MainGame_BGM.wav";

        std::error_code error;
        if (std::filesystem::exists(besideArchitecture, error))
        {
            return besideArchitecture;
        }

        // Fallback for running directly from a project/output directory.
        return executableDirectory /
            L"Assets" / L"Audio" / L"MainGame_BGM.wav";
    }
}

void NoWayDown::BgmPlayer::StartMainGame()
{
    if (isPlaying)
    {
        return;
    }

    const std::filesystem::path path = MainBgmPath();
    if (path.empty())
    {
        return;
    }

    isPlaying = PlaySoundW(
        path.c_str(),
        nullptr,
        SND_FILENAME | SND_ASYNC | SND_LOOP | SND_NODEFAULT) != FALSE;
}

void NoWayDown::BgmPlayer::Stop()
{
    if (!isPlaying)
    {
        return;
    }

    PlaySoundW(nullptr, nullptr, 0);
    isPlaying = false;
}
