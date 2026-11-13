#include "MouseParticleEffect.h"
#include "PipelineState.h"
#include "PrimitiveMesh.h"
#include "TextureManager.h"

#include <cmath>
#include <numbers>

/// <summary>
/// 初期化
/// </summary>
/// <param name="device"></param>
/// <param name="srvDescriptorHeap"></param>
/// <param name="srvStartIndex"></param>
/// <param name="textureManager"></param>
void MouseParticleEffect::Initialize(
	ID3D12Device* device,
	const DescriptorHeap& srvDescriptorHeap,
	uint32_t srvStartIndex,
	const TextureManager& textureManager) {

	// 形の色をそのまま見せるため、白テクスチャを使う
	const uint32_t whiteTextureIndex = textureManager.FindTextureIndex("white.png");

	/// --- 飛び散る立方体 ---

	// AL2のヒットエフェクト (ランダムな大きさ・ばらけた位置・重力・摩擦) をもとに、
	// 上向きに打ち上げて落ちていく花火のような動きにする
	ParticleGroupSettings burst;
	burst.name = "Burst Cube";
	burst.vertices = PrimitiveMesh::CreateCube();
	burst.maxCount = 300;
	burst.textureIndex = whiteTextureIndex;

	// クリックした位置から少しばらけさせる (AL2: 30px四方 → 約0.2m四方)
	burst.spawnMin = {-0.1f, -0.1f, -0.1f};
	burst.spawnMax = {0.1f, 0.1f, 0.1f};

	// 大きさはランダム (立方体は-1～1なので、半分の大きさが0.05～0.14m)
	burst.scaleMin = 0.05f;
	burst.scaleMax = 0.14f;

	// 真上から±60度くらいの範囲へ、勢いよく打ち上げる (m/s)
	burst.isUpward = true;
	burst.spreadAngle = std::numbers::pi_v<float> / 3.0f;
	burst.speedMin = 3.0f;
	burst.speedMax = 5.0f;

	// 重力で下に引っ張り、空気抵抗で少しずつ減速させる
	burst.acceleration = {0.0f, -7.0f, 0.0f};
	burst.drag = 0.8f;

	// 全ての軸でくるくる回る (rad/s)
	burst.isRotateAllAxes = true;
	burst.angularSpeedMin = 1.0f * std::numbers::pi_v<float>;
	burst.angularSpeedMax = 4.0f * std::numbers::pi_v<float>;

	// 寿命 (s) 落ちながら小さく・薄くなって消える
	burst.lifeTimeMin = 1.5f;
	burst.lifeTimeMax = 2.5f;
	burst.isShrinkOverLife = true;

	// 最初の色はランダムで、1秒で色相環の0.6周分ずつ色が変わっていく
	burst.isRandomColor = true;
	burst.hueShiftSpeed = 0.6f;

	// 陰影をつけて立体に見せ、α値で薄くなるように通常のブレンドで描画する
	burst.isLightingEnabled = true;
	burst.blendMode = BlendMode::kNormal;

	// クリックしたときだけ出す
	burst.isEmitOnDemand = true;

	burstGroup_.Initialize(device, srvDescriptorHeap, srvStartIndex, burst);

	/// --- カーソルから出る円 ---

	// AL2のマウスパーティクル (カーソルの周りに出て、ゆっくり漂いながら小さく薄くなる) をもとにする
	ParticleGroupSettings trail;
	trail.name = "Mouse Trail Circle";
	trail.vertices = PrimitiveMesh::CreateCircle();
	trail.maxCount = 100;
	trail.textureIndex = whiteTextureIndex;

	// カーソルの周りに少しばらけさせる (AL2: ±20px → 約±0.13m)
	trail.spawnMin = {-0.13f, -0.13f, 0.0f};
	trail.spawnMax = {0.13f, 0.13f, 0.0f};

	// 大きさは同じ (AL2: 半径10px → 約0.06m)
	trail.scaleMin = 0.06f;
	trail.scaleMax = 0.06f;

	// 画面に沿ったランダムな方向へ、ゆっくり漂う (m/s)
	trail.speedMin = 0.0f;
	trail.speedMax = 0.6f;

	// 空気抵抗で少しずつ止まる (AL2: 毎フレーム0.99倍 → 1秒で約0.55倍)
	trail.drag = 0.6f;

	// 寿命 (s) (AL2: 52～151フレーム)
	trail.lifeTimeMin = 0.9f;
	trail.lifeTimeMax = 2.5f;
	trail.isShrinkOverLife = true;

	// 明るい緑で、加算で光らせる (AL2: 0x77FF77)
	trail.tintColor = {0.47f, 1.0f, 0.47f, 1.0f};
	trail.blendMode = BlendMode::kAdd;

	// カーソルが画面上にあるときだけ出す
	trail.isEmitOnDemand = true;

	trailGroup_.Initialize(device, srvDescriptorHeap, srvStartIndex + 1, trail);

	/// --- カーソルの輪 ---

	// 中が空いた輪を1つだけ描画する (AL2: 半径15pxの緑のワイヤーフレームの円 → 約0.1m)
	cursor_.Initialize(
		device,
		srvDescriptorHeap,
		srvStartIndex + 2,
		PrimitiveMesh::CreateRing(0.8f),
		1);
	cursor_.GetTransform().scale = {0.1f, 0.1f, 1.0f};
	cursor_.GetColor() = {0.0f, 1.0f, 0.0f, 1.0f};
	cursor_.GetBlendMode() = BlendMode::kNormal;
	cursorTextureIndex_ = whiteTextureIndex;
}

