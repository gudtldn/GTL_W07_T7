#include "LuaCoachActor.h"
#include "Engine/Classes/Components/Collision/CapsuleComponent.h"

ALuaCoachActor::ALuaCoachActor()
{
    CapsuleComponent = AddComponent<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->AttachToComponent(RootComponent);
    SetScriptPath(std::optional<std::filesystem::path>(std::filesystem::path(L"C:\\Users\\Jungle\\Documents\\8WeekGitHub\\GTL_W08_T8\\EngineSIU\\GameJam/Lua\\Coach.lua")));
}
