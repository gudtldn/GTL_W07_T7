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
    rd.CullMode = D3D11_CULL_FRONT;
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

    int directionalNum = DirectionalLights.Num();
    if (prevDirectionalNum < directionalNum) 
    {
        AddDirectionalShadowResource(directionalNum - prevDirectionalNum);
    }
    else if (prevDirectionalNum > directionalNum) 
    {
        DeleteDirectionalShadowResource(prevDirectionalNum - directionalNum);
    }
    prevDirectionalNum = directionalNum;

}

void FDirectionalShadowMap::UpdateViewProjMatrices(int index, FEditorViewportClient& ViewCamera, const FVector& LightDir)
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

    FMatrix LightView = JungleMath::CreateLookAtMatrix(Eye, center, FVector(0, 0, 1));

    FVector minB{ FLT_MAX,  FLT_MAX,  FLT_MAX };
    FVector maxB{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (auto& p : points)
    {
        FVector ls = LightView.TransformPosition(p);
        minB.X = FMath::Min(minB.X, ls.X); maxB.X = FMath::Max(maxB.X, ls.X);
        minB.Y = FMath::Min(minB.Y, ls.Y); maxB.Y = FMath::Max(maxB.Y, ls.Y);
        minB.Z = FMath::Min(minB.Z, ls.Z); maxB.Z = FMath::Max(maxB.Z, ls.Z);
    }

    const float nearZ = 0.0f;
    const float farZ = maxB.Z;
    FMatrix LightProj = JungleMath::CreateOrthoOffCenterProjectionMatrix(minB.X, maxB.X, minB.Y, maxB.Y, nearZ, farZ);

    DirectionalShadowResources[index].DirectionalView = LightView;
    DirectionalShadowResources[index].DirectionalProj = LightProj;
    DirectionalShadowResources[index].DirectionalViewProj = LightView * LightProj;
}

void FDirectionalShadowMap::UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(WorldMatrix));
    ObjectData.UUIDColor = UUIDColor;
    ObjectData.bIsSelected = bIsSelected;

    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
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

    for (int i = 0; i < DirectionalShadowResources.Num(); i++) 
    {
        // 뎁스 타겟 설정
        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, DirectionalShadowResources[i].ShadowDSV);
        Graphics->DeviceContext->ClearDepthStencilView(DirectionalShadowResources[i].ShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

        D3D11_VIEWPORT vp = { 0, 0, (FLOAT)ShadowResolution, (FLOAT)ShadowResolution, 0, 1 };
        Graphics->DeviceContext->RSSetViewports(1, &vp);

        PrepareRenderState();
        BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), DirectionalShadowResources[i].DirectionalViewProj);


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
}
void FDirectionalShadowMap::SetShadowResource(int tStart)
{
    for (int i = 0; i < DirectionalShadowResources.Num(); i++) 
    {
        Graphics->DeviceContext->PSSetShaderResources(tStart + i, 1, &DirectionalShadowResources[i].ShadowSRV);
    }

    
}

void FDirectionalShadowMap::SetShadowSampler(int sStart)
{
    Graphics->DeviceContext->PSSetSamplers(sStart, 1, &ShadowSampler);
}

FMatrix FDirectionalShadowMap::GetDirectionalView(int index)
{
    // Resource 생성 안되었을 때 요청 받을 경우에 방어 코드
    if (DirectionalShadowResources.Num() - 1 < index) 
    {
        return FMatrix();
    }

    return DirectionalShadowResources[index].DirectionalView;
}

FMatrix FDirectionalShadowMap::GetDirectionalProj(int index)
{
    // Resource 생성 안되었을 때 요청 받을 경우에 방어 코드
    if (DirectionalShadowResources.Num() - 1 < index)
    {
        return FMatrix();
    }
    return DirectionalShadowResources[index].DirectionalProj;
}

void FDirectionalShadowMap::AddDirectionalShadowResource(int num)
{
    for (int i = 0; i < num; i++) 
    {
        CreateDepthTexture();
    }
}

void FDirectionalShadowMap::DeleteDirectionalShadowResource(int num)
{
    for (int i = prevDirectionalNum - 1; i >= prevDirectionalNum - num; i--) {
        // 1) 기존 리소스 해제
        if (DirectionalShadowResources[i].DepthStencilTexture)
        {
            DirectionalShadowResources[i].DepthStencilTexture->Release();
            DirectionalShadowResources[i].DepthStencilTexture = nullptr;
        }
        if (DirectionalShadowResources[i].ShadowDSV)
        {
            DirectionalShadowResources[i].ShadowDSV->Release();
            DirectionalShadowResources[i].ShadowDSV = nullptr;
        }
        if (DirectionalShadowResources[i].ShadowSRV)
        {
            DirectionalShadowResources[i].ShadowSRV->Release();
            DirectionalShadowResources[i].ShadowSRV = nullptr;
        }

        DirectionalShadowResources.RemoveAt(i);
    }   
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
    FDirectionalShadowResource directionalShadowResource;

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
    FEngineLoop::GraphicDevice.Device->CreateTexture2D(&shadowMapDesc, nullptr, &directionalShadowResource.DepthStencilTexture);


    // 2-2) Depth-Stencil View (DSV)
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(directionalShadowResource.DepthStencilTexture, &depthStencilViewDesc, &directionalShadowResource.ShadowDSV);

    // 2-3) Shader Resource View (SRV)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    Graphics->Device->CreateShaderResourceView(directionalShadowResource.DepthStencilTexture, &srvDesc, &directionalShadowResource.ShadowSRV);

    DirectionalShadowResources.Add(directionalShadowResource);

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
            UpdateViewProjMatrices(DirectionalLightsCount, *Viewport, Light->GetDirection());
            DirectionalLightsCount++;
        }
    }

}

ID3D11ShaderResourceView* FDirectionalShadowMap::GetShadowViewSRV(int index)
{
    if (DirectionalShadowResources.Num() - 1 < index) 
    {
        return nullptr;
    }

    return DirectionalShadowResources[index].ShadowSRV;
}
