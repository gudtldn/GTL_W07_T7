#include "PropertyEditorPanel.h"
#include <filesystem>
#include <shellapi.h>

#include "tinyfiledialogs.h"
#include "World/World.h"
#include "Actors/Player.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/Light/AmbientLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/FLoaderOBJ.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
#include "Engine/Engine.h"
#include "Components/HeightFogComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/Collision/BoxComponent.h"
#include "Components/Collision/CapsuleComponent.h"
#include "Components/Collision/SphereComponent.h"
#include "Developer/Lua/LuaActor.h"
#include "GameFramework/Actor.h"
#include "Engine/AssetManager.h"
#include "UObject/UObjectIterator.h"

#include "Renderer/Shadow/SpotLightShadowMap.h"
#include "Renderer/Shadow/PointLightShadowMap.h"
#include "Renderer/Shadow/DirectionalShadowMap.h"

void PropertyEditorPanel::Render()
{
    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.65f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = (Height) * 0.3f + 15.0f;

    ImVec2 MinSize(140, 370);
    ImVec2 MaxSize(FLT_MAX, 900);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    /* Render Start */
    ImGui::Begin("Detail", nullptr, PanelFlags);

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
        return;
    AEditorPlayer* player = Engine->GetEditorPlayer();
    AActor* PickedActor = Engine->GetSelectedActor();

    if (ALuaActor* LuaActor = Cast<ALuaActor>(PickedActor))
    {
        namespace fs = std::filesystem;

        const fs::path SolutionPath = fs::current_path().parent_path();
        const fs::path LuaFolderPath = SolutionPath / "GameJam/Lua";

        const std::optional<fs::path>& LuaPath = LuaActor->GetScriptPath();

        const ImVec2 WindowMax = ImGui::GetWindowContentRegionMax();
        if (LuaPath.has_value())
        {
            ImGui::Text("Lua Script: %s", LuaPath.value().lexically_relative(LuaFolderPath).generic_string().c_str());
        }
        else
        {
            ImGui::Text("Lua Script: None");
        }

        // 정상적인 파일이 있는경우
        if (LuaPath.has_value())
        {
            const fs::path& SomePath = LuaPath.value();
            if (fs::exists(LuaPath.value()))
            {
                if (ImGui::Button("Open In Editor", ImVec2(WindowMax.x / 2.0f - 10.0f, 32)))
                {
                    HINSTANCE hInst = ShellExecute(
                        nullptr,
                        L"open",
                        SomePath.c_str(),
                        nullptr,
                        nullptr,
                        SW_SHOWNORMAL
                    );
    
                    if (reinterpret_cast<INT_PTR>(hInst) <= 32)
                    {
                        ImGui::OpenPopup("Error");
                        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
                        ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                        ImGui::Text("Failed to open Lua File");
                        ImGui::EndPopup();
                    }
                }
                ImGui::SameLine();
            }
            else
            {
                UE_LOG(ELogLevel::Error, "[%s]Invalid Lua Script Path", *LuaActor->GetName());
                LuaActor->SetScriptPath(std::nullopt);
            }
        }

        static std::optional<fs::path> SelectedPath;
        if (ImGui::Button("Select Lua File", ImVec2(LuaPath.has_value() ? WindowMax.x / 2.0f - 10.0f : WindowMax.x, 32)))
        {
            if (LuaPath.has_value())
            {
                SelectedPath = LuaPath;
            }
            else
            {
                SelectedPath.reset();
            }
            ImGui::OpenPopup("Select Lua File");
        }

        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Select Lua File", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
        {
            if (ImGui::BeginCombo(
                "##Lua Script List", SelectedPath.value_or("None").filename().generic_string().c_str(), ImGuiComboFlags_HeightLarge
            ))
            {
                if (fs::exists(LuaFolderPath))
                {
                    for (const auto& Entry : fs::recursive_directory_iterator(LuaFolderPath))
                    {
                        if (Entry.is_regular_file())
                        {
                            const std::string& FileDisplay = Entry.path().lexically_relative(LuaFolderPath).generic_string();
                            if (ImGui::Selectable(FileDisplay.c_str(), SelectedPath == Entry.path()))
                            {
                                SelectedPath = Entry;
                            }
                        }
                    }
                }
                else
                {
                    fs::create_directories(LuaFolderPath);
                }

                if (ImGui::Selectable("Create New Lua Script", false))
                {
                    char const* FilterPatterns[] = { "*.lua" };
                    const char* SelectedFilePath = tinyfd_saveFileDialog(
                        "Create Lua Script",
                        (LuaFolderPath / "NewLuaScript.lua").generic_string().c_str(),
                        std::size(FilterPatterns),
                        FilterPatterns,
                        "Lua(.lua) file"
                    );

                    if (SelectedFilePath)
                    {
                        // TempPath가 GameJam/Lua폴더 안에 있는지
                        fs::path TempPath = SelectedFilePath;
                        if (LuaFolderPath.compare(TempPath.parent_path()) <= 0)
                        {
                            fs::create_directories(TempPath.parent_path());
                            fs::copy_file(
                                fs::current_path() / "Engine/Source/Developer/Lua/LuaTemplate/LuaActorTemplate.lua",
                                TempPath
                            );
                            SelectedPath = std::move(TempPath);
                        }
                        else
                        {
                            UE_LOG(ELogLevel::Error, "Invalid Lua Script Path");
                        }
                    }
                }

                ImGui::EndCombo();
            }

            const ImVec2 ContentHalfSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.0f - 4.0f, 0);
            if (ImGui::Button("Select", ContentHalfSize))
            {
                LuaActor->SetScriptPath(SelectedPath);
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ContentHalfSize))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    if (PickedActor)
    {
        ImGui::SetItemDefaultFocus();
        // TreeNode 배경색을 변경 (기본 상태)
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            Location = PickedActor->GetActorLocation();
            Rotation = PickedActor->GetActorRotation();
            Scale = PickedActor->GetActorScale();

            FImGuiWidget::DrawVec3Control("Location", Location, 0, 85);
            ImGui::Spacing();

            FImGuiWidget::DrawRot3Control("Rotation", Rotation, 0, 85);
            ImGui::Spacing();

            FImGuiWidget::DrawVec3Control("Scale", Scale, 0, 85);
            ImGui::Spacing();

            PickedActor->SetActorLocation(Location);
            PickedActor->SetActorRotation(Rotation);
            PickedActor->SetActorScale(Scale);

            std::string coordiButtonLabel;
            if (player->GetCoordMode() == ECoordMode::CDM_WORLD)
                coordiButtonLabel = "World";
            else if (player->GetCoordMode() == ECoordMode::CDM_LOCAL)
                coordiButtonLabel = "Local";

            if (ImGui::Button(coordiButtonLabel.c_str(), ImVec2(ImGui::GetWindowContentRegionMax().x * 0.9f, 32)))
            {
                player->AddCoordiMode();
            }
            ImGui::TreePop(); // 트리 닫기
        }
        ImGui::PopStyleColor();
    }

    if (PickedActor)
    {
        if (ImGui::Button("Duplicate"))
        {
            UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
            AActor* NewActor = Engine->ActiveWorld->DuplicateActor(Engine->GetSelectedActor());
            Engine->SelectActor(NewActor);
        }
    }

    if(PickedActor)
        if (UPointLightComponent* pointlightObj = PickedActor->GetComponentByClass<UPointLightComponent>())
        {
            int pointNum = 0;
            for (const auto iter : TObjectRange<UPointLightComponent>())
            {
                if (iter != pointlightObj) {
                    pointNum++;
                }
                else {
                    break;
                }
            }

            GEngineLoop.Renderer.PointLightShadowMapPass->RenderLinearDepth(pointNum, pointlightObj);

            // Shadow Depth Map 시각화
            TArray<ID3D11ShaderResourceView*> shaderSRVs = GEngineLoop.Renderer.PointLightShadowMapPass->GetShadowViewSRVArray();
            TArray<FVector> directions = GEngineLoop.Renderer.PointLightShadowMapPass->GetDirectionArray();
            TArray<FVector> ups = GEngineLoop.Renderer.PointLightShadowMapPass->GetUpArray();

            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

            if (ImGui::TreeNodeEx("PointLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawColorProperty("Light Color",
                    [&]() { return pointlightObj->GetLightColor(); },
                    [&](FLinearColor c) { pointlightObj->SetLightColor(c); });

                float Intensity = pointlightObj->GetIntensity();
                if (ImGui::SliderFloat("Intensity", &Intensity, 0.0f, 160.0f, "%.1f"))
                    pointlightObj->SetIntensity(Intensity);

                float Radius = pointlightObj->GetRadius();
                if (ImGui::SliderFloat("Radius", &Radius, 0.01f, 200.f, "%.1f")) {
                    pointlightObj->SetRadius(Radius);
                }

                // ─ Shadow Map 미리보기 (1열) ─
                ImGui::Separator();
                ImGui::Text("Shadow Maps (6 faces):");

                const int   mapsCount = shaderSRVs.Num();
                const float imgSize = 256.0f; // 원하는 크기로 조정

                for (int i = 0; i < mapsCount; ++i)
                {
                    ImGui::Text("Direction %1.f %1.f %1.f", directions[i].X, directions[i].Y, directions[i].Z);
                    ImGui::Text("Up %1.f %1.f %1.f", ups[i].X, ups[i].Y, ups[i].Z);
                    ImTextureID texID = (ImTextureID)shaderSRVs[i];
                    ImGui::Image(texID, ImVec2(imgSize, imgSize));
                    ImGui::Spacing();    // 이미지 사이에 약간의 여백
                }

                ImGui::TreePop();
            }

            ImGui::PopStyleColor();
        }

    if (PickedActor)
        if (USpotLightComponent* SpotLightComp = PickedActor->GetComponentByClass<USpotLightComponent>())
        {
            int spotNum = 0;
            for (const auto iter : TObjectRange<USpotLightComponent>())
            {
                if (iter != SpotLightComp) {
                    spotNum++;
                }
                else {
                    break;
                }
            }

            FEngineLoop::Renderer.SpotLightShadowMapPass->RenderLinearDepth(spotNum);

            // Shadow Depth Map 시각화
            ID3D11ShaderResourceView* shaderSRV = FEngineLoop::Renderer.SpotLightShadowMapPass->GetShadowDebugSRV();
            //FVector direction = GEngineLoop.Renderer.PointLightShadowMapPass->GetDirection();
            //FVector up = GEngineLoop.Renderer.PointLightShadowMapPass->GetUp();

            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

            if (ImGui::TreeNodeEx("SpotLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawColorProperty(
                    "Light Color",
                    [&] { return SpotLightComp->GetLightColor(); },
                    [&](FLinearColor c) { SpotLightComp->SetLightColor(c); }
                );

                float Intensity = SpotLightComp->GetIntensity();
                if (ImGui::SliderFloat("Intensity", &Intensity, 0.0f, 5000.0f, "%.1f"))
                    SpotLightComp->SetIntensity(Intensity);

                float Radius = SpotLightComp->GetRadius();
                if (ImGui::SliderFloat("Radius", &Radius, 0.01f, 200.f, "%.1f"))
                {
                    SpotLightComp->SetRadius(Radius);
                }

                LightDirection = SpotLightComp->GetDirection();
                FImGuiWidget::DrawVec3Control("Direction", LightDirection, 0, 85);

                float OuterDegree = SpotLightComp->GetOuterDegree();
                float InnerDegree = SpotLightComp->GetInnerDegree();

                if (ImGui::SliderFloat("InnerDegree", &InnerDegree, 0.f, 90.0f, "%.1f"))
                {
                    SpotLightComp->SetInnerDegree(InnerDegree);
                    SpotLightComp->SetOuterDegree(
                        FMath::Max(InnerDegree, OuterDegree)
                    );
                }

                if (ImGui::SliderFloat("OuterDegree", &OuterDegree, 0.f, 90.f, "%.1f"))
                {
                    SpotLightComp->SetOuterDegree(OuterDegree);
                    SpotLightComp->SetInnerDegree(
                        FMath::Min(OuterDegree, InnerDegree)
                    );
                }

                // ─ Shadow Map 미리보기 (1열) ─
                ImGui::Separator();
                ImGui::Text("Testing SpotLight:");

                const float imgSize = 256.0f; // 원하는 크기로 조정

                ImGui::Text("Direction %.01f %.01f %.01f", LightDirection.X, LightDirection.Y, LightDirection.Z);
                ImTextureID texID = (ImTextureID)shaderSRV;
                ImGui::Image(texID, ImVec2(imgSize, imgSize));
                ImGui::Spacing(); // 이미지 사이에 약간의 여백

                ImGui::TreePop();
            }

            ImGui::PopStyleColor();
        }

    if (PickedActor)
        if (UDirectionalLightComponent* dirlightObj = PickedActor->GetComponentByClass<UDirectionalLightComponent>())
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

            int directionalNum = 0;
            for (const auto iter : TObjectRange<UDirectionalLightComponent>())
            {
                if (iter != dirlightObj) {
                    directionalNum++;
                }
                else {
                    break;
                }
            }

            // Shadow Depth Map 시각화
            ID3D11ShaderResourceView* shaderSRV = FEngineLoop::Renderer.DirectionalShadowMap->GetShadowViewSRV(directionalNum);


            if (ImGui::TreeNodeEx("DirectionalLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawColorProperty("Light Color",
                    [&]() { return dirlightObj->GetLightColor(); },
                    [&](FLinearColor c) { dirlightObj->SetLightColor(c); });

                float Intensity = dirlightObj->GetIntensity();
                if (ImGui::SliderFloat("Intensity", &Intensity, 0.0f, 150.0f, "%.1f"))
                    dirlightObj->SetIntensity(Intensity);

                LightDirection = dirlightObj->GetDirection();
                FImGuiWidget::DrawVec3Control("Direction", LightDirection, 0, 85);

                // ─ Shadow Map 미리보기 (1열) ─
                ImGui::Separator();
                ImGui::Text("Directional Light Shadow SRV:");

                const float imgSize = 256.0f; // 원하는 크기로 조정

                ImGui::Text("Direction %.01f %.01f %.01f", LightDirection.X, LightDirection.Y, LightDirection.Z);
                ImTextureID texID = (ImTextureID)shaderSRV;
                ImGui::Image(texID, ImVec2(imgSize, imgSize));
                ImGui::Spacing(); // 이미지 사이에 약간의 여백

                ImGui::TreePop();
            }

            ImGui::PopStyleColor();
        }

    if(PickedActor)
        if (UAmbientLightComponent* ambientLightObj = PickedActor->GetComponentByClass<UAmbientLightComponent>())
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

            if (ImGui::TreeNodeEx("AmbientLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                DrawColorProperty("Light Color",
                    [&]() { return ambientLightObj->GetLightColor(); },
                    [&](FLinearColor c) { ambientLightObj->SetLightColor(c); });
                ImGui::TreePop();
            }

            ImGui::PopStyleColor();
        }

    if (PickedActor)
        if (UProjectileMovementComponent* ProjectileComp = (PickedActor->GetComponentByClass<UProjectileMovementComponent>()))
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

            if (ImGui::TreeNodeEx("Projectile Movement Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                float InitialSpeed = ProjectileComp->GetInitialSpeed();
                if (ImGui::InputFloat("InitialSpeed", &InitialSpeed, 0.f, 10000.0f, "%.1f"))
                    ProjectileComp->SetInitialSpeed(InitialSpeed);

                float MaxSpeed = ProjectileComp->GetMaxSpeed();
                if (ImGui::InputFloat("MaxSpeed", &MaxSpeed, 0.f, 10000.0f, "%.1f"))
                    ProjectileComp->SetMaxSpeed(MaxSpeed);

                float Gravity = ProjectileComp->GetGravity();
                if (ImGui::InputFloat("Gravity", &Gravity, 0.f, 10000.f, "%.1f"))
                    ProjectileComp->SetGravity(Gravity); 
                
                float ProjectileLifetime = ProjectileComp->GetLifetime();
                if (ImGui::InputFloat("Lifetime", &ProjectileLifetime, 0.f, 10000.f, "%.1f"))
                    ProjectileComp->SetLifetime(ProjectileLifetime);

                FVector currentVelocity = ProjectileComp->GetVelocity();

                float velocity[3] = { currentVelocity.X, currentVelocity.Y, currentVelocity.Z };

                if (ImGui::InputFloat3("Velocity", velocity, "%.1f")) {
                    ProjectileComp->SetVelocity(FVector(velocity[0], velocity[1], velocity[2]));
                }
                
                ImGui::TreePop();
            }

            ImGui::PopStyleColor();
        }

    if (PickedActor) 
    {
        if (URigidbodyComponent* RigidbodyComp = (PickedActor->GetComponentByClass<URigidbodyComponent>())) 
        {
            RenderForRigidbody(RigidbodyComp);
        }
    }

    RenderCollisionSection(PickedActor);

    // TODO: 추후에 RTTI를 이용해서 프로퍼티 출력하기
    if (PickedActor)
        if (UTextComponent* textOBj = Cast<UTextComponent>(PickedActor->GetRootComponent()))
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            if (ImGui::TreeNodeEx("Text Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
            {
                if (textOBj) {
                    textOBj->SetTexture(L"Assets/Texture/font.png");
                    textOBj->SetRowColumnCount(106, 106);
                    FWString wText = textOBj->GetText();
                    int len = WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr);
                    std::string u8Text(len, '\0');
                    WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, u8Text.data(), len, nullptr, nullptr);

                    static char buf[256];
                    strcpy_s(buf, u8Text.c_str());

                    ImGui::Text("Text: ", buf);
                    ImGui::SameLine();
                    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                    if (ImGui::InputText("##Text", buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        textOBj->ClearText();
                        int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
                        FWString newWText(wlen, L'\0');
                        MultiByteToWideChar(CP_UTF8, 0, buf, -1, newWText.data(), wlen);
                        textOBj->SetText(newWText.c_str());
                    }
                    ImGui::PopItemFlag();
                }
                ImGui::TreePop();
            }
            ImGui::PopStyleColor();
        }

    // TODO: 추후에 RTTI를 이용해서 프로퍼티 출력하기
    if (PickedActor)
        if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(PickedActor->GetRootComponent()))
        {
            RenderForStaticMesh(StaticMeshComponent);
            RenderForMaterial(StaticMeshComponent);
        }

    if (PickedActor)
        if (UHeightFogComponent* FogComponent = Cast<UHeightFogComponent>(PickedActor->GetRootComponent()))
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            if (ImGui::TreeNodeEx("Exponential Height Fog", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
            {
                FLinearColor currColor = FogComponent->GetFogColor();

                float r = currColor.R;
                float g = currColor.G;
                float b = currColor.B;
                float a = currColor.A;
                float h, s, v;
                float lightColor[4] = { r, g, b, a };

                // Fog Color
                if (ImGui::ColorPicker4("##Fog Color", lightColor,
                    ImGuiColorEditFlags_DisplayRGB |
                    ImGuiColorEditFlags_NoSidePreview |
                    ImGuiColorEditFlags_NoInputs |
                    ImGuiColorEditFlags_Float))

                {

                    r = lightColor[0];
                    g = lightColor[1];
                    b = lightColor[2];
                    a = lightColor[3];
                    FogComponent->SetFogColor(FLinearColor(r, g, b, a));
                }
                RGBToHSV(r, g, b, h, s, v);
                // RGB/HSV
                bool changedRGB = false;
                bool changedHSV = false;

                // RGB
                ImGui::PushItemWidth(50.0f);
                if (ImGui::DragFloat("R##R", &r, 0.001f, 0.f, 1.f)) changedRGB = true;
                ImGui::SameLine();
                if (ImGui::DragFloat("G##G", &g, 0.001f, 0.f, 1.f)) changedRGB = true;
                ImGui::SameLine();
                if (ImGui::DragFloat("B##B", &b, 0.001f, 0.f, 1.f)) changedRGB = true;
                ImGui::Spacing();

                // HSV
                if (ImGui::DragFloat("H##H", &h, 0.1f, 0.f, 360)) changedHSV = true;
                ImGui::SameLine();
                if (ImGui::DragFloat("S##S", &s, 0.001f, 0.f, 1)) changedHSV = true;
                ImGui::SameLine();
                if (ImGui::DragFloat("V##V", &v, 0.001f, 0.f, 1)) changedHSV = true;
                ImGui::PopItemWidth();
                ImGui::Spacing();

                if (changedRGB && !changedHSV)
                {
                    // RGB -> HSV
                    RGBToHSV(r, g, b, h, s, v);
                    FogComponent->SetFogColor(FLinearColor(r, g, b, a));
                }
                else if (changedHSV && !changedRGB)
                {
                    // HSV -> RGB
                    HSVToRGB(h, s, v, r, g, b);
                    FogComponent->SetFogColor(FLinearColor(r, g, b, a));
                }

                float FogDensity = FogComponent->GetFogDensity();
                if (ImGui::SliderFloat("Density", &FogDensity, 0.00f, 3.0f))
                {
                    FogComponent->SetFogDensity(FogDensity);
                }

                float FogDistanceWeight = FogComponent->GetFogDistanceWeight();
                if (ImGui::SliderFloat("Distance Weight", &FogDistanceWeight, 0.00f, 3.0f))
                {
                    FogComponent->SetFogDistanceWeight(FogDistanceWeight);
                }

                float FogHeightFallOff = FogComponent->GetFogHeightFalloff();
                if (ImGui::SliderFloat("Height Fall Off", &FogHeightFallOff, 0.001f, 0.15f))
                {
                    FogComponent->SetFogHeightFalloff(FogHeightFallOff);
                }

                float FogStartDistance = FogComponent->GetStartDistance();
                if (ImGui::SliderFloat("Start Distance", &FogStartDistance, 0.00f, 50.0f))
                {
                    FogComponent->SetStartDistance(FogStartDistance);
                }

                float FogEndtDistance = FogComponent->GetEndDistance();
                if (ImGui::SliderFloat("End Distance", &FogEndtDistance, 0.00f, 50.0f))
                {
                    FogComponent->SetEndDistance(FogEndtDistance);
                }

                ImGui::TreePop();
            }
            ImGui::PopStyleColor();
        }
    ImGui::End();
}

