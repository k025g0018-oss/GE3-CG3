#include "PrimitiveMesh.h"

#include <cmath>
#include <numbers>

/// <summary>
/// 四角形
/// </summary>
/// <returns></returns>
std::vector<VertexData> PrimitiveMesh::CreateQuad() {

	std::vector<VertexData> vertices;

	// 1枚目の三角形
	vertices.push_back({.position = {1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f}}); // 左上
	vertices.push_back({.position = {-1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f}}); // 右上
	vertices.push_back({.position = {1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f}}); // 左下

	// 2枚目の三角形
	vertices.push_back({.position = {1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f}}); // 左下
	vertices.push_back({.position = {-1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f}}); // 右上
	vertices.push_back({.position = {-1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f}}); // 右下

	return vertices;
}

/// <summary>
/// 三角錐
/// 頂点(0,1,0)と、y=-1の正三角形の底面でできた4面の三角錐
/// 面ごとに法線を持たせ、外側から見て時計回りに並べる (裏面カリングで消えないように)
/// </summary>
/// <returns></returns>
std::vector<VertexData> PrimitiveMesh::CreatePyramid() {
	std::vector<VertexData> vertices;

	// 底面
	vertices.push_back({.position = {0.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {0.5f, 0.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});
	vertices.push_back({.position = {-0.8660f, -1.0000f, -0.5000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});
	vertices.push_back({.position = {0.8660f, -1.0000f, -0.5000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});
	// 側面1
	vertices.push_back({.position = {0.0000f, 1.0000f, 0.0000f, 1.0f}, .texcoord = {0.5f, 0.0f}, .normal = {0.8402f, 0.2425f, 0.4851f}});
	vertices.push_back({.position = {0.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.8402f, 0.2425f, 0.4851f}});
	vertices.push_back({.position = {0.8660f, -1.0000f, -0.5000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.8402f, 0.2425f, 0.4851f}});
	// 側面2
	vertices.push_back({.position = {0.0000f, 1.0000f, 0.0000f, 1.0f}, .texcoord = {0.5f, 0.0f}, .normal = {0.0000f, 0.2425f, -0.9701f}});
	vertices.push_back({.position = {0.8660f, -1.0000f, -0.5000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, 0.2425f, -0.9701f}});
	vertices.push_back({.position = {-0.8660f, -1.0000f, -0.5000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0000f, 0.2425f, -0.9701f}});
	// 側面3
	vertices.push_back({.position = {0.0000f, 1.0000f, 0.0000f, 1.0f}, .texcoord = {0.5f, 0.0f}, .normal = {-0.8402f, 0.2425f, 0.4851f}});
	vertices.push_back({.position = {-0.8660f, -1.0000f, -0.5000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {-0.8402f, 0.2425f, 0.4851f}});
	vertices.push_back({.position = {0.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {-0.8402f, 0.2425f, 0.4851f}});

	return vertices;
}

/// <summary>
/// 立方体
/// 6面 × 三角形2枚 = 36頂点
/// 面ごとに法線を持たせ、外側から見て時計回りに並べる (裏面カリングで消えないように)
/// </summary>
/// <returns></returns>
std::vector<VertexData> PrimitiveMesh::CreateCube() {
	std::vector<VertexData> vertices;

	// 前 (-Z)
	vertices.push_back({.position = {1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {0.0000f, 0.0000f, -1.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, 0.0000f, -1.0000f}});
	vertices.push_back({.position = {-1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0000f, 0.0000f, -1.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, 0.0000f, -1.0000f}});
	vertices.push_back({.position = {-1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0000f, 0.0000f, -1.0000f}});
	vertices.push_back({.position = {-1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0000f, 0.0000f, -1.0000f}});
	// 後ろ (+Z)
	vertices.push_back({.position = {-1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {0.0000f, 0.0000f, 1.0000f}});
	vertices.push_back({.position = {-1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, 0.0000f, 1.0000f}});
	vertices.push_back({.position = {1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0000f, 0.0000f, 1.0000f}});
	vertices.push_back({.position = {-1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, 0.0000f, 1.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0000f, 0.0000f, 1.0000f}});
	vertices.push_back({.position = {1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0000f, 0.0000f, 1.0000f}});
	// 左 (-X)
	vertices.push_back({.position = {-1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {-1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {-1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {-1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {-1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {-1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {-1.0000f, 0.0000f, 0.0000f}});
	// 右 (+X)
	vertices.push_back({.position = {1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {1.0000f, 0.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {1.0000f, 0.0000f, 0.0000f}});
	// 上 (+Y)
	vertices.push_back({.position = {1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {0.0000f, 1.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, 1.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0000f, 1.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, 1.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, 1.0000f, -1.0000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0000f, 1.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, 1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0000f, 1.0000f, 0.0000f}});
	// 下 (-Y)
	vertices.push_back({.position = {-1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});
	vertices.push_back({.position = {-1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, -1.0000f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});
	vertices.push_back({.position = {1.0000f, -1.0000f, 1.0000f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0000f, -1.0000f, 0.0000f}});

	return vertices;
}

/// <summary>
/// 円の上の点の頂点を作る (板と同じくZ=0の面に置き、法線は+Z)
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <returns></returns>
static VertexData MakeFlatVertex(float x, float y) {

	// 板(CreateQuad)と同じく、x=1でu=0、y=1でv=0になるように貼る
	return {
		.position = {x, y, 0.0f, 1.0f},
		.texcoord = {0.5f - x * 0.5f, 0.5f - y * 0.5f},
		.normal = {0.0f, 0.0f, 1.0f}
	};
}

/// <summary>
/// 円
/// 中心と円周上の2点で三角形を作り、ピザのように一周並べる
/// 角度が増える向き(反時計回り)に並べると、板と同じ+Z向きの面になる
/// </summary>
/// <param name="segmentCount">分割数 (多いほど丸くなる)</param>
/// <returns></returns>
std::vector<VertexData> PrimitiveMesh::CreateCircle(uint32_t segmentCount) {

	std::vector<VertexData> vertices;

	// 1つの三角形が受け持つ角度 (rad)
	const float kAngleStep = 2.0f * std::numbers::pi_v<float> / static_cast<float>(segmentCount);

	for (uint32_t index = 0; index < segmentCount; ++index) {
		// この三角形の両端の角度
		const float angle0 = kAngleStep * static_cast<float>(index);
		const float angle1 = kAngleStep * static_cast<float>(index + 1);

		// 中心 → 円周の点 → 次の円周の点
		vertices.push_back(MakeFlatVertex(0.0f, 0.0f));
		vertices.push_back(MakeFlatVertex(std::cos(angle0), std::sin(angle0)));
		vertices.push_back(MakeFlatVertex(std::cos(angle1), std::sin(angle1)));
	}

	return vertices;
}

/// <summary>
/// リング
/// 外側の円周と内側の円周の間を、四角形(三角形2枚)でつないで一周並べる
/// </summary>
/// <param name="innerRadius">内側の半径 (0.0～1.0) 1.0に近いほど細い輪になる</param>
/// <param name="segmentCount">分割数 (多いほど丸くなる)</param>
/// <returns></returns>
std::vector<VertexData> PrimitiveMesh::CreateRing(float innerRadius, uint32_t segmentCount) {

	std::vector<VertexData> vertices;

	// 1つの四角形が受け持つ角度 (rad)
	const float kAngleStep = 2.0f * std::numbers::pi_v<float> / static_cast<float>(segmentCount);

	for (uint32_t index = 0; index < segmentCount; ++index) {
		// この四角形の両端の角度
		const float angle0 = kAngleStep * static_cast<float>(index);
		const float angle1 = kAngleStep * static_cast<float>(index + 1);

		// 外側の2点と内側の2点
		const VertexData outer0 = MakeFlatVertex(std::cos(angle0), std::sin(angle0));
		const VertexData outer1 = MakeFlatVertex(std::cos(angle1), std::sin(angle1));
		const VertexData inner0 = MakeFlatVertex(std::cos(angle0) * innerRadius, std::sin(angle0) * innerRadius);
		const VertexData inner1 = MakeFlatVertex(std::cos(angle1) * innerRadius, std::sin(angle1) * innerRadius);

		// 1枚目 外側 → 次の外側 → 内側
		vertices.push_back(outer0);
		vertices.push_back(outer1);
		vertices.push_back(inner0);

		// 2枚目 内側 → 次の外側 → 次の内側
		vertices.push_back(inner0);
		vertices.push_back(outer1);
		vertices.push_back(inner1);
	}

	return vertices;
}
