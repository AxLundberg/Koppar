#pragma once
#include <vector>
#include <DirectXMath.h>

template<class V, class M>
class IndexedObjTriangleList
{
public:
	IndexedObjTriangleList() = default;
	IndexedObjTriangleList(
		std::vector<V> verts_in,
		std::vector<int> indices_in,
		std::vector<int> subsetStartIndex_in,
		std::vector<std::wstring> texFileArray_in,
		std::vector<M> materials_in)
		:
		vertices(std::move(verts_in)),
		indices(std::move(indices_in)),
		subsetStartIndex(std::move(subsetStartIndex_in)),
		textureNameArray(std::move(texFileArray_in)),
		materials(std::move(materials_in))
	{
		assert(vertices.size() > 2);
		assert(indices.size() % 3 == 0);
	}

	void Transform(DirectX::FXMMATRIX matrix)
	{
		for (auto& v : vertices)
		{
			const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&v.pos);
			DirectX::XMStoreFloat3(
				&v.pos,
				DirectX::XMVector3Transform(pos, matrix)
			);
		}
	}
	// asserts face-independent vertices w/ normals cleared to zero
	void SetNormalsIndependentFlat() noexcept
	{
		using namespace DirectX;
		assert(indices.size() % 3 == 0 && indices.size() > 0);
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			/*The primitive*/
			auto& v0 = vertices[indices[i]];
			auto& v1 = vertices[indices[i + 1]];
			auto& v2 = vertices[indices[i + 2]];
			/*Vertex Position*/
			const auto p0 = XMLoadFloat3(&v0.pos);
			const auto p1 = XMLoadFloat3(&v1.pos);
			const auto p2 = XMLoadFloat3(&v2.pos);
			/*Calculate normal*/
			const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));
			/*Store Normal*/
			XMStoreFloat3(&v0.norm, n);
			XMStoreFloat3(&v1.norm, n);
			XMStoreFloat3(&v2.norm, n);
		}
	}
public:
	std::vector<V> vertices;
	std::vector<int> indices;
	std::vector<int> subsetStartIndex;
	std::vector<std::wstring> textureNameArray;
	std::vector<M> materials;
};