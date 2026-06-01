#include "SoundManager.h"



bool SoundManager::FileExists(const std::wstring& path) {

    DWORD attrib = GetFileAttributesW(path.c_str());

    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));

}



void SoundManager::PlaySoundFile(const std::wstring& path) {

    if (FileExists(path)) {

        PlaySoundW(path.c_str(), NULL, SND_ASYNC | SND_FILENAME);

    }

}



void SoundManager::PlayJumpSound() {

    PlaySoundFile(L"sounds/jump.wav");

}



void SoundManager::PlayCollectSound() {

    PlaySoundFile(L"sounds/collect.wav");

}



void SoundManager::PlayVictorySound() {

    PlaySoundFile(L"sounds/victory.wav");

}



void SoundManager::PlayLoseSound() {

    PlaySoundFile(L"sounds/lose.wav");   

}