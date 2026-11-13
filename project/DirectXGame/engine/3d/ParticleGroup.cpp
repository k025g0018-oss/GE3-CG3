#include "ParticleGroup.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>

/// <summary>
/// 初期化
/// </summary>
/// <param name="device"></param>
/// <param name="srvDescriptorHeap"></param>
/// <param name="srvIndex"></param>
/// <param name="settings"></param>
void ParticleGroup::Initialize(
	ID3D12Device* device,
	const DescriptorHeap& srvDescriptorHeap,
	uint32_t srvIndex,
	const ParticleGroupSettings& settings) {

	assert(settings.maxCount > 0);

	/// --- 設定 ---

	settings_ = settings;

	// 乱数生成器をランダムなシードで初期化する
	std::random_device seedGenerator;
	randomEngine_.seed(seedGenerator());

	/// --- 描画 ---

	// 形状と最大数を渡して、Instancing描画の準備をする
	instancingParticle_.Initialize(
		device,
		srvDescriptorHeap,
		srvIndex,
		settings_.vertices,
		settings_.maxCount);

	// ふわっと現れるようにする
	instancingParticle_.SetFadeInRatio(settings_.fadeInRatio);

	// 立体の形状は陰影をつける
	instancingParticle_.SetLightingEnabled(settings_.isLightingEnabled);

	// 設定のBlendModeで描画する
	blendMode_ = settings_.blendMode;

	// 不透明な立体は、ブレンドせずにそのまま描画する
	if (settings_.isSolid) {
		blendMode_ = BlendMode::kNone;
	}
	instancingParticle_.GetBlendMode() = blendMode_;

	/// --- 最初の粒 ---

	// 経過時間もランダムにして、最初から画面にちりばめた状態にする
	Randomize();
}

/// <summary>
/// 全ての粒を発生させ直す
/// </summary>
void ParticleGroup::Randomize() {

	std::vector<Particle>& particles = instancingParticle_.GetParticles();
	for (uint32_t index = 0; index < settings_.maxCount; ++index) {
		particles[index] = MakeNewParticle(true);

		// Emitで出す粒は、最初は全て消えている状態にする (寿命0 = 描画されない)
		if (settings_.isEmitOnDemand) {
			particles[index].lifeTime = 0.0f;
			particles[index].currentTime = 0.0f;
		}
	}
}

/// <summary>
/// 指定した位置から粒を発生させる
/// </summary>
/// <param name="worldPosition"></param>
/// <param name="count"></param>
void ParticleGroup::Emit(const Vector3& worldPosition, uint32_t count) {

	/// --- ワールド座標 → 粒の座標 ---

	// 粒の位置は親(InstancingParticleのTransform)から見た位置なので、親の逆行列で戻す
	// (親はカメラへ向けるためにY軸で180度回っているので、そのままだと左右が反転する)
	const Transform& parent = instancingParticle_.GetTransform();
	const Matrix4x4 parentMatrix = Matrix4x4::MakeAffineMatrix(parent.scale, parent.rotate, parent.translate);
	const Vector3 localPosition = Vector3::Transform(worldPosition, Matrix4x4::Inverse(parentMatrix));

	/// --- 空いている粒を使って発生させる ---

	std::vector<Particle>& particles = instancingParticle_.GetParticles();
	const uint32_t instanceCount = instancingParticle_.GetInstanceCount();
	uint32_t emitCount = 0;

	for (uint32_t index = 0; index < instanceCount && emitCount < count; ++index) {
		// 生きている粒は使わない
		if (particles[index].currentTime < particles[index].lifeTime) {
			continue;
		}

		// 新しい粒を作り、発生範囲(spawnMin～spawnMax)のずれを足した位置に置く
		particles[index] = MakeNewParticle(false);
		particles[index].transform.translate = particles[index].transform.translate + localPosition;

		++emitCount;
	}
}

