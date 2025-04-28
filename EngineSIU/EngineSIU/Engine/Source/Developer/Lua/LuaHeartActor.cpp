#include "LuaHeartActor.h"
#include "Engine/Classes/Components/RigidbodyComponent.h"
#include "Engine/Classes/Components/Collision/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FLoaderOBJ.h"

ALuaHeartActor::ALuaHeartActor()
{
    RigidbodyComp = AddComponent<URigidbodyComponent>(TEXT("RigidbodyComponent"));
    SphereComp = AddComponent<USphereComponent>(TEXT("SphereComponent"));
    SphereComp->SetupAttachment(RootComponent);
    SphereComp->SetSphereRadius(2.0f);

    UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(RootComponent);
    StaticMeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/GameJam/NewHeart.obj"));

    StaticMeshComp->SetRelativeRotation(FRotator(0, 90, 0));


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

void ALuaHeartActor::SetPlayerIndex(int InPlayerIndex)
{
    (void)CallLuaFunction("SetPlayerIndex", InPlayerIndex);
}
