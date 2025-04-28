#include "CollisionRenderPass.h"

#include "UnrealClient.h"
#include "Components/Collision/BoxComponent.h"
#include "Components/Collision/CapsuleComponent.h"
#include "Components/Collision/SphereComponent.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/Engine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"

FCollisionRenderPass::FCollisionRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
    , VertexCollisionShader(nullptr)
    , PixelCollisionShader(nullptr)
    , DummyVertexBuffer(nullptr)
{
}

FCollisionRenderPass::~FCollisionRenderPass()
{
    ReleaseCollisionBoxBuffer();
    ReleaseCollisionSphereBuffer();
    ReleaseCollisionCapsuleBuffer();
    
    if (DummyVertexBuffer)
    {
        DummyVertexBuffer->Release();
        DummyVertexBuffer = nullptr;
    }
    
    if (CollisionCountConstantBuffer)
    {
        CollisionCountConstantBuffer->Release();
        CollisionCountConstantBuffer = nullptr;
    }

}

void FCollisionRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
    
    CreateShader();
    CreateDummyBuffer();
    CreateConstantBuffer();
}

void FCollisionRenderPass::PrepareRender()
{
    for (const auto iter : TObjectRange<UShapeComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (auto Box = Cast<UBoxComponent>(iter))
            {
                FEngineLoop::PrimitiveDrawBatch.AddCollisionBoxBatch(CreateCollisionBox(Box));
            }

            if (auto Sphere = Cast<USphereComponent>(iter))
            {
                FEngineLoop::PrimitiveDrawBatch.AddCollisionSphereBatch(CreateCollisionSphere(Sphere));
            }

            if (auto Capsule = Cast<UCapsuleComponent>(iter))
            {
                FEngineLoop::PrimitiveDrawBatch.AddCollisionCapsuleBatch(CreateCollisionCapsule(Capsule));
            }
        }
    }
}

void FCollisionRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    constexpr EResourceType ResourceType = EResourceType::ERT_Editor;

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, ViewportResource->GetDepthStencilView());

    ProcessRendering();

    /** Rendertarget release */
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FCollisionRenderPass::ClearRenderArr()
{
}

void FCollisionRenderPass::CreateShader()
{
    HRESULT hr = ShaderManager->AddVertexShader(L"VertexCollisionShader", L"Shaders/ShaderCollision.hlsl", "mainVS");
    hr = ShaderManager->AddPixelShader(L"PixelCollisionShader", L"Shaders/ShaderCollision.hlsl", "mainPS");

    VertexCollisionShader = ShaderManager->GetVertexShaderByKey(L"VertexCollisionShader");
    PixelCollisionShader = ShaderManager->GetPixelShaderByKey(L"PixelCollisionShader");
}

void FCollisionRenderPass::CreateDummyBuffer()
{
    FSimpleVertex Vertices[2] = {{}, {}};

    D3D11_BUFFER_DESC VBDesc = {};
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.ByteWidth = sizeof(Vertices);
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA VBInitData = {};
    VBInitData.pSysMem = Vertices;
    
    HRESULT HR = Graphics->Device->CreateBuffer(&VBDesc, &VBInitData, &DummyVertexBuffer);
}

void FCollisionRenderPass::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.ByteWidth = (sizeof(FCollisionCountConstants) + 0xf) & 0xfffffff0;
    
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &CollisionCountConstantBuffer);
}

void FCollisionRenderPass::PrepareDebugLineShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexCollisionShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelCollisionShader, nullptr, 0);

    if (Graphics && Graphics->DeviceContext)
    {
        // Register collision count constant buffer (register b0) to vertex shader.
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &CollisionCountConstantBuffer);

        // Register collision SRVs (registers t0, t1, t2) to vertex shader
        Graphics->DeviceContext->VSSetShaderResources(0, 1, &CollisionBoxSRV);
        Graphics->DeviceContext->VSSetShaderResources(1, 1, &CollisionSphereSRV);
        Graphics->DeviceContext->VSSetShaderResources(2, 1, &CollisionCapsuleSRV);
    }
}

