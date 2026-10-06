#ifndef CASCADE_COUNT
#define CASCADE_COUNT 4
#endif

struct VS_IN
{
    float4 pos : POSITION0;
    float4 uv : TEXCOORD0;
    float4 normal : NORMAL0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 normalW : NORMAL;
    float4 posW : WORLDPOS;
    float4 viewPos : VIEWPOS;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gWorld;
    float4x4 gWorldView;
    float4x4 gInvTrWorldView;
};

cbuffer cbPerScene : register(b2)
{
    float4 lightPos;
    float4 lightColor;
    float4 ambientSpecularPowType;
    float4x4 gT;
    float4 cameraPos;
};

cbuffer cbCascade : register(b1)
{
    float4x4 gViewProj[CASCADE_COUNT + 1];
    float4 gDistances;
};

Texture2D gDiffuseTexture : register(t0);
Texture2DArray CascadeShadowMap : register(t1);
SamplerState gSampler : register(s0);
SamplerComparisonState DepthSampler : register(s1);

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;

    output.pos = mul(float4(input.pos.xyz, 1.0f), gWorldViewProj);
    output.uv = input.uv.xy;
    //output.normalW = mul(float4(input.normal.xyz, 0.0f), gInvTrWorldView);
    output.normalW = mul(float4(input.normal.xyz, 0.0f), gWorld);
    output.posW = mul(float4(input.pos.xyz, 1.0f), gWorld);
    output.viewPos = mul(float4(input.pos.xyz, 1.0f), gWorldView);

    return output;
}

float ShadowCalculation(float4 posWorldSpace, float4 posViewSpace, float dotN)
{
    float depthValue = abs(posViewSpace.z);

    int layer = -1;
    for (int i = 0; i < CASCADE_COUNT; ++i)
    {
        if (depthValue < gDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = CASCADE_COUNT;
    }

    float4 posLightSpace = mul(float4(posWorldSpace.xyz, 1.0), gViewProj[layer]);
    float3 projCoords = posLightSpace.xyz / posLightSpace.w;

    projCoords = (mul(float4(projCoords, 1.0f), gT)).xyz;
    float currentDepth = projCoords.z;

    if (currentDepth > 1.0f)
    {
        return 0.0f;
    }

    float bias = max(0.05f * (1.0f - dotN), 0.005f);
    const float biasModifier = 0.5f;
    if (layer == CASCADE_COUNT)
    {
        bias *= 1 / (1000.0 * biasModifier);
    }
    else
    {
        bias *= 1 / (gDistances[layer] * biasModifier);
    }

    // PCF
    float shadow = 0.0f;
    float2 texelSize = 1.0f / 1024.0f;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            shadow += CascadeShadowMap.SampleCmp(DepthSampler, float3(projCoords.xy + float2(x, y) * texelSize, layer), currentDepth - bias);
        }
    }
    shadow /= 9.0f;

    return shadow;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float4 texColor = gDiffuseTexture.Sample(gSampler, input.uv);

    float3 N = normalize(input.normalW.xyz);
    float3 L = normalize(lightPos.xyz);
    //float3 V = normalize(-input.viewPos.xyz);
    float3 V = normalize(cameraPos.xyz - input.posW.xyz);

    float shadow = ShadowCalculation(input.posW, input.viewPos, dot(N, L));

    float3 baseColor = texColor.rgb;

    // Ambient
    float3 ambient = ambientSpecularPowType.x * lightColor.xyz * baseColor;

    // Diffuse
    float NdotL = max(dot(N, L), 0.0f);
    float3 diffuse = NdotL * baseColor * lightColor.xyz;

    // Specular
    float3 R = reflect(-L, N);
    float RdotV = max(dot(R, V), 0.0f);
    float spec = pow(RdotV, ambientSpecularPowType.z);
    float3 specular = ambientSpecularPowType.y * spec * lightColor.xyz;

    float3 finalColor = ambient + (1.0f - shadow) * (diffuse + specular);
    
    if (ambientSpecularPowType.w > 0.5f)
    {
        float depthValue = abs(input.viewPos.z);
        int cascadeIndex = -1;
        for (int i = 0; i < CASCADE_COUNT; ++i)
        {
            if (depthValue < gDistances[i])
            {
                cascadeIndex = i;
                break;
            }
        }
        if (cascadeIndex == -1)
            cascadeIndex = CASCADE_COUNT;

        float3 cascadeColors[4] =
        {
            float3(1.0, 0.2, 0.2),
            float3(0.2, 1.0, 0.2),
            float3(0.2, 0.4, 1.0),
            float3(1.0, 1.0, 0.2),
        };

        finalColor = lerp(finalColor, cascadeColors[cascadeIndex], 0.5f);
    }
    
    return float4(finalColor, texColor.a);
}
