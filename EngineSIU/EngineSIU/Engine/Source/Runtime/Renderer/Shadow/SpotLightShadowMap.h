#pragma once
#include "Define.h"
#include "Math/Matrix.h"
#include "UnrealEd/EditorViewportClient.h"

class FDXDBufferManager;
class FGraphicsDevice;
class FDXDShaderManager;
class USpotLightComponent;

class FSpotLightShadowMap
{
public:
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphic, FDXDShaderManager* InShaderManager);
    void CalculateSpotLightPSMMatrix(const FSpotLightInfo& Info);
    FVector CalculateUpVector(const FVector& dir);

    void CalculateBoundingBox(const TArray<FVector>& Points, FVector& OutMin, FVector& OutMax);

    void UpdateSpotLightViewProjMatrices(const FSpotLightInfo& Info);
    void PrepareRender();
    TArray<FVector> CalculateFrustumCorners(const FMatrix& ViewMatrix, const FMatrix& ProjMatrix);
    void RenderShadowMap();
    void UpdateConstantBuffer();
    void ClearRenderArr();

    void SetShadowResource(int tStart);
    void SetShadowSampler(int sStart);

    ID3D11ShaderResourceView* GetShadowSRV();
    ID3D11ShaderResourceView* GetShadowViewSRV();
    void RenderLinearDepth();

private:
    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;

    ID3D11VertexShader* DepthVS = nullptr;
    ID3D11InputLayout* DepthIL = nullptr;

    TArray<USpotLightComponent*> SpotLights;

    ID3D11Texture2D* DepthStencilBuffer = nullptr;
    ID3D11DepthStencilView* ShadowDSV = nullptr;
    ID3D11ShaderResourceView* ShadowSRV = nullptr;
    ID3D11SamplerState* ShadowSampler = nullptr;
    uint32 ShadowMapSize = 1024;
    FMatrix SpotLightViewProjMatrix = {};
    FMatrix SpotLightPSMMatrix = {};

    // Begin ImGui Debug
    ID3D11Texture2D* DepthLinearBuffer = { nullptr };
    ID3D11ShaderResourceView* ShadowViewSRV = { nullptr };
    ID3D11RenderTargetView* ShadowViewRTV = { nullptr };
    ID3D11SamplerState* LinearSampler = nullptr;
    ID3D11VertexShader* FullscreenVS = nullptr;
    ID3D11InputLayout* FullscreenIL = nullptr;
    ID3D11PixelShader* DepthVisualizePS = nullptr;
    // End ImGui Debug
};
