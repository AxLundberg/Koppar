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
		mPosition(pos), mDirection(dir)
	{
		mScale = DirectX::XMFLOAT3{ 1,1,1 };
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
			//model.vertices[i].pos.y += translation;
			auto tmp = XMVectorSet(model.vertices[i].pos.x, model.vertices[i].pos.y, model.vertices[i].pos.z, 1.0f);
			XMStoreFloat3(&model.vertices[i].pos, XMVector3Transform(tmp, XMMatrixRotationRollPitchYaw( PI/2, 0.0f,  0.0f)));
			//XMStoreFloat3(&model.vertices[i].pos, tmp);
			if (i<4)
				model.vertices[i].color = { 2/5.0f, 0, 2/5.0f, 1.0f };
		}
		//set vertex colors for mesh
		model.vertices[8].color = { 1.0f, 1/5.0f, 1.0f, 1.0f};

		std::vector<std::unique_ptr<Bindable>> bindablePtrs;
		bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, model.vertices));
		bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, model.indices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"ColorIndexVS.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));

		bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"ColorIndexPS.cso"));
		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			//{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },

			//{ "TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, ied, pvsbc));
				DirectX::XMFLOAT4 bpos = { mPosition.x, mPosition.y, mPosition.z, 1.0f };
		return std::make_unique<Vecmesh>(gfx, std::move(bindablePtrs), bpos, id);
	}
	void Draw(Graphics& gfx) const
	{
		using namespace DirectX;

		XMVECTOR scale = XMLoadFloat3(&mScale);
		XMVECTOR position = XMLoadFloat3(&mPosition);
		XMVECTOR normalizedDir = XMVector3Normalize(XMLoadFloat3(&mDirection));

		// Create the rotation part of the matrix
		XMVECTOR up = XMVectorSet(.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR rotationQuaternion = DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixLookToLH(DirectX::g_XMZero, normalizedDir, up));
		XMMATRIX rotationMatrix = XMMatrixTranspose(XMMatrixRotationQuaternion(rotationQuaternion));
		
		// Create the scaling part of the matrix
		XMMATRIX scalingMatrix = XMMatrixScalingFromVector(scale);

		for (size_t i = 0; i < power; i++)
		{
			XMVECTOR translation = XMVectorScale(normalizedDir, static_cast<float>(i) * 3.0f); // Calculate translation vector
			XMVECTOR newPosition = XMVectorAdd(position, translation);
			XMMATRIX translationMatrix = XMMatrixTranslationFromVector(newPosition);
			
			XMMATRIX tf = scalingMatrix * rotationMatrix * translationMatrix;
			meshPtrs[i]->Draw(gfx, tf);
		}
	}
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept
	{
		for (size_t i = 0; i < power; i++)
		{
			meshPtrs[i]->Update(dt, pos);
		}
	}
	DirectX::XMFLOAT3 GetPosition()
	{
		return mPosition;
	}
	void SetRotation(DirectX::XMMATRIX& dir)
	{
		DirectX::XMStoreFloat4x4(&mRotation, dir);
	}
	void SetDirection(DirectX::XMVECTOR& dir)
	{
		DirectX::XMStoreFloat3(&mDirection, dir);
	}
	void SetScale(DirectX::XMVECTOR& scale)
	{
		DirectX::XMStoreFloat3(&mScale, scale);
	}
	void SetPosition(DirectX::XMVECTOR& pos)
	{
		DirectX::XMStoreFloat3(&mPosition, pos);
	}
	void SetPower(float force)
	{
		power = (int)force;
		if (power > MAX_POWER)
			power = MAX_POWER;
	}
private:
	// positional
	static constexpr int MAX_POWER = 4;
	DirectX::XMFLOAT3 mScale;
	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mDirection;
	DirectX::XMFLOAT4X4 mRotation;
	int power = 4;
private:
	std::vector<std::unique_ptr<Vecmesh>> meshPtrs;
	//std::vector<boneNode> boneTree;
	//std::vector<DirectX::XMFLOAT4X4> bts;

	//std::unique_ptr<NodeAi> pRoot;
};