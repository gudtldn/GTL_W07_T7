#include "SpotLightShadowMap.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Classes/Components/Light/SpotLightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/EditorEngine.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"

//#include "Shadow/PerspectiveShadowMapping.h"

#include "UnrealEd/EditorViewportClient.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "Engine/EditorEngine.h"

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

    for (auto SpotLight : SpotLights)
    {
        FSpotLightInfo SpotLightInfo;
        SpotLightInfo.Position = SpotLight->GetWorldLocation();
        SpotLightInfo.Direction = SpotLight->GetDirection();
        // 여기에서 psm을 업데이트하는 것 같음.
        // Begin Test
        UpdateSpotLightViewProjMatrices(SpotLightInfo);
        CalculateSpotLightPSMMatrix(SpotLightInfo);
        // End Test
    }
}

void FSpotLightShadowMap::UpdateSpotLightViewProjMatrices(const FSpotLightInfo& Info)
{   
    float fovY = Info.OuterRad * 2.0f; // 외부 원뿔 각도
    float aspectRatio = 1.0f; // 정방형 섀도우 맵
    float nearZ = 0.1f;
    float farZ = Info.Radius;
    FMatrix Projection = JungleMath::CreateProjectionMatrix(fovY, aspectRatio, nearZ, farZ);

    FVector dir = Info.Direction;

    FVector up;
    if (FMath::Abs(dir.Z) > 0.99f)
    {
        up = FVector(1.0f, 0.0f, 0.0f);
    }
    else
    {
        up = FVector(0.0f, 0.0f, 1.0f);
    }

    FVector right = FVector::CrossProduct(up, dir).GetSafeNormal();
    up = FVector::CrossProduct(dir, right).GetSafeNormal();

    FVector target = Info.Position + dir;
    FMatrix View = JungleMath::CreateViewMatrix(Info.Position, target, up);

    SpotLightViewProjMatrix = View * Projection;
}

//void FSpotLightShadowMap::CalculateSpotLightPSMMatrix(const FSpotLightInfo& Info)
//{
//    std::shared_ptr<FEditorViewportClient> ActiveViewportClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
//    FMatrix CameraView = ActiveViewportClient->GetViewMatrix();
//    FMatrix CameraProj = ActiveViewportClient->GetProjectionMatrix();
//    FMatrix CameraViewProj = CameraView * CameraProj;
//    FMatrix InvCameraViewProj = FMatrix::Inverse(CameraViewProj);
//
//    SpotLightPSMMatrix = SpotLightViewProjMatrix*InvCameraViewProj;
//}

void FSpotLightShadowMap::CalculateSpotLightPSMMatrix(const FSpotLightInfo& Info)
{
    std::shared_ptr<FEditorViewportClient> ActiveViewportClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();

    FMatrix CameraView = ActiveViewportClient->GetViewMatrix();
    FMatrix CameraProj = ActiveViewportClient->GetProjectionMatrix();

    // 카메라의 frustum을 생성
    TArray<FVector> FrustumCorners = CalculateFrustumCorners(CameraView, CameraProj);

    // 업이 잘못들어갔을 확률이 꽤 높음.
    FMatrix LightView = JungleMath::CreateViewMatrix(
        Info.Position,
        Info.Position + Info.Direction,
        CalculateUpVector(Info.Direction)
    );
    FMatrix InvLightView = FMatrix::Inverse(LightView);

    TArray<FVector> LightSpaceCorners;
    for (const FVector& Corner : FrustumCorners)
    {
        FVector LightSpaceCorner = InvLightView.TransformPosition(Corner);
        LightSpaceCorners.Add(LightSpaceCorner);
    }

    // frustum을 
    FVector MinBound, MaxBound;
    CalculateBoundingBox(LightSpaceCorners, MinBound, MaxBound);

    float nearPlane = MinBound.Z;  // 가장 가까운 평면
    float farPlane = MaxBound.Z;   // 가장 먼 평면
    float width = MaxBound.X - MinBound.X;
    float height = MaxBound.Y - MinBound.Y;

    FMatrix OptimizedProjection = JungleMath::CreateOrthoOffCenterProjectionMatrix(
    //FMatrix OptimizedProjection = JungleMath::CreatePerspectiveOffCenterProjectionMatrix(
        MinBound.X, MaxBound.X,
        MinBound.Y, MaxBound.Y,
        nearPlane, farPlane
    );

    SpotLightPSMMatrix = LightView * OptimizedProjection;
}

FVector FSpotLightShadowMap::CalculateUpVector(const FVector& dir)
{
    if (FMath::Abs(dir.Z) > 0.99f)
    {
        return FVector(1.0f, 0.0f, 0.0f);
    }
    return FVector(0.0f, 1.0f, 0.0f);
}

void FSpotLightShadowMap::CalculateBoundingBox(const TArray<FVector>& Points, FVector& OutMin, FVector& OutMax)
{
    if (Points.Num() == 0) return;

    OutMin = Points[0];
    OutMax = Points[0];

    for (const FVector& Point : Points)
    {
        OutMin.X = FMath::Min(OutMin.X, Point.X);
        OutMin.Y = FMath::Min(OutMin.Y, Point.Y);
        OutMin.Z = FMath::Min(OutMin.Z, Point.Z);

        OutMax.X = FMath::Max(OutMax.X, Point.X);
        OutMax.Y = FMath::Max(OutMax.Y, Point.Y);
        OutMax.Z = FMath::Max(OutMax.Z, Point.Z);
    }
}

TArray<FVector> FSpotLightShadowMap::CalculateFrustumCorners(const FMatrix& ViewMatrix, const FMatrix& ProjMatrix)
{
    // 카메라 절두체의 8개 코너 포인트 계산
    FMatrix InvViewProj = FMatrix::Inverse(ViewMatrix * ProjMatrix);

    TArray<FVector> Corners;
    float cornerPoints[8][4] = {
        {-1, -1, 0, 1}, {1, -1, 0, 1}, {1, 1, 0, 1}, {-1, 1, 0, 1},
        {-1, -1, 1, 1}, {1, -1, 1, 1}, {1, 1, 1, 1}, {-1, 1, 1, 1}
    };

    for (int i = 0; i < 8; ++i)
    {
        FVector4 clipSpaceCorner(cornerPoints[i][0], cornerPoints[i][1], cornerPoints[i][2], cornerPoints[i][3]);
        FVector4 worldSpaceCorner = FMatrix::TransformVector(clipSpaceCorner, InvViewProj);
        Corners.Add(FVector(worldSpaceCorner.X, worldSpaceCorner.Y, worldSpaceCorner.Z));
    }

    return Corners;
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
    //BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), SpotLightViewProjMatrix);
    // Begin Test
    BufferManager->UpdateConstantBuffer(TEXT("FShadowViewProj"), SpotLightPSMMatrix);
    // End Test

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

void FSpotLightShadowMap::UpdateConstantBuffer()
{
    FSpotLightShadowData SpotLightShadowData;
    //SpotLightShadowData.SpotLightViewProj = SpotLightViewProjMatrix;
    SpotLightShadowData.SpotLightViewProj = SpotLightPSMMatrix;
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
    //depthMapData.ViewProj = SpotLightViewProjMatrix;           // light ViewProj
    depthMapData.ViewProj = SpotLightPSMMatrix;
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
