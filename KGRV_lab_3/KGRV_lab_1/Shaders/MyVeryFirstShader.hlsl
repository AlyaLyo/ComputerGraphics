/*
cbuffer Constants : register(b0)
{
    float4x4 WorldViewProj;
};
*/

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
/*
PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
    output.pos = mul(WorldViewProj, input.pos);
	output.col = input.col;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 col = input.col;
#ifdef TEST
	if (input.pos.x > 400) col = TCOLOR;
#endif
	return col;
}
*/

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
	//output.pos = input.pos;
    output.pos = mul(float4(input.pos.xyz, 1.0f), gWorldViewProj);
    output.col = input.col;
	
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return input.col;;
}