void FCollisionRenderPass::ProcessRendering()
{
    PrepareDebugLineShader();
    ResetObjectConstant();

    UpdateBoxBuffer();
    UpdateSphereBuffer();
    UpdateCapsuleBuffer();
    UpdateCollisionConstantBuffer();

    DrawLines();

    FEngineLoop::PrimitiveDrawBatch.RemoveCollisionContainers();
}

void FCollisionRenderPass::DrawLines() const
{
    UINT NumBox = FEngineLoop::PrimitiveDrawBatch.GetCollisionBoxCount();
    UINT NumSphere = FEngineLoop::PrimitiveDrawBatch.GetCollisionSphereCount();
    UINT NumCapsule = FEngineLoop::PrimitiveDrawBatch.GetCollisionCapsuleCount();
    
    UINT Stride = sizeof(FSimpleVertex);
    UINT Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &DummyVertexBuffer, &Stride, &Offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    constexpr UINT VertexCountPerInstance = 2;
    UINT InstanceCount =
        (NumBox * BoxLineCount) +
        (NumSphere * SphereLineCount) +
        (NumCapsule * CapsuleLineCount);
    
    Graphics->DeviceContext->DrawInstanced(VertexCountPerInstance, InstanceCount, 0, 0);
}

void FCollisionRenderPass::ReloadShader()
{
    VertexCollisionShader = ShaderManager->GetVertexShaderByKey(L"VertexCollisionShader");
    PixelCollisionShader = ShaderManager->GetPixelShaderByKey(L"PixelCollisionShader");
}

ID3D11Buffer* FCollisionRenderPass::CreateCollisionBoxBuffer(UINT NumBoxes) const
{
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof(FCollisionBox) * NumBoxes;
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    BufferDesc.StructureByteStride = sizeof(FCollisionBox);

    ID3D11Buffer* Buffer = nullptr;
    Graphics->Device->CreateBuffer(&BufferDesc, nullptr, &Buffer);
    return Buffer;
}

ID3D11Buffer* FCollisionRenderPass::CreateCollisionSphereBuffer(UINT NumSpheres) const
{
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof(FCollisionSphere) * NumSpheres;
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    BufferDesc.StructureByteStride = sizeof(FCollisionSphere);

    ID3D11Buffer* Buffer = nullptr;
    Graphics->Device->CreateBuffer(&BufferDesc, nullptr, &Buffer);
    return Buffer;
}

ID3D11Buffer* FCollisionRenderPass::CreateCollisionCapsuleBuffer(UINT NumCapsules) const
{
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof(FCollisionCapsule) * NumCapsules;
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    BufferDesc.StructureByteStride = sizeof(FCollisionCapsule);

    ID3D11Buffer* Buffer = nullptr;
    Graphics->Device->CreateBuffer(&BufferDesc, nullptr, &Buffer);
    return Buffer;
}

ID3D11ShaderResourceView* FCollisionRenderPass::CreateCollisionBoxSRV(ID3D11Buffer* Buffer, UINT NumBoxes) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.ElementOffset = 0;
    SRVDesc.Buffer.NumElements = NumBoxes;

    ID3D11ShaderResourceView* SRV = nullptr;
    Graphics->Device->CreateShaderResourceView(Buffer, &SRVDesc, &SRV);
    return SRV;
}

ID3D11ShaderResourceView* FCollisionRenderPass::CreateCollisionSphereSRV(ID3D11Buffer* Buffer, UINT NumSpheres) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.ElementOffset = 0;
    SRVDesc.Buffer.NumElements = NumSpheres;

    ID3D11ShaderResourceView* SRV = nullptr;
    Graphics->Device->CreateShaderResourceView(Buffer, &SRVDesc, &SRV);
    return SRV;
}

ID3D11ShaderResourceView* FCollisionRenderPass::CreateCollisionCapsuleSRV(ID3D11Buffer* Buffer, UINT NumCapsules) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.ElementOffset = 0;
    SRVDesc.Buffer.NumElements = NumCapsules;

    ID3D11ShaderResourceView* SRV = nullptr;
    Graphics->Device->CreateShaderResourceView(Buffer, &SRVDesc, &SRV);
    return SRV;
}

