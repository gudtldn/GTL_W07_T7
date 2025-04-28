#include "SoundManager.h"
#include <cassert>

FSoundManager* FSoundManager::Instance = nullptr;

FSoundManager* FSoundManager::Get()
{
    if (!Instance)
    {
        Instance = new FSoundManager();
        Instance->Initialize();
    }
    return Instance;
}

void FSoundManager::Initialize()
{
    FMOD_RESULT Result = FMOD::System_Create(&System); // Create system
    ErrorCheck(Result);

    Result = System->getVersion(&Version); // Get version
    ErrorCheck(Result);

    Result = System->init(256, FMOD_INIT_NORMAL, ExtraDriverData); // Set max channel to 256.
    ErrorCheck(Result);

    // initialize silent sound for Main channel 
    FMOD_CREATESOUNDEXINFO ExInfo = {};
    ExInfo.cbsize           = sizeof(ExInfo);
    ExInfo.length           = 48000 * sizeof(short) * 1; // 48kHz × 2byte × 1channel
    ExInfo.format           = FMOD_SOUND_FORMAT_PCM16;
    ExInfo.numchannels      = 1;
    ExInfo.defaultfrequency = 48000;

    FMOD::Sound* SilentSound = nullptr;
    System->createSound(
        nullptr,
        FMOD_OPENUSER | FMOD_LOOP_OFF,
        &ExInfo,
        &SilentSound
    );

    CreateSound("Contents\\Sound\\shoot.wav", false);
    
    System->playSound(SilentSound, nullptr, true, &MainChannel);
}

void FSoundManager::Release() const
{
    for (auto& Pair : SoundMap)
    {
        Pair.second->release();
    }

    if (System)
    {
        System->close();
        System->release();
    }
}

bool FSoundManager::CreateSound(const std::string& SoundName, bool bLoop)
{
    const std::string FullPath = GetPath(SoundName).string();
    
    FMOD::Sound* Sound = nullptr;
    FMOD_RESULT FmodResult = System->createSound(FullPath.c_str(), bLoop ? FMOD_DEFAULT | FMOD_LOOP_NORMAL : FMOD_DEFAULT, nullptr, &Sound);

    if (ErrorCheck(FmodResult))
    {
        return false;
    }

    SoundMap[SoundName] = Sound;
    return true;
}

void FSoundManager::Update() const
{
    if (System)
    {
        System->update();
    }
}

FMOD::Channel* FSoundManager::PlaySound(const std::string& SoundName, bool bLoop)
{
    FMOD::Sound* Sound = GetSound(SoundName);
    if (!Sound)
    {
        CreateSound(SoundName, bLoop);
        Sound = GetSound(SoundName);
    }

    FMOD::Channel* Channel = nullptr;
    Sound->setMode(bLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
    FMOD_RESULT FmodResult = System->playSound(Sound, nullptr, false, &Channel);
    ErrorCheck(FmodResult);
    
    ChannelMap[SoundName] = Channel;

    return Channel;
}

void FSoundManager::PlaySound(const std::string& SoundName, FMOD::Channel* Channel, bool bLoop)
{
    FMOD::Sound* Sound = GetSound(SoundName);
    if (!Sound)
    {
        CreateSound(SoundName, bLoop);
        Sound = GetSound(SoundName);
    }
    
    Sound->setMode(bLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
    FMOD_RESULT FmodResult = System->playSound(Sound, nullptr, false, &Channel);
    ErrorCheck(FmodResult);
}

void FSoundManager::StopSound(const std::string& ChannelName)
{
    if (FMOD::Channel* Channel = GetChannel(ChannelName))
    {
        Channel->stop();
        ChannelMap.erase(ChannelName);
    }
}

void FSoundManager::PauseSound(const std::string& ChannelName) const
{
    if (FMOD::Channel* Channel = GetChannel(ChannelName))
    {
        Channel->setPaused(true);
    }
}

void FSoundManager::ResumeSound(const std::string& ChannelName) const
{
    if (FMOD::Channel* Channel = GetChannel(ChannelName))
    {
        Channel->setPaused(false);
    }
}

FMOD::Sound* FSoundManager::GetSound(const std::string& SoundName) const
{
    auto It = SoundMap.find(SoundName);
    return (It != SoundMap.end()) ? It->second : nullptr;
}

FMOD::Channel* FSoundManager::GetChannel(const std::string& ChannelName) const
{
    auto It = ChannelMap.find(ChannelName);
    return (It != ChannelMap.end()) ? It->second : nullptr;
}

std::filesystem::path FSoundManager::GetPath(const std::string& SoundPath)
{
    std::filesystem::path Path = std::filesystem::current_path() / SoundPath;
    return Path;
}
