#include "FadeRenderPass.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Define.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include <wchar.h>
#include <UObject/UObjectIterator.h>
#include <Engine/Engine.h>

#include "RendererHelpers.h"
#include "UnrealClient.h"
#include "PropertyEditor/ShowFlags.h"

#include "Engine/Source/Runtime/Engine/Classes/Camera/PlayerCameraManager.h"

FFadeRenderPass::FFadeRenderPass()
{


}

FFadeRenderPass::~FFadeRenderPass()
{
    ReleaseShader();
}

void FFadeRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    Graphics = InGraphics;
    BufferManager = InBufferManager;
    ShaderManager = InShaderManager;
    CreateShader();
    CreateBlendState();
}

void FFadeRenderPass::PrepareRender()
{
    // TODO 현재 Player가 소유한 1개의 PlayerCameraManager를 정확히 가져올것
    for (const auto iter : TObjectRange<APlayerCameraManager>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            FLinearColor FadeLinearColor = iter->GetFadeConstant();
            FadeColor = FVector(FadeLinearColor.R, FadeLinearColor.G, FadeLinearColor.B);
            FadeAlpha = FadeLinearColor.A;
        }
    }

}

void FFadeRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_PP_Fog;
    FRenderTargetRHI* RenderTargetRHI = Viewport->GetViewportResource()->GetRenderTarget(ResourceType);

    ViewportResource->ClearRenderTarget(Graphics->DeviceContext, ResourceType);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);

    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, &ViewportResource->GetDepthStencilSRV());

    PrepareRenderState();

    UpdateFadeConstant();

    float blendFactor[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->OMSetBlendState(BlendState, blendFactor, 0xffffffff);

    // 풀스크린 삼각형 그리기
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->Draw(3, 0);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, NullSRV);
}


void FFadeRenderPass::CreateShader()
{
    // 정점 셰이더 및 입력 레이아웃 생성
    HRESULT hr = ShaderManager->AddVertexShader(L"FadeVertexShader", L"Shaders/FadeEffect.hlsl", "mainVS");
    if (FAILED(hr))
    {
        return;
    }
    // 픽셀 셰이더 생성
    hr = ShaderManager->AddPixelShader(L"FadePixelShader", L"Shaders/FadeEffect.hlsl", "mainPS");
    if (FAILED(hr))
    {
        return;
    }

    // 생성된 셰이더와 입력 레이아웃 획득
    VertexShader = ShaderManager->GetVertexShaderByKey(L"FadeVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"FadePixelShader");
}

void FFadeRenderPass::ReleaseShader()
{
    
}

void FFadeRenderPass::CreateBlendState()
{
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = Graphics->Device->CreateBlendState(&blendDesc, &BlendState);
    if (FAILED(hr))
    {
        MessageBox(NULL, L"AlphaBlendState 생성에 실패했습니다!", L"Error", MB_ICONERROR | MB_OK);
    }
}

void FFadeRenderPass::ClearRenderArr()
{

}

void FFadeRenderPass::ReloadShader()
{
    VertexShader = ShaderManager->GetVertexShaderByKey(L"FadeVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"FadePixelShader");
}

void FFadeRenderPass::PrepareRenderState()
{
    // 셰이더 설정
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Rasterizer Setting이 필요한가?

    TArray<FString> PSBufferKeys = {
        TEXT("FFadeConstants")
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);
}

void FFadeRenderPass::UpdateFadeConstant()
{
    FFadeConstants Constants = { 
        Constants.FadeAlpha = FadeAlpha,
        Constants.FadeColor = FadeColor,
    };

    // 상수버퍼 업데이트
    BufferManager->UpdateConstantBuffer(TEXT("FFadeConstants"), Constants);
}


