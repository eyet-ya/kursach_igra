#pragma once

#include <windows.h>

#include <string>

#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")



class SoundManager {

public:

    static void PlayJumpSound();

    static void PlayCollectSound();

    static void PlayVictorySound();

    static void PlayLoseSound();      



private:

    static bool FileExists(const std::wstring& path);

    static void PlaySoundFile(const std::wstring& path);

};