void PropertyEditorPanel::RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const
{
    float mx = FMath::Max(r, FMath::Max(g, b));
    float mn = FMath::Min(r, FMath::Min(g, b));
    float delta = mx - mn;

    v = mx;

    if (mx == 0.0f) {
        s = 0.0f;
        h = 0.0f;
        return;
    }
    else {
        s = delta / mx;
    }

    if (delta < 1e-6) {
        h = 0.0f;
    }
    else {
        if (r >= mx) {
            h = (g - b) / delta;
        }
        else if (g >= mx) {
            h = 2.0f + (b - r) / delta;
        }
        else {
            h = 4.0f + (r - g) / delta;
        }
        h *= 60.0f;
        if (h < 0.0f) {
            h += 360.0f;
        }
    }
}

void PropertyEditorPanel::HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const
{
    // h: 0~360, s:0~1, v:0~1
    float c = v * s;
    float hp = h / 60.0f;             // 0~6 구간
    float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
    float m = v - c;

    if (hp < 1.0f) { r = c;  g = x;  b = 0.0f; }
    else if (hp < 2.0f) { r = x;  g = c;  b = 0.0f; }
    else if (hp < 3.0f) { r = 0.0f; g = c;  b = x; }
    else if (hp < 4.0f) { r = 0.0f; g = x;  b = c; }
    else if (hp < 5.0f) { r = x;  g = 0.0f; b = c; }
    else { r = c;  g = 0.0f; b = x; }

    r += m;  g += m;  b += m;
}

