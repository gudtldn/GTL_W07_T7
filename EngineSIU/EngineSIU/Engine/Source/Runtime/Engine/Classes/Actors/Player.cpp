#include "Player.h"

#include "UnrealClient.h"
#include "World/World.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Components/Light/LightComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "Engine/EditorEngine.h"

AEditorPlayer::AEditorPlayer()
{
    playerCameraManager = FObjectFactory::ConstructObject<
APlayerCameraManager>(this);
}


void AEditorPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Input();
}

void AEditorPlayer::Input()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (io.WantCaptureKeyboard) return;

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (!bLeftMouseDown)
        {   
            bLeftMouseDown = true;

            POINT mousePos;
            GetCursorPos(&mousePos);
            GetCursorPos(&m_LastMousePos);
            ScreenToClient(GEngineLoop.AppWnd, &mousePos);

            /*
            uint32 UUID = FEngineLoop::GraphicDevice.GetPixelUUID(mousePos);
            // TArray<UObject*> objectArr = GetWorld()->GetObjectArr();
            for ( const USceneComponent* obj : TObjectRange<USceneComponent>())
            {
                if (obj->GetUUID() != UUID) continue;

                UE_LOG(ELogLevel::Display, *obj->GetName());
            }
            */

            FVector pickPosition;

            std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
            ScreenToViewSpace(mousePos.x, mousePos.y, ActiveViewport, pickPosition);
            bool res = PickGizmo(pickPosition, ActiveViewport.get());
            if (!res) PickActor(pickPosition);
        }
        else
        {
            PickedObjControl();
        }
    }
    else
    {
        if (bLeftMouseDown)
        {
            bLeftMouseDown = false;
            std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
            ActiveViewport->SetPickedGizmoComponent(nullptr);
        }
    }
}

void AEditorPlayer::ProcessGizmoIntersection(UStaticMeshComponent* Component, const FVector& PickPosition, FEditorViewportClient* InActiveViewport, bool& bIsPickedGizmo)
{
    int maxIntersect = 0;
    float minDistance = FLT_MAX;
    float Distance = 0.0f;
    int currentIntersectCount = 0;
    if (!Component) return;
    if (RayIntersectsObject(PickPosition, Component, Distance, currentIntersectCount))
    {
        if (Distance < minDistance)
        {
            minDistance = Distance;
            maxIntersect = currentIntersectCount;
            //GetWorld()->SetPickingGizmo(iter);
            InActiveViewport->SetPickedGizmoComponent(Component);
            bIsPickedGizmo = true;
        }
        else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
        {
            maxIntersect = currentIntersectCount;
            //GetWorld()->SetPickingGizmo(iter);
            InActiveViewport->SetPickedGizmoComponent(Component);
            bIsPickedGizmo = true;
        }
    }
}

bool AEditorPlayer::PickGizmo(FVector& pickPosition, FEditorViewportClient* InActiveViewport)
{
    bool isPickedGizmo = false;
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (Engine->GetSelectedActor())
    {
        if (ControlMode == CM_TRANSLATION)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetArrowArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
        else if (ControlMode == CM_ROTATION)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetDiscArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
        else if (ControlMode == CM_SCALE)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetScaleArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
    }
    return isPickedGizmo;
}

void AEditorPlayer::PickActor(const FVector& pickPosition)
{
    if (!(ShowFlags::GetInstance().currentFlags & EEngineShowFlags::SF_Primitives)) return;

    const UActorComponent* Possible = nullptr;
    int maxIntersect = 0;
    float minDistance = FLT_MAX;
    for (const auto iter : TObjectRange<UPrimitiveComponent>())
    {
        UPrimitiveComponent* pObj;
        if (iter->IsA<UPrimitiveComponent>() || iter->IsA<ULightComponentBase>())
        {
            pObj = static_cast<UPrimitiveComponent*>(iter);
        }
        else
        {
            continue;
        }

        if (pObj && !pObj->IsA<UGizmoBaseComponent>())
        {
            float Distance = 0.0f;
            int currentIntersectCount = 0;
            if (RayIntersectsObject(pickPosition, pObj, Distance, currentIntersectCount))
            {
                if (Distance < minDistance)
                {
                    minDistance = Distance;
                    maxIntersect = currentIntersectCount;
                    Possible = pObj;
                }
                else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                {
                    maxIntersect = currentIntersectCount;
                    Possible = pObj;
                }
            }
        }
    }
    if (Possible)
    {
        Cast<UEditorEngine>(GEngine)->SelectActor(Possible->GetOwner());
    }
    else
    {
        Cast<UEditorEngine>(GEngine)->SelectActor(nullptr);
    }
}

