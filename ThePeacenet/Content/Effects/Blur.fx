// Effect used by the BackgroundBlur GUI control.
// Based on https://github.com/CartBlanche/MonoGame-Samples/blob/master/SamplesContentBuilder/SamplesContentBuilderContent/Effects/GaussianBlur.fx.

#if OPENGL
	#define SV_POSITION POSITION
	#define VS_SHADERMODEL vs_3_0
	#define PS_SHADERMODEL ps_3_0
#else
	#define VS_SHADERMODEL vs_4_0_level_9_1
	#define PS_SHADERMODEL ps_4_0_level_9_1
#endif

#define SAMPLE_COUNT 15

Texture2D SpriteTexture; // HLSL syntax error: Unexpected NEW_IDENTIFIER
float2 SampleOffsets[SAMPLE_COUNT];

float SampleWeights[SAMPLE_COUNT];

sampler2D SpriteTextureSampler = sampler_state
{
	Texture = <SpriteTexture>;
};

struct VertexShaderOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
	float2 TextureCoordinates : TEXCOORD0;
};

float4 MainPS(VertexShaderOutput input) : COLOR
{
	float4 c = 0;

	// Combine a number of weighted image filter taps.
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		c += tex2D(SpriteTextureSampler, input.TextureCoordinates + SampleOffsets[i]) * SampleWeights[i];
	}

	return c;
}

technique SpriteDrawing
{
	pass P0
	{
		PixelShader = compile PS_SHADERMODEL MainPS();
	}
};