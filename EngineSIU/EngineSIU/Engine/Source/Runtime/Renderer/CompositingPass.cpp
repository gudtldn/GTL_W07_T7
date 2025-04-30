#include "CompositingPass.h"

#include <array>

#include "Define.h"
#include "RendererHelpers.h"
#include "UnrealClient.h"
#include "Engine/EditorEngine.h"
#include "UnrealEd/EditorViewportClient.h"

FCompositingPass::FCompositingPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FCompositingPass::~FCompositingPass()
{
    
}

void FCompositingPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManage;

    ShaderManager->AddVertexShader(L"Compositing", L"Shaders/CompositingShader.hlsl", "mainVS");
    ShaderManager->AddPixelShader(L"Compositing", L"Shaders/CompositingShader.hlsl", "mainPS");

    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    Graphics->Device->CreateSamplerState(&SamplerDesc, &Sampler);
}

void FCompositingPass::PrepareRender()
{
}

void FCompositingPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // Setup
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    if (!ViewportResource)
    {
        return;
    }

    const EResourceType ResourceType = EResourceType::ERT_Compositing; 
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetViewportResource()->GetRenderTargets().Find(ResourceType);

    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_Scene)->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_PostProcess), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_PP_Fog)->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_EditorOverlay), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_Editor)->SRV);

    // 1) 렌더 타겟 바인딩 & 클리어
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);
    Graphics->DeviceContext->ClearRenderTargetView(RenderTargetRHI->RTV, ViewportResource->GetClearColor(ResourceType).data());

    // 2) **레터박스 뷰포트 설정** (이 위치가 핵심)
    uint32 ScreenW = static_cast<uint32>(ViewportResource->GetD3DViewport().Width);
    uint32 ScreenH = static_cast<uint32>(ViewportResource->GetD3DViewport().Height);

    // 플레이어 카메라 매니저로부터 레터박스 영역 계산
    int vpX, vpY, vpW, vpH;
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    Engine->GetEditorPlayer()->GetPlayerCameraManager()
        ->GetLetterBoxViewport(ScreenW, ScreenH, vpX, vpY, vpW, vpH);
    D3D11_VIEWPORT LetterboxVp = {
        static_cast<float>(vpX),    // 좌측 X
        static_cast<float>(vpY),    // 좌측 Y
        static_cast<float>(vpW),    // 폭
        static_cast<float>(vpH),    // 높이
        0.0f, 1.0f
    };
    Graphics->DeviceContext->RSSetViewports(1, &LetterboxVp);
    
    Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Sampler);

    // TODO: 아래 상수 버퍼가 필요한지 생각해보기
    // Update Constant Buffer
    FViewModeConstants ViewModeConstantData = {};
    ViewModeConstantData.ViewMode = static_cast<uint32>(Viewport->GetViewMode());
    // BufferManager->UpdateConstantBuffer<FViewModeConstants>("FViewModeConstants", ViewModeConstantData);

    // Render
    ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(L"Compositing");
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"Compositing");
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    Graphics->DeviceContext->Draw(6, 0);

    // Finish
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    // Clear
    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_PostProcess), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_EditorOverlay), 1, NullSRV);
}

void FCompositingPass::ClearRenderArr()
{
}
