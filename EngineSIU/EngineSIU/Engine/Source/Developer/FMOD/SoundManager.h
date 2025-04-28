#pragma once
#include <filesystem>
#include <iostream>
#include <unordered_map>

#include "fmod.hpp"
#include "fmod_errors.h"



class FSoundManager
{
public:
    FSoundManager() = default;
    ~FSoundManager() = default;
    
    FSoundManager(const FSoundManager&) = delete;
    FSoundManager& operator=(const FSoundManager&) = delete;
    FSoundManager(FSoundManager&&) = delete;
    FSoundManager& operator=(FSoundManager&&) = delete;

    static FSoundManager* Get();
    
    void Initialize();
    void Release() const;

    bool CreateSound(const std::string& SoundName, bool bLoop = false);
    
    /** Update FMOD system, in loop-statement */
    void Update() const;

    FMOD::Channel* PlaySound(const std::string& SoundName, bool bLoop);
    void PlaySound(const std::string& SoundName, FMOD::Channel* Channel, bool bLoop);
    
    void StopSound(const std::string& ChannelName);
    void PauseSound(const std::string& ChannelName) const;
    void ResumeSound(const std::string& ChannelName) const;

    FMOD::Sound* GetSound(const std::string& SoundName) const;
    FMOD::Channel* GetChannel(const std::string& ChannelName) const;

    inline FMOD::System* GetSystem() const;
    inline unsigned int GetVersion() const;

    /** Main background sound channel */
    inline FMOD::Channel* GetMainChannel() const;

public:
    /** FMOD main sound channel */
    FMOD::Channel* MainChannel = nullptr;
    
private:
    /**
     * 
     * @return if true, occurred error.
     */
    static inline bool ErrorCheck(FMOD_RESULT);

    static inline std::filesystem::path GetPath(const std::string& SoundPath);
    
private:
    static FSoundManager* Instance;
    /** FMOD system */
    FMOD::System* System;

    /** FMOD channel driver data */
    void* ExtraDriverData;

    /** FMOD version */
    unsigned int Version;

    /** FMOD sound map */
    std::unordered_map<std::string, FMOD::Sound*> SoundMap;

    /** FMOD channel map */
    std::unordered_map<std::string, FMOD::Channel*> ChannelMap;
};

//************************
// Inline                *
//************************

inline FMOD::System* FSoundManager::GetSystem() const
{
    return System;
}

inline unsigned int FSoundManager::GetVersion() const
{
    return Version;
}

inline FMOD::Channel* FSoundManager::GetMainChannel() const
{
    return MainChannel;
}

inline bool FSoundManager::ErrorCheck(FMOD_RESULT Result)
{
    if (Result != FMOD_OK)
    {
        std::cerr << "FMOD Error: " << FMOD_ErrorString(Result) << '\n';
        return true;
    }

    return false;
}
