#include "PointLightShadowMap.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Classes/Components/Light/PointLightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/EditorEngine.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

void FPointLightShadowMap::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphic;
    ShaderManager = InShaderManager;

    // ShadowMap들은 TArray로 관리하므로 추가될때 생성

    for (uint32 face = 0; face < faceNum; face++) 
    {
        // Linear Depth 시각화용
        D3D11_TEXTURE2D_DESC linDesc = {};
        linDesc.Width = ShadowMapSize;
        linDesc.Height = ShadowMapSize;
        linDesc.MipLevels = 1;
        linDesc.ArraySize = 1;
        linDesc.Format = DXGI_FORMAT_R32_TYPELESS;  // 또는 직접 R32_FLOAT 로 해도 무방
        linDesc.SampleDesc = { 1,0 };
        linDesc.Usage = D3D11_USAGE_DEFAULT;
        // ▶ RTV 바인드를 추가해 줍니다
        linDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        linDesc.CPUAccessFlags = 0;
        linDesc.MiscFlags = 0;

        HRESULT hr = Graphics->Device->CreateTexture2D(&linDesc, nullptr, &DepthLinearBuffer[face]);
        assert(SUCCEEDED(hr));

        // Linear Depth 시각화용 SRV 생성 (쉐이더에서 깊이 읽기)
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = Graphics->Device->CreateShaderResourceView(DepthLinearBuffer[face], &srvDesc, &ShadowViewSRV[face]);
        assert(SUCCEEDED(hr));

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        hr = Graphics->Device->CreateRenderTargetView(
            DepthLinearBuffer[face], &rtvDesc, &ShadowViewRTV[face]);
        assert(SUCCEEDED(hr));

    }

    // 4) 비교 샘플러 생성 (쉐도우 비교 샘플링용)
    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.BorderColor[0] = 1.0f;
    sampDesc.BorderColor[1] = 1.0f;
    sampDesc.BorderColor[2] = 1.0f;
    sampDesc.BorderColor[3] = 1.0f;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

    HRESULT hr = Graphics->Device->CreateSamplerState(&sampDesc, &ShadowSampler);
    assert(SUCCEEDED(hr));
    
    // 리니어 샘플러 생성
    D3D11_SAMPLER_DESC sampLinearDesc = {};
    sampLinearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampLinearDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampLinearDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampLinearDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampLinearDesc.MipLODBias = 0.0f;
    sampLinearDesc.MaxAnisotropy = 1;
    sampLinearDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;      // 일반 샘플링용
    sampLinearDesc.MinLOD = 0;
    sampLinearDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = Graphics->Device->CreateSamplerState(&sampLinearDesc, &LinearSampler);
    assert(SUCCEEDED(hr));

    // 깊이 전용 버텍스 셰이더 가져오기
    DepthVS = ShaderManager->GetVertexShaderByKey(L"DepthOnlyVS");
    DepthIL = ShaderManager->GetInputLayoutByKey(L"DepthOnlyVS");

    FullscreenVS = ShaderManager->GetVertexShaderByKey(L"FullScreenVS");
    FullscreenIL = ShaderManager->GetInputLayoutByKey(L"FullScreenVS");
    DepthVisualizePS = ShaderManager->GetPixelShaderByKey(L"DepthCubeVisualizePS");
}

void FPointLightShadowMap::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
            {
                PointLights.Add(PointLight);
            }
        }
    }
    
    if (PointLights.Num() > 0) 
    {
        // Light 갯수에 맞춰 TArray 동적할당
        int pointLightNum = PointLights.Num();
        if (prevShadowCubeNum < pointLightNum) {
            AddPointLightShadowCube(pointLightNum - prevShadowCubeNum);
        }
        else if (prevShadowCubeNum > pointLightNum) {
            DeletePointLightShadowCube(prevShadowCubeNum - pointLightNum);
        }
        prevShadowCubeNum = PointLightShadowCubes.Num();
    }

    for (int i = 0; i < PointLights.Num(); i++) 
    {
        UpdatePointLightViewProjMatrices(i, PointLights[i]->GetWorldLocation(), PointLights[i]->GetRadius());
    }
}

