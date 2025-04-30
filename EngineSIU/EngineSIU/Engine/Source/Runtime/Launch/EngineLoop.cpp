#include "EngineLoop.h"
#include "ImGuiManager.h"
#include "UnrealClient.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Developer/Lua/LuaManager.h"
#include "Developer/FMOD/SoundManager.h"
#include "Engine/EditorEngine.h"
#include "LevelEditor/SLevelEditor.h"
#include "PropertyEditor/ViewportTypePanel.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/UnrealEd.h"
#include "World/World.h"
#include "Engine/Classes/Camera/PlayerCameraManager.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

FGraphicsDevice FEngineLoop::GraphicDevice;
FRenderer FEngineLoop::Renderer;
UPrimitiveDrawBatch FEngineLoop::PrimitiveDrawBatch;
FResourceMgr FEngineLoop::ResourceManager;
uint32 FEngineLoop::TotalAllocationBytes = 0;
uint32 FEngineLoop::TotalAllocationCount = 0;
bool FEngineLoop::bIsGameMode = false;
EGameState FEngineLoop::GameState = None;

FEngineLoop::FEngineLoop()
    : AppWnd(nullptr)
    , UIMgr(nullptr)
    , LevelEditor(nullptr)
    , UnrealEditor(nullptr)
    , BufferManager(nullptr)
{
}

int32 FEngineLoop::PreInit()
{
    return 0;
}

int32 FEngineLoop::Init(HINSTANCE hInstance)
{
    /* must be initialized before window. */
    WindowInit(hInstance);

    UnrealEditor = new UnrealEd();
    BufferManager = new FDXDBufferManager();
    UIMgr = new UImGuiManager;
    AppMessageHandler = std::make_unique<FSlateAppMessageHandler>();
    LevelEditor = new SLevelEditor();
    CollisionSubsystem = new FCollisionSubsystem();

    
    GraphicDevice.Initialize(AppWnd);
    BufferManager->Initialize(GraphicDevice.Device, GraphicDevice.DeviceContext);
    Renderer.Initialize(&GraphicDevice, BufferManager);
    PrimitiveDrawBatch.Initialize(&GraphicDevice);
    UIMgr->Initialize(AppWnd, GraphicDevice.Device, GraphicDevice.DeviceContext);
    ResourceManager.Initialize(&Renderer, &GraphicDevice);
    UnrealEditor->Initialize();
    
    uint32 ClientWidth = 0;
    uint32 ClientHeight = 0;
    GetClientSize(ClientWidth, ClientHeight);
    LevelEditor->Initialize(ClientWidth, ClientHeight);

    GEngine = FObjectFactory::ConstructObject<UEditorEngine>(nullptr);
    GEngine->Init();

    FSoundManager::Get()->Initialize();
    // FSoundManager::Get()->PlaySound("Contents\\Sound\\background.mp3", FSoundManager::Get().MainChannel, true);
    
    UpdateUI();

    return 0;
}

void FEngineLoop::Render() const
{
    GraphicDevice.Prepare();

    // ShadowMap Render 시키기
    Renderer.RenderShadowMap();

    
    if (LevelEditor->IsMultiViewport())
    {
        std::shared_ptr<FEditorViewportClient> ActiveViewportCache = GetLevelEditor()->GetActiveViewportClient();
        for (int i = 0; i < 4; ++i)
        {
            LevelEditor->SetActiveViewportClient(i);
            Renderer.Render(LevelEditor->GetActiveViewportClient());
        }
        
        for (int i = 0; i < 4; ++i)
        {
            LevelEditor->SetActiveViewportClient(i);
            Renderer.RenderViewport(LevelEditor->GetActiveViewportClient());
        }
        GetLevelEditor()->SetActiveViewportClient(ActiveViewportCache);
    }
    else
    {
        Renderer.Render(LevelEditor->GetActiveViewportClient());
        
        Renderer.RenderViewport(LevelEditor->GetActiveViewportClient());
    }
    
}

