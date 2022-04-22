#define NK_ROOTSIGNATURE ""\
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),"\
"DescriptorTable("\
  "CBV(b0, numDescriptors = 1, flags = DATA_VOLATILE),"\
  "SRV(t0, numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)"\
"),"\
"RootConstants(num32BitConstants = 1, b1),"\
"StaticSampler(s0, "\
  "filter = FILTER_MIN_MAG_MIP_LINEAR,"\
  "addressU = TEXTURE_ADDRESS_CLAMP,"\
  "addressV = TEXTURE_ADDRESS_CLAMP,"\
  "addressW = TEXTURE_ADDRESS_CLAMP,"\
  "comparisonFunc = COMPARISON_ALWAYS"\
")"

cbuffer buffer0 : register(b0)
{
  float4x4 ProjectionMatrix;
};
static uint texture_index : register(b1);

sampler sampler0 : register(s0);
Texture2D<float4> textures[] : register(t0);

struct VS_INPUT
{
  float2 pos : POSITION;
  float4 col : COLOR0;
  float2 uv  : TEXCOORD0;
};

struct PS_INPUT
{
  float4 pos : SV_POSITION;
  float4 col : COLOR0;
  float2 uv  : TEXCOORD0;
};

[RootSignature(NK_ROOTSIGNATURE)]
PS_INPUT vs(VS_INPUT input)
{
  PS_INPUT output;
  output.pos = mul(ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
  output.col = input.col;
  output.uv  = input.uv;
  return output;
}

[RootSignature(NK_ROOTSIGNATURE)]
float4 ps(PS_INPUT input) : SV_Target
{
  return input.col * textures[texture_index].Sample(sampler0, input.uv);
}
