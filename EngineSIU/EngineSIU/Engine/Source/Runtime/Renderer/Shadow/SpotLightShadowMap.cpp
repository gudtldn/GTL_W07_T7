#include "SpotLightShadowMap.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Classes/Components/Light/SpotLightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/EditorEngine.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "LevelEditor/SLevelEditor.h"
void FSpotLightShadowMap::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphic;
    ShaderManager = InShaderManager;

    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = ShadowMapSize;
    texDesc.Height = ShadowMapSize;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS;                     // Typeless
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT hr = Graphics->Device->CreateTexture2D(&texDesc, nullptr, &DepthStencilBuffer);
    assert(SUCCEEDED(hr));

    // Begin Test
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

    hr = Graphics->Device->CreateTexture2D(&linDesc, nullptr, &DepthLinearBuffer);
    assert(SUCCEEDED(hr));
    // End Test

    // 2) DSV 생성 (뎁스 기록용)
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = Graphics->Device->CreateDepthStencilView(DepthStencilBuffer, &dsvDesc, &ShadowDSV);
    assert(SUCCEEDED(hr));

    // 3) SRV 생성 (쉐이더에서 깊이 읽기)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = Graphics->Device->CreateShaderResourceView(DepthStencilBuffer, &srvDesc, &ShadowSRV);
    assert(SUCCEEDED(hr));


    hr = Graphics->Device->CreateShaderResourceView(DepthLinearBuffer, &srvDesc, &ShadowViewSRV);
    assert(SUCCEEDED(hr));

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    hr = Graphics->Device->CreateRenderTargetView(
        DepthLinearBuffer, &rtvDesc, &ShadowViewRTV);
    assert(SUCCEEDED(hr));

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
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = Graphics->Device->CreateSamplerState(&sampDesc, &ShadowSampler);
    assert(SUCCEEDED(hr));

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
    DepthVisualizePS = ShaderManager->GetPixelShaderByKey(L"DepthVisualizePS");


}

void FSpotLightShadowMap::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
            {
                SpotLights.Add(SpotLight);
            }
        }
    }

    //if (SpotLights.Num() > 0)
    //{
    //    FSpotLightInfo SpotLightInfo;
    //    SpotLightInfo.Position = SpotLights[0]->GetWorldLocation();
    //    SpotLightInfo.Direction = SpotLights[0]->GetDirection();

    //    UpdateSpotLightViewProjMatrices(SpotLightInfo);
    //}

    // Begin Test
    for (auto SpotLight : SpotLights)
    {
        FSpotLightInfo SpotLightInfo;
        SpotLightInfo.Position = SpotLight->GetWorldLocation();
        SpotLightInfo.Direction = SpotLight->GetDirection();
        UpdateSpotLightViewProjMatrices(SpotLightInfo);
    }
    // End Test
}


FVector ComputeFrustumCenter(const TArray<FVector>& FrustumCorners)
{
    FVector center = FVector::ZeroVector;
    for (const FVector& corner : FrustumCorners)
    {
        center += corner;
    }
    return center / FrustumCorners.Num();
}

//void FSpotLightShadowMap::UpdateSpotLightViewProjMatrices(const FSpotLightInfo& Info)
//{
//    FEditorViewportClient* ViewCamera = GEngineLoop.GetLevelEditor()->GetActiveViewportClient().get();
//
//    auto corners = ViewCamera->GetFrustumCorners();
//    FVector frustumCenter = ComputeFrustumCenter(corners);
//
//    // (2) 스포트라이트의 방향과 위치로 View 매트릭스 생성
//    FVector lightDir = Info.Direction.GetSafeNormal();
//    float dist = ViewCamera->FarClip;
//
//    // 프러스텀 중심을 기준으로 Spot 방향의 반대 방향으로 떨어져 위치 설정
//    FVector eye = frustumCenter - lightDir * 100;
//    FVector target = frustumCenter;
//    FVector up(0.0f, 0.0f, 1.0f);
//
//    FMatrix spotView = JungleMath::CreateLookToMatrix(eye, lightDir, up);
//
//    // (3) 라이트 공간에서 정확한 프러스텀 영역 찾기 (Directional과 동일)
//    FBoundingBox lightSpaceBounds(FVector(FLT_MAX, FLT_MAX, FLT_MAX), FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX));
//
//    for (const FVector& corner : corners)
//    {
//        FVector lspCorner = spotView.TransformPosition(corner);
//        lightSpaceBounds.Expand(lspCorner);
//    }
//
//    // (4) 계산된 Bounds를 기준으로 Orthographic Projection 생성
//    const float nearZ = FMath::Max(0.1f, lightSpaceBounds.min.Z);
//    float farZ = lightSpaceBounds.max.Z;
//
//    FMatrix spotProjection = JungleMath::CreateOrthoOffCenterProjectionMatrix(
//        lightSpaceBounds.min.X, lightSpaceBounds.max.X,
//        lightSpaceBounds.min.Y, lightSpaceBounds.max.Y,
//        nearZ, farZ
//    );
//
//    // (5) 최종 PSM Shadow 행렬
//    SpotLightViewProjMatrix = spotView * spotProjection;
//}