/// <summary>
/// 更新
/// </summary>
/// <param name="viewMatrix"></param>
/// <param name="projectionMatrix"></param>
/// <param name="deltaTime"></param>
/// <param name="mouseWorldPosition"></param>
/// <param name="isMouseOnScreen"></param>
/// <param name="isMouseClicked"></param>
void MouseParticleEffect::Update(
	const Matrix4x4& viewMatrix,
	const Matrix4x4& projectionMatrix,
	float deltaTime,
	const Vector3& mouseWorldPosition,
	bool isMouseOnScreen,
	bool isMouseClicked) {

	isMouseOnScreen_ = isMouseOnScreen;

	/// --- 発生 ---

	if (isMouseOnScreen_) {
		// クリックした場所から、立方体をまとめて出す
		if (isMouseClicked) {
			burstGroup_.Emit(mouseWorldPosition, burstCount_);
		}

		// 一定の間隔で、カーソルの周りから円を1つ出す
		trailEmitTimer_ += deltaTime;
		if (trailEmitInterval_ > 0.0f && trailEmitTimer_ >= trailEmitInterval_) {
			trailGroup_.Emit(mouseWorldPosition, 1);
			trailEmitTimer_ = 0.0f;
		}
	} else {
		// 画面の外にいる間は数えない
		trailEmitTimer_ = 0.0f;
	}

	/// --- 移動 ---

	// 既に出ている粒は、カーソルが画面の外にあっても動かし続ける
	burstGroup_.Update(viewMatrix, projectionMatrix, deltaTime);
	trailGroup_.Update(viewMatrix, projectionMatrix, deltaTime);

	/// --- カーソルの輪 ---

	// 輪の親をカーソルの位置に動かし、WVPを更新する
	cursor_.GetTransform().translate = mouseWorldPosition;
	cursor_.TransferToGPU(viewMatrix, projectionMatrix);
}

/// <summary>
/// 描画
/// </summary>
/// <param name="commandList"></param>
/// <param name="textureManager"></param>
/// <param name="particlePipelineState"></param>
void MouseParticleEffect::Draw(
	ID3D12GraphicsCommandList* commandList,
	const TextureManager& textureManager,
	const PipelineState& particlePipelineState) const {

	/// --- 飛び散る立方体 ---

	commandList->SetPipelineState(particlePipelineState.GetPipelineState(burstGroup_.GetBlendMode()));
	burstGroup_.Draw(commandList, textureManager.GetSrvHandle(burstGroup_.GetTextureIndex()));

	/// --- カーソルから出る円 ---

	commandList->SetPipelineState(particlePipelineState.GetPipelineState(trailGroup_.GetBlendMode()));
	trailGroup_.Draw(commandList, textureManager.GetSrvHandle(trailGroup_.GetTextureIndex()));

	/// --- カーソルの輪 ---

	// カーソルが画面の上にあるときだけ、一番最後に描画する
	if (isMouseOnScreen_ && isCursorVisible_) {
		commandList->SetPipelineState(particlePipelineState.GetPipelineState(BlendMode::kNormal));
		cursor_.Draw(commandList, textureManager.GetSrvHandle(cursorTextureIndex_));
	}
}

/// <summary>
/// 終了処理
/// </summary>
void MouseParticleEffect::Finalize() {

	burstGroup_.Finalize();
	trailGroup_.Finalize();
	cursor_.Finalize();
}

/// <summary>
/// 画面上の位置を、Z=planeZの平面上のワールド座標に変換する
/// </summary>
/// <param name="screenRatio"></param>
/// <param name="viewMatrix"></param>
/// <param name="projectionMatrix"></param>
/// <param name="planeZ"></param>
/// <returns></returns>
Vector3 MouseParticleEffect::ScreenToWorld(
	const Vector2& screenRatio,
	const Matrix4x4& viewMatrix,
	const Matrix4x4& projectionMatrix,
	float planeZ) {

	/// --- 画面 → NDC ---

	// 画面の左上(0,0)～右下(1,1) を、NDCの左上(-1,1)～右下(1,-1) に直す (Yは上下が逆)
	const float ndcX = screenRatio.x * 2.0f - 1.0f;
	const float ndcY = 1.0f - screenRatio.y * 2.0f;

	/// --- NDC → ワールド ---

	// View × Projection の逆行列で、NDCからワールド座標へ戻す
	const Matrix4x4 inverseViewProjection = Matrix4x4::Inverse(Matrix4x4::Multiply(viewMatrix, projectionMatrix));

	// カーソルの真下にある、手前(Near: z=0)と奥(Far: z=1)の2点
	const Vector3 nearPoint = Vector3::Transform({ndcX, ndcY, 0.0f}, inverseViewProjection);
	const Vector3 farPoint = Vector3::Transform({ndcX, ndcY, 1.0f}, inverseViewProjection);

	/// --- 平面との交点 ---

	// 手前から奥へ向かう線
	const Vector3 direction = farPoint - nearPoint;

	// 線が平面と平行なときは交わらないので、手前の点を返す
	if (std::abs(direction.z) < 0.0001f) {
		return nearPoint;
	}

	// 線の上で、Zが平面と同じになる割合 t を求める (near + direction × t)
	const float t = (planeZ - nearPoint.z) / direction.z;
	return nearPoint + direction * t;
}
