
#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16
#define MAX_AMBIENT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3
#define AMBIENT_LIGHT       4

#define PI 3.14159

struct FAmbientLightInfo
{
    float4 AmbientColor;
};

struct FDirectionalLightInfo
{
    float4 LightColor;

    float3 Direction;
    float Intensity;
};

struct FPointLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    int Type;
    float Intensity;
    float Attenuation;
    float Padding;
};

struct FSpotLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    float3 Direction;
    float Intensity;

    int Type;
    float InnerRad;
    float OuterRad;
    float Attenuation;
};

cbuffer Lighting : register(b0)
{
    FAmbientLightInfo Ambient[MAX_AMBIENT_LIGHT];
    FDirectionalLightInfo Directional[MAX_DIRECTIONAL_LIGHT];
    FPointLightInfo PointLights[MAX_POINT_LIGHT];
    FSpotLightInfo SpotLights[MAX_SPOT_LIGHT];
    
    int DirectionalLightsCount;
    int PointLightsCount;
    int SpotLightsCount;
    int AmbientLightsCount;
};

Texture2D ShadowMap : register(t10);
TextureCube<float> ShadowCube : register(t11);
Texture2D<float> ShadowTexture : register(t12);
SamplerComparisonState ShadowSampler : register(s10);

// Point Light 의 6개 face 뷰·프로젝션 행렬과 바이어스
cbuffer SpotShadowData : register(b6)
{
    row_major matrix SpotLightViewProj;
    float ShadowBias; // 깊이 비교 시 바이어스
    float3 _padding; // 16바이트 정렬용
};

cbuffer PointShadowData : register(b7)
{
    row_major matrix PointLightViewProj[6];
};

cbuffer DirectionalShadowData : register(b5)
{
    row_major matrix LightView;
    row_major matrix LightProj;
    float worldTexelScale;
};


float CalculateAttenuation(float Distance, float AttenuationFactor, float Radius)
{
    if (Distance > Radius)
    {
        return 0.0;
    }

    float Falloff = 1.0 / (1.0 + AttenuationFactor * Distance * Distance);
    float SmoothFactor = (1.0 - (Distance / Radius)); // 부드러운 falloff

    return Falloff * SmoothFactor;
}

float CalculateSpotEffect(float3 LightDir, float3 SpotDir, float InnerRadius, float OuterRadius, float SpotFalloff)
{
    float Dot = dot(-LightDir, SpotDir); // [-1,1]
    
    float SpotEffect = smoothstep(cos(OuterRadius / 2), cos(InnerRadius / 2), Dot);
    
    return SpotEffect * pow(max(Dot, 0.0), SpotFalloff);
}

float CalculateDiffuse(float3 WorldNormal, float3 LightDir)
{
    return max(dot(WorldNormal, LightDir), 0.0);
}

float CalculateSpecular(float3 WorldNormal, float3 ToLightDir, float3 ViewDir, float Shininess, float SpecularStrength = 0.5)
{
#ifdef LIGHTING_MODEL_GOURAUD
    float3 ReflectDir = reflect(-ToLightDir, WorldNormal);
    float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess);
#else
    float3 HalfDir = normalize(ToLightDir + ViewDir); // Blinn-Phong
    float Spec = pow(max(dot(WorldNormal, HalfDir), 0.0), Shininess);
#endif
    return Spec * SpecularStrength;
}
float SampleSpotShadow(float3 worldPos, float3 spotLightPos, float3 spotLightDir, float spotOuterAngle)
{
    //// 1) Normal과 Light 방향 각도 기반 Bias 계산
    //float NdotL = dot(normalize(worldNormal), spotLightDir);
    //float slopeScale = 0.01f; // 기울기에 민감도 감소 (PSM 특성 반영)
    //float minBias = 0.005f; // 최소 Bias 감소
    //float maxBias = 0.05f; // 최대 Bias 조정
    
    //// 각도가 클수록(표면이 수직일수록) Bias 증가
    //float angleFactor = pow(1.0f - saturate(NdotL), 2.0f);
    //float bias = lerp(minBias, maxBias, angleFactor) * slopeScale;

    //// 2) 라이트 클립 공간 변환
    //float4 lightSpacePos = mul(float4(worldPos, 1.0f), SpotLightViewProj);
    //float2 uv = lightSpacePos.xy / lightSpacePos.w * 0.5f + 0.5f;
    //uv.y = 1.0f - uv.y;

    //// 3) Perspective-aware Bias: 클립 공간 W 성분으로 스케일링
    //float depthBias = bias * lightSpacePos.w; // 핵심 수정 부분
    //float depth = (lightSpacePos.z - depthBias) / lightSpacePos.w;

    //if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
    //    return 1.0f;

    //// 4) PCF 샘플링 (기존과 동일)
    float4 lightSpacePos = mul(float4(worldPos, 1.0f), SpotLightViewProj);
    float2 uv = lightSpacePos.xy / lightSpacePos.w * 0.5f + 0.5f;
    uv.y = 1 - uv.y;
    float depth = lightSpacePos.z / lightSpacePos.w - ShadowBias;
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
        return 1.f;
    
    float shadow = 0.0f;
    uint width, height;
    ShadowMap.GetDimensions(width, height);
    float2 texelSize = float2(1.0 / width, 1.0 / height);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            shadow += ShadowMap.SampleCmpLevelZero(
                ShadowSampler,
                uv.xy + offset,
                depth
            );
        }
    }
    shadow /= 9.0;
    return shadow;
}

