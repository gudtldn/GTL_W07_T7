#include "LuaCoachActor.h"
#include "LuaManager.h"
#include "Engine/Classes/Components/Collision/BoxComponent.h"
#include "Classes/Components/StaticMeshComponent.h"


ALuaCoachActor::ALuaCoachActor()
{
    Affection = 0;
    BoxComponent = AddComponent<UBoxComponent>(TEXT("BoxComponent"));
    BoxComponent->AttachToComponent(RootComponent);

    std::filesystem::path LuaFolderPath = std::filesystem::current_path().parent_path() / "GameJam" / "Lua";
    std::filesystem::path coachScript = LuaFolderPath / "Coach.lua";

    LuaScriptPath = coachScript;
    
    // 새 경로가 유효하다면 새로 등록 및 로드
    if (LuaScriptPath.has_value() && !LuaScriptPath->empty())
    {
        FLuaManager::Get().RegisterActor(this, *LuaScriptPath);
        sol::protected_function Factory = FLuaManager::Get().GetActorFactory(*LuaScriptPath);
        if (Factory.valid())
        {
            // ... (BeginPlay와 유사하게 SelfTable 생성 및 BeginPlay 호출) ...
            sol::protected_function_result Result = Factory(this);
            if (Result.valid() && Result.get_type() == sol::type::table)
            {
                SelfTable = Result;
                SelfTable["cpp_actor"] = this;
                (void)CallLuaFunction("BeginPlay"); // 경로 변경 시에도 BeginPlay 호출
            }
            else
            {
                // Log an error if the factory function result is invalid or not a table
                UE_LOG(ELogLevel::Error, "Lua factory function for script '%s' returned an invalid result or non-table type.", LuaScriptPath.value().generic_string().c_str());
                SelfTable = sol::lua_nil;
            }
        }
        else
        {
            // Log an error if the factory function itself is invalid
            UE_LOG(ELogLevel::Error, "Lua factory function for script '%s' is invalid.", LuaScriptPath.value().generic_string().c_str());
            SelfTable = sol::lua_nil;
        }
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

    if (ObjName == L"Contents/GameJam\\BSH\\Pbr/eye_mask.obj") 
    {
        CoachName = "Seung-Hyeon Baek";
    }


    (void)CallLuaFunction("SetCoachName", CoachName);
}

void ALuaCoachActor::SetAffection(int NewAffection)
{
    Affection = NewAffection;
}

int ALuaCoachActor::GetAffection() const
{
    return Affection;
}
