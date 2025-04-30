#include "ControlEditorPanel.h"

#include "World/World.h"

#include "Actors/Player.h"
#include "Actors/LightActor.h"
#include "Actors/FireballActor.h"

#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/SphereComp.h"
#include "Components/ParticleSubUVComponent.h"
#include "Components/TextComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Components/RigidbodyComponent.h"

#include "Engine/FLoaderOBJ.h"
#include "Engine/StaticMeshActor.h"
#include "LevelEditor/SLevelEditor.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "tinyfiledialogs.h"

#include "Actors/Cube.h"

#include "Engine/EditorEngine.h"
#include <Actors/HeightFogActor.h>
#include "Actors/PointLightActor.h"
#include "Actors/DirectionalLightActor.h"
#include "Actors/SpotLightActor.h"
#include "Actors/AmbientLightActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Developer/Lua/LuaActor.h"
#include "Developer/Lua/LuaCoachActor.h"
#include "Developer/Lua/LuaHeartActor.h"
#include "Developer/Lua/LuaGameModeActor.h"
#include "Developer/Lua/LuaPlayer.h"

void ControlEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);

    float PanelWidth = (Width) * 0.8f;
    float PanelHeight = 45.0f;

    float PanelPosX = 1.0f;
    float PanelPosY = 1.0f;

    ImVec2 MinSize(300, 50);
    ImVec2 MaxSize(FLT_MAX, 50);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    /* Render Start */
    ImGui::Begin("Control Panel", nullptr, PanelFlags);

    CreateMenuButton(IconSize, IconFont);

    ImGui::SameLine();

    CreateFlagButton();

    ImGui::SameLine();

    CreateModifyButton(IconSize, IconFont);

    ImGui::SameLine();

    /* Get Window Content Region */
    float ContentWidth = ImGui::GetWindowContentRegionMax().x;
    ImGui::SameLine();
    CreateLightSpawnButton(IconSize, IconFont);

    ImGui::SameLine();
    ImGui::PushFont(IconFont);

    CreatePIEButton(IconSize, IconFont);

    ImGui::SameLine();

    /* Move Cursor X Position */
    ImGui::SetCursorPosX(ContentWidth - (IconSize.x * 3.0f + 16.0f));

    CreateSRTButton(IconSize);
    ImGui::PopFont();

    ImGui::End();
}