void AEditorPlayer::AddControlMode()
{
    ControlMode = static_cast<EControlMode>((ControlMode + 1) % CM_END);
}

void AEditorPlayer::AddCoordiMode()
{
    CoordMode = static_cast<ECoordMode>((CoordMode + 1) % CDM_END);
}

void AEditorPlayer::ScreenToViewSpace(int32 ScreenX, int32 ScreenY, const std::shared_ptr<FEditorViewportClient>& ActiveViewport, FVector& RayOrigin)
{
    FRect Rect = ActiveViewport->GetViewport()->GetRect();
    
    float ViewportX = static_cast<float>(ScreenX) - Rect.TopLeftX;
    float ViewportY = static_cast<float>(ScreenY) - Rect.TopLeftY;

    FMatrix ProjectionMatrix = ActiveViewport->GetProjectionMatrix();
    
    RayOrigin.X = ((2.0f * ViewportX / Rect.Width) - 1) / ProjectionMatrix[0][0];
    RayOrigin.Y = -((2.0f * ViewportY / Rect.Height) - 1) / ProjectionMatrix[1][1];
    
    if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->IsOrthographic())
    {
        RayOrigin.Z = 0.0f;  // 오쏘 모드에서는 unproject 시 near plane 위치를 기준
    }
    else
    {
        RayOrigin.Z = 1.0f;  // 퍼스펙티브 모드: near plane
    }
}

int AEditorPlayer::RayIntersectsObject(const FVector& PickPosition, USceneComponent* Component, float& HitDistance, int& IntersectCount)
{
    FMatrix WorldMatrix = Component->GetWorldMatrix();
	FMatrix ViewMatrix = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();
    
    bool bIsOrtho = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->IsOrthographic();
    

    if (bIsOrtho)
    {
        // 오쏘 모드: ScreenToViewSpace()에서 계산된 pickPosition이 클립/뷰 좌표라고 가정
        FMatrix inverseView = FMatrix::Inverse(ViewMatrix);
        // pickPosition을 월드 좌표로 변환
        FVector worldPickPos = inverseView.TransformPosition(PickPosition);  
        // 오쏘에서는 픽킹 원점은 unproject된 픽셀의 위치
        FVector rayOrigin = worldPickPos;
        // 레이 방향은 카메라의 정면 방향 (평행)
        FVector orthoRayDir = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->OrthogonalCamera.GetForwardVector().GetSafeNormal();

        // 객체의 로컬 좌표계로 변환
        FMatrix LocalMatrix = FMatrix::Inverse(WorldMatrix);
        FVector LocalRayOrigin = LocalMatrix.TransformPosition(rayOrigin);
        FVector LocalRayDir = (LocalMatrix.TransformPosition(rayOrigin + orthoRayDir) - LocalRayOrigin).GetSafeNormal();
        
        IntersectCount = Component->CheckRayIntersectionLocal(LocalRayOrigin, LocalRayDir, HitDistance);
        return IntersectCount;
    }
    else
    {
        FMatrix inverseMatrix = FMatrix::Inverse(WorldMatrix * ViewMatrix);
        FVector cameraOrigin = { 0,0,0 };
        FVector pickRayOrigin = inverseMatrix.TransformPosition(cameraOrigin);
        // 퍼스펙티브 모드의 기존 로직 사용
        FVector transformedPick = inverseMatrix.TransformPosition(PickPosition);
        FVector rayDirection = (transformedPick - pickRayOrigin).GetSafeNormal();
        
        IntersectCount = Component->CheckRayIntersectionLocal(pickRayOrigin, rayDirection, HitDistance);

        if (IntersectCount > 0)
        {
            FVector LocalHitPoint = pickRayOrigin + rayDirection * HitDistance;

            FVector WorldHitPoint = WorldMatrix.TransformPosition(LocalHitPoint);

            FVector WorldRayOrigin;
            FMatrix InverseView = FMatrix::Inverse(ViewMatrix);
            WorldRayOrigin = InverseView.TransformPosition(cameraOrigin);

            float WorldDistance = FVector::Distance(WorldRayOrigin, WorldHitPoint);

            HitDistance = WorldDistance;
        }
        return IntersectCount;
    }
}

