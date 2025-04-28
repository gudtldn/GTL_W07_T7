#include "LuaHeartActor.h"
#include "Engine/Classes/Components/RigidbodyComponent.h"

ALuaHeartActor::ALuaHeartActor()
{
    RigidbodyComp = AddComponent<URigidbodyComponent>(TEXT("RigidbodyComponent"));

    std::filesystem::path LuaFolderPath = std::filesystem::current_path().parent_path() / "GameJam" / "Lua";
    std::filesystem::path heartScript = LuaFolderPath / "Heart.lua";

    // 2) 파일 존재 확인 (있을 때만 적용)
    if (std::filesystem::exists(heartScript)) {
        SetScriptPath(heartScript);
    }
    else {
        UE_LOG(ELogLevel::Error, "Heart.lua not found: %s", heartScript.generic_string().c_str());
    }

}
