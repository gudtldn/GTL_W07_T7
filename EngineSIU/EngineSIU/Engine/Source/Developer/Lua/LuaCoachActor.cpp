#include "LuaCoachActor.h"
#include "Engine/Classes/Components/Collision/BoxComponent.h"
#include "Classes/Components/StaticMeshComponent.h"

ALuaCoachActor::ALuaCoachActor()
{
    BoxComponent = AddComponent<UBoxComponent>(TEXT("BoxComponent"));
    BoxComponent->AttachToComponent(RootComponent);

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

void ALuaCoachActor::BeginPlay()
{
    Super::BeginPlay();

    FWString ObjName;
    UStaticMeshComponent* StaticComp = Cast<UStaticMeshComponent>(RootComponent);
    if (StaticComp->GetStaticMesh()) 
    {
        ObjName = StaticComp->GetStaticMesh()->GetOjbectName();
    }

    std::string CoachName = "NoneName";

    if (ObjName == L"Contents/GameJam\\LCG\\Shaded/base.obj") 
    {
        CoachName = "Changgeun Lim";
    }


    (void)CallLuaFunction("SetCoachName", CoachName);
}