void PropertyEditorPanel::RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp) const
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Static Mesh", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ImGui::Text("StaticMesh");
        ImGui::SameLine();

        const TMap<FName, FAssetInfo> Assets = UAssetManager::Get().GetAssetRegistry();

        const char* PreviewName = "None";
        if (StaticMeshComp->GetStaticMesh())
        {
            PreviewName = *StaticMeshComp->GetStaticMesh()->GetRenderData()->DisplayName;
        }

        if (ImGui::BeginCombo("##StaticMesh", PreviewName, ImGuiComboFlags_None))
        {
            for (const auto& Asset : Assets)
            {
                if (ImGui::Selectable(GetData(Asset.Value.AssetName.ToString()), false))
                {
                    FString MeshName = Asset.Value.PackagePath.ToString() + "/" + Asset.Value.AssetName.ToString();
                    UStaticMesh* StaticMesh = FManagerOBJ::GetStaticMesh(MeshName.ToWideString());
                    if (StaticMesh)
                    {
                        StaticMeshComp->SetStaticMesh(StaticMesh);
                    }
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TreePop();
    }
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ImGui::Text("Add");
        ImGui::SameLine();

        TArray<UClass*> CompClasses;
        GetChildOfClass(UActorComponent::StaticClass(), CompClasses);
        CompClasses.Sort([](auto const& Class1, auto const& Class2) -> bool
        {
            return Class1->GetName() < Class2->GetName();
        });

        if (ImGui::BeginCombo("##AddComponent", "Components", ImGuiComboFlags_None))
        {
            for (UClass* Class : CompClasses)
            {
                if (ImGui::Selectable(GetData(Class->GetName()), false))
                {
                    // TODO: 임시로 static uint32 NewCompIndex사용
                    static uint32 NewCompIndex = 0;
                    USceneComponent* NewComp = Cast<USceneComponent>(
                        StaticMeshComp->GetOwner()->AddComponent(
                            Class,
                            FString::Printf(TEXT("%s_%d"), *Class->GetName(), NewCompIndex++)
                        )
                    );
                    if (NewComp)
                    {
                        NewComp->SetupAttachment(StaticMeshComp);
                    }
                    // 추후 Engine으로부터 SelectedComponent 받아서 선택된 Comp 아래로 붙일 수있으면 붙이기.
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}


void PropertyEditorPanel::RenderForMaterial(UStaticMeshComponent* StaticMeshComp)
{
    if (StaticMeshComp->GetStaticMesh() == nullptr)
    {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        for (uint32 i = 0; i < StaticMeshComp->GetNumMaterials(); ++i)
        {
            if (ImGui::Selectable(GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    std::cout << GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()) << std::endl;
                    SelectedMaterialIndex = i;
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }

        if (ImGui::Button("    +    ")) {
            IsCreateMaterial = true;
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("SubMeshes", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        auto subsets = StaticMeshComp->GetStaticMesh()->GetRenderData()->MaterialSubsets;
        for (uint32 i = 0; i < subsets.Num(); ++i)
        {
            std::string temp = "subset " + std::to_string(i);
            if (ImGui::Selectable(temp.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    StaticMeshComp->SetselectedSubMeshIndex(i);
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }
        std::string temp = "clear subset";
        if (ImGui::Selectable(temp.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(0))
                StaticMeshComp->SetselectedSubMeshIndex(-1);
        }

        ImGui::TreePop();
    }

    ImGui::PopStyleColor();

    if (SelectedMaterialIndex != -1)
    {
        RenderMaterialView(SelectedStaticMeshComp->GetMaterial(SelectedMaterialIndex));
    }
    if (IsCreateMaterial) {
        RenderCreateMaterialView();
    }
}

void PropertyEditorPanel::RenderMaterialView(UMaterial* Material)
{
    ImGui::SetNextWindowSize(ImVec2(380, 400), ImGuiCond_Once);
    ImGui::Begin("Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    FVector MatDiffuseColor = Material->GetMaterialInfo().Diffuse;
    FVector MatSpecularColor = Material->GetMaterialInfo().Specular;
    FVector MatAmbientColor = Material->GetMaterialInfo().Ambient;
    FVector MatEmissiveColor = Material->GetMaterialInfo().Emissive;

    float dr = MatDiffuseColor.X;
    float dg = MatDiffuseColor.Y;
    float db = MatDiffuseColor.Z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Material Name |");
    ImGui::SameLine();
    ImGui::Text(*Material->GetMaterialInfo().MaterialName);
    ImGui::Separator();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", (float*)&DiffuseColorPick, BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        Material->SetDiffuse(NewColor);
    }

    float sr = MatSpecularColor.X;
    float sg = MatSpecularColor.Y;
    float sb = MatSpecularColor.Z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", (float*)&SpecularColorPick, BaseFlag))
    {
        FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        Material->SetSpecular(NewColor);
    }


    float ar = MatAmbientColor.X;
    float ag = MatAmbientColor.Y;
    float ab = MatAmbientColor.Z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", (float*)&AmbientColorPick, BaseFlag))
    {
        FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        Material->SetAmbient(NewColor);
    }


    float er = MatEmissiveColor.X;
    float eg = MatEmissiveColor.Y;
    float eb = MatEmissiveColor.Z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", (float*)&EmissiveColorPick, BaseFlag))
    {
        FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        Material->SetEmissive(NewColor);
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Choose Material");
    ImGui::Spacing();

    ImGui::Text("Material Slot Name |");
    ImGui::SameLine();
    ImGui::Text(GetData(SelectedStaticMeshComp->GetMaterialSlotNames()[SelectedMaterialIndex].ToString()));

    ImGui::Text("Override Material |");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    // 메테리얼 이름 목록을 const char* 배열로 변환
    std::vector<const char*> materialChars;
    for (const auto& material : FManagerOBJ::GetMaterials()) {
        materialChars.push_back(*material.Value->GetMaterialInfo().MaterialName);
    }

    //// 드롭다운 표시 (currentMaterialIndex가 범위를 벗어나지 않도록 확인)
    //if (currentMaterialIndex >= FManagerOBJ::GetMaterialNum())
    //    currentMaterialIndex = 0;

    if (ImGui::Combo("##MaterialDropdown", &CurMaterialIndex, materialChars.data(), FManagerOBJ::GetMaterialNum())) {
        UMaterial* material = FManagerOBJ::GetMaterial(materialChars[CurMaterialIndex]);
        SelectedStaticMeshComp->SetMaterial(SelectedMaterialIndex, material);
    }

    if (ImGui::Button("Close"))
    {
        SelectedMaterialIndex = -1;
        SelectedStaticMeshComp = nullptr;
    }

    ImGui::End();
}

void PropertyEditorPanel::RenderCreateMaterialView()
{
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Once);
    ImGui::Begin("Create Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    ImGui::Text("New Name");
    ImGui::SameLine();
    static char materialName[256] = "New Material";
    // 기본 텍스트 입력 필드
    ImGui::SetNextItemWidth(128);
    if (ImGui::InputText("##NewName", materialName, IM_ARRAYSIZE(materialName))) {
        tempMaterialInfo.MaterialName = materialName;
    }

    FVector MatDiffuseColor = tempMaterialInfo.Diffuse;
    FVector MatSpecularColor = tempMaterialInfo.Specular;
    FVector MatAmbientColor = tempMaterialInfo.Ambient;
    FVector MatEmissiveColor = tempMaterialInfo.Emissive;

    float dr = MatDiffuseColor.X;
    float dg = MatDiffuseColor.Y;
    float db = MatDiffuseColor.Z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Set Property");
    ImGui::Indent();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", (float*)&DiffuseColorPick, BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        tempMaterialInfo.Diffuse = NewColor;
    }

    float sr = MatSpecularColor.X;
    float sg = MatSpecularColor.Y;
    float sb = MatSpecularColor.Z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", (float*)&SpecularColorPick, BaseFlag))
    {
        FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        tempMaterialInfo.Specular = NewColor;
    }


    float ar = MatAmbientColor.X;
    float ag = MatAmbientColor.Y;
    float ab = MatAmbientColor.Z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", (float*)&AmbientColorPick, BaseFlag))
    {
        FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        tempMaterialInfo.Ambient = NewColor;
    }


    float er = MatEmissiveColor.X;
    float eg = MatEmissiveColor.Y;
    float eb = MatEmissiveColor.Z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", (float*)&EmissiveColorPick, BaseFlag))
    {
        FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        tempMaterialInfo.Emissive = NewColor;
    }
    ImGui::Unindent();

    ImGui::NewLine();
    if (ImGui::Button("Create Material")) {
        FManagerOBJ::CreateMaterial(tempMaterialInfo);
    }

    ImGui::NewLine();
    if (ImGui::Button("Close"))
    {
        IsCreateMaterial = false;
    }

    ImGui::End();
}

void PropertyEditorPanel::RenderForRigidbody(URigidbodyComponent* RigidbodyComp)
{
    if (RigidbodyComp == nullptr)
        return;

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("RigidBody", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Reset Rigidbody")) 
        {
            RigidbodyComp->ResetRigidbody();
        }

        // --- Velocity ---
        {
            FVector velVec = RigidbodyComp->GetVelocity();
            float vel[3] = { velVec.X, velVec.Y, velVec.Z };
            if (ImGui::InputFloat3("Velocity", vel))
            {
                RigidbodyComp->SetVelocity(FVector(vel[0], vel[1], vel[2]));
            }
        }

        // --- Angular Velocity ---
        {
            FVector angVelVec = RigidbodyComp->GetAngularVelocity();
            float angVel[3] = { angVelVec.X, angVelVec.Y, angVelVec.Z };
            if (ImGui::InputFloat3("AngularVelocity", angVel))
            {
                RigidbodyComp->SetAngularVelocity(FVector(angVel[0], angVel[1], angVel[2]));
            }
        }

        ImGui::Separator();

        // --- Accumulated Force & Torque (read-only) ---
        {
            FVector accForce = RigidbodyComp->GetAccumulatedForce();
            FVector accTorque = RigidbodyComp->GetAccumulatedTorque();

            float forceAccum[3] = { accForce.X,  accForce.Y,  accForce.Z };
            float torqueAccum[3] = { accTorque.X, accTorque.Y, accTorque.Z };

            // Accumulated는 관찰만 가능
            ImGui::InputFloat3("Force Accumulated", forceAccum);
            ImGui::InputFloat3("Torque Accumulated", torqueAccum);
        }

        ImGui::Separator();

        // Force / Point 입력 버퍼
        static float force[3] = { 0,0,0 };
        static float point[3] = { 0,0,0 };

        ImGui::InputFloat3("Force", force);
        ImGui::InputFloat3("Point", point);

        if (ImGui::Button("Apply Force"))
        {
            RigidbodyComp->ApplyForceAtPoint(
                FVector(force[0], force[1], force[2]),
                FVector(point[0], point[1], point[2])
            );
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply Impulse"))
        {
            RigidbodyComp->ApplyImpulseAtPoint(
                FVector(force[0], force[1], force[2]),
                FVector(point[0], point[1], point[2])
            );
        }

        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}

void PropertyEditorPanel::RenderCollisionSection(AActor* PickedActor)
{
    if (PickedActor)
    {
        if (UBoxComponent* BoxComponent = Cast<UBoxComponent>(PickedActor->GetComponentByClass<UBoxComponent>()))
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            if (ImGui::TreeNodeEx("Box Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {

                bool bGenerateOverlapEvents = BoxComponent->bGenerateOverlapEvents;
                if (ImGui::Checkbox("GenerateOverlapEvents", &bGenerateOverlapEvents))
                {
                    BoxComponent->bGenerateOverlapEvents = bGenerateOverlapEvents;
                }

                bool bBlockComponent = BoxComponent->bBlockComponent;
                if (ImGui::Checkbox("SimulatePhysics", &bBlockComponent))
                {
                    BoxComponent->bBlockComponent = bBlockComponent;
                }
                
                BoxExtent = BoxComponent->GetUnscaledBoxExtent();
                FImGuiWidget::DrawVec3Control("Extent", BoxExtent, 0, 85);
                BoxComponent->SetBoxExtent(BoxExtent);
                
                ImGui::TreePop();
            }
            ImGui::PopStyleColor();
        }

        if (USphereComponent* SphereComponent = Cast<USphereComponent>(PickedActor->GetComponentByClass<USphereComponent>()))
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            if (ImGui::TreeNodeEx("Sphere Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {

                bool bGenerateOverlapEvents = SphereComponent->bGenerateOverlapEvents;
                if (ImGui::Checkbox("GenerateOverlapEvents", &bGenerateOverlapEvents))
                {
                    SphereComponent->bGenerateOverlapEvents = bGenerateOverlapEvents;
                }

                bool bBlockComponent = SphereComponent->bBlockComponent;
                if (ImGui::Checkbox("SimulatePhysics", &bBlockComponent))
                {
                    SphereComponent->bBlockComponent = bBlockComponent;
                }
                
                float Radius = SphereComponent->GetUnscaledSphereRadius();
                ImGui::DragFloat("Radius", &Radius, 0.01f, 0.0f, FLT_MAX);
                SphereComponent->SetSphereRadius(Radius);
                
                ImGui::TreePop();
            }
            ImGui::PopStyleColor();
        }

        if (UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(PickedActor->GetComponentByClass<UCapsuleComponent>()))
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            if (ImGui::TreeNodeEx("Capsule Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {

                bool bGenerateOverlapEvents = CapsuleComponent->bGenerateOverlapEvents;
                if (ImGui::Checkbox("GenerateOverlapEvents", &bGenerateOverlapEvents))
                {
                    CapsuleComponent->bGenerateOverlapEvents = bGenerateOverlapEvents;
                }

                bool bBlockComponent = CapsuleComponent->bBlockComponent;
                if (ImGui::Checkbox("SimulatePhysics", &bBlockComponent))
                {
                    CapsuleComponent->bBlockComponent = bBlockComponent;
                }
                
                float Radius = CapsuleComponent->GetUnScaledCapsuleRadius();
                float HalfHeight = CapsuleComponent->GetUnScaledCapsuleHalfHeight();
                
                ImGui::DragFloat("Radius", &Radius, 0.01f, 0.0f, FLT_MAX);
                ImGui::DragFloat("Half Height", &HalfHeight, 0.01f, 0.0f, FLT_MAX);

                CapsuleComponent->SetCapsuleSize(Radius, HalfHeight);
                
                ImGui::TreePop();
            }
            ImGui::PopStyleColor();
        }
        ImGui::Dummy(ImVec2(0, 5)); // Padding
    }
}

void PropertyEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
