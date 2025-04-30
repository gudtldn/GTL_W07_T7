// FadeEffect.hlsl

// 상수 버퍼 : b0 슬롯
cbuffer FadeEffectCB : register(b0)
{
    float fadeAlpha; // 0.0 = 투명, 1.0 = 완전 페이드
    float3 fadeColor; // 페이드 색상 (예: 검정 = 0,0,0)
};

struct VSOut
{
    float4 pos : SV_Position;
};

// 풀스크린 트라이앵글 정점 위치
static const float2 triVerts[3] =
{
    float2(-1, -1),
    float2(-1, 3),
    float2(3, -1)
};

VSOut mainVS(uint vertexID : SV_VertexID)
{
    VSOut o;
    o.pos = float4(triVerts[vertexID], 0, 1);
    return o;
}

float4 mainPS(VSOut IN) : SV_Target
{
    // fadeColor는 그대로, alpha만 블렌드 스테이트로 적용
    return float4(fadeColor, fadeAlpha);
}
