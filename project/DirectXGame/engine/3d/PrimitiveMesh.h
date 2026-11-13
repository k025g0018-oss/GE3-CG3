#pragma once

#include "VertexData.h"

#include <vector>

///// ----- プリミティブ形状 ----- /////

/// <summary>
/// 板・三角錐・立方体など簡易的な形状の頂点をプログラム上で構築する
/// </summary>
namespace PrimitiveMesh {

	/// --- 四角形 ---

	// 2x2の板ポリゴン (三角形2枚、6頂点)
	std::vector<VertexData> CreateQuad();

	/// --- 三角錐 ---

	// 底面が正三角形の三角錐 (4面、12頂点) 高さ2、-1～1に収まる
	std::vector<VertexData> CreatePyramid();

	/// --- 立方体 ---

	// 2x2x2の立方体 (6面、36頂点)
	std::vector<VertexData> CreateCube();

};