// ——— 그림자 샘플링 헬퍼 ———
float SamplePointShadow(float3 ToLight, float3 worldPos)
{
    int faceIdx = 0;
    float3 absToL = abs(ToLight);
    if (absToL.x > absToL.y && absToL.x > absToL.z)
        faceIdx = (ToLight.x > 0) ? 1 : 0;
    else if (absToL.y > absToL.z)
        faceIdx = (ToLight.y > 0) ? 3 : 2;
    else
        faceIdx = (ToLight.z > 0) ? 5 : 4;
    
    // 뷰·프로젝션 변환
    float4 proj = mul(float4(worldPos, 1), PointLightViewProj[faceIdx]);
    proj.xyz /= proj.w;
    
    float depth = proj.z - ShadowBias;
    float shadow = 0.0f;
    
    // Begin Sphere PCF Sampling
    float3 sampleDirections[20];
    
    for (int i = 0; i < 20; i++)
    {
        float offset = 0.002 * (i / 20.0);
        
        float theta = i * (PI * 2) / 20;
        sampleDirections[i] = normalize(ToLight) +
                             float3(cos(theta) * offset,
                                    sin(theta) * offset,
                                    (i % 5) * 0.001 - 0.002);
    }
    
    for (int i = 0; i < 20; i++)
    {
        shadow += ShadowCube.SampleCmpLevelZero(
            ShadowSampler,
            normalize(-1.0f * sampleDirections[i]),
            depth
        );
    }
    
    return shadow / 20.0;
    
    // End Sphere PCF Sampling
    
    //// Begin Disk PCF Sampling
    //float diskRadius = 0.005;
    
    //float3 lightDir = normalize(ToLight);
    //float3 tangent, bitangent;
    
    //if (abs(lightDir.x) < 0.99)
    //    tangent = normalize(cross(float3(1, 0, 0), lightDir));
    //else
    //    tangent = normalize(cross(float3(0, 1, 0), lightDir));
        
    //bitangent = normalize(cross(lightDir, tangent));
    
    //for (int i = 0; i < 12; i++)
    //{
    //    float angle = i * (PI * 2) / 12;
    //    float r = diskRadius * (i % 3) / 3.0; 
        
    //    float3 offset = tangent * (r * cos(angle)) +
    //                   bitangent * (r * sin(angle));
                        
    //    float3 sampleDir = normalize(lightDir + offset);
        
    //    float4 proj = mul(float4(worldPos, 1), PointLightViewProj[faceIdx]);
    //    proj.xyz /= proj.w;
    //    float depth = proj.z - ShadowBias;
        
    //    shadow += ShadowCube.SampleCmpLevelZero(
    //        ShadowSampler,
    //        -sampleDir,
    //        depth
    //    );
    //}
    
    //return shadow / 12.0;
    //// End Disk PCF Sampling
}

float4 PointLight(int Index, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FPointLightInfo LightInfo = PointLights[Index];
    
    float3 ToLight = LightInfo.Position - WorldPosition;
    float Distance = length(ToLight);
    
    float Attenuation = CalculateAttenuation(Distance, LightInfo.Attenuation, LightInfo.Radius);
    if (Attenuation <= 0.0)
    {
        return float4(0.f, 0.f, 0.f, 0.f);
    }
    
    float3 LightDir = normalize(ToLight);
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);
#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = (DiffuseFactor * DiffuseColor) * LightInfo.LightColor.rgb;
#else
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif

    float shadow = SamplePointShadow(ToLight, WorldPosition);
    
    //return float4(shadow, 0, 0, 1);
    
    return float4(Lit * Attenuation * LightInfo.Intensity * shadow, 1.0);
}

