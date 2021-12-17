#pragma once
#include "IndexedTriangleList.h"
#include <DirectXMath.h>
#include <initializer_list>

class Cube
{
public:
	template<class V>
	static IndexedTriangleList<V> Make()
	{
		namespace dx = DirectX;

		constexpr float side = 1.0f / 2.0f;

		std::vector<V> vertices(8);
		vertices[0].pos = {-side, -side, -side };   // 0                 2 ____________3
		vertices[1].pos = { side, -side, -side };   // 1			     /|	          /|
		vertices[2].pos = {-side,  side, -side };   // 2			    / |		     / |
		vertices[3].pos = { side,  side, -side };   // 3			   /  |		    /  |
		vertices[4].pos = {-side, -side,  side };   // 4	  	   	  /  0|________/___|1
		vertices[5].pos = { side, -side,  side };   // 5	         /   /        /	  /
		vertices[6].pos = {-side,  side,  side };   // 6           6/___/_______ /7  /
		vertices[7].pos = { side,  side,  side };   // 7           |   /        |   /
												    //		       |  /         |  /
												    //	           | /          | /
												    //		      4|/___________|5
		return{
			std::move(vertices),{
				0,2,1, 2,3,1,
				1,3,5, 3,7,5,
				2,6,3, 3,6,7,
				4,5,7, 4,7,6,
				0,4,2, 2,4,6,
				0,1,4, 1,5,4
			}
		};
	}
	template<class V>
	static IndexedTriangleList<V> MakeSkinned()
	{
		namespace dx = DirectX;

		constexpr float side = 0.5f;

		std::vector<V> vertices(14);

		vertices[0].pos = { -side,-side,-side };
		vertices[1].pos = { side,-side,-side };
		vertices[2].pos = { -side,side,-side };
		vertices[3].pos = { side,side,-side };
		vertices[4].pos = { -side,-side,side };
		vertices[5].pos = { side,-side,side };
		vertices[6].pos = { -side,side,side };
		vertices[7].pos = { side,side,side };
		vertices[8].pos = { -side,-side,-side };
		vertices[9].pos = { side,-side,-side };
		vertices[10].pos = { -side,-side,-side };
		vertices[11].pos = { -side,-side,side };
		vertices[12].pos = { side,-side,-side };
		vertices[13].pos = { side,-side,side };

		auto x0 = 1.0f    / 3072.0f;
		auto x1 = 1025.0f / 3072.0f;
		auto x2 = 2047.0f / 3072.0f;
		auto x3 = 3072.0f / 3072.0f;
		auto y0 = 1.0f    / 4096.0f;
		auto y1 = 1025.0f / 4096.0f;
		auto y2 = 2047.0f / 4096.0f;
		auto y3 = 3072.0f / 4096.0f;
		auto y4 = 4096.0f / 4096.0f;
		vertices[0].texCoord = { x2, y0 };
		vertices[1].texCoord = { x1, y0 };
		vertices[2].texCoord = { x2, y1 };
		vertices[3].texCoord = { x1, y1 };
		vertices[4].texCoord = { x2, y3 };
		vertices[5].texCoord = { x1, y3 };
		vertices[6].texCoord = { x2, y2 };
		vertices[7].texCoord = { x1, y2 };
		vertices[8].texCoord = { x2, y4 };
		vertices[9].texCoord = { x1, y4 };
		vertices[10].texCoord = { x3, y1 };
		vertices[11].texCoord = { x3, y2 };
		vertices[12].texCoord = { x0, y1 };
		vertices[13].texCoord = { x0, y2 };

		//vertices[0].texCoord = { 0.666f, 0.0f };
		//vertices[1].texCoord = { 0.334f, 0.0f };
		//vertices[2].texCoord = { 0.666f, 0.251f };
		//vertices[3].texCoord = { 0.334f, 0.251f };
		//vertices[4].texCoord = { 0.666f, 0.75f };
		//vertices[5].texCoord = { 0.334f, 0.75f };
		//vertices[6].texCoord = { 0.666f, 0.499f };
		//vertices[7].texCoord = { 0.334f, 0.499f };
		//vertices[8].texCoord = { 0.666f, 1.0f };
		//vertices[9].texCoord = { 0.334f, 1.0f };
		//vertices[10].texCoord = { 1.0f, 0.251f };
		//vertices[11].texCoord = { 1.0f, 0.499f };
		//vertices[12].texCoord = { 0.0f, 0.251f };
		//vertices[13].texCoord = { 0.0f, 0.499f };
		return{
			std::move(vertices),{
				0,2,1,   2,3,1,
				4,8,5,   5,8,9,
				2,6,3,   3,6,7,
				4,5,7,   4,7,6,
				2,10,11, 2,11,6,
				12,3,7,  12,7,13
			}
		};
	}
	template<class V>
	static IndexedTriangleList<V> MakeSkinnedIndependent()
	{
		namespace dx = DirectX;

		constexpr float side = 0.5f;

		std::vector<V> vertices(24);

		vertices[0].pos = { -side, -side, -side };
		vertices[1].pos = { side, -side, -side };
		vertices[2].pos = { -side,  side, -side };
		vertices[3].pos = { side,  side, -side };
		vertices[4].pos = { -side, -side,  side };
		vertices[5].pos = { side, -side,  side };
		vertices[6].pos = { -side,  side,  side };
		vertices[7].pos = { side,  side,  side };
		vertices[8].pos = { -side, -side, -side };
		vertices[9].pos = { -side,  side, -side };
		vertices[10].pos = { -side, -side,  side };
		vertices[11].pos = { -side,  side,  side };
		vertices[12].pos = { side, -side, -side };
		vertices[13].pos = { side,  side, -side };
		vertices[14].pos = { side, -side,  side };
		vertices[15].pos = { side,  side,  side };
		vertices[16].pos = { -side, -side, -side };
		vertices[17].pos = { side, -side, -side };
		vertices[18].pos = { -side, -side,  side };
		vertices[19].pos = { side, -side,  side };
		vertices[20].pos = { -side,  side, -side };
		vertices[21].pos = { side,  side, -side };
		vertices[22].pos = { -side,  side,  side };
		vertices[23].pos = { side,  side,  side };

		auto hx = 1.0f / (4096.0f * 2.0f);
		auto x0 = 0.0f    / 4096.0f;
		auto x1 = 1025.0f / 4096.0f;
		auto x2 = 2047.0f / 4096.0f;
		auto x3 = 3072.0f / 4096.0f;
		auto x4 = 4096.0f / 4096.0f;

		auto hy = 1.0f / 3072.0f;
		auto y0 = 0.0f    / 3072.0f;
		auto y1 = 1025.0f / 3072.0f;
		auto y2 = 2046.0f / 3072.0f;
		auto y3 = 3072.0f / 3072.0f;
		//back
		vertices[0].texCoord  = {  1.0f, (2.0f/3.0f) - hy };
		vertices[1].texCoord  = {  1.0f, (1.0f/3.0f) + 2.5f*hy };
		vertices[2].texCoord  = { 0.75f, (2.0f/3.0f) - hy };
		vertices[3].texCoord  = { 0.75f, (1.0f/3.0f) + 2.5f*hy };
		//front
		vertices[4].texCoord  = { x1, y2 };
		vertices[6].texCoord  = { x2, y2 };
		vertices[7].texCoord  = { x2, y1 };
		vertices[5].texCoord  = { x1, y1 };
		//bot
		vertices[8].texCoord  = { x1, y3 };
		vertices[9].texCoord  = { x2, y3 };
		vertices[10].texCoord = { x1, y2 };
		vertices[11].texCoord = { x2, y2 };
		//top
		vertices[12].texCoord = { x1, y0 };
		vertices[13].texCoord = { x2, y0 };
		vertices[14].texCoord = { x1, 1024.0f / 3072.0f };
		vertices[15].texCoord = { x2, 1024.0f / 3072.0f };
		//left
		vertices[17].texCoord = { x0, y1 };
		vertices[19].texCoord = { x1, y1 };
		vertices[16].texCoord = { x0, y2 };
		vertices[18].texCoord = { x1, y2 };
		//right
		vertices[23].texCoord = { x2, y1 };
		vertices[21].texCoord = { x3, y1 };
		vertices[22].texCoord = { x2, y2 };
		vertices[20].texCoord = { x3, y2 };

		return{
			std::move(vertices),{
				   0,2,1,     2,3,1,
				   4,5,7,     4,7,6,
				  8,10,9,   10,11,9,
				12,13,15,  12,15,14,
				16,17,18,  18,17,19,
				20,23,21,  20,22,23
			}
		};
	}

