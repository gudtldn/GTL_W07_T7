#include "Engine.h"

#include "EditorEngine.h"
#include "Developer/Lua/LuaManager.h"
#include "UnrealEd/SceneManager.h"
#include "UObject/Casts.h"
#include "World/World.h"

UEngine* GEngine = nullptr;

void UEngine::Init()
{
    FLuaManager::Get().Initialize();
}

void UEngine::Tick(float DeltaTime)
{
    FLuaManager::Get().CheckForScriptChanges();
}

FWorldContext* UEngine::GetWorldContextFromWorld(const UWorld* InWorld)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->World() == InWorld)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

FWorldContext& UEngine::CreateNewWorldContext(EWorldType InWorldType)
{
    FWorldContext* NewWorldContext = new FWorldContext();
    WorldList.Add(NewWorldContext);
    NewWorldContext->WorldType = InWorldType;
    NewWorldContext->ContextHandle = FName(*FString::Printf(TEXT("WorldContext_%d"), NextWorldContextHandle++));

    return *NewWorldContext;
}


void UEngine::LoadWorld(const FString& FileName) const
{
    SceneManager::LoadSceneFromJsonFile(*FileName, *ActiveWorld);
}

void UEngine::SaveWorld(const FString& FileName) const
{
    SceneManager::SaveSceneToJsonFile(*FileName, *ActiveWorld);
}
