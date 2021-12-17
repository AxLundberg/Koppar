#pragma once
#include "BaseBindables.h"	
#include "BaseDrawables.h"
#include "ModelData.h"
#include "Node.h"
#include "Mesh.h"
#include "Stencil.h"

class Model
{
public:
	Model(Graphics& gfx, const std::wstring fileName, bool RHcoords, float scale)
	{
		const auto model = ModelData(fileName, RHcoords, scale);
		meshTree = model.meshTree;
		boundingBoxRadius = model.boundingBoxRadius;

		for (int i = 0; i < model.meshes.size(); i++)
		{
			meshPtrs.push_back(ParseMesh<>( gfx, model.meshes[i]) );
		}
		pRoot = ParseNode(0u, 1u, RHcoords);
		//pRoot = ParseNode(*pScene->mRootNode);
	}
	Model(Graphics& gfx, const std::wstring fileName, bool RHcoords, float scale, DirectX::FXMMATRIX transformation)
	{
		auto model = ModelData(fileName, RHcoords, scale);
		meshTree = model.meshTree;
		boundingBoxRadius = model.boundingBoxRadius;
		bBoxRadii.x = model.boundingBoxRadius;
		bBoxRadii.y = model.boundingBoxRadius;
		bBoxRadii.z = model.boundingBoxRadius;
		XMStoreFloat3(&bBoxPos, transformation.r[3]);

		for (auto& m : model.meshes)
		{
			for (auto& v : m.vertices)
			{
				const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&v.pos);
				DirectX::XMStoreFloat3(
					&v.pos,
					DirectX::XMVector3Transform(pos, transformation)
				);
			}
			meshPtrs.push_back(ParseMesh<>(gfx, m));
		}

		pRoot = ParseNode(0u, 1u, RHcoords);
		//pRoot = ParseNode(*pScene->mRootNode);
	}
	void Draw(Graphics& gfx) const
	{
		pRoot->Draw(gfx, DirectX::XMMatrixIdentity());
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX transform) const
	{
		pRoot->Draw(gfx, transform);
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX transform, std::array<DirectX::XMVECTOR, 6>& frustrum) const
	{
		if (!FrustrumCull(transform, frustrum))
		{
			pRoot->Draw(gfx, transform);
		}
	}
	void DrawEffects(Graphics& gfx, DirectX::FXMMATRIX transform) const
	{
		pRoot->DrawEffects(gfx, transform);
	}
	void DrawEffects(Graphics& gfx) const
	{
		pRoot->DrawEffects(gfx, DirectX::XMMatrixIdentity());
	}
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox()
	{
		return { bBoxPos, bBoxRadii };
	}