void ControlEditorPanel::CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9ad", ButtonSize)) // Menu
    {
        bOpenMenu = !bOpenMenu;
    }
    ImGui::PopFont();

    if (bOpenMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 55), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(135, 170), ImGuiCond_Always);

        ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (ImGui::MenuItem("New World"))
        {
            if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
            {
                EditorEngine->NewWorld();
            }
        }

        if (ImGui::MenuItem("Load World"))
        {
            char const* lFilterPatterns[1] = { "*.scene" };
            const char* FileName = tinyfd_openFileDialog("Open Scene File", "", 1, lFilterPatterns, "Scene(.scene) file", 0);

            if (FileName == nullptr)
            {
                tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                ImGui::End();
                return;
            }
            if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
            {
                EditorEngine->NewWorld();
                EditorEngine->LoadWorld(FileName);
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save World"))
        {
            char const* lFilterPatterns[1] = { "*.scene" };
            const char* FileName = tinyfd_saveFileDialog("Save Scene File", "", 1, lFilterPatterns, "Scene(.scene) file");

            if (FileName == nullptr)
            {
                ImGui::End();
                return;
            }
            if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
            {
                EditorEngine->SaveWorld(FileName);
            }

            tinyfd_messageBox("알림", "저장되었습니다.", "ok", "info", 1);
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Import"))
        {
            if (ImGui::MenuItem("Wavefront (.obj)"))
            {
                char const* lFilterPatterns[1] = { "*.obj" };
                const char* FileName = tinyfd_openFileDialog("Open OBJ File", "", 1, lFilterPatterns, "Wavefront(.obj) file", 0);

                if (FileName != nullptr)
                {
                    std::cout << FileName << std::endl;

                    if (FManagerOBJ::CreateStaticMesh(FileName) == nullptr)
                    {
                        tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                    }
                }
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit"))
        {
            ImGui::OpenPopup("프로그램 종료");
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("프로그램 종료", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("정말 프로그램을 종료하시겠습니까?");
            ImGui::Separator();

            float ContentWidth = ImGui::GetWindowContentRegionMax().x;

            /* Move Cursor X Position */
            ImGui::SetCursorPosX(ContentWidth - (160.f + 10.0f));

            if (ImGui::Button("OK", ImVec2(80, 0))) { PostQuitMessage(0); }

            ImGui::SameLine();

            ImGui::SetItemDefaultFocus();
            ImGui::PushID("CancelButtonWithQuitWindow");
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            if (ImGui::Button("Cancel", ImVec2(80, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::EndPopup();
        }

        ImGui::End();
    }
}

void ControlEditorPanel::CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c4", ButtonSize)) // Slider
    {
        ImGui::OpenPopup("SliderControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("SliderControl"))
    {
        ImGui::Text("Grid Scale");
        GridScale = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetGridSize();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##Grid Scale", &GridScale, 0.1f, 1.0f, 20.0f, "%.1f"))
        {
            GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(GridScale);
        }
        ImGui::Separator();

        ImGui::Text("Camera FOV");
        FOV = &GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->ViewFOV;
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##Fov", FOV, 0.1f, 30.0f, 120.0f, "%.1f"))
        {
            //GEngineLoop.GetWorld()->GetCamera()->SetFOV(FOV);

        }
        ImGui::Spacing();

        ImGui::Text("Camera Speed");
        CameraSpeed = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##CamSpeed", &CameraSpeed, 0.1f, 0.198f, 192.0f, "%.1f"))
        {
            GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetCameraSpeed(CameraSpeed);
        }

        ImGui::EndPopup();
    }

    ImGui::SameLine();

    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c8", ButtonSize))
    {
        ImGui::OpenPopup("PrimitiveControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("PrimitiveControl"))
    {
        struct Primitive {
            const char* label;
            int obj;
        };

        static const Primitive primitives[] = {
            { .label= "Cube",      .obj= OBJ_CUBE },
            { .label= "Sphere",    .obj= OBJ_SPHERE },
            { .label= "PointLight", .obj= OBJ_POINTLIGHT },
            { .label= "SpotLight", .obj= OBJ_SPOTLIGHT },
            { .label= "DirectionalLight", .obj= OBJ_DIRECTIONALLGIHT },
            { .label= "AmbientLight", .obj= OBJ_AMBIENTLIGHT },
            { .label= "Particle",  .obj= OBJ_PARTICLE },
            { .label= "Text",      .obj= OBJ_TEXT },
            { .label= "Fireball",  .obj = OBJ_FIREBALL},
            { .label= "Fog",       .obj= OBJ_FOG },
            {.label = "PCM",       .obj = OBJ_PCM },
            { .label= "LuaActor",       .obj= OBJ_LUA_ACTOR },
            { .label = "LuaCoachActor",       .obj = OBJ_LUA_COACH_ACTOR },
            { .label = "LuaHeartActor",       .obj = OBJ_LUA_HEART_ACTOR },
            {.label = "LuaGameModeActor",       .obj = OBJ_LUA_GAMEMODE_ACTOR },
            { .label= "LuaPlayer",       .obj= OBJ_LUA_PLAYER }
        };

        for (const auto& primitive : primitives)
        {
            if (ImGui::Selectable(primitive.label))
            {
                // GEngineLoop.GetWorld()->SpawnObject(static_cast<OBJECTS>(primitive.obj));
                UWorld* World = GEngine->ActiveWorld;
                AActor* SpawnedActor = nullptr;
                switch (static_cast<OBJECTS>(primitive.obj))
                {
                case OBJ_SPHERE:
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_SPHERE"));
                    USphereComp* SphereComp = SpawnedActor->AddComponent<USphereComp>(TEXT("SphereComponent"));
                    SphereComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Sphere.obj"));

                    URigidbodyComponent* RigidbodyComp = SpawnedActor->AddComponent<URigidbodyComponent>(TEXT("RigidbodyComponent"));

                    break;
                }
                case OBJ_CUBE:
                {
                    // TODO: 다른 부분들 전부 Actor만 소환하도록 하고, Component 생성은 Actor가 자체적으로 하도록 변경.
                    ACube* CubeActor = World->SpawnActor<ACube>();
                    CubeActor->SetActorLabel(TEXT("OBJ_CUBE"));
                    break;
                }

                case OBJ_SPOTLIGHT:
                {
                    ASpotLight* SpotActor = World->SpawnActor<ASpotLight>();
                    SpotActor->SetActorLabel(TEXT("OBJ_SPOTLIGHT"));
                    break;
                }
                case OBJ_POINTLIGHT:
                {
                    APointLight* LightActor = World->SpawnActor<APointLight>();
                    LightActor->SetActorLabel(TEXT("OBJ_POINTLIGHT"));
                    break;
                }
                case OBJ_DIRECTIONALLGIHT:
                {
                    ADirectionalLight* LightActor = World->SpawnActor<ADirectionalLight>();
                    LightActor->SetActorLabel(TEXT("OBJ_DIRECTIONALLGIHT"));
                    break;
                }
                case OBJ_AMBIENTLIGHT:
                {
                    AAmbientLight* LightActor = World->SpawnActor<AAmbientLight>();
                    LightActor->SetActorLabel(TEXT("OBJ_AMBIENTLIGHT"));
                    break;
                }
                case OBJ_PARTICLE:
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_PARTICLE"));
                    UParticleSubUVComponent* ParticleComponent = SpawnedActor->AddComponent<UParticleSubUVComponent>(TEXT("ParticleSubUVComponent"));
                    ParticleComponent->SetTexture(L"Assets/Texture/T_Explosion_SubUV.png");
                    ParticleComponent->SetRowColumnCount(6, 6);
                    ParticleComponent->SetRelativeScale3D(FVector(10.0f, 10.0f, 1.0f));
                    ParticleComponent->Activate();
                    SpawnedActor->SetActorTickInEditor(true);
                    break;
                }
                case OBJ_TEXT:
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_TEXT"));
                    UTextComponent* TextComponent = SpawnedActor->AddComponent<UTextComponent>(TEXT("TextComponent"));
                    TextComponent->SetTexture(L"Assets/Texture/font.png");
                    TextComponent->SetRowColumnCount(106, 106);
                    TextComponent->SetText(L"안녕하세요 Jungle 1");

                    break;
                }
                case OBJ_FIREBALL:
                {
                    SpawnedActor = World->SpawnActor<AFireballActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_FIREBALL"));

                    break;
                }
                case OBJ_FOG:
                {
                    SpawnedActor = World->SpawnActor<AHeightFogActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_FOG"));
                    break;
                }
                case OBJ_PCM:
                {
                    SpawnedActor = World->SpawnActor<APlayerCameraManager>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_PCM"));
                }
                case OBJ_LUA_ACTOR:
                {
                    SpawnedActor = World->SpawnActor<ALuaActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_LUA_ACTOR"));
                    break;
                }
                case OBJ_LUA_COACH_ACTOR:
                {
                    SpawnedActor = World->SpawnActor<ALuaCoachActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_LUA_COACH_ACTOR"));
                    break;
                }
                case OBJ_LUA_HEART_ACTOR:
                {
                    SpawnedActor = World->SpawnActor<ALuaHeartActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_LUA_HEART_ACTOR"));
                    break;
                }
                case OBJ_LUA_GAMEMODE_ACTOR:
                {
                    SpawnedActor = World->SpawnActor<ALuaGameModeActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_LUA_GAMEMODE_ACTOR"));
                    break;
                }
                case OBJ_LUA_PLAYER:
                {
                        SpawnedActor = World->SpawnActor<ALuaPlayer>();
                        SpawnedActor->SetActorLabel(TEXT("OBJ_LUA_PLAYER"));
                        CERTSRV_E_BAD_REQUEST_KEY_ARCHIVAL;
                }
                    
                case OBJ_TRIANGLE:
                case OBJ_CAMERA:
                case OBJ_END:
                    break;
                }

                if (SpawnedActor)
                {
                    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
                    Engine->SelectActor(Engine->GetSelectedActor());
                }
                
            }
        }
        ImGui::EndPopup();
    }
}

void ControlEditorPanel::CreateFlagButton() const
{
    auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();

    const char* ViewTypeNames[] = { "Perspective", "Top", "Bottom", "Left", "Right", "Front", "Back" };
    ELevelViewportType ActiveViewType = ActiveViewport->GetViewportType();
    FString TextViewType = ViewTypeNames[ActiveViewType];

    if (ImGui::Button(GetData(TextViewType), ImVec2(120, 32)))
    {
        // toggleViewState = !toggleViewState;
        ImGui::OpenPopup("ViewControl");
    }

    if (ImGui::BeginPopup("ViewControl"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(ViewTypeNames); i++)
        {
            bool bIsSelected = ((int)ActiveViewport->GetViewportType() == i);
            if (ImGui::Selectable(ViewTypeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewportType((ELevelViewportType)i);
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    const char* ViewModeNames[] = { 
        "Lit_Gouraud", "Lit_Lambert", "Lit_Phong", 
        "Unlit", "Wireframe", "Scene Depth", "World Normal"
    };
    uint32 ViewModeCount = sizeof(ViewModeNames) / sizeof(ViewModeNames[0]);
    
    int RawViewMode = (int)ActiveViewport->GetViewMode();
    int SafeIndex = (RawViewMode >= 0) ? (RawViewMode % ViewModeCount) : 0;
    FString ViewModeControl = ViewModeNames[SafeIndex];

    ImVec2 ViewModeTextSize = ImGui::CalcTextSize(GetData(ViewModeControl));

    if (ImGui::Button(GetData(ViewModeControl), ImVec2(30 + ViewModeTextSize.x, 32)))
    {
        ImGui::OpenPopup("ViewModeControl");
    }

    if (ImGui::BeginPopup("ViewModeControl"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(ViewModeNames); i++)
        {
            bool bIsSelected = (static_cast<int>(ActiveViewport->GetViewMode()) == i);
            if (ImGui::Selectable(ViewModeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewMode(static_cast<EViewModeIndex>(i));
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Show", ImVec2(60, 32)))
    {
        ImGui::OpenPopup("ShowControl");
    }

    const char* items[] = { "AABB", "Primitive", "BillBoard", "UUID", "Fog"};
    uint64 ActiveViewportFlags = ActiveViewport->GetShowFlag();

    if (ImGui::BeginPopup("ShowControl"))
    {
        bool selected[IM_ARRAYSIZE(items)] =
        {
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_UUIDText)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Fog)) !=0
        };  // 각 항목의 체크 상태 저장

        for (int i = 0; i < IM_ARRAYSIZE(items); i++)
        {
            ImGui::Checkbox(items[i], &selected[i]);
        }
        ActiveViewport->SetShowFlag(ConvertSelectionToFlags(selected));
        ImGui::EndPopup();
    }
}

void ControlEditorPanel::CreatePIEButton(ImVec2 ButtonSize, ImFont* IconFont) const
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
        return;

    ImVec2 WindowSize = ImGui::GetIO().DisplaySize;

    float CenterX = (WindowSize.x - ButtonSize.x) / 2.5f;

    ImGui::SetCursorScreenPos(ImVec2(CenterX - 40.0f, 10.0f));
    
    if (ImGui::Button("\ue9a8", ButtonSize)) // Play
    {
        UE_LOG(ELogLevel::Display, TEXT("PIE Button Clicked"));
        Engine->StartPIE();
    }

    ImGui::SetCursorScreenPos(ImVec2(CenterX - 10.0f, 10.0f));
    if (ImGui::Button("\ue9e4", ButtonSize)) // Stop
    {
        UE_LOG(ELogLevel::Display, TEXT("Stop Button Clicked"));
        Engine->EndPIE();
    }
    
}

// code is so dirty / Please refactor
void ControlEditorPanel::CreateSRTButton(ImVec2 ButtonSize) const
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    AEditorPlayer* Player = Engine->GetEditorPlayer();

    ImVec4 ActiveColor = ImVec4(0.00f, 0.00f, 0.85f, 1.0f);

    EControlMode ControlMode = Player->GetControlMode();

    if (ControlMode == CM_TRANSLATION)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9bc", ButtonSize)) // Move
    {
        Player->SetMode(CM_TRANSLATION);
    }
    if (ControlMode == CM_TRANSLATION)
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (ControlMode == CM_ROTATION)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9d3", ButtonSize)) // Rotate
    {
        Player->SetMode(CM_ROTATION);
    }
    if (ControlMode == CM_ROTATION)
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    if (ControlMode == CM_SCALE)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9ab", ButtonSize)) // Scale
    {
        Player->SetMode(CM_SCALE);
    }
    if (ControlMode == CM_SCALE)
    {
        ImGui::PopStyleColor();
    }
}

uint64 ControlEditorPanel::ConvertSelectionToFlags(const bool selected[]) const
{
    uint64 flags = EEngineShowFlags::None;

    if (selected[0])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_AABB);
    if (selected[1])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Primitives);
    if (selected[2])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_BillboardText);
    if (selected[3])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_UUIDText);
    if (selected[4])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Fog);
    return flags;
}


void ControlEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}


void ControlEditorPanel::CreateLightSpawnButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    UWorld* World = GEngine->ActiveWorld;
    ImVec2 WindowSize = ImGui::GetIO().DisplaySize;

    float CenterX = (WindowSize.x - ButtonSize.x) / 2.5f;

    ImGui::SetCursorScreenPos(ImVec2(CenterX + 40.0f, 10.0f));
    const char* label = "Light";
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 padding = ImGui::GetStyle().FramePadding;
    ImVec2 buttonSize = ImVec2(
        textSize.x + padding.x * 2.0f,
        textSize.y + padding.y * 2.0f
    );
    buttonSize.y = ButtonSize.y;
    if (ImGui::Button("Light", buttonSize))
    {
        ImGui::OpenPopup("LightGeneratorControl");
    }

    if (ImGui::BeginPopup("LightGeneratorControl"))
    {
        struct LightGeneratorMode {
            const char* label;
            int mode;
        };

        static const LightGeneratorMode modes[] = {
            {.label = "Generate",      .mode = ELightGridGenerator::Generate },
            {.label = "Delete",    .mode = ELightGridGenerator::Delete },
            {.label = "Reset", .mode = ELightGridGenerator::Reset },
        };

        for (const auto& mode : modes)
        {
            if (ImGui::Selectable(mode.label))
            {
                switch (mode.mode)
                {
                case ELightGridGenerator::Generate:
                    LightGridGenerator.GenerateLight(World);
                    break;
                case ELightGridGenerator::Delete:
                    LightGridGenerator.DeleteLight(World);
                    break;
                case ELightGridGenerator::Reset:
                    LightGridGenerator.Reset(World);
                    break;
                }
            }
        }

        ImGui::EndPopup();
    }
}
