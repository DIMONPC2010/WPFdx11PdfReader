Texture2D ObjTexture : register( t0 );
SamplerState ObjSamplerState : register( s0 );


cbuffer cbChangesEveryFrame : register( b0 )
{
	float4x4 WVP;
	float vInversion;
	int fBookmark;
};


struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

VS_OUTPUT VS(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD)
{
    VS_OUTPUT output;

    output.Pos = mul(inPos, WVP);
    output.TexCoord = inTexCoord;

    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 cBlack = {0,0,0,0};

	if(fBookmark == 1)
	{
		if((input.TexCoord.x < 0.15 || input.TexCoord.x > 0.2 || input.TexCoord.y > 0.1))
			return  abs(vInversion - ObjTexture.Sample(ObjSamplerState, input.TexCoord));
		else
			return abs(vInversion - cBlack);
	}
	else
		return  abs(vInversion - ObjTexture.Sample(ObjSamplerState, input.TexCoord));
}