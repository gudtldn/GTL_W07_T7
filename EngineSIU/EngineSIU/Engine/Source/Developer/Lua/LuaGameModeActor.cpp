#include "LuaGameModeActor.h"

ALuaGameModeActor::ALuaGameModeActor()
{
    std::filesystem::path LuaFolderPath = std::filesystem::current_path().parent_path() / "GameJam" / "Lua";
    std::filesystem::path gameModeScript = LuaFolderPath / "GameMode.lua";

    // 2) 파일 존재 확인 (있을 때만 적용)
    if (std::filesystem::exists(gameModeScript)) {
        SetScriptPath(gameModeScript);
    }
    else {
        UE_LOG(ELogLevel::Error, "Heart.lua not found: %s", gameModeScript.generic_string().c_str());
    }
}
