#pragma once

#include "DirectionalShadowMap.h"
#include "Math/JungleMath.h"
#include "UnrealEd/PrimitiveDrawBatch.h"

#include "RendererHelpers.h"
#include "Launch/EngineLoop.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/UnrealClient.h"

#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"

#include "Components/StaticMeshComponent.h"


FDirectionalShadowMap::~FDirectionalShadowMap()
{
    if (ShadowRasterizer)
        ShadowRasterizer->Release();
    if (ShadowSampler)
        ShadowSampler->Release();
    if (dsState)
        dsState->Release();
}

void FDirectionalShadowMap::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphic;
    ShaderManager = InShaderManager;

    CreateComparisonSampler();
    CreateShadowRasterizer();
    CreateDepthStencilState();
    LoadShadowShaders();
    CreateDepthTexture();
}

void FDirectionalShadowMap::CreateComparisonSampler()
{

    D3D11_SAMPLER_DESC comparisonSamplerDesc;
    ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.BorderColor[0] = 1.0f;
    comparisonSamplerDesc.BorderColor[1] = 1.0f;
    comparisonSamplerDesc.BorderColor[2] = 1.0f;
    comparisonSamplerDesc.BorderColor[3] = 1.0f;
    comparisonSamplerDesc.MinLOD = 0.f;
    comparisonSamplerDesc.MaxLOD = 0.f;
    comparisonSamplerDesc.MipLODBias = 0.f;
    comparisonSamplerDesc.MaxAnisotropy = 0;
    comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    Graphics->Device->CreateSamplerState(&comparisonSamplerDesc, &ShadowSampler);
}

void FDirectionalShadowMap::CreateShadowRasterizer()
{
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    rd.CullMode = D3D11_CULL_BACK;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthBias = 100.f;
    rd.DepthBiasClamp = 0.5f;
    rd.SlopeScaledDepthBias = 0.006f;
    rd.DepthClipEnable = TRUE;
    rd.ScissorEnable = FALSE;
    rd.MultisampleEnable = FALSE;
    rd.AntialiasedLineEnable = FALSE;
    Graphics->Device->CreateRasterizerState(&rd, &ShadowRasterizer);
}

void FDirectionalShadowMap::CreateDepthStencilState()
{
    // 2) Depth-only 스텐실 상태 생성/바인딩
 /*   D3D11_DEPTH_STENCIL_DESC dsDesc{};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    Graphics->Device->CreateDepthStencilState(&dsDesc, &dsState);*/
}

void FDirectionalShadowMap::LoadShadowShaders()
{
    DepthVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    DepthIL = ShaderManager->GetInputLayoutByKey(L"DepthOnlyVS");
}


void FDirectionalShadowMap::PrepareRenderState()
{

    Graphics->DeviceContext->VSSetShader(DepthVS, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(DepthIL);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    
    BufferManager->BindConstantBuffer(TEXT("FShadowViewProj"), 0, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FDirectionalLightViewProj"), 5, EShaderStage::Pixel);

}

void FDirectionalShadowMap::CollectStaticMeshes()
{
    StaticMeshComponents.Empty();
    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            StaticMeshComponents.Add(iter);
        }
    }

}

void FDirectionalShadowMap::CollectDirectionalLights()
{
    DirectionalLights.Empty();

    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(iter))
            {
                DirectionalLights.Add(DirectionalLight);
            }
        }
    }
}

void FDirectionalShadowMap::UpdateViewProjMatrices(FEditorViewportClient& ViewCamera, const FVector& LightDir)
{

    TArray<FVector> points = ViewCamera.GetFrustumCorners();

    for (auto* Comp : StaticMeshComponents)
    {
        if (!Comp || !Comp->GetStaticMesh()) continue;
        FBoundingBox bbox = Comp->GetBoundingBox();
        FMatrix     wm = Comp->GetWorldMatrix();
        for (int i = 0; i < 8; ++i)
            points.Add(bbox.TransformWorld(wm).GetCorner(i));
    }

    FVector center = FVector::ZeroVector;
    for (auto& p : points) center += p;
    center /= points.Num();

    float radius = 0.0f;
    for (auto& p : points)
    {
        float proj = FMath::Abs(FVector::DotProduct(p - center, LightDir));
        radius = FMath::Max(radius, proj);
    }


    FVector Eye = FVector(
        center.X - LightDir.X * radius,
        center.Y - LightDir.Y * radius,
        center.Z - LightDir.Z * radius
    );

    FVector up = FMath::Abs(LightDir.Z) > 0.99f
        ? FVector(1, 0, 0)
        : FVector(0, 0, 1);

    FMatrix LightView = JungleMath::CreateLookAtMatrix(Eye, center, up);

    FVector minB{ FLT_MAX,  FLT_MAX,  FLT_MAX };
    FVector maxB{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (auto& p : points)
    {
        FVector ls = LightView.TransformPosition(p);
        minB.X = FMath::Min(minB.X, ls.X); maxB.X = FMath::Max(maxB.X, ls.X);
        minB.Y = FMath::Min(minB.Y, ls.Y); maxB.Y = FMath::Max(maxB.Y, ls.Y);
        minB.Z = FMath::Min(minB.Z, ls.Z); maxB.Z = FMath::Max(maxB.Z, ls.Z);
    }

    const float nearZ = FMath::Max(0.1f, minB.Z);
    const float farZ = maxB.Z;
    FMatrix LightProj = JungleMath::CreateOrthoOffCenterProjectionMatrix(minB.X, maxB.X, minB.Y, maxB.Y, nearZ, farZ);

    DirectionalLightViewProjMatrix = LightView * LightProj;

    float worldSizeX = maxB.X - minB.X;
    float worldSizeY = maxB.Y - minB.Y;
    float maxWorldSize = FMath::Max(worldSizeX, worldSizeY);
    float worldTexelScale = maxWorldSize / static_cast<float>(ShadowResolution);

    FDirectionalLightViewProj vp{ LightView, LightProj, worldTexelScale };
    FShadowViewProj LightVP{ DirectionalLightViewProjMatrix };
    BufferManager->UpdateConstantBuffer(TEXT("FDirectionalLightViewProj"), vp);
    BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), LightVP);
}