/// <summary>
/// 更新
/// </summary>
/// <param name="viewMatrix"></param>
/// <param name="projectionMatrix"></param>
/// <param name="deltaTime"></param>
void ParticleGroup::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix, float deltaTime) {

	std::vector<Particle>& particles = instancingParticle_.GetParticles();
	const uint32_t count = instancingParticle_.GetInstanceCount();

	for (uint32_t index = 0; index < count; ++index) {
		Particle& particle = particles[index];

		/// --- 発生と消滅 ---

		// 寿命が尽きたら
		if (particle.lifeTime <= particle.currentTime) {
			// Emitで出す粒は、次に呼ばれるまで消えたままにする
			if (settings_.isEmitOnDemand) {
				continue;
			}

			// 新しい粒として再発生させる
			particle = MakeNewParticle(false);
		}

		// 経過時間を足す
		particle.currentTime += deltaTime;

		/// --- 移動 ---

		// 加速度(m/s^2) × 経過時間(s) で速度を変える (重力で下に引っ張る)
		particle.velocity = particle.velocity + settings_.acceleration * deltaTime;

		// 空気抵抗で少しずつ減速させる (経過時間に合わせた割合を掛ける)
		if (settings_.drag > 0.0f) {
			particle.velocity = particle.velocity * std::exp(-settings_.drag * deltaTime);
		}

		// 速度(m/s) × 経過時間(s) で位置を進める
		particle.transform.translate = particle.transform.translate + particle.velocity * deltaTime;

		/// --- 回転 ---

		// 回転の速さ(rad/s) × 経過時間(s) で回す
		particle.transform.rotate = particle.transform.rotate + particle.angularVelocity * deltaTime;

		/// --- 拡縮 ---

		// sinで 1.0 を中心に大きくなったり小さくなったりさせる
		const float pulse = 1.0f + settings_.scalePulseAmount * std::sin(particle.currentTime * particle.scalePulseSpeed);
		particle.transform.scale = particle.baseScale * pulse;

		// 寿命に合わせて小さくする (1.0 → 0.0)
		if (settings_.isShrinkOverLife) {
			const float lifeRatio = particle.currentTime / particle.lifeTime;
			const float remainRatio = (std::max)(0.0f, 1.0f - lifeRatio); // Windowsのmaxマクロと衝突しないように()で囲む
			particle.transform.scale = particle.transform.scale * remainRatio;
		}

		/// --- 色 ---

		// 色相を時間で進めて、色をだんだん変える (α値はInstancingParticleが寿命から計算する)
		if (settings_.isRandomColor && settings_.hueShiftSpeed != 0.0f) {
			particle.color = HueToColor(particle.hue + particle.currentTime * settings_.hueShiftSpeed);
		}
	}

	/// --- GPUへ転送 ---

	// 生きている粒をStructuredBufferへ書き込む
	instancingParticle_.TransferToGPU(viewMatrix, projectionMatrix);
}

/// <summary>
/// 描画
/// </summary>
/// <param name="commandList"></param>
/// <param name="textureHandle"></param>
void ParticleGroup::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) const {

	instancingParticle_.Draw(commandList, textureHandle);
}

/// <summary>
/// 終了
/// </summary>
void ParticleGroup::Finalize() {

	instancingParticle_.Finalize();
}