FCollisionBox FCollisionRenderPass::CreateCollisionBox(const UBoxComponent* BoxComponent)
{
    FCollisionBox OutBox = {};

    OutBox.WorldMatrix = BoxComponent->GetWorldMatrix();
    
    OutBox.Center = BoxComponent->GetRelativeLocation();
    OutBox.Extent = BoxComponent->GetScaledBoxExtent();
    
    FLinearColor Color(BoxComponent->ShapeColor);
    OutBox.Color = Color;

    return OutBox;
}

FCollisionSphere FCollisionRenderPass::CreateCollisionSphere(const USphereComponent* SphereComponent)
{
    FCollisionSphere OutSphere = {};

    OutSphere.WorldMatrix = SphereComponent->GetWorldMatrix();

    OutSphere.Center = SphereComponent->GetRelativeLocation();
    OutSphere.Radius = SphereComponent->GetScaledSphereRadius();

    FLinearColor Color(SphereComponent->ShapeColor);
    OutSphere.Color = Color;
    
    return OutSphere;
}

FCollisionCapsule FCollisionRenderPass::CreateCollisionCapsule(const UCapsuleComponent* CapsuleComponent)
{
    FCollisionCapsule OutCapsule = {};

    OutCapsule.WorldMatrix = CapsuleComponent->GetWorldMatrix();
    
    OutCapsule.Center = CapsuleComponent->GetRelativeLocation();
    OutCapsule.Radius = CapsuleComponent->GetScaledCapsuleRadius();
    OutCapsule.HalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();

    FLinearColor Color(CapsuleComponent->ShapeColor);
    OutCapsule.Color = Color;
    
    return OutCapsule;
}


void FCollisionRenderPass::UpdateBoxBuffer()
{
    if (FEngineLoop::PrimitiveDrawBatch.IsCollisionBoxOverLimit())
    {
        ReleaseCollisionBoxBuffer();
        
        UINT BoxCount = static_cast<UINT>(FEngineLoop::PrimitiveDrawBatch.GetCollisionBoxCount());
        CollisionBoxStructuredBuffer = CreateCollisionBoxBuffer(BoxCount);
        CollisionBoxSRV = CreateCollisionBoxSRV(CollisionBoxStructuredBuffer, BoxCount);
    }

    if (CollisionBoxStructuredBuffer && CollisionBoxSRV)
    {
        MapBoxBuffer();
    }
}

void FCollisionRenderPass::UpdateSphereBuffer()
{
    if (FEngineLoop::PrimitiveDrawBatch.IsCollisionSphereOverLimit())
    {
        ReleaseCollisionSphereBuffer();
        
        UINT SphereCount = static_cast<UINT>(FEngineLoop::PrimitiveDrawBatch.GetCollisionSphereCount());
        CollisionSphereStructuredBuffer = CreateCollisionSphereBuffer(SphereCount);
        CollisionSphereSRV = CreateCollisionSphereSRV(CollisionSphereStructuredBuffer, SphereCount);
    }
    
    if (CollisionSphereStructuredBuffer && CollisionSphereSRV)
    {
        MapSphereBuffer();
    }
}

void FCollisionRenderPass::UpdateCapsuleBuffer()
{
    if (FEngineLoop::PrimitiveDrawBatch.IsCollisionCapsuleOverLimit())
    {
        ReleaseCollisionCapsuleBuffer();
        
        UINT CapsuleCount = static_cast<UINT>(FEngineLoop::PrimitiveDrawBatch.GetCollisionCapsuleCount());
        CollisionCapsuleStructuredBuffer = CreateCollisionCapsuleBuffer(CapsuleCount);
        CollisionCapsuleSRV = CreateCollisionCapsuleSRV(CollisionCapsuleStructuredBuffer, CapsuleCount);
    }

    if (CollisionCapsuleStructuredBuffer && CollisionCapsuleSRV)
    {
        MapCapsuleBuffer();
    }
}

