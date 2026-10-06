struct VS_IN
{
    float4 pos : POSITION0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 skyCoord : TEXCOORD0;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
    float4x4 gWorld;
};

TextureCube SkyboxTexture : register(t0);
SamplerState gSampler : register(s0);

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;

    output.pos = mul(float4(input.pos.xyz, 1.0f), gWorldViewProj);
    output.pos.z = output.pos.w;
    output.skyCoord = input.pos.xyz;

    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return SkyboxTexture.Sample(gSampler, input.skyCoord);
}