void FSpotLightShadowMap::UpdateSpotLightViewProjMatrices(const FSpotLightInfo& Info)
{
    // --- Step 1: Standard Spotlight View & Projection ---

    FVector lightPos = Info.Position;
    FVector lightDir = Info.Direction.GetSafeNormal();
    float outerConeAngleRad = Info.OuterRad;
    float lightRange = Info.Radius; // Far Plane
    float nearPlane = 0.1f; // Near Plane

    // Robust Up Vector
    FVector up = FVector(0.0f, 0.0f, 1.0f);
    if (FMath::Abs(lightDir.Z) > 0.999f)
    {
        up = FVector(1.0f, 0.0f, 0.0f);
    }
    FVector targetPoint = lightPos + lightDir * 10.0f;

    FEditorViewportClient* viewCamera = GEngineLoop.GetLevelEditor()->GetActiveViewportClient().get();
    FMatrix spotView = JungleMath::CreateLookAtMatrix(lightPos, targetPoint, up);
    FMatrix spotProjection = JungleMath::CreateProjectionMatrix(
        Info.OuterRad,
        1.0f,
        nearPlane,
        lightRange
    );

    FMatrix lightViewProj = spotView * spotProjection;


    auto cameraFrustumCorners = viewCamera->GetFrustumCorners();
    if (!cameraFrustumCorners.IsEmpty()) // 코너를 가져왔는지 확인
    {
        FBoundingBox lightNdcBounds(FVector(FLT_MAX, FLT_MAX, FLT_MAX), FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX));
        bool projectedPointExists = false; // 명확성을 위해 이름 변경

        for (const FVector& worldCorner : cameraFrustumCorners)
        {
            FVector4 clipCorner = lightViewProj.TransformFVector4(FVector4(worldCorner, 1.0f));

            if (clipCorner.W > KINDA_SMALL_NUMBER) // 라이트 앞에 있는 점만 고려
            {
                FVector ndcCorner;
                ndcCorner.X = clipCorner.X / clipCorner.W;
                ndcCorner.Y = clipCorner.Y / clipCorner.W;
                ndcCorner.Z = clipCorner.Z / clipCorner.W;

                lightNdcBounds.Expand(ndcCorner);
                projectedPointExists = true;
            }
        }
        bool bPsmApplied = false;               // PSM 적용 여부 플래그

        FMatrix cropMatrix = {};
        // 실제로 점이 투영되었고 바운드가 유효한지 확인
        if (projectedPointExists &&
            lightNdcBounds.min.X < lightNdcBounds.max.X - KINDA_SMALL_NUMBER && // 허용 오차 사용
            lightNdcBounds.min.Y < lightNdcBounds.max.Y - KINDA_SMALL_NUMBER)
        {
            // Crop 행렬 계산 전에 바운드를 유효한 NDC 범위 [-1, 1]로 클램핑
            float minX = FMath::Max(-1.0f, lightNdcBounds.min.X);
            float maxX = FMath::Min(1.0f, lightNdcBounds.max.X);
            float minY = FMath::Max(-1.0f, lightNdcBounds.min.Y);
            float maxY = FMath::Min(1.0f, lightNdcBounds.max.Y);

            // 클램핑 후 범위가 유효한지 다시 확인
            if (FMath::Abs(maxX - minX) > KINDA_SMALL_NUMBER && FMath::Abs(maxY - minY) > KINDA_SMALL_NUMBER)
            {
                float scaleX = 2.0f / (maxX - minX);
                float scaleY = 2.0f / (maxY - minY);
                float offsetX = -(maxX + minX) / (maxX - minX);
                float offsetY = -(maxY + minY) / (maxY - minY);

                // 스케일 및 오프셋 적용 (Row-Major)
                cropMatrix = FMatrix::Identity;
                cropMatrix.M[0][0] = scaleX;
                cropMatrix.M[1][1] = scaleY;
                cropMatrix.M[3][0] = offsetX;
                cropMatrix.M[3][1] = offsetY;
                bPsmApplied = true;
            }
            // else: 클램핑된 범위가 유효하지 않으면 cropMatrix는 Identity 유지
        }
        // else: 유효한 투영 점이 없거나 바운드가 유효하지 않으면 cropMatrix는 Identity 유지

        SpotLightViewProjMatrix = lightViewProj;
    }
    else
    {
        SpotLightViewProjMatrix = lightViewProj;
       
    }

    FShadowViewProj LightVPData{ SpotLightViewProjMatrix };


    BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), LightVPData);

    FSpotLightShadowData SpotLightShadowData;
    SpotLightShadowData.SpotLightViewProj = SpotLightViewProjMatrix;
    BufferManager->UpdateConstantBuffer(TEXT("FSpotLightShadowData"), SpotLightShadowData);
}
void FSpotLightShadowMap::RenderShadowMap()
{
    if (SpotLights.Num() <= 0) return;

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

    D3D11_VIEWPORT vp = { 0, 0, (FLOAT)ShadowMapSize, (FLOAT)ShadowMapSize, 0, 1 };
    Graphics->DeviceContext->RSSetViewports(1, &vp);

    // 파이프라인 세팅
    Graphics->DeviceContext->VSSetShader(DepthVS, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(DepthIL);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    BufferManager->BindConstantBuffer(TEXT("FShadowViewProj"), 0, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FShadowObjWorld"), 1, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FSpotLightShadowData"), 6, EShaderStage::Pixel);


    // 메시 렌더
    for (auto* Comp : MeshComps)
    {
        if (!Comp->GetStaticMesh()) continue;
        auto* RenderData = Comp->GetStaticMesh()->GetRenderData();
        if (!RenderData) continue;

        // 월드 행렬 상수
        FMatrix W = Comp->GetWorldMatrix();
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

void FSpotLightShadowMap::UpdateConstantBuffer()
{

    BufferManager->BindConstantBuffer(TEXT("FSpotLightShadowData"), 6, EShaderStage::Pixel);


    FSpotLightShadowData SpotLightShadowData;
    SpotLightShadowData.SpotLightViewProj = SpotLightViewProjMatrix;
    BufferManager->UpdateConstantBuffer(TEXT("FSpotLightShadowData"), SpotLightShadowData);

}

void FSpotLightShadowMap::ClearRenderArr()
{
    SpotLights.Empty();
}

void FSpotLightShadowMap::SetShadowResource(int tStart)
{
    Graphics->DeviceContext->PSSetShaderResources(tStart, 1, &ShadowSRV);
}

void FSpotLightShadowMap::SetShadowSampler(int sStart)
{
    Graphics->DeviceContext->PSSetSamplers(sStart, 1, &ShadowSampler);
}

ID3D11ShaderResourceView* FSpotLightShadowMap::GetShadowSRV()
{
    return ShadowSRV;
}

void FSpotLightShadowMap::RenderLinearDepth()
{
    if (DepthStencilBuffer == nullptr) return;

    // ─── 0) 기존 RenderTargets, DepthStencilView, Viewports 백업 ───
    ID3D11RenderTargetView* oldRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    ID3D11DepthStencilView* oldDSV = nullptr;
    Graphics->DeviceContext->OMGetRenderTargets(
        D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, oldRTVs, &oldDSV);

    UINT numVP = D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX;
    D3D11_VIEWPORT oldVPs[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    Graphics->DeviceContext->RSGetViewports(&numVP, oldVPs);

    BufferManager->BindConstantBuffer(TEXT("FDepthMapData"), 0, EShaderStage::Pixel);

    // (A) RTV 세팅 & 클리어
    ID3D11RenderTargetView* rtvs[] = { ShadowViewRTV };
    Graphics->DeviceContext->OMSetRenderTargets(1, rtvs, nullptr);
    const float clearColor[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->ClearRenderTargetView(ShadowViewRTV, clearColor);

    // (B) 뷰포트 설정
    D3D11_VIEWPORT vp = { 0.f, 0.f, (float)ShadowMapSize, (float)ShadowMapSize, 0.f, 1.f };
    Graphics->DeviceContext->RSSetViewports(1, &vp);

    // (C) 풀스크린 파이프라인 바인딩
    Graphics->DeviceContext->IASetInputLayout(FullscreenIL);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->VSSetShader(FullscreenVS, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(DepthVisualizePS, nullptr, 0);

    // (D) 원본 Depth SRV 와 리니어 샘플러 바인딩
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &ShadowSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &LinearSampler);

    // (E) 카메라(라이트) 매트릭스 및 Near/Far/Gamma 상수 업데이트
    FDepthMapData depthMapData;
    depthMapData.ViewProj = SpotLightViewProjMatrix;           // light ViewProj
    depthMapData.Params.X = 0.1f;                                     // Near plane
    // TODO Light의 범위를 저장해 뒀다가 Far Plane 값에 적용 필요함
    // 일단 임시로 20 값을 넣어 뒀음
    depthMapData.Params.Y = 20.0f;                   // Far plane = Light Radius
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

ID3D11ShaderResourceView* FSpotLightShadowMap::GetShadowViewSRV()
{
    return ShadowViewSRV;
}
