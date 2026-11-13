#include "Particle.hlsli"

#define float32_t4 float4
#define float32_t4x4 float4x4
#define float32_t3 float3
#define int32_t int

// C++側のMaterialと同じ並びにする
struct Material {
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    
    // 0: Lambert、1: Half Lambert
    int32_t lightingMode;
};

///// ----- ここに宣言しなさい ----- /////
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);

// Particleではライティングを行わないため、平行光源(gDirectionalLight)は宣言しない

struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float32_t4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    // テクスチャから色を取得する
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // Dissolve (αTest) テクスチャのα値が閾値以下のPixelを棄却する
    // 閾値を上げると抜ける部分が増え、下げると減る
    if (textureColor.a <= input.dissolveThreshold)
    {
        discard;
    }
    
    // かんたんのためにLightingは行わない
    // VertexShaderから送られてきたインスタンスごとの色を掛ける
    output.color = gMaterial.color * textureColor * input.color;
    
    // 最終的なα値が0のときだけPixelを棄却する
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}