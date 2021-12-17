#pragma once
#include "BaseBindables.h"	
#include "BaseDrawables.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "Cone.h"
#include "Vecmesh.h"

class Vector
{
public:
	Vector(Graphics& gfx, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 dir, int nVecs)
		:
		position(pos), direction(dir)
	{
		for (size_t i = 0; i < nVecs; i++)
		{
			meshPtrs.push_back(CreateVector(gfx, dir, (int)i ));
		}
	}
	std::unique_ptr<Vecmesh> CreateVector(Graphics& gfx, DirectX::XMFLOAT3 dir, int id)
	{
		using namespace DirectX;

		struct Vertex
		{
			XMFLOAT3 pos = {};
			XMFLOAT4 color = { 4/5.0f, 0, 4/5.0f, 1.0f };
		};

		float size = 1.0f;
		float translation = size * 3.0f * id;

		auto model = Cone::MakeVector<Vertex>(size);
		for (size_t i = 0; i < model.vertices.size(); i++)
		{
			model.vertices[i].pos.y += translation;
			auto tmp = XMVectorSet(model.vertices[i].pos.x, model.vertices[i].pos.y, model.vertices[i].pos.z, 1.0f);
			XMStoreFloat3(&model.vertices[i].pos, XMVector3Transform(tmp, XMMatrixRotationRollPitchYaw( PI/2, 0.0f,  0.0f)));
			if (i<4)
				model.vertices[i].color = { 2/5.0f, 0, 2/5.0f, 1.0f };
		}
		//set vertex colors for mesh
		model.vertices[8].color = { 1.0f, 1/5.0f, 1.0f, 1.0f};

		std::vector<std::unique_ptr<Bindable>> bindablePtrs;
		bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, model.vertices));
		bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, model.indices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"vecVS.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));

		bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"vecPS.cso"));
		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },

			//{ "TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		DirectX::XMFLOAT4 bpos = { position.x, position.y, position.z, 1.0f };
		return std::make_unique<Vecmesh>(gfx, std::move(bindablePtrs), bpos, id);
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX transform) const
	{
		for (size_t i = 0; i < meshPtrs.size(); i++)
		{
			meshPtrs[i]->Draw(gfx, transform);
		}
	}
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept
	{
		for (size_t i = 0; i < meshPtrs.size(); i++)
		{
			meshPtrs[i]->Update(dt, pos);
		}
	}
	DirectX::XMFLOAT3& GetPosition()
	{
		return position;
	}
private:
	// positional
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 direction;
private:
	std::vector<std::unique_ptr<Vecmesh>> meshPtrs;
	//std::vector<boneNode> boneTree;
	//std::vector<DirectX::XMFLOAT4X4> bts;

	//std::unique_ptr<NodeAi> pRoot;
};