#pragma once
#include "Core/HAL/PlatformType.h"
#include "Engine/ResourceMgr.h"
#include "LevelEditor/SlateAppMessageHandler.h"
#include "Renderer/Renderer.h"
#include "Subsystem/CollisionSubsystem.h"
#include "UnrealEd/PrimitiveDrawBatch.h"


class FSlateAppMessageHandler;
class UnrealEd;
class UImGuiManager;
class UWorld;
class FEditorViewportClient;
class SSplitterV;
class SSplitterH;
class FGraphicDevice;
class SLevelEditor;
class APlayerCameraManager;

class FDXDBufferManager;

enum EGameState : uint8
{
    None,
    Lobby,
    Play,
};

class FEngineLoop
{
public:
    FEngineLoop();

    int32 PreInit();
    int32 Init(HINSTANCE hInstance);
    void Render() const;
    void Tick();
    void Exit();

    void GetClientSize(uint32& OutWidth, uint32& OutHeight) const;

private:
    void WindowInit(HINSTANCE hInstance);
    static LRESULT CALLBACK AppWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    void UpdateUI() const;

public:
    static FGraphicsDevice GraphicDevice;
    static FRenderer Renderer;
    static UPrimitiveDrawBatch PrimitiveDrawBatch;
    static FResourceMgr ResourceManager;
    static uint32 TotalAllocationBytes;
    static uint32 TotalAllocationCount;
    
    static bool bIsGameMode;
    static EGameState GameState;
    
    HWND AppWnd;
    bool bIsEnableShaderHotReload = true; // TODO: ImGui에서 변경가능하게 설정하기

    APlayerCameraManager* PCM;
private:
    UImGuiManager* UIMgr;
    //TODO: GWorld 제거, Editor들 EditorEngine으로 넣기

    std::unique_ptr<FSlateAppMessageHandler> AppMessageHandler;
    SLevelEditor* LevelEditor;
    UnrealEd* UnrealEditor;
    FDXDBufferManager* BufferManager; //TODO: UEngine으로 옮겨야함.
    FCollisionSubsystem* CollisionSubsystem;

    bool bIsExit = false;
    int32 TargetFPS = 60;

public:
    SLevelEditor* GetLevelEditor() const { return LevelEditor; }
    UnrealEd* GetUnrealEditor() const { return UnrealEditor; }
    FCollisionSubsystem* GetCollisionSubsystem() const { return CollisionSubsystem; }

    FSlateAppMessageHandler* GetAppMessageHandler() const { return AppMessageHandler.get(); }
};
