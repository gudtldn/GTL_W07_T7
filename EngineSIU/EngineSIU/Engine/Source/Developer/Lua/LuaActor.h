#pragma once
#include <optional>
#include <filesystem>

#include "Engine/StaticMeshActor.h"


class ALuaActor : public AStaticMeshActor
{
    DECLARE_CLASS(ALuaActor, AStaticMeshActor)

public:
    ALuaActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    std::optional<std::filesystem::path> GetScriptPath() const;
    void SetScriptPath(const std::optional<std::filesystem::path>& Path);

private:
    // TODO: .scene에 저장을 어떻게 하지?
    std::optional<std::filesystem::path> LuaScriptPath;
};