/// <summary>
/// 設定に合わせて、新しい粒を1つ発生させる
/// </summary>
/// <param name="isFirstSpawn"></param>
/// <returns></returns>
Particle ParticleGroup::MakeNewParticle(bool isFirstSpawn) {

	Particle particle{};

	/// --- 位置 ---

	// 発生範囲の中のランダムな位置 (m)
	particle.transform.translate = {
		RandomRange(settings_.spawnMin.x, settings_.spawnMax.x),
		RandomRange(settings_.spawnMin.y, settings_.spawnMax.y),
		RandomRange(settings_.spawnMin.z, settings_.spawnMax.z)
	};

	/// --- 速度 ---

	// 上向きなら真上(90度)から±spreadAngleの範囲、それ以外は画面に沿ったランダムな方向へ進む (m/s)
	const float kHalfPi = 0.5f * std::numbers::pi_v<float>;
	const float direction = settings_.isUpward
		? kHalfPi + RandomRange(-settings_.spreadAngle, settings_.spreadAngle)
		: RandomRange(0.0f, 2.0f * std::numbers::pi_v<float>);
	const float speed = RandomRange(settings_.speedMin, settings_.speedMax);
	particle.velocity = {
		std::cos(direction) * speed,
		std::sin(direction) * speed,
		0.0f
	};

	/// --- 大きさ ---

	// 縦横同じ倍率で、ランダムな大きさにする
	const float scale = RandomRange(settings_.scaleMin, settings_.scaleMax);
	particle.baseScale = {scale, scale, scale};
	particle.transform.scale = particle.baseScale;

	// 拡縮の速さをランダムに決める (rad/s)
	particle.scalePulseSpeed = RandomRange(settings_.scalePulseSpeedMin, settings_.scalePulseSpeedMax);

	/// --- 回転 ---

	const float kTwoPi = 2.0f * std::numbers::pi_v<float>;
	const float angularSpeed = RandomRange(settings_.angularSpeedMin, settings_.angularSpeedMax);

	if (settings_.isRotateAllAxes) {
		// 最初の向きを全ての軸でランダムにする
		particle.transform.rotate = {
			RandomRange(0.0f, kTwoPi),
			RandomRange(0.0f, kTwoPi),
			RandomRange(0.0f, kTwoPi)
		};

		// ランダムな向きのベクトルを作り、XYZそれぞれの回転の速さに割り振る (rad/s)
		Vector3 axis = {
			RandomRange(-1.0f, 1.0f),
			RandomRange(-1.0f, 1.0f),
			RandomRange(-1.0f, 1.0f)
		};
		const float axisLength = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
		if (axisLength > 0.0001f) {
			axis = axis * (1.0f / axisLength);
		} else {
			axis = {0.0f, 1.0f, 0.0f};
		}
		particle.angularVelocity = axis * angularSpeed;
	} else {
		// 回転するときは、最初の向きもランダムにする
		if (angularSpeed != 0.0f) {
			particle.transform.rotate.z = RandomRange(0.0f, kTwoPi);
		}

		// 右回りか左回りかをランダムに決める (rad/s)
		const float rotateDirection = (RandomRange(0.0f, 1.0f) < 0.5f) ? -1.0f : 1.0f;
		particle.angularVelocity = {0.0f, 0.0f, angularSpeed * rotateDirection};
	}

	/// --- 色 ---

	if (settings_.isRandomColor && settings_.hueShiftSpeed != 0.0f) {
		// 色をだんだん変えるときは、最初の色相だけランダムに決める
		particle.hue = RandomRange(0.0f, 1.0f);
		particle.color = HueToColor(particle.hue);
	} else if (settings_.isRandomColor) {
		// 暗くなりすぎないように、0.3～1.0の範囲でRGBをランダムに決める
		particle.color = {
			RandomRange(0.3f, 1.0f),
			RandomRange(0.3f, 1.0f),
			RandomRange(0.3f, 1.0f),
			1.0f
		};
	} else {
		// 設定の色を使う (白ならテクスチャの色そのまま)
		particle.color = settings_.tintColor;
	}

	/// --- 寿命 ---

	if (settings_.isImmortal) {
		// 寿命をなくし、消えずにずっと残るようにする (Windowsのmaxマクロと衝突しないように()で囲む)
		particle.lifeTime = (std::numeric_limits<float>::max)();
		particle.currentTime = 0.0f;
	} else {
		particle.lifeTime = RandomRange(settings_.lifeTimeMin, settings_.lifeTimeMax);

		// 最初の発生だけ経過時間もランダムにして、消えるタイミングをばらばらにする
		particle.currentTime = isFirstSpawn ? RandomRange(0.0f, particle.lifeTime) : 0.0f;
	}

	return particle;
}

/// <summary>
/// min～maxの間でランダムな値を返す
/// </summary>
/// <param name="min"></param>
/// <param name="max"></param>
/// <returns></returns>
float ParticleGroup::RandomRange(float min, float max) {

	// 範囲がない場合は、そのままの値を返す
	if (max <= min) {
		return min;
	}

	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine_);
}

/// <summary>
/// 色相(0.0～1.0)から、鮮やかな色を作る
/// </summary>
/// <param name="hue"></param>
/// <returns></returns>
Vector4 ParticleGroup::HueToColor(float hue) {

	// 1.0を超えても1周して戻るように、小数部分だけ使う (例: 1.25 → 0.25)
	const float wrappedHue = hue - std::floor(hue);

	// 色相環を6つに分けて考える (赤→黄→緑→水色→青→紫→赤)
	const float h6 = wrappedHue * 6.0f;

	// RGBそれぞれ、色相からの距離で明るさを決める (HSVのS=1、V=1)
	// 例: hue=0.0 → 赤(1,0,0)、hue=1/6 → 黄(1,1,0)、hue=1/3 → 緑(0,1,0)
	const float r = std::clamp(std::abs(h6 - 3.0f) - 1.0f, 0.0f, 1.0f);
	const float g = std::clamp(2.0f - std::abs(h6 - 2.0f), 0.0f, 1.0f);
	const float b = std::clamp(2.0f - std::abs(h6 - 4.0f), 0.0f, 1.0f);

	// 原色が強すぎないように、少しだけ白を混ぜて柔らかい色にする
	const float kWhiteMix = 0.15f;
	return {
		r + (1.0f - r) * kWhiteMix,
		g + (1.0f - g) * kWhiteMix,
		b + (1.0f - b) * kWhiteMix,
		1.0f
	};
}
