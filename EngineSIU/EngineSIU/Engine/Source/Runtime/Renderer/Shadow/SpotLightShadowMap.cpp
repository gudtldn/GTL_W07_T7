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
    float outerConeAngleRad = FMath::DegreesToRadians(Info.OuterRad);
    float lightRange = Info.Radius; // Far Plane
    float nearPlane = 0.1f; // Near Plane

    // Robust Up Vector
    FVector up = FVector(0.0f, 0.0f, 1.0f);
    if (FMath::Abs(lightDir.Z) > 0.999f)
    {
        up = FVector(1.0f, 0.0f, 0.0f);
    }

    FMatrix spotView = JungleMath::CreateLookToMatrix(lightPos, lightDir, up);
    FMatrix spotProjection = JungleMath::CreateProjectionMatrix(
        Info.OuterRad,
        1.0f,
        nearPlane,
        lightRange
    );

    // Base transformation: World -> Light Clip Space
    FMatrix lightViewProj = spotView * spotProjection;

    // --- Step 2: Project Camera Frustum into Light Clip Space ---

    FEditorViewportClient* viewCamera = GEngineLoop.GetLevelEditor()->GetActiveViewportClient().get();
    if (!viewCamera) return; // Safety check

    auto cameraFrustumCorners = viewCamera->GetFrustumCorners(); // Get world space corners

    FBoundingBox lightClipSpaceBounds(FVector(FLT_MAX, FLT_MAX, FLT_MAX), FVector(-FLT_MAX, -FLT_MAX, -FLT_MAX));
    bool boundsValid = false;

    for (const FVector& worldCorner : cameraFrustumCorners)
    {
        // Transform world corner to light clip space
        FVector4 clipCorner = lightViewProj.TransformFVector4(FVector4(worldCorner, 1.0f));

        // Perspective Divide (W must be positive for points in front of light's near plane)
        if (clipCorner.W > KINDA_SMALL_NUMBER) // Avoid division by zero/negative W
        {
            FVector ndcCorner; // Normalized Device Coordinates [-1, 1] range (approx)
            ndcCorner.X = clipCorner.X / clipCorner.W;
            ndcCorner.Y = clipCorner.Y / clipCorner.W;
            ndcCorner.Z = clipCorner.Z / clipCorner.W; // Z is [0, 1] or [-1, 1] depending on projection matrix

            // Important: Only consider points actually inside the light's frustum volume
            // (Check against NDC bounds, Z needs careful check based on projection matrix range)
            // For simplicity here, we check X and Y, assuming Z check might be complex
            // A more robust check would involve clipping the frustum against the light cone first.
            if (ndcCorner.X >= -1.0f && ndcCorner.X <= 1.0f &&
                ndcCorner.Y >= -1.0f && ndcCorner.Y <= 1.0f &&
                ndcCorner.Z >= 0.0f && ndcCorner.Z <= 1.0f) // Assuming Z range [0, 1] for LH projection
            {
                lightClipSpaceBounds.Expand(ndcCorner);
                boundsValid = true;
            }
        }
        // Note: A more robust method involves clipping the camera frustum polygon
        // against the light frustum planes instead of just transforming corners.
    }

    // --- Step 3 & 4: Calculate Crop Matrix ---

    FMatrix cropMatrix = FMatrix::Identity; // Default to identity if bounds are invalid

    if (boundsValid &&
        lightClipSpaceBounds.min.X < lightClipSpaceBounds.max.X && // Check for valid range
        lightClipSpaceBounds.min.Y < lightClipSpaceBounds.max.Y)
    {
        // Clamp bounds to [-1, 1] just in case
        float minX = FMath::Max(-1.0f, lightClipSpaceBounds.min.X);
        float maxX = FMath::Min(1.0f, lightClipSpaceBounds.max.X);
        float minY = FMath::Max(-1.0f, lightClipSpaceBounds.min.Y);
        float maxY = FMath::Min(1.0f, lightClipSpaceBounds.max.Y);

        // Avoid division by zero if the range is tiny
        if (FMath::Abs(maxX - minX) < KINDA_SMALL_NUMBER || FMath::Abs(maxY - minY) < KINDA_SMALL_NUMBER)
        {
            // Bounds are too small, likely camera frustum is outside or edge-on
            // Fallback to standard projection (or handle differently)
            // For now, we keep cropMatrix as Identity
        }
        else
        {
            // Calculate scale and offset to map [minX, maxX] -> [-1, 1] and [minY, maxY] -> [-1, 1]
            float scaleX = 2.0f / (maxX - minX);
            float scaleY = 2.0f / (maxY - minY);
            float offsetX = -(maxX + minX) / (maxX - minX);
            float offsetY = -(maxY + minY) / (maxY - minY);

            // Create the crop matrix (Row-Major assumed for FMatrix)
            // This matrix transforms coordinates already in light clip space [-1, 1]
            // It scales and shifts the sub-rectangle [minX,maxX]x[minY,maxY] to fill [-1,1]x[-1,1]
            cropMatrix = FMatrix::Identity;
            cropMatrix.M[0][0] = scaleX;
            cropMatrix.M[1][1] = scaleY;
            cropMatrix.M[3][0] = offsetX; // Translation applied in the last row for row-major
            cropMatrix.M[3][1] = offsetY;
            // Z is usually not cropped/scaled in basic PSM, but could be for LiSPSM etc.
            // cropMatrix.M[2][2] = scaleZ;
            // cropMatrix.M[3][2] = offsetZ;
        }
    }
    else
    {
        // Handle the case where no part of the camera frustum is inside the light frustum
        // Option 1: Keep cropMatrix as Identity (render full spotlight shadow map - potentially wasteful)
        // Option 2: Mark this shadow map as unnecessary / inactive
        // Option 3: Use a default small area (less ideal)
        // Current code defaults to Identity (Option 1)
    }


    // --- Step 5: Final Shadow Matrix ---
    // Apply the crop matrix *after* the standard light view-projection
    SpotLightViewProjMatrix = lightViewProj * cropMatrix;
    FShadowViewProj LightVPData{ SpotLightViewProjMatrix };
    BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), LightVPData);

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
    FSpotLightShadowData SpotLightShadowData;
    SpotLightShadowData.SpotLightViewProj = SpotLightViewProjMatrix;
    SpotLightShadowData.ShadowBias = 0.005f;
    BufferManager->UpdateConstantBuffer(TEXT("FSpotLightShadowData"), SpotLightShadowData);
    BufferManager->BindConstantBuffer(TEXT("FSpotLightShadowData"), 6, EShaderStage::Pixel);
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
