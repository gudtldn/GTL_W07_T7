#pragma once

#define _TCHAR_DEFINED
#include <wrl.h>
#include "PrimitiveComponent.h"


class UBillboardComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UBillboardComponent, UPrimitiveComponent)

public:
    UBillboardComponent();
    virtual ~UBillboardComponent();
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersectionLocal(
        FVector& rayOrigin,
        FVector& rayDirection,
        float& pfNearHitDistance
    ) override;

    virtual void SetTexture(const FWString& _fileName);
    void SetUUIDParent(USceneComponent* _parent);
    FMatrix CreateBillboardMatrix() const;
    FString GetTexturePath() const { return TexturePath; }

    float finalIndexU = 0.0f;
    float finalIndexV = 0.0f;
    std::shared_ptr<FTexture> Texture;

    bool bIsEditorBillboard = false;

protected:
    USceneComponent* m_parent = nullptr;
    FString TexturePath = TEXT("default");

    // NDC 픽킹을 위한 내부 함수 : quadVertices는 월드 공간 정점 배열
    bool CheckPickingOnNDC(const TArray<FVector>& quadVertices, float& hitDistance) const;

};
