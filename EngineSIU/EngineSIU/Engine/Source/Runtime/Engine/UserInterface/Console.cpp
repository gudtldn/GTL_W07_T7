#include "Console.h"
#include <cstdarg>
#include <cstdio>

#include "Actors/DirectionalLightActor.h"
#include "Actors/PointLightActor.h"
#include "Actors/SpotLightActor.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectHash.h"


void FStatOverlay::ToggleStat(const std::string& Command)
{
    if (Command == "stat fps")
    {
        bShowFps = true;
        bShowRender = true;
    }
    else if (Command == "stat memory")
    {
        bShowMemory = true;
        bShowRender = true;
    }
    else if (Command == "stat light")
    {
        bShowLight = true;
        bShowRender = true;
    }
    else if (Command == "stat all")
    {
        StatFlags = 0xFF;
    }
    else if (Command == "stat none")
    {
        // 모든 Flag 끄기
        StatFlags = 0x00;
    }
}

void FStatOverlay::Render(ID3D11DeviceContext* Context, UINT InWidth, UINT InHeight) const
{
    if (!bShowRender)
    {
        return;
    }

    const ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;
    // 창 크기를 화면의 50%로 설정합니다.
    const ImVec2 WindowSize = {DisplaySize.x * 0.5f, DisplaySize.y * 0.5f};
    // 창을 중앙에 배치하기 위해 위치를 계산합니다.
    const ImVec2 WindowPos = {(DisplaySize.x - WindowSize.x) * 0.5f, (DisplaySize.y - WindowSize.y) * 0.5f};

    ImGui::SetNextWindowPos(WindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(WindowSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
    ImGui::Begin(
        "Stat Overlay", nullptr,
        ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoInputs
        | ImGuiWindowFlags_AlwaysAutoResize
    );

    if (bShowFps)
    {
        // FPS 정보 출력
        const float Fps = ImGui::GetIO().Framerate;
        ImGui::Text("FPS: %.1f (%.1f ms)", Fps, 1000.0f / Fps);
    }

    if (bShowMemory)
    {
        ImGui::SeparatorText("Memory Usage");
        ImGui::Text("Allocated Object Count: %llu", FPlatformMemory::GetAllocationCount<EAT_Object>());
        ImGui::Text("Allocated Object Memory: %llu Byte", FPlatformMemory::GetAllocationBytes<EAT_Object>());
        ImGui::Text("Allocated Container Count: %llu", FPlatformMemory::GetAllocationCount<EAT_Container>());
        ImGui::Text("Allocated Container Memory: %llu Byte", FPlatformMemory::GetAllocationBytes<EAT_Container>());

        ImGui::SeparatorText("Shader Usage");
        ImGui::Text(
            "Vertex Shader: %.2f MB",
            static_cast<float>(FEngineLoop::Renderer.ShaderManager->GetTotalPixelShaderSize()) / 1024.0f / 1024.0f
        );
        ImGui::Text(
            "Pixel Shader: %.2f MB",
            static_cast<float>(FEngineLoop::Renderer.ShaderManager->GetTotalVertexShaderSize()) / 1024.0f / 1024.0f
        );
    }

    if (bShowLight)
    {
        ImGui::SeparatorText("Light Info");
        ImGui::Text("Num of Point Light: %u", GetNumOfObjectsByClass(APointLight::StaticClass()));
        ImGui::Text("Num of Spot Light: %u", GetNumOfObjectsByClass(ASpotLight::StaticClass()));
        ImGui::Text("Num of Directional Light: %u", GetNumOfObjectsByClass(ADirectionalLight::StaticClass()));
    }

    ImGui::PopStyleColor(2);
    ImGui::End();
}

// 싱글톤 인스턴스 반환
FConsole& FConsole::GetInstance()
{
    static FConsole Instance;
    return Instance;
}

// 로그 초기화
void FConsole::Clear()
{
    Items.Empty();
}

// 로그 추가
void FConsole::AddLog(ELogLevel Level, const ANSICHAR* Fmt, ...)
{
    va_list Args;
    va_start(Args, Fmt);
    // AddLog(Level, Fmt, Args);

    char Buf[1024];
    vsnprintf_s(Buf, sizeof(Buf), _TRUNCATE, Fmt, Args);

    Items.Emplace(Level, std::string(Buf));
    bScrollToBottom = true;
    va_end(Args);
}

void FConsole::AddLog(ELogLevel Level, const WIDECHAR* Fmt, ...)
{
    va_list Args;
    va_start(Args, Fmt);
    // AddLog(Level, Fmt, Args);

    wchar_t Buf[1024];
    _vsnwprintf_s(Buf, sizeof(Buf), _TRUNCATE, Fmt, Args);

    Items.Emplace(Level, FString(Buf).ToAnsiString());
    bScrollToBottom = true;
    va_end(Args);
}

// void FConsole::AddLog(ELogLevel Level, const char* Fmt, va_list Args)
// {
//     char Buf[1024];
//     vsnprintf(Buf, sizeof(Buf), Fmt, Args);
//
//     Items.Emplace(Level, std::string(Buf));
//     bScrollToBottom = true;
// }


// 콘솔 창 렌더링
void FConsole::Draw()
{
    if (!bWasOpen) return;
    // 창 크기 및 위치 계산
    const ImVec2 DisplaySize = ImGui::GetIO().DisplaySize;

    // 콘솔 창의 크기와 위치 설정
    const float ExpandedHeight = DisplaySize.y * 0.4f; // 확장된 상태일 때 높이 (예: 화면의 40%)
    constexpr float CollapsedHeight = 30.0f;           // 축소된 상태일 때 높이
    const float CurrentHeight = bExpand ? ExpandedHeight : CollapsedHeight;

    // 왼쪽 하단에 위치하도록 계산 (창의 좌측 하단이 화면의 좌측 하단에 위치)
    const ImVec2 WindowSize(DisplaySize.x * 0.5f, CurrentHeight); // 폭은 화면의 40%
    const ImVec2 WindowPos(0, DisplaySize.y - CurrentHeight);

    // 창을 표시하고 닫힘 여부 확인
    Overlay.Render(FEngineLoop::GraphicDevice.DeviceContext, Width, Height);

    // 창 위치와 크기를 고정
    ImGui::SetNextWindowPos(WindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(WindowSize, ImGuiCond_Always);

    bExpand = ImGui::Begin("Console", &bWasOpen);

    // 창을 접었을 경우 UI를 표시하지 않음
    if (!bExpand)
    {
        ImGui::End();
        return;
    }

    // 버튼 UI (로그 수준별 추가)
    if (ImGui::Button("Clear")) { Clear(); }
    ImGui::SameLine();
    if (ImGui::Button("Copy")) { ImGui::LogToClipboard(); }

    ImGui::Separator();

    // 필터 입력 창
    ImGui::Text("Filter:");
    ImGui::SameLine();

    Filter.Draw("##Filter", 100);

    ImGui::SameLine();

    // 로그 수준을 선택할 체크박스
    ImGui::Checkbox("Show Display", &bShowLogTemp);
    ImGui::SameLine();
    ImGui::Checkbox("Show Warning", &bShowWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Show Error", &bShowError);

    ImGui::Separator();
    // 로그 출력 (필터 적용)
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    TArray CopyItems = Items;
    for (const FLogEntry& Item : CopyItems)
    {
        if (!Filter.PassFilter(Item.Message.ToAnsiString().c_str())) continue;

        // 로그 수준에 맞는 필터링
        if ((Item.Level == ELogLevel::Display && !bShowLogTemp) ||
            (Item.Level == ELogLevel::Warning && !bShowWarning) ||
            (Item.Level == ELogLevel::Error && !bShowError))
        {
            continue;
        }

        // 색상 지정
        ImVec4 Color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        switch (Item.Level)
        {
        case ELogLevel::Display: Color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            break; // 기본 흰색
        case ELogLevel::Warning: Color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
            break; // 노란색
        case ELogLevel::Error: Color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            break; // 빨간색
        }

        ImGui::TextColored(Color, "%s", *Item.Message);
    }
    if (bScrollToBottom)
    {
        ImGui::SetScrollHereY(1.0f);
        bScrollToBottom = false;
    }
    ImGui::EndChild();

    ImGui::Separator();

    // 입력창
    if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (InputBuf[0])
        {
            AddLog(ELogLevel::Display, ">> %s", InputBuf);
            ExecuteCommand(InputBuf);
            bScrollToBottom = true; // 자동 스크롤
        }
        InputBuf[0] = '\0';
    }

    ImGui::End();
}

void FConsole::ExecuteCommand(const std::string& Command)
{
    AddLog(ELogLevel::Display, "Executing command: %s", Command.c_str());

    if (Command == "clear")
    {
        Clear();
    }
    else if (Command == "help")
    {
        AddLog(ELogLevel::Display, "Available commands:");
        AddLog(ELogLevel::Display, " - clear: Clears the console");
        AddLog(ELogLevel::Display, " - help: Shows available commands");
        AddLog(ELogLevel::Display, " - stat fps: Toggle FPS display");
        AddLog(ELogLevel::Display, " - stat memory: Toggle Memory display");
        AddLog(ELogLevel::Display, " - stat light: Toggle Light display");
        AddLog(ELogLevel::Display, " - stat all: Show all stat overlays");
        AddLog(ELogLevel::Display, " - stat none: Hide all stat overlays");
    }
    else if (Command.starts_with("stat "))
    {
        // stat 명령어 처리
        Overlay.ToggleStat(Command);
    }
    else
    {
        AddLog(ELogLevel::Error, "Unknown command: %s", Command.c_str());
    }
}

void FConsole::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}
