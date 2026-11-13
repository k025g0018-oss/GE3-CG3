#include "ParticleGroup.h"

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

	// 不透明な立体は、テクスチャのα値で抜けないようにする
	if (settings_.isSolid) {
		instancingParticle_.GetBlendMode() = BlendMode::kNone;
		blendMode_ = BlendMode::kNone;
	}

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

		// 寿命が尽きたら、新しい粒として再発生させる
		if (particle.lifeTime <= particle.currentTime) {
			particle = MakeNewParticle(false);
		}

		// 経過時間を足す
		particle.currentTime += deltaTime;

		/// --- 移動 ---

		// 速度(m/s) × 経過時間(s) で位置を進める
		particle.transform.translate = particle.transform.translate + particle.velocity * deltaTime;

		/// --- 回転 ---

		// 回転の速さ(rad/s) × 経過時間(s) で回す
		particle.transform.rotate = particle.transform.rotate + particle.angularVelocity * deltaTime;

		/// --- 拡縮 ---

		// sinで 1.0 を中心に大きくなったり小さくなったりさせる
		const float pulse = 1.0f + settings_.scalePulseAmount * std::sin(particle.currentTime * particle.scalePulseSpeed);
		particle.transform.scale = particle.baseScale * pulse;
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

	// 画面に沿ったランダムな方向へ、ランダムな速さで進む (m/s)
	const float direction = RandomRange(0.0f, 2.0f * std::numbers::pi_v<float>);
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

	if (settings_.isRandomColor) {
		// 暗くなりすぎないように、0.3～1.0の範囲でRGBをランダムに決める
		particle.color = {
			RandomRange(0.3f, 1.0f),
			RandomRange(0.3f, 1.0f),
			RandomRange(0.3f, 1.0f),
			1.0f
		};
	} else {
		// テクスチャの色をそのまま使う
		particle.color = {1.0f, 1.0f, 1.0f, 1.0f};
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
