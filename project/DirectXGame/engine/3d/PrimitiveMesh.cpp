#include "PrimitiveMesh.h"

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
