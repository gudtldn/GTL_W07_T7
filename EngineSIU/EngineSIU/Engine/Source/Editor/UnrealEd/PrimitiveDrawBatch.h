// ============================================================================
// UPrimitiveDrawBatch.h (일부 발췌)
// ============================================================================

#pragma once
#include "Define.h"
#include <d3d11.h>

class FGraphicsDevice;

class UPrimitiveDrawBatch
{
public:
    UPrimitiveDrawBatch() = default;
    ~UPrimitiveDrawBatch();

    // 초기화 및 릴리즈
    void Initialize(FGraphicsDevice* graphics);
    void ReleaseResources();

    // 그리드 초기화 및 배치 준비 관련
    void InitializeGrid(float Spacing, int GridCount);
    void PrepareBatch(FLinePrimitiveBatchArgs& OutArgs);
    void RemoveArr();

    // 버퍼 초기화
    void InitializeVertexBuffer();

    // 업데이트 함수들
    void UpdateGridConstantBuffer(const FGridParameters& GridParams) const;
    void UpdateBoundingBoxBuffers();
    void UpdateConeBuffers();
    void UpdateOBBBuffers();
    void UpdateLinePrimitiveCountBuffer(int NumBoundingBoxes, int NumCones) const;

    // 릴리즈 함수들
    void ReleaseBoundingBoxBuffers();
    void ReleaseConeBuffers();
    void ReleaseOBBBuffers();

    // 프리미티브 렌더링 관련
    void AddAABBToBatch(const FBoundingBox& LocalAABB, const FVector& Center, const FMatrix& ModelMatrix);
    void AddOBBToBatch(const FBoundingBox& LocalAABB, const FVector& Center, const FMatrix& ModelMatrix);
    void AddConeToBatch(const FVector& Center, float Radius, float Height, int Segments, const FVector4& Color, const FMatrix& ModelMatrix);
    void AddCollisionBoxBatch(const FCollisionBox& Box);
    void AddCollisionSphereBatch(const FCollisionSphere& Sphere);
    void AddCollisionCapsuleBatch(const FCollisionCapsule& Capsule);

    // 프리미티브 버퍼 생성 함수들
    void CreatePrimitiveBuffers();
    ID3D11Buffer* CreateStaticVertexBuffer() const;
    ID3D11Buffer* CreateBoundingBoxBuffer(UINT NumBoundingBoxes) const;
    ID3D11Buffer* CreateOBBBuffer(UINT NumBoundingBoxes) const;
    ID3D11Buffer* CreateConeBuffer(UINT NumCones) const;

    // SRV 생성 함수들
    ID3D11ShaderResourceView* CreateBoundingBoxSRV(ID3D11Buffer* Buffer, UINT NumBoundingBoxes);
    ID3D11ShaderResourceView* CreateOBBSRV(ID3D11Buffer* Buffer, UINT NumBoundingBoxes);
    ID3D11ShaderResourceView* CreateConeSRV(ID3D11Buffer* Buffer, UINT NumCones);

    // 버퍼 업데이트 (데이터 복사) 함수들
    void UpdateBoundingBoxBuffer(ID3D11Buffer* Buffer, const TArray<FBoundingBox>& BoundingBoxes, int NumBoundingBoxes) const;
    void UpdateOBBBuffer(ID3D11Buffer* Buffer, const TArray<FOBB>& OBBs, int NumOBBs) const;
    void UpdateConesBuffer(ID3D11Buffer* Buffer, const TArray<FCone>& Cones, int NumCones) const;

    // 파이프라인 관련 (렌더러에서 호출하는 "prepare" 함수)
    void PrepareLineResources() const;

    /*************************
     * Collision             *
     *************************/

    /** Updates when exceeded after comparison. */
    bool IsCollisionBoxOverLimit();

    /** Updates when exceeded after comparison. */
    bool IsCollisionSphereOverLimit();

    /** Updates when exceeded after comparison. */
    bool IsCollisionCapsuleOverLimit();

    FORCEINLINE int GetCollisionBoxCount() const
    {
        return CollisionBoxes.Num();
    }
    FORCEINLINE int GetCollisionSphereCount() const
    {
        return CollisionSpheres.Num();
    }
    FORCEINLINE int GetCollisionCapsuleCount() const
    {
        return CollisionCapsules.Num();
    }

    FORCEINLINE const TArray<FCollisionBox>& GetCollisionBoxes() const
    {
        return CollisionBoxes;
    }

    FORCEINLINE const TArray<FCollisionSphere>& GetCollisionSpheres() const
    {
        return CollisionSpheres;
    }

    FORCEINLINE const TArray<FCollisionCapsule>& GetCollisionCapsules() const
    {
        return CollisionCapsules;
    }
    
    /** Remove collision containers */
    void RemoveCollisionContainers();
    
private:
    // Graphics 디바이스 (초기화 시 전달받음)
    FGraphicsDevice* Graphics = nullptr;

    // 상수 및 프리미티브 버퍼
    ID3D11Buffer* GridConstantBuffer = nullptr;
    ID3D11Buffer* LinePrimitiveBuffer = nullptr;

    // 쉐이더 리소스 뷰 (SRV)
    ID3D11ShaderResourceView* BoundingBoxSRV = nullptr;
    ID3D11ShaderResourceView* ConeSRV = nullptr;
    ID3D11ShaderResourceView* OBBSRV = nullptr;

    // 버퍼들
    ID3D11Buffer* VertexBuffer = nullptr;
    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    ID3D11Buffer* ConesBuffer = nullptr;
    ID3D11Buffer* OBBBuffer = nullptr;
    ID3D11ShaderResourceView* ConesSRV = nullptr;

    // 할당된 용량 추적
    size_t AllocatedBoundingBoxCapacity = 0;
    size_t AllocatedConeCapacity = 0;
    size_t AllocatedOBBCapacity = 0;

    size_t AllocatedCollisionBoxCapacity = 0;
    size_t AllocatedCollisionSphereCapacity = 0;
    size_t AllocatedCollisionCapsuleCapacity = 0;

    // 프리미티브 데이터 컨테이너
    TArray<FBoundingBox> BoundingBoxes;
    TArray<FOBB> OrientedBoundingBoxes;
    TArray<FCone> Cones;

    // Collision Data Container
    TArray<FCollisionBox> CollisionBoxes;
    TArray<FCollisionSphere> CollisionSpheres;
    TArray<FCollisionCapsule> CollisionCapsules;

    // 그리드 파라미터 및 추가 데이터
    FGridParameters GridParameters;
    int ConeSegmentCount = 0;
};
