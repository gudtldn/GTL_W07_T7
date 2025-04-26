#include "LuaActor.h"

ALuaActor::ALuaActor()
    : LuaScriptPath(std::nullopt)
{
}

void ALuaActor::BeginPlay()
{
    Super::BeginPlay();
 
}

void ALuaActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void ALuaActor::Destroyed()
{
    

    Super::Destroyed();
}

void ALuaActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    

    Super::EndPlay(EndPlayReason);
}

std::optional<std::filesystem::path> ALuaActor::GetScriptPath() const
{
    return LuaScriptPath;
}

void ALuaActor::SetScriptPath(const std::optional<std::filesystem::path>& Path)
{
    LuaScriptPath = Path;
}
