#pragma once
#include "IRenderPass.h"
#include "Define.h"

class UCapsuleComponent;
class USphereComponent;
class UBoxComponent;
class FDXDBufferManager;
class FGraphicsDevice;
class FDXDShaderManager;

class FCollisionRenderPass : public IRenderPass
{
public:
    FCollisionRenderPass();
    ~FCollisionRenderPass() override;

    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    void PrepareRender() override;
    void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    void ClearRenderArr() override;

    void CreateShader();
    void CreateDummyBuffer();
    void CreateConstantBuffer();
    void PrepareDebugLineShader() const;
    void ProcessRendering();
    void DrawLines() const;

    void ReloadShader();
private:
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    /** Shader property */
    ID3D11VertexShader* VertexCollisionShader;
    ID3D11PixelShader* PixelCollisionShader;

    /** Dummy buffer */
    ID3D11Buffer* DummyVertexBuffer;

    /** Collision count constant buffer */
    ID3D11Buffer* CollisionCountConstantBuffer = nullptr;

    /** Collisions structured buffer */
    ID3D11Buffer* CollisionBoxStructuredBuffer = nullptr;
    ID3D11Buffer* CollisionSphereStructuredBuffer = nullptr;
    ID3D11Buffer* CollisionCapsuleStructuredBuffer = nullptr;

    /** Collisions structured SRVs. */
    ID3D11ShaderResourceView* CollisionBoxSRV = nullptr;
    ID3D11ShaderResourceView* CollisionSphereSRV = nullptr;
    ID3D11ShaderResourceView* CollisionCapsuleSRV = nullptr;

    /**
     * Primitive segments set
     * If change value, must change ShaderCollision.hlsl
     */
    const UINT BoxLineCount = 12;
    const UINT SphereSegments = 32;
    const UINT SphereLineCount = 3 * SphereSegments;
    const UINT CapsulesSegments = 16;
    const UINT CapsuleLineCount = 4 + 4 * CapsulesSegments;
    
private:
    /** Create collision buffers */
    ID3D11Buffer* CreateCollisionBoxBuffer(UINT NumBoxes) const;
    ID3D11Buffer* CreateCollisionSphereBuffer(UINT NumSpheres) const;
    ID3D11Buffer* CreateCollisionCapsuleBuffer(UINT NumCapsules) const;

    /** Create collision SRVs */
    ID3D11ShaderResourceView* CreateCollisionBoxSRV(ID3D11Buffer* Buffer, UINT NumBoxes) const;
    ID3D11ShaderResourceView* CreateCollisionSphereSRV(ID3D11Buffer* Buffer, UINT NumSpheres) const;
    ID3D11ShaderResourceView* CreateCollisionCapsuleSRV(ID3D11Buffer* Buffer, UINT NumCapsules) const;

    /** Create collision struct */
    static FCollisionBox CreateCollisionBox(const UBoxComponent* BoxComponent);
    static FCollisionSphere CreateCollisionSphere(const USphereComponent* SphereComponent);
    static FCollisionCapsule CreateCollisionCapsule(const UCapsuleComponent* CapsuleComponent);
    
    /** Update buffers */
    void UpdateBoxBuffer();
    void UpdateSphereBuffer();
    void UpdateCapsuleBuffer();
    void UpdateCollisionConstantBuffer() const;

    /** Mapping buffers */
    void MapBoxBuffer() const;
    void MapSphereBuffer() const;
    void MapCapsuleBuffer() const;

    /** Reset object constant */
    void ResetObjectConstant() const;

    /** Release collision buffer */
    void ReleaseCollisionBoxBuffer();
    void ReleaseCollisionSphereBuffer();
    void ReleaseCollisionCapsuleBuffer();
};