	template<class V>
	static IndexedTriangleList<V> MakeIndependent()
	{
		namespace dx = DirectX;

		constexpr float side = 0.5f;

		std::vector<V> vertices(24);

		vertices[0].pos  = { -side, -side, -side };
		vertices[1].pos  = {  side, -side, -side };
		vertices[2].pos  = { -side,  side, -side };
		vertices[3].pos  = {  side,  side, -side };
		vertices[4].pos  = { -side, -side,  side };
		vertices[5].pos  = {  side, -side,  side };
		vertices[6].pos  = { -side,  side,  side };
		vertices[7].pos  = {  side,  side,  side };
		vertices[8].pos  = { -side, -side, -side };
		vertices[9].pos  = { -side,  side, -side };
		vertices[10].pos = { -side, -side,  side };
		vertices[11].pos = { -side,  side,  side };
		vertices[12].pos = {  side, -side, -side };
		vertices[13].pos = {  side,  side, -side };
		vertices[14].pos = {  side, -side,  side };
		vertices[15].pos = {  side,  side,  side };
		vertices[16].pos = { -side, -side, -side };
		vertices[17].pos = {  side, -side, -side };
		vertices[18].pos = { -side, -side,  side };
		vertices[19].pos = {  side, -side,  side };
		vertices[20].pos = { -side,  side, -side };
		vertices[21].pos = {  side,  side, -side };
		vertices[22].pos = { -side,  side,  side };
		vertices[23].pos = {  side,  side,  side };

		return{
			std::move(vertices),{
				   0,2,1,     2,3,1,
				   4,5,7,     4,7,6,
				  8,10,9,   10,11,9,
				12,13,15,  12,15,14,
				16,17,18,  18,17,19,
				20,23,21,  20,22,23
			}
		};
	}
};