void FDirectionalShadowMap::RenderShadowMap()
{

    if (DirectionalLights.Num() <= 0) return;

    // 모든 스태틱 메시 컴포넌트 수집
    TArray<UStaticMeshComponent*> MeshComps;
    for (auto* Comp : TObjectRange<UStaticMeshComponent>())
    {
        if (Comp->GetWorld() == GEngine->ActiveWorld)
        {
            MeshComps.Add(Comp);
        }
    }

    // 뎁스 타겟 설정
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, ShadowDSV);
    Graphics->DeviceContext->ClearDepthStencilView(ShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT vp = { 0, 0, (FLOAT)ShadowResolution, (FLOAT)ShadowResolution, 0, 1 };
    Graphics->DeviceContext->RSSetViewports(1, &vp);

    PrepareRenderState();


    // 메시 렌더
    for (auto* Comp : MeshComps)
    {
        if (!Comp->GetStaticMesh()) continue;
        auto* RenderData = Comp->GetStaticMesh()->GetRenderData();
        if (!RenderData) continue;

        // 월드 행렬 상수
        FMatrix W = Comp->GetWorldMatrix();
        BufferManager->BindConstantBuffer(TEXT("FShadowObjWorld"), 1, EShaderStage::Vertex);
        BufferManager->UpdateConstantBuffer(TEXT("FShadowObjWorld"), W);

        UINT stride = sizeof(FStaticMeshVertex);
        UINT offset = 0;
        auto* vb = RenderData->VertexBuffer;
        auto* ib = RenderData->IndexBuffer;

        Graphics->DeviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        if (ib)
        {
            Graphics->DeviceContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
            Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        }
        else
        {
            Graphics->DeviceContext->Draw(RenderData->Vertices.Num(), 0);
        }
    }
}
void FDirectionalShadowMap::SetShadowResource(int tStart)
{
    Graphics->DeviceContext->PSSetShaderResources(tStart, 1, &ShadowSRV);
}

void FDirectionalShadowMap::SetShadowSampler(int sStart)
{
    Graphics->DeviceContext->PSSetSamplers(sStart, 1, &ShadowSampler);
}

void FDirectionalShadowMap::ChangeViewportSize()
{
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = ShadowResolution;
    vp.Height = ShadowResolution;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    Graphics->DeviceContext->RSSetViewports(1, &vp);
}

void FDirectionalShadowMap::CreateDepthTexture()
{
    // 1) 기존 리소스 해제
    if (DepthStencilTexture) { DepthStencilTexture->Release(); DepthStencilTexture = nullptr; }
    if (ShadowDSV) { ShadowDSV->Release();           ShadowDSV = nullptr; }
    if (ShadowSRV) { ShadowSRV->Release();           ShadowSRV = nullptr; }


    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
    shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    shadowMapDesc.MipLevels = 0;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
    shadowMapDesc.CPUAccessFlags = 0;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.SampleDesc.Quality = 0;
    shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    shadowMapDesc.Height = (UINT)ShadowResolution;
    shadowMapDesc.Width = (UINT)ShadowResolution;
    FEngineLoop::GraphicDevice.Device->CreateTexture2D(&shadowMapDesc, nullptr, &DepthStencilTexture);


    // 2-2) Depth-Stencil View (DSV)
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(DepthStencilTexture, &depthStencilViewDesc, &ShadowDSV);

    // 2-3) Shader Resource View (SRV)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    Graphics->Device->CreateShaderResourceView(DepthStencilTexture, &srvDesc, &ShadowSRV);

}

void FDirectionalShadowMap::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    CollectStaticMeshes();
    CollectDirectionalLights();

    int DirectionalLightsCount = 0;
    for (auto Light : DirectionalLights)
    {
        if (DirectionalLightsCount < MAX_DIRECTIONAL_LIGHT)
        {
            Light->GetDirectionalLightInfo().Direction;
            UpdateViewProjMatrices(*Viewport, Light->GetDirection());
            DirectionalLightsCount++;
        }
    }

}

ID3D11ShaderResourceView* FDirectionalShadowMap::GetShadowViewSRV()
{
    return ShadowSRV;
}
