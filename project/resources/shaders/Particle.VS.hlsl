#define float32_t4 float4
#define float32_t4x4 float4x4
#define float32_t3 float3
#define float32_t3x3 float3x3

#include "Particle.hlsli"

// パーティクル1粒分の、GPUが描画に利用するデータ
struct ParticleForGPU {
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4 color;
};
// インスタンスの数だけパーティクルのデータが並んだ配列として受け取る
StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct VertexShaderInput {
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

// SV_InstanceIDで、今処理しているインスタンスの番号を受け取る
VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    // 自分の番号の行列を取り出して変換する
    output.position = mul(input.position, gParticle[instanceId].WVP);
    output.texcoord = input.texcoord;
     // ローカル法線をワールド座標系へ変換して正規化する
    output.normal = normalize(mul(input.normal, (float32_t3x3) gParticle[instanceId].World));
    // instanceIdはVertexShaderでしか使えないので、色をここで取り出してPixelShaderへ送る
    output.color = gParticle[instanceId].color;
    
    return output;
}