void AEditorPlayer::PickedObjControl()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    FEditorViewportClient* ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient().get();
    if (Engine && Engine->GetSelectedActor() && ActiveViewport->GetPickedGizmoComponent())
    {
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        int32 deltaX = currentMousePos.x - m_LastMousePos.x;
        int32 deltaY = currentMousePos.y - m_LastMousePos.y;

        // USceneComponent* pObj = GetWorld()->GetPickingObj();
        AActor* PickedActor = Engine->GetSelectedActor();
        UGizmoBaseComponent* Gizmo = static_cast<UGizmoBaseComponent*>(ActiveViewport->GetPickedGizmoComponent());
        switch (ControlMode)
        {
        case CM_TRANSLATION:
            // ControlTranslation(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);
            // SLevelEditor에 있음
            break;
        case CM_SCALE:
            ControlScale(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);

            break;
        case CM_ROTATION:
            ControlRotation(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);
            break;
        default:
            break;
        }
        m_LastMousePos = currentMousePos;
    }
}

void AEditorPlayer::ControlRotation(USceneComponent* Component, UGizmoBaseComponent* Gizmo, float DeltaX, float DeltaY)
{
    const auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
    const FViewportCamera* ViewTransform = ActiveViewport->GetViewportType() == LVT_Perspective
                                                        ? &ActiveViewport->PerspectiveCamera
                                                        : &ActiveViewport->OrthogonalCamera;

    FVector CameraForward = ViewTransform->GetForwardVector();
    FVector CameraRight = ViewTransform->GetRightVector();
    FVector CameraUp = ViewTransform->GetUpVector();

    FQuat currentRotation = Component->GetWorldRotation().ToQuaternion();

    FQuat rotationDelta;

    if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleX)
    {
        float rotationAmount = (CameraUp.Z >= 0 ? -1.0f : 1.0f) * DeltaY * 0.01f;
        rotationAmount = rotationAmount + (CameraRight.X >= 0 ? 1.0f : -1.0f) * DeltaX * 0.01f;

        rotationDelta = FQuat(FVector(1.0f, 0.0f, 0.0f), rotationAmount); // ���� X �� ���� ȸ��
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleY)
    {
        float rotationAmount = (CameraRight.X >= 0 ? 1.0f : -1.0f) * DeltaX * 0.01f;
        rotationAmount = rotationAmount + (CameraUp.Z >= 0 ? 1.0f : -1.0f) * DeltaY * 0.01f;

        rotationDelta = FQuat(FVector(0.0f, 1.0f, 0.0f), rotationAmount); // ���� Y �� ���� ȸ��
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleZ)
    {
        float rotationAmount = (CameraForward.X <= 0 ? -1.0f : 1.0f) * DeltaX * 0.01f;
        rotationDelta = FQuat(FVector(0.0f, 0.0f, 1.0f), rotationAmount); // ���� Z �� ���� ȸ��
    }
    if (CoordMode == CDM_LOCAL)
    {
        Component->SetRelativeRotation(currentRotation * rotationDelta);
    }
    else if (CoordMode == CDM_WORLD)
    {
        Component->SetRelativeRotation(rotationDelta * currentRotation);
    }
}

void AEditorPlayer::ControlScale(USceneComponent* Component, UGizmoBaseComponent* Gizmo, float DeltaX, float DeltaY)
{
    const auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
    const FViewportCamera* ViewTransform = ActiveViewport->GetViewportType() == LVT_Perspective
                                                        ? &ActiveViewport->PerspectiveCamera
                                                        : &ActiveViewport->OrthogonalCamera;
    FVector CameraRight = ViewTransform->GetRightVector();
    FVector CameraUp = ViewTransform->GetUpVector();
    
    // 월드 좌표계에서 카메라 방향을 고려한 이동
    if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleX)
    {
        // 카메라의 오른쪽 방향을 X축 이동에 사용
        FVector moveDir = CameraRight * DeltaX * 0.05f;
        Component->AddScale(FVector(moveDir.X, 0.0f, 0.0f));
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleY)
    {
        // 카메라의 오른쪽 방향을 Y축 이동에 사용
        FVector moveDir = CameraRight * DeltaX * 0.05f;
        Component->AddScale(FVector(0.0f, moveDir.Y, 0.0f));
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleZ)
    {
        // 카메라의 위쪽 방향을 Z축 이동에 사용
        FVector moveDir = CameraUp * -DeltaY * 0.05f;
        Component->AddScale(FVector(0.0f, 0.0f, moveDir.Z));
    }
}