float4 SpotLight(int Index, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FSpotLightInfo LightInfo = SpotLights[Index];
    
    float3 ToLight = LightInfo.Position - WorldPosition;
    float Distance = length(ToLight);
    
    float Attenuation = CalculateAttenuation(Distance, LightInfo.Attenuation, LightInfo.Radius);
    if (Attenuation <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    
    float3 LightDir = normalize(ToLight);
    float SpotlightFactor = CalculateSpotEffect(LightDir, normalize(LightInfo.Direction), LightInfo.InnerRad, LightInfo.OuterRad, LightInfo.Attenuation);
    if (SpotlightFactor <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);

#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = DiffuseFactor * DiffuseColor * LightInfo.LightColor.rgb;
#else
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif
    
    //return float4(Lit * Attenuation * SpotlightFactor * LightInfo.Intensity, 1.0);
    float shadow = SampleSpotShadow(WorldPosition, LightInfo.Position, LightInfo.Direction, LightInfo.OuterRad);
    //float shadow = SampleSpotShadow(WorldNormal, WorldPosition, LightInfo.Position, LightInfo.Direction, LightInfo.OuterRad);
    
    return float4(Lit * Attenuation * SpotlightFactor * LightInfo.Intensity * shadow, 1.0);
    //return float4(shadow, shadow, shadow, 1.0);
}

float4 DirectionalLight(int nIndex, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FDirectionalLightInfo LightInfo = Directional[nIndex];
    
    float3 LightDir = normalize(-LightInfo.Direction);
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);
    
#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = DiffuseFactor * DiffuseColor * LightInfo.LightColor.rgb;
#else
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif
    return float4(Lit * LightInfo.Intensity, 1.0);
}
bool InRange(float val, float min, float max)
{
    return (min <= val && val <= max);
}


float GetLightFromShadowMap(int idx, float3 WorldPos, float3 WorldNorm)
{
    float NdotL = dot(normalize(WorldNorm), Directional[idx].Direction);
    
    float slopeScale = 0.5f;
    float minBias = 0.0001f;
    float maxBias = 0.001f;
    float bias = saturate(slopeScale * (1 - NdotL));
    bias = clamp(bias, minBias, maxBias);

    float angle = acos(NdotL);
    bias *= pow(angle / (0.5f * 3.14159265f), 2.0f);
    

    float4 lp = mul(float4(WorldPos, 1), LightView);
    float4 cp = mul(lp, LightProj);
    
    float2 uv = cp.xy / cp.w * 0.5f + 0.5f;
    float dist = cp.z / cp.w - bias;
    uv.y = 1 - uv.y;
    uv = saturate(uv);
    
    const int R = 1;
    const float kernel[3][3] =
    {
        { 1, 2, 1 },
        { 2, 4, 2 },
        { 1, 2, 1 }
    };
    float sum = 0, weightSum = 16;
    for (int i = -R; i <= R; ++i)
    {
        for (int j = -R; j <= R; ++j)
        {
            float2 off = uv + float2(i, j) * (1.0f / 2048);
            float s = InRange(off.x, 0, 1) && InRange(off.y, 0, 1)
                ? ShadowMap.SampleCmpLevelZero(ShadowSampler, off, dist).r
                : 1.0f;
            sum += s * kernel[i + R][j + R];
        }
    }
    float Light = 0;
    Light = sum / weightSum;
    return Light;

    
   // // Ambient 및 감마 보정
   // //const float ambient = 1.f;
   //// Light = pow(Light, 1/2.2f);
   // return Light;
}



float4 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    float4 FinalColor = float4(0.0, 0.0, 0.0, 0.0);
    
    // 다소 비효율적일 수도 있음.
    [unroll(MAX_POINT_LIGHT)]
    for (int i = 0; i < PointLightsCount; i++)
    {
        FinalColor += PointLight(i, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }
    [unroll(MAX_SPOT_LIGHT)]
    for (int j = 0; j < SpotLightsCount; j++)
    {
       FinalColor += SpotLight(j, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }
    
    [unroll(MAX_DIRECTIONAL_LIGHT)]
    for (int k = 0; k < DirectionalLightsCount; k++)
    {
        FinalColor += DirectionalLight(k, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }
    [unroll(MAX_AMBIENT_LIGHT)]
    for (int l = 0; l < AmbientLightsCount; l++)
    {
        FinalColor += float4(Ambient[l].AmbientColor.rgb, 0.0);
        FinalColor.a = 1.0;
    }
    
        [unroll(MAX_DIRECTIONAL_LIGHT)]
    for (int k = 0; k < DirectionalLightsCount; k++)
    {
        FinalColor *= GetLightFromShadowMap(k, WorldPosition, WorldNormal);
    }
    return FinalColor;
}
