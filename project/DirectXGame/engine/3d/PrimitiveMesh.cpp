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