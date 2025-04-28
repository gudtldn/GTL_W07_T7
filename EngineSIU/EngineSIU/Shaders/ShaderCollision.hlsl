#include "ShaderRegisters.hlsl"

static const float PI = 3.14159265358979f;
static const float TWO_PI = 6.28318530718f;

static const uint BoxEdgeCount = 12; // Segments

static const uint SphereSegments = 32;
static const uint SphereEdgeCount = 96; // Segments * 3

static const uint CapsuleSegments = 16;
static const uint CapsuleEdgeCount = 68; // 4 + Segments * 4

struct FCollisionBox
{
    row_major matrix WorldMatrix;
    
    float3 Center;
    float Pad0;
    
    float3 Extent;
    float Pad1;

    float4 Color;
};

struct FCollisionSphere
{
    row_major matrix WorldMatrix;
    
    float3 Center;
    float Radius;

    float4 Color;
};

struct FCollisionCapsule
{
    row_major matrix WorldMatrix;
    
    float3 Center;
    float Radius;

    float HalfHeight;
    float3 Pad0;

    float4 Color;
};

cbuffer CollisionCountCB : register(b0)
{
    int BoxCount;
    int SphereCount;
    int CapsuleCount;
    int Pad0;
}

StructuredBuffer<FCollisionBox> CollisionBoxSB : register(t0);
StructuredBuffer<FCollisionSphere> CollisionSphereSB : register(t1);
StructuredBuffer<FCollisionCapsule> CollisionCapsuleSB : register(t2);


float3 ComputeCollisionBoxPosition(uint VertexId, uint InstanceId)
{
    uint BoxIndex  = InstanceId / BoxEdgeCount;
    uint EdgeIndex = InstanceId % BoxEdgeCount;
    FCollisionBox B = CollisionBoxSB[BoxIndex];

    // 8개 코너 계산 (Extent = half-size)
    float3 C = B.Center;
    float3 E = B.Extent;
    float3 Corners[8] = {
        C + float3(-E.x, -E.y, -E.z),
        C + float3(+E.x, -E.y, -E.z),
        C + float3(+E.x, +E.y, -E.z),
        C + float3(-E.x, +E.y, -E.z),
        C + float3(-E.x, -E.y, +E.z),
        C + float3(+E.x, -E.y, +E.z),
        C + float3(+E.x, +E.y, +E.z),
        C + float3(-E.x, +E.y, +E.z),
    };

    // 12개 엣지 (정점 인덱스 페어)
    static const int2 EdgePairs[BoxEdgeCount] = {
        int2(0,1), int2(1,2), int2(2,3), int2(3,0),
        int2(4,5), int2(5,6), int2(6,7), int2(7,4),
        int2(0,4), int2(1,5), int2(2,6), int2(3,7)
    };

    int2 P = EdgePairs[EdgeIndex];
    return (VertexId == 0) ? Corners[P.x] : Corners[P.y];
}

float3 ComputeCollisionSpherePosition(uint VertexId, uint InstanceId)
{
    uint SphereIndex  = InstanceId / SphereEdgeCount;
    uint LineIndex    = InstanceId % SphereEdgeCount;
    FCollisionSphere S = CollisionSphereSB[SphereIndex];

    float3 C = S.Center;
    float  R = S.Radius;

    // 평면 선택: 0=XY,1=XZ,2=YZ
    uint Plane  = LineIndex / SphereSegments;
    uint SegIdx = LineIndex % SphereSegments;

    float a0 = TWO_PI * SegIdx       / SphereSegments;
    float a1 = TWO_PI * (SegIdx + 1) / SphereSegments;

    float3 p0, p1;
    if (Plane == 0) {
        // XY 평면 (Z 고정)
        p0 = C + float3(cos(a0)*R, sin(a0)*R, 0);
        p1 = C + float3(cos(a1)*R, sin(a1)*R, 0);
    }
    else if (Plane == 1) {
        // XZ 평면 (Y 고정)
        p0 = C + float3(cos(a0)*R, 0, sin(a0)*R);
        p1 = C + float3(cos(a1)*R, 0, sin(a1)*R);
    }
    else {
        // YZ 평면 (X 고정)
        p0 = C + float3(0, cos(a0)*R, sin(a0)*R);
        p1 = C + float3(0, cos(a1)*R, sin(a1)*R);
    }

    return (VertexId == 0) ? p0 : p1;
}

