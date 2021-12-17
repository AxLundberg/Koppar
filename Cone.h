#pragma once
#include "IndexedTriangleList.h"
#include <DirectXMath.h>
#include "MaMath.h"

class Cone
{
public:
	template<class V>
	static IndexedTriangleList<V> MakeTesselated(int longDiv)
	{
		namespace dx = DirectX;
		assert(longDiv >= 3);

		const auto base = dx::XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f);
		const float longitudeAngle = 2.0f * PI / longDiv;

		// base vertices
		std::vector<V> vertices;
		for (int iLong = 0; iLong < longDiv; iLong++)
		{
			vertices.emplace_back();
			auto v = dx::XMVector3Transform(
				base,
				dx::XMMatrixRotationZ(longitudeAngle * iLong)
			);
			dx::XMStoreFloat3(&vertices.back().pos, v);
		}
		// the center
		vertices.emplace_back();
		vertices.back().pos = { 0.0f,0.0f,-1.0f };
		const auto iCenter = (UINT)(vertices.size() - 1);
		// the tip 
		vertices.emplace_back();
		vertices.back().pos = { 0.0f,0.0f,1.0f };
		const auto iTip = (UINT)(vertices.size() - 1);


		// base indices
		std::vector<int> indices;
		for (int iLong = 0; iLong < longDiv; iLong++)
		{
			indices.push_back(iCenter);
			indices.push_back((iLong + 1) % longDiv);
			indices.push_back(iLong);
		}

		// cone indices
		for (int iLong = 0; iLong < longDiv; iLong++)
		{
			indices.push_back(iLong);
			indices.push_back((iLong + 1) % longDiv);
			indices.push_back(iTip);
		}

		return { std::move(vertices),std::move(indices) };
	}
	template<class V>
	static IndexedTriangleList<V> MakeVector(float side)
	{
		namespace dx = DirectX;

		side = side/4.0f;

		std::vector<V> vertices(9);
		vertices[0].pos = { -side, 0.0f, -side };
		vertices[1].pos = { -side, 0.0f,  side };
		vertices[2].pos = {  side, 0.0f,  side };
		vertices[3].pos = {  side, 0.0f, -side };
		vertices[4].pos = { -side, 8*side, -side };
		vertices[5].pos = { -side, 8*side,  side };
		vertices[6].pos = {  side, 8*side,  side };
		vertices[7].pos = {  side, 8*side, -side };
		vertices[8].pos = {  0.0f, 12*side,  0.0f };

		/*for (auto& v : vertices)
		{
			dx::XMVECTOR vec = dx::XMVector4Transform(dx::XMVectorSet(v.pos.x, v.pos.y, v.pos.z, 1.0f),  dx::XMMatrixRotationX(PI/2));
			dx::XMFLOAT4 tmp = {};
			dx::XMStoreFloat4(&tmp, vec);
			v.pos.x = tmp.x;
			v.pos.y = tmp.y;
			v.pos.z = tmp.z;
		}*/

		return{
			std::move(vertices),{
				0,2,1, 0,3,2,
				0,1,4, 4,1,5,
				3,0,4, 3,4,7,
				2,3,7, 2,7,6,
				1,2,5, 5,2,6,
				5,6,8, 6,7,8,
				7,4,8, 4,5,8,
			}
		};
	}
	template<class V>
	static IndexedTriangleList<V> Make()
	{
		return MakeTesselated<V>(24);
	}
};