private:
	bool FrustrumCull(DirectX::FXMMATRIX transform, std::array<DirectX::XMVECTOR, 6>& frustrum) const
	{
		using namespace DirectX;

		XMVECTOR tmp = {};
		XMVECTOR why = {};
		XMVECTOR stupid = {};
		XMFLOAT3 center = {};

		XMMatrixDecompose(&why, &stupid, &tmp, transform);
		
		XMStoreFloat3(&center, tmp);

		auto checkPoint = [&](float x, float y, float z) {
			for (size_t i = 0; i < 6; i++)
			{
				if (XMVectorGetX(XMPlaneDotCoord(frustrum[i], XMVectorSet(x, y, z, 1.0f))) < 0.0f)
				{
					return false;
				}
			}
			return true;
		};

		for (signed char i = -1; i < 2; i += 2) {
			for (signed char j = -1; j < 2; j += 2) {
				for (signed char k = -1; k < 2; k += 2)
				{
					auto x = center.x + i * boundingBoxRadius;
					auto y = center.y + j * boundingBoxRadius;
					auto z = center.z + k * boundingBoxRadius;
					if (checkPoint(x, y, z)) 
					{
						return false;  //one corner of bounding box was within frustrum
					}
					else if (!checkPoint(x, y, z))
					{
						int blyat = 0;
					}
				}
			}
		}

		return true;
	}
	template<class T>
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, T& mesh)
	{
		namespace dx = DirectX;

		std::vector<std::unique_ptr<Bindable>> bindablePtrs;

		bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, mesh.vertices));
		bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, mesh.indices));
		
		auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));
		if (!mesh.material.hasNormMap)
		{
			bindablePtrs.push_back(std::make_unique<GeometryShader>(gfx, L"PhongGS.cso"));
		}
		bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"matPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> t_ied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		};

		bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, t_ied, pvsbc));

		struct PSMeshMaterial
		{
			DirectX::XMFLOAT4 diffuseColor = {0.1f,0.1f ,0.1f, 1.0f};
			DirectX::XMFLOAT4 specularColor = { 0.1f,0.1f ,0.1f, 1.0f };
			float specularIntensity = 0.0f;
			float specularPower = 10.0f;
			BOOL hasNormalMap = FALSE;
			BOOL hasDiffuseTexture = FALSE;
		} pmc;

		pmc.diffuseColor  = mesh.material.diffColor;
		pmc.specularColor = mesh.material.specColor;
		pmc.specularPower = mesh.material.specularPower;

		if (mesh.material.hasTexture)
		{
			pmc.hasDiffuseTexture = TRUE;
			std::wstring wstr = mesh.material.texName;
			bindablePtrs.push_back(std::make_unique<Texture>(gfx, wstr, 0u));
		}
		if (mesh.material.hasNormMap)
		{
			//pmc.hasNormalMap = TRUE;
			std::wstring wstr = mesh.material.nMapName;
			bindablePtrs.push_back(std::make_unique<Texture>(gfx, wstr, 1u));
		}
		bindablePtrs.push_back(std::make_unique<Sampler>(gfx));
		bindablePtrs.push_back(std::make_unique<Sampler>(gfx, 1u));
		bindablePtrs.push_back(std::make_unique<Sampler>(gfx, 2u));
		bindablePtrs.push_back(std::make_unique<PixelConstantBuffer<PSMeshMaterial>>(gfx, pmc, 1u));
		bindablePtrs.push_back(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));

		//-----------TESTING SHADOWING -------------//
		std::vector<std::unique_ptr<Bindable>> bindableEffectPtrs;

		auto pevs = std::make_unique<VertexShader>(gfx, L"ShadowVS.cso");
		auto pevsbc = pevs->GetBytecode();
		bindableEffectPtrs.push_back(std::move(pevs));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> t_eied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
		};

		bindableEffectPtrs.push_back(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));
		bindableEffectPtrs.push_back(std::make_unique<InputLayout>(gfx, t_eied, pevsbc));
		//-------------------------------------------//

		return std::make_unique<Mesh>(gfx, std::move(bindablePtrs), std::move(bindableEffectPtrs));
	}
	std::unique_ptr<Node> ParseNode(UINT id, UINT c_id, bool rhCoords)
	{
		namespace dx = DirectX;
		DirectX::XMMATRIX test = {};
		if (id>0)
		{
			test = dx::XMMatrixTranspose(meshPtrs[id].get()->GetTransformXM());
		}
		else
		{
			DirectX::XMFLOAT4X4 transfor;
			DirectX::XMStoreFloat4x4(
				&transfor,
				DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f)
			);
			test = dx::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&transfor));
		}
		const auto transform = test;

		UINT nChildrn = meshTree[id];
		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(size_t((size_t)nChildrn+1));

		curMeshPtrs.push_back(meshPtrs.at(id).get());
		for (size_t i = 0; i < nChildrn; i++)
		{
			curMeshPtrs.push_back(meshPtrs.at(c_id+i).get());
		}

		UINT ncID = c_id + nChildrn; //next child/leaf ID
		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), transform);
		for (UINT nID = c_id; nID < c_id + nChildrn; nID++)
		{
			pNode->AddChild(ParseNode(nID, ncID, rhCoords));
			ncID += meshTree[nID];
		}

		return pNode;
	}
private:
	float boundingBoxRadius = 0.0f;
	DirectX::XMFLOAT3 bBoxRadii = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 bBoxPos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4X4 mTransformation = {};
	std::unique_ptr<Node> pRoot;
	std::vector<UINT> meshTree;
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
};