void FPointLightShadowMap::UpdatePointLightViewProjMatrices(int index, const FVector& pointLightPos, const float lightRadius)
{   
    FMatrix Projection = JungleMath::CreateProjectionMatrix(FMath::DegreesToRadians(90.0f), 1.0f, 0.1f, lightRadius);

    for (int face = 0; face < 6; ++face)
    {
        FVector dir = Directions[face];
        FVector up = Ups[face];
        FVector target = pointLightPos + dir;

        FMatrix View = JungleMath::CreateViewMatrix(pointLightPos, target, up);

        PointLightShadowCubes[index].PointLightViewProjMatrix[face] = View * Projection;
    }
}



void FPointLightShadowMap::RenderShadowMap()
{
    if (PointLights.Num() <= 0) return;

    // 모든 스태틱 메시 컴포넌트 수집
    TArray<UStaticMeshComponent*> MeshComps;
    for (auto* Comp : TObjectRange<UStaticMeshComponent>())
    {
        if (Comp->GetWorld() == GEngine->ActiveWorld)
        {
            MeshComps.Add(Comp);
        }
    }

    for (int i = 0; i < PointLightShadowCubes.Num(); i++) 
    {
        // 각 큐브맵 Face에 대해 렌더링
        for (int face = 0; face < 6; ++face)
        {
            // 뎁스 타겟 설정
            Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, PointLightShadowCubes[i].ShadowDSV[face]);
            Graphics->DeviceContext->ClearDepthStencilView(PointLightShadowCubes[i].ShadowDSV[face], D3D11_CLEAR_DEPTH, 1.0f, 0);

            D3D11_VIEWPORT vp = { 0, 0, (FLOAT)ShadowMapSize, (FLOAT)ShadowMapSize, 0, 1 };
            Graphics->DeviceContext->RSSetViewports(1, &vp);

            // 파이프라인 세팅
            Graphics->DeviceContext->VSSetShader(DepthVS, nullptr, 0);
            Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
            Graphics->DeviceContext->IASetInputLayout(DepthIL);
            Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // ViewProj 상수 업데이트
            BufferManager->BindConstantBuffer(TEXT("FShadowViewProj"), 0, EShaderStage::Vertex);
            // FMatrix를 담는 단일 structure라면 struct으로 안 감싸도 괜찮을까?
            // 테스트 해봐야지
            // 단일은 안감싸도 잘 작동
            BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), PointLightShadowCubes[i].PointLightViewProjMatrix[face]);

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
}


void FPointLightShadowMap::ClearRenderArr()
{
    PointLights.Empty();
}

void FPointLightShadowMap::SetShadowResource(int tStart)
{
    for (int i = 0; i < PointLightShadowCubes.Num(); i++) 
    {
        Graphics->DeviceContext->PSSetShaderResources(tStart + i, 1, &PointLightShadowCubes[i].ShadowCubeSRV);
    }
}

void FPointLightShadowMap::SetShadowSampler(int sStart)
{
    Graphics->DeviceContext->PSSetSamplers(sStart, 1, &ShadowSampler);
}

void FPointLightShadowMap::AddPointLightShadowCube(int num)
{
    for (int i = 0; i < num; i++) {
        FPointLightShadowCube pointLightShadowCube;

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = ShadowMapSize;
        texDesc.Height = ShadowMapSize;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 6;                                   // ← 6면
        texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        texDesc.SampleDesc = { 1,0 };
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        HRESULT hr = Graphics->Device->CreateTexture2D(&texDesc, nullptr, &pointLightShadowCube.DepthCube);
        assert(SUCCEEDED(hr));

        for (int face = 0; face < faceNum; face++) {
            // 2) DSV 생성 (뎁스 기록용)
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice = 0;
            dsvDesc.Texture2DArray.ArraySize = 1;
            dsvDesc.Texture2DArray.FirstArraySlice = face;               // ← 각 면 슬라이스
            hr = Graphics->Device->CreateDepthStencilView(pointLightShadowCube.DepthCube, &dsvDesc, &pointLightShadowCube.ShadowDSV[face]);
            assert(SUCCEEDED(hr));
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvCubeDesc = {};
        srvCubeDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvCubeDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvCubeDesc.TextureCube.MostDetailedMip = 0;
        srvCubeDesc.TextureCube.MipLevels = 1;
        hr = Graphics->Device->CreateShaderResourceView(pointLightShadowCube.DepthCube, &srvCubeDesc, &pointLightShadowCube.ShadowCubeSRV);
        assert(SUCCEEDED(hr));

        PointLightShadowCubes.Add(pointLightShadowCube);
    }
}

void FPointLightShadowMap::DeletePointLightShadowCube(int num)
{
    for (int i = prevShadowCubeNum - 1; i >= prevShadowCubeNum - num; i--) {
        PointLightShadowCubes[i].DepthCube->Release();
        PointLightShadowCubes[i].ShadowCubeSRV->Release();
        for (int face = 0; face < faceNum; face++) {
            PointLightShadowCubes[i].ShadowDSV[face]->Release();
        }
        PointLightShadowCubes.RemoveAt(i);
    }
}

void FPointLightShadowMap::RenderLinearDepth(int lightIndex, UPointLightComponent* pointLightComp)
{
    if (PointLightShadowCubes.Num() - 1 < lightIndex) 
    {
        lightIndex = 0;
    }

    if (PointLightShadowCubes[lightIndex].DepthCube == nullptr) return;

    // ─── 0) 기존 RenderTargets, DepthStencilView, Viewports 백업 ───
    ID3D11RenderTargetView* oldRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    ID3D11DepthStencilView* oldDSV = nullptr;
    Graphics->DeviceContext->OMGetRenderTargets(
        D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, oldRTVs, &oldDSV);

    UINT numVP = D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX;
    D3D11_VIEWPORT oldVPs[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    Graphics->DeviceContext->RSGetViewports(&numVP, oldVPs);

    BufferManager->BindConstantBuffer(TEXT("FDepthMapData"), 0, EShaderStage::Pixel);
    
    Graphics->DeviceContext->OMSetRenderTargets(6, ShadowViewRTV, nullptr);
    const float clearColor[4] = { 0, 0, 0, 0 };
    for (int face = 0; face < faceNum; face++) {
        Graphics->DeviceContext->ClearRenderTargetView(ShadowViewRTV[face], clearColor);
    }

    // (B) 뷰포트 설정
    D3D11_VIEWPORT vp = { 0.f, 0.f, (float)ShadowMapSize, (float)ShadowMapSize, 0.f, 1.f };
    Graphics->DeviceContext->RSSetViewports(1, &vp);

    // (C) 풀스크린 파이프라인 바인딩
    Graphics->DeviceContext->IASetInputLayout(FullscreenIL);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->VSSetShader(FullscreenVS, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(DepthVisualizePS, nullptr, 0);

    // (D) 원본 Depth SRV 와 리니어 샘플러 바인딩
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &PointLightShadowCubes[lightIndex].ShadowCubeSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &LinearSampler);

    // (E) 카메라(라이트) 매트릭스 및 Near/Far/Gamma 상수 업데이트
    FDepthMapData depthMapData;
    depthMapData.Params.X = 0.1f;                                     // Near plane
    depthMapData.Params.Y = pointLightComp->GetRadius();                   // Far plane = Light Radius
    depthMapData.Params.Z = 1.0f / 2.2f;                             // invGamma (예: gamma=2.2)
    depthMapData.Params.W = 0;
    BufferManager->UpdateConstantBuffer(TEXT("FDepthMapData"), depthMapData);

    // (F) 풀스크린 삼각형 드로우
    Graphics->DeviceContext->Draw(3, 0);
    

    // ─── 3) 이전 RTV/DSV & Viewports 복구 ───────────────
    Graphics->DeviceContext->RSSetViewports(numVP, oldVPs);
    Graphics->DeviceContext->OMSetRenderTargets(
        D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
        oldRTVs, oldDSV);

    // ─── 4) Release 참조 카운트 낮추기 ─────────────────
    for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        if (oldRTVs[i]) oldRTVs[i]->Release();
    if (oldDSV) oldDSV->Release();
}

TArray<ID3D11ShaderResourceView*> FPointLightShadowMap::GetShadowViewSRVArray()
{
    TArray<ID3D11ShaderResourceView*> arr;
    for (int i = 0; i < faceNum; ++i)
    {
        arr.Add(ShadowViewSRV[i]);
    }
    return arr;
}


TArray<FVector> FPointLightShadowMap::GetDirectionArray()
{
    TArray<FVector> arr;
    for (int i = 0; i < faceNum; ++i)
    {
        arr.Add(Directions[i]);
    }
    return arr;
}

TArray<FVector> FPointLightShadowMap::GetUpArray()
{
    TArray<FVector> arr;
    for (int i = 0; i < faceNum; ++i)
    {
        arr.Add(Ups[i]);
    }
    return arr;
}

FMatrix FPointLightShadowMap::GetViewProjMatrix(int index, int face)
{
    return PointLightShadowCubes[index].PointLightViewProjMatrix[face];
}

