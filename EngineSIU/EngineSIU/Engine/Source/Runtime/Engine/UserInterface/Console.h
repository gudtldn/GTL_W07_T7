#pragma once
#include "Container/Array.h"
#include "D3D11RHI/GraphicDevice.h"
#include "HAL/PlatformType.h"
#include "ImGUI/imgui.h"
#include "PropertyEditor/IWindowToggleable.h"

static consteval const char* GetFileName(const char* Path)
{
    const char* Name = Path;
    while (*Path)
    {
        if (*Path == '/' || *Path == '\\')
        {
            Name = Path + 1;
        }
        Path++;
    }
    return Name;
}

#define __FILENAME__ GetFileName(__FILE__)

#define UE_LOG(Level, Fmt, ...) FConsole::GetInstance().AddLog(Level, "[%s:%d]" Fmt, __FILENAME__, __LINE__, __VA_ARGS__)


enum class ELogLevel : uint8
{
    Display,
    Warning,
    Error
};


class FStatOverlay
{
public:
    // TODO: 추후에 enum 비트연산으로 바꿔도 좋을듯
    union
    {
        struct  // NOLINT(clang-diagnostic-nested-anon-types)
        {
            uint8 bShowFps : 1;
            uint8 bShowMemory : 1;
            uint8 bShowLight : 1;
            uint8 bShowRender : 1;
        };
        uint8 StatFlags = 0; // 기본적으로 다 끄기
    };

    void ToggleStat(const std::string& Command);
    void Render(ID3D11DeviceContext* Context, UINT InWidth, UINT InHeight) const;

private:
};

class FConsole : public IWindowToggleable
{
private:
    FConsole() = default;

public:
    static FConsole& GetInstance(); // 참조 반환으로 변경

    void Clear();
    void AddLog(ELogLevel Level, const ANSICHAR* Fmt, ...);
    void AddLog(ELogLevel Level, const WIDECHAR* Fmt, ...);
    // void AddLog(ELogLevel Level, const char* fmt, va_list args);
    void Draw();
    void ExecuteCommand(const std::string& Command);
    void OnResize(HWND hWnd);

    virtual void Toggle() override
    {
        if (bWasOpen)
        {
            bWasOpen = false;
        }
        else
        {
            bWasOpen = true;
        }
    }

public:
    struct FLogEntry
    {
        ELogLevel Level;
        FString Message;
    };

    TArray<FLogEntry> Items;
    char InputBuf[256] = "";
    bool bScrollToBottom = false;

    ImGuiTextFilter Filter; // 필터링을 위한 ImGuiTextFilter

    // 추가된 멤버 변수들
    bool bShowLogTemp = true; // LogTemp 체크박스
    bool bShowWarning = true; // Warning 체크박스
    bool bShowError = true;   // Error 체크박스

    bool bWasOpen = true;
    bool bShowFps = false;
    bool bShowMemory = false;

    FStatOverlay Overlay;

private:
    bool bExpand = true;
    uint32 Width;
    uint32 Height;
};