void FEngineLoop::Tick()
{
    LARGE_INTEGER Frequency;
    const double targetFrameTime = 1000.0 / TargetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

    QueryPerformanceFrequency(&Frequency);

    LARGE_INTEGER startTime, endTime;
    double elapsedTime = 0.0;

    while (bIsExit == false)
    {
        QueryPerformanceCounter(&startTime);
        float DeltaTime = elapsedTime / 1000.f;

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); // 키보드 입력 메시지를 문자메시지로 변경
            DispatchMessage(&msg);  // 메시지를 WndProc에 전달

            if (msg.message == WM_QUIT)
            {
                bIsExit = true;
                break;
            }
        }

        GEngine->Tick(DeltaTime);
        CollisionSubsystem->Tick();
        LevelEditor->Tick(DeltaTime);
        Render();

        UIMgr->BeginFrame();
        UnrealEditor->Render(bIsGameMode);
        FConsole::GetInstance().Draw();
        if (GEngine->ActiveWorld->WorldType != EWorldType::Editor)
            FLuaManager::Get().RenderImGuiFromLua();
        UIMgr->EndFrame();

        // Pending 처리된 오브젝트 제거
        GUObjectArray.ProcessPendingDestroyObjects();

        GraphicDevice.SwapBuffer();

        FSoundManager::Get()->Update();

#if _DEBUG
        if (bIsEnableShaderHotReload)
        {
            (void)Renderer.HandleHotReloadShader();
        }
#endif

        do
        {
            Sleep(0);
            QueryPerformanceCounter(&endTime);
            elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.f / Frequency.QuadPart;
        } while (elapsedTime < targetFrameTime);
    }
}

void FEngineLoop::GetClientSize(uint32& OutWidth, uint32& OutHeight) const
{
    RECT ClientRect = {};
    GetClientRect(AppWnd, &ClientRect);
            
    OutWidth = ClientRect.right - ClientRect.left;
    OutHeight = ClientRect.bottom - ClientRect.top;
}

void FEngineLoop::Exit()
{
    LevelEditor->Release();
    UIMgr->Shutdown();
    ResourceManager.Release(&Renderer);
    Renderer.Release();
    GraphicDevice.Release();
    FSoundManager::Get()->Release();

    delete UnrealEditor;
    delete BufferManager;
    delete UIMgr;
    delete LevelEditor;
}

void FEngineLoop::WindowInit(HINSTANCE hInstance)
{
    WCHAR WindowClass[] = L"JungleWindowClass";

    WCHAR Title[] = L"Game Tech Lab";

    WNDCLASSW wc{};
    wc.lpfnWndProc = AppWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WindowClass;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

    RegisterClassW(&wc);

    AppWnd = CreateWindowExW(
        0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1300, 1000,
        nullptr, nullptr, hInstance, nullptr
    );
}

LRESULT CALLBACK FEngineLoop::AppWndProc(HWND hWnd, uint32 Msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
    {
        return true;
    }

    switch (Msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            auto LevelEditor = GEngineLoop.GetLevelEditor();
            if (LevelEditor)
            {
                FEngineLoop::GraphicDevice.Resize(hWnd);
                
                uint32 ClientWidth = 0;
                uint32 ClientHeight = 0;
                GEngineLoop.GetClientSize(ClientWidth, ClientHeight);
            
                LevelEditor->ResizeEditor(ClientWidth, ClientHeight);
            }
        }
        GEngineLoop.UpdateUI();
        break;

    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (GetAsyncKeyState(VK_MENU) & 0x8000))
        {
            GraphicDevice.ToggleFullScreen(hWnd);
            return 0;
        }
        break;
    case WM_KEYDOWN:
        if (wParam == VK_F5 && (GetAsyncKeyState(VK_F5) & 0x8000))
        {
            if (bIsGameMode) return 0;
            
            bIsGameMode = true;
            GraphicDevice.ToggleFullScreen(hWnd);
            FConsole::GetInstance().bWasOpen = false;
            GameState = Lobby;
            FSoundManager::Get()->MainChannel = FSoundManager::Get()->PlaySound("Contents\\Sound\\lobby.mp3", true);
            
            return 0;
        }
    default:
        GEngineLoop.AppMessageHandler->ProcessMessage(hWnd, Msg, wParam, lParam);
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

    return 0;
}

void FEngineLoop::UpdateUI() const
{
    FConsole::GetInstance().OnResize(AppWnd);
    if (GEngineLoop.GetUnrealEditor())
    {
        GEngineLoop.GetUnrealEditor()->OnResize(AppWnd);
    }
    ViewportTypePanel::GetInstance().OnResize(AppWnd);
}
