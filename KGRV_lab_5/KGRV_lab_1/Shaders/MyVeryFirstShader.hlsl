#ifndef PHONG_LIGHTING

struct VS_IN
{
    float4 pos : POSITION0;
    float4 col : COLOR0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
    output.pos = mul(float4(input.pos.xyz, 1.0f), gWorldViewProj);
    output.col = input.col;
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return input.col;
}

#else // PHONG_LIGHTING

struct VS_IN
{
    float4 pos    : POSITION0;
    float4 col    : COLOR0;
    float4 normal : NORMAL0;
};

struct PS_IN
{
    float4 pos     : SV_POSITION;
    float4 col     : COLOR;
    float3 normalW : TEXCOORD1;
    float3 posW    : TEXCOORD2;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gWorld;
};

cbuffer cbLight : register(b1)
{
    float4 gLightDir;
    float4 gLightColor;
    float4 gCameraPos;
    float4 gMaterialParams;
    float4 gMaterialSpecular;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN)0;

    float4 worldPos = mul(float4(input.pos.xyz, 1.0f), gWorld);
    output.pos     = mul(float4(input.pos.xyz, 1.0f), gWorldViewProj);
    output.col     = input.col;
    output.normalW = normalize(mul(float4(input.normal.xyz, 0.0f), gWorld).xyz);
    output.posW    = worldPos.xyz;

    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 N = normalize(input.normalW);
    float3 L = normalize(gLightDir.xyz);
    float3 V = normalize(gCameraPos.xyz - input.posW);
    float3 R = reflect(-L, N);

    float3 baseColor    = input.col.rgb;
    float3 matAmbient   = gMaterialParams.xyz;
    float  shininess    = gMaterialParams.w;
    float3 matSpecular  = gMaterialSpecular.xyz;

    float3 ambient = matAmbient * gLightColor.xyz * baseColor;

    float NdotL = max(dot(N, L), 0.0f);
    float3 diffuse = NdotL * baseColor * gLightColor.xyz;

    float RdotV = max(dot(R, V), 0.0f);
    float spec  = pow(RdotV, shininess);
    float3 specular = spec * matSpecular * gLightColor.xyz;

    float3 finalColor = ambient + diffuse + specular;
    return float4(finalColor, input.col.a);
}

#endif // PHONG_LIGHTING
