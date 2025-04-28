#include "LuaCoachActor.h"
#include "Engine/Classes/Components/Collision/CapsuleComponent.h"

ALuaCoachActor::ALuaCoachActor()
{
    CapsuleComponent = AddComponent<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->AttachToComponent(RootComponent);

    std::filesystem::path LuaFolderPath = std::filesystem::current_path().parent_path() / "GameJam" / "Lua";
    std::filesystem::path coachScript = LuaFolderPath / "Coach.lua";

    // 2) 파일 존재 확인 (있을 때만 적용)
    if (std::filesystem::exists(coachScript)) {
        SetScriptPath(coachScript);
    }
    else {
        UE_LOG(ELogLevel::Error, "Coach.lua not found: %s", coachScript.generic_string().c_str());
    }
}