float3 ComputeCollisionCapsulePosition(uint VertexId, uint InstanceId)
{
    uint CapIndex  = InstanceId / CapsuleEdgeCount;
    uint LineIndex = InstanceId % CapsuleEdgeCount;
    FCollisionCapsule C = CollisionCapsuleSB[CapIndex];

    float3 center     = C.Center;
    float  radius     = C.Radius;
    float  halfHeight = C.HalfHeight;

    // 블록 구분
    const uint cylCount  = 4;                   // 실린더 세로 엣지
    const uint circCount = 2 * CapsuleSegments; // 위/아래 원
    const uint arcCount  = CapsuleSegments;     // 반구 아크 per 평면

    float3 rawPos;

    // 1) 실린더 세로선
    if (LineIndex < cylCount)
    {
        static const float2 dirs[4] = {
            float2( 1, 0), float2(-1, 0),
            float2( 0, 1), float2( 0,-1)
        };
        float2 d = dirs[LineIndex] * radius;
        float3 top    = center + float3(d.x, +halfHeight, d.y);
        float3 bottom = center + float3(d.x, -halfHeight, d.y);
        rawPos = (VertexId == 0) ? top : bottom;
    }
    else
    {
        LineIndex -= cylCount;

        // 2) 위/아래 원
        if (LineIndex < circCount)
        {
            uint idx     = LineIndex;
            uint which   = idx / CapsuleSegments; 
            uint segIdx2 = idx % CapsuleSegments;
            float a0 = TWO_PI * segIdx2       / CapsuleSegments;
            float a1 = TWO_PI * (segIdx2 + 1) / CapsuleSegments;
            float y = (which == 0) ? +halfHeight : -halfHeight;

            float3 p0 = center + float3(cos(a0)*radius, y, sin(a0)*radius);
            float3 p1 = center + float3(cos(a1)*radius, y, sin(a1)*radius);
            rawPos = (VertexId == 0) ? p0 : p1;
        }
        else
        {
            LineIndex -= circCount;

            // 3) XY평면 반구
            if (LineIndex < arcCount)
            {
                uint idx       = LineIndex;
                bool isBottom  = (idx & 1) != 0;
                uint i         = idx >> 1;
                float p0 = PI * i       / (CapsuleSegments/2);
                float p1 = PI * (i+1)   / (CapsuleSegments/2);

                float y0 = (isBottom ? -halfHeight - sin(p0)*radius
                                     : +halfHeight + sin(p0)*radius);
                float y1 = (isBottom ? -halfHeight - sin(p1)*radius
                                     : +halfHeight + sin(p1)*radius);

                float3 v0 = center + float3(cos(p0)*radius, y0, 0);
                float3 v1 = center + float3(cos(p1)*radius, y1, 0);
                rawPos = (VertexId == 0) ? v0 : v1;
            }
            else
            {
                // 4) YZ평면 반구
                LineIndex -= arcCount;
                uint idx       = LineIndex;
                bool isBottom  = (idx & 1) != 0;
                uint i         = idx >> 1;
                float p0 = PI * i       / (CapsuleSegments/2);
                float p1 = PI * (i+1)   / (CapsuleSegments/2);

                float y0 = (isBottom ? -halfHeight - sin(p0)*radius
                                     : +halfHeight + sin(p0)*radius);
                float y1 = (isBottom ? -halfHeight - sin(p1)*radius
                                     : +halfHeight + sin(p1)*radius);

                float3 v0 = center + float3(0, y0, cos(p0)*radius);
                float3 v1 = center + float3(0, y1, cos(p1)*radius);
                rawPos = (VertexId == 0) ? v0 : v1;
            }
        }
    }
    
    // rawPos = (x, y, z) → (x, z, y)
    return float3(rawPos.x, rawPos.z, rawPos.y);
}

struct VS_INPUT
{
    uint VertexID : SV_VertexID; // 0 or 1 : line begin, end position.
    uint InstanceID : SV_InstanceID; // Instance ID - Box, Sphere, Capsule
};

struct PS_INPUT
{
    float4 Position : SV_Position;
    float4 WorldPosition : POSITION;
    float4 Color : COLOR;
    uint InstanceID : SV_InstanceID;
};

/** Vertex Shader */
PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    float3 pos;
    float4 color;
    row_major matrix WorldMat;

    uint BoxLineCount = BoxCount * BoxEdgeCount;
    uint SphereLineCount = SphereCount * SphereEdgeCount;
    
    if (input.InstanceID < BoxLineCount)
    {
        pos = ComputeCollisionBoxPosition(input.VertexID, input.InstanceID);

        uint BoxIndex = input.InstanceID / BoxEdgeCount;
        color = CollisionBoxSB[BoxIndex].Color;
        WorldMat = CollisionBoxSB[BoxIndex].WorldMatrix;
    }
    else if (input.InstanceID < BoxLineCount + SphereLineCount)
    {
        uint Rel = input.InstanceID - BoxLineCount;
        pos = ComputeCollisionSpherePosition(input.VertexID,Rel);

        uint SphereIndex = Rel / SphereEdgeCount;
        color = CollisionSphereSB[SphereIndex].Color;
        WorldMat = CollisionSphereSB[SphereIndex].WorldMatrix;
    }
    else
    {
        uint Rel = input.InstanceID - BoxLineCount - SphereLineCount;
        pos = ComputeCollisionCapsulePosition(input.VertexID,Rel);

        uint CapsuleIndex = Rel / CapsuleEdgeCount;
        color = CollisionCapsuleSB[CapsuleIndex].Color;
        WorldMat = CollisionCapsuleSB[CapsuleIndex].WorldMatrix;
    }

    /** Transform **/ 
    output.Position = float4(pos, 1.f);
    output.Position = mul(output.Position, WorldMat);
    output.WorldPosition = output.Position;
    
    output.Position = mul(output.Position, ViewMatrix);
    output.Position = mul(output.Position, ProjectionMatrix);
    
    output.Color = color;
    output.InstanceID = input.InstanceID;
    return output;
}

/** Pixel Shader */
float4 mainPS(PS_INPUT input) : SV_Target
{
    return input.Color;
}
