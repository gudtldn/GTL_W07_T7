#include "GameLobbyPanel.h"

#include "Developer/FMOD/SoundManager.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "ImGui/imgui_internal.h"

GameLobbyPanel::GameLobbyPanel()
    : Width(0)
    , Height(0)
{
    Textures.Add(FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/lcg.png"));
    Textures.Add(FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/bsh.png"));
    Textures.Add(FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/ksw.png"));
    Textures.Add(FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/jungle.png"));
    Textures.Add(FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/logo.png"));
    Textures.Add(FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/titledesc.png"));

    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();

    /** Font load */
    FontSize24 = io.Fonts->Fonts[DEFAULT_FONT_24];
    FontSize32 = io.Fonts->Fonts[DEFAULT_FONT_32];
    FontSize48 = io.Fonts->Fonts[DEFAULT_FONT_48];
}

void GameLobbyPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    
    ImGuiWindowFlags WindowFlags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse;
    
    if (FEngineLoop::GameState != Lobby && FEngineLoop::GameState != None)
    {
        ImDrawList* DrawList = ImGui::GetForegroundDrawList();
        DrawList->AddCircleFilled(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), 5.f, IM_COL32(255,0,0,255));
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(-5,-5), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x + 10, io.DisplaySize.y + 10), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    ImGui::Begin("Main", nullptr, WindowFlags);

    DrawBackground(io.DisplaySize);
    
    ImGui::PushFont(FontSize48);
    DrawTitle(io.DisplaySize);
    ImGui::PopFont();

    ImGui::PopStyleColor();
    ImGui::End();
}

void GameLobbyPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}

bool GameLobbyPanel::CreateButton(const char* Label, const ImVec2& ButtonSize, const ImVec4& InColorNormal, const ImVec4& InColorHover)
{
    ImGui::PushID(Label);

    bool IsClicked = ImGui::InvisibleButton(Label, ButtonSize);

    ImU32 ColorNormal = ImGui::GetColorU32(InColorNormal);
    ImU32 ColorHover = ImGui::GetColorU32(InColorHover);

    // InvisibleButton이 차지한 사각형 정보 가져오기
    ImVec2 ButtonMin = ImGui::GetItemRectMin();
    ImVec2 ButtonMax = ImGui::GetItemRectMax();
    ImVec2 ButtonCenter = ImVec2(
        (ButtonMin.x + ButtonMax.x) * 0.5f,
        (ButtonMin.y + ButtonMax.y) * 0.5f
    );

    // 텍스트 크기 측정
    ImVec2 TextSize = ImGui::CalcTextSize(Label);

    // 텍스트 출력 위치 계산 (Button 중앙에 정렬)
    ImVec2 TextPos = ImVec2(
        ButtonCenter.x - TextSize.x * 0.5f,
        ButtonCenter.y - TextSize.y * 0.5f
    );

    // 원하는 색으로 텍스트 출력
    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    ImU32 TextColor = ImGui::IsItemHovered() ? ColorHover : ColorNormal;

    DrawList->AddText(TextPos, TextColor, Label);

    ImGui::PopID();
    
    return IsClicked;
}

void GameLobbyPanel::DrawBackground(const ImVec2& WindowSize)
{
    ImVec2 SpaceSize = ImVec2(WindowSize.x * 0.7f, WindowSize.y);
    ImVec2 StartPos = ImVec2(WindowSize.x * 0.3f, 0);
    
    static float ElapsedTime = 0.0f;
    static int CurrentImageIndex = 0;
    static int NextImageIndex = (CurrentImageIndex + 1) % (Textures.Num() - 3);
    
    ImGuiIO& io = ImGui::GetIO();
    ElapsedTime += io.DeltaTime;
    
    constexpr float TransitionDuration = 3.0f; // During 3sec.
    float Progress = ElapsedTime / TransitionDuration;
    Progress = ImClamp(Progress, 0.0f, 1.0f);

    ImGui::Image((ImTextureID)(intptr_t)Textures[3]->TextureSRV, WindowSize);
    
    ImGui::SetCursorPos(ImVec2(StartPos.x + SpaceSize.x * 0.5f - Textures[CurrentImageIndex]->Width * 0.5f, WindowSize.y * 0.5f - Textures[CurrentImageIndex]->Height * 0.2f));
    ImGui::Image((ImTextureID)(intptr_t)Textures[CurrentImageIndex]->TextureSRV,
    ImVec2(Textures[CurrentImageIndex]->Width * 0.7f, Textures[CurrentImageIndex]->Height * 0.7f),
        ImVec2(0,0),
        ImVec2(1,1),
        ImVec4(1,1,1,1.0f - Progress)
        );
    
    ImGui::SetCursorPos(ImVec2(StartPos.x + SpaceSize.x * 0.5f - Textures[NextImageIndex]->Width * 0.5f, WindowSize.y * 0.5f - Textures[NextImageIndex]->Height * 0.2f));
    ImGui::Image((ImTextureID)(intptr_t)Textures[NextImageIndex]->TextureSRV,
        ImVec2(Textures[NextImageIndex]->Width * 0.7f, Textures[NextImageIndex]->Height * 0.7f),
        ImVec2(0,0),
        ImVec2(1,1),
        ImVec4(1,1,1,Progress)
        );

    if (ElapsedTime >= TransitionDuration + 2)
    {
        ElapsedTime = 0.0f;
        CurrentImageIndex = NextImageIndex;
        NextImageIndex = (CurrentImageIndex + 1) % (Textures.Num() - 3);
    }

    ImGui::SetCursorPos(ImVec2(WindowSize.x * 0.5f + (Textures[5]->Width * 0.4f) * 0.5f, 30.f));
    ImGui::Image((ImTextureID)(intptr_t)Textures[5]->TextureSRV, ImVec2(Textures[5]->Width * 0.4f, Textures[5]->Height * 0.4f));
}

void GameLobbyPanel::DrawTitle(const ImVec2& WindowSize)
{
    ImVec2 SpaceSize = ImVec2(WindowSize.x * 0.3f, WindowSize.y);

    float TargetWidth = SpaceSize.x * 0.9f;

    float AspectRatio = (float)Textures[4]->Height / (float)Textures[4]->Width;
    float TargetHeight = TargetWidth * AspectRatio;

    // 로고 좌상단 위치
    ImVec2 LogoTopLeft = ImVec2(
        SpaceSize.x * 0.5f - TargetWidth * 0.5f,
        TargetHeight * 0.25f
    );

    // 로고 중심 위치
    ImVec2 LogoCenter = ImVec2(
        LogoTopLeft.x + TargetWidth * 0.5f,
        LogoTopLeft.y + TargetHeight * 0.5f
    );

    // 로고 출력
    ImGui::SetCursorPos(LogoTopLeft);
    ImGui::Image(
        (ImTextureID)(intptr_t)Textures[4]->TextureSRV,
        ImVec2(TargetWidth, TargetHeight)
    );

    // 버튼 사이즈
    ImVec2 ButtonSize = ImVec2(250, 100);

    // 첫 번째 버튼 위치 (로고 아래에서 Y축 오프셋 적용)
    ImVec2 Button1CenterPos = ImVec2(
        LogoCenter.x,
        WindowSize.y - WindowSize.y * 0.3f + ButtonSize.y * 0.5f
    );

    // 두 번째 버튼 위치 (첫 번째 버튼 밑에)
    ImVec2 Button2CenterPos = ImVec2(
        LogoCenter.x,
        Button1CenterPos.y + ButtonSize.y
    );

    // 폰트 적용
    ImGui::PushFont(FontSize48);

    // Shoot 버튼
    ImGui::SetCursorPos(ImVec2(
        Button1CenterPos.x - ButtonSize.x * 0.5f,
        Button1CenterPos.y - ButtonSize.y * 0.5f
    ));
    if (CreateButton("Shoot", ButtonSize,
        ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
        ImVec4(1.0f, 0.0f, 0.0f, 1.0f)))
    {
        // Shoot 눌렀을 때 처리
        FEngineLoop::GameState = Play;
        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            EditorEngine->StartPIE();
            FSoundManager::Get()->StopSound("Contents\\Sound\\lobby.mp3");
            FSoundManager::Get()->MainChannel = FSoundManager::Get()->PlaySound("Contents\\Sound\\game.mp3", true);
        }
    }

    // Quit 버튼
    ImGui::SetCursorPos(ImVec2(
        Button2CenterPos.x - ButtonSize.x * 0.5f,
        Button2CenterPos.y - ButtonSize.y * 0.5f
    ));
    if (CreateButton("Quit", ButtonSize,
        ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
        ImVec4(1.0f, 0.0f, 0.0f, 1.0f)))
    {
        FEngineLoop::bIsGameMode = false;
        FEngineLoop::GameState = None;
        if (FSoundManager::Get()->GetMainChannel())
        {
            FSoundManager::Get()->MainChannel->stop();
        }
    }

    ImGui::PopFont();
}
