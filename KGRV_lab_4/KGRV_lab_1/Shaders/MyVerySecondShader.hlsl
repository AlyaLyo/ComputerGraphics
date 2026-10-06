struct VS_IN
{
	float4 pos : POSITION0;
	float4 uv  : TEXCOORD0;
    float4 normal : NORMAL0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD;
    float3 normalW : TEXCOORD1;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gWorld;
};

Texture2D gDiffuseTexture : register(t0);
SamplerState gSampler : register(s0);

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;

    output.pos = mul(float4(input.pos.xyz, 1.0f), gWorldViewProj);
    output.uv = input.uv.xy;
    float3 normalW = mul(float4(input.normal.xyz, 0.0f), gWorld).xyz;
    output.normalW = normalize(normalW);

    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
	float4 texColor = gDiffuseTexture.Sample(gSampler, input.uv);
	return texColor;
}