void FCollisionRenderPass::UpdateCollisionConstantBuffer() const
{
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(CollisionCountConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Buffer Map 실패, HRESULT: 0x%X"), hr);
        return;
    }

    auto Data = static_cast<FCollisionCountConstants*>(MappedResource.pData);
    Data->BoxCount = FEngineLoop::PrimitiveDrawBatch.GetCollisionBoxCount();
    Data->SphereCount = FEngineLoop::PrimitiveDrawBatch.GetCollisionSphereCount();
    Data->CapsuleCount = FEngineLoop::PrimitiveDrawBatch.GetCollisionCapsuleCount();
    Data->Pad0 = -1;
    Graphics->DeviceContext->Unmap(CollisionCountConstantBuffer, 0);

    Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &CollisionCountConstantBuffer);
}

void FCollisionRenderPass::MapBoxBuffer() const
{
    if (!CollisionBoxStructuredBuffer)
    {
        return;
    }

    const TArray<FCollisionBox>& Boxes = FEngineLoop::PrimitiveDrawBatch.GetCollisionBoxes();
    
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    Graphics->DeviceContext->Map(CollisionBoxStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    auto Data = static_cast<FCollisionBox*>(MappedResource.pData);
    
    for (int i = 0; i < Boxes.Num(); i++)
    {
        Data[i] = Boxes[i];
    }
    
    Graphics->DeviceContext->Unmap(CollisionBoxStructuredBuffer, 0);
}

void FCollisionRenderPass::MapSphereBuffer() const
{
    if (!CollisionSphereStructuredBuffer)
    {
        return;
    }

    const TArray<FCollisionSphere>& Spheres = FEngineLoop::PrimitiveDrawBatch.GetCollisionSpheres();
    
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    Graphics->DeviceContext->Map(CollisionSphereStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    auto Data = static_cast<FCollisionSphere*>(MappedResource.pData);
    
    for (int i = 0; i < Spheres.Num(); i++)
    {
        Data[i] = Spheres[i];
    }
    
    Graphics->DeviceContext->Unmap(CollisionSphereStructuredBuffer, 0);
}

void FCollisionRenderPass::MapCapsuleBuffer() const
{
    if (!CollisionCapsuleStructuredBuffer)
    {
        return;
    }

    const TArray<FCollisionCapsule>& Capsules = FEngineLoop::PrimitiveDrawBatch.GetCollisionCapsules();
    
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    Graphics->DeviceContext->Map(CollisionCapsuleStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    auto Data = static_cast<FCollisionCapsule*>(MappedResource.pData);
    
    for (int i = 0; i < Capsules.Num(); i++)
    {
        Data[i] = Capsules[i];
    }
    
    Graphics->DeviceContext->Unmap(CollisionCapsuleStructuredBuffer, 0);
}

void FCollisionRenderPass::ResetObjectConstant() const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = FMatrix::Identity;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(FMatrix::Identity));
    ObjectData.UUIDColor = FVector4(0, 0, 0, 0);
    ObjectData.bIsSelected = false;
    
    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
    BufferManager->BindConstantBuffer(TEXT("FObjectConstantBuffer"), 0, EShaderStage::Vertex);
}

void FCollisionRenderPass::ReleaseCollisionBoxBuffer()
{
    if (CollisionBoxStructuredBuffer)
    {
        CollisionBoxStructuredBuffer->Release();
        CollisionBoxStructuredBuffer = nullptr;
    }

    if (CollisionBoxSRV)
    {
        CollisionBoxSRV->Release();
        CollisionBoxSRV = nullptr;
    }
}

void FCollisionRenderPass::ReleaseCollisionSphereBuffer()
{
    if (CollisionSphereStructuredBuffer)
    {
        CollisionSphereStructuredBuffer->Release();
        CollisionSphereStructuredBuffer = nullptr;
    }

    if (CollisionSphereSRV)
    {
        CollisionSphereSRV->Release();
        CollisionSphereSRV = nullptr;
    }
}

void FCollisionRenderPass::ReleaseCollisionCapsuleBuffer()
{
    if (CollisionCapsuleStructuredBuffer)
    {
        CollisionCapsuleStructuredBuffer->Release();
        CollisionCapsuleStructuredBuffer = nullptr;
    }

    if (CollisionCapsuleSRV)
    {
        CollisionCapsuleSRV->Release();
        CollisionCapsuleSRV = nullptr;
    }
}

