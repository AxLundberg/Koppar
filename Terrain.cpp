#include "Terrain.h"
#include "BaseBindables.h"
#include "Plane.h"
#include "Stencil.h"

Terrain::Terrain(Graphics& gfx, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, int reach, float heightAttenuation, float divX, float divY, float width, float height, std::wstring filename)
	:
	pos(pos), rot{rot.x, rot.y, rot.z},
	divX(divX), divY(divY),
	width(width), height(height)
{
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{
		using namespace DirectX;
		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 tangent;
			DirectX::XMFLOAT3 bitangent;
			DirectX::XMFLOAT3 normal;
			struct
			{
				float u;
				float v;
			} texCoord;
		};

		assert(heightAttenuation != 0);
		auto model = Plane::MakeHeightMap<Vertex>(width, height, divX, divY, heightAttenuation, reach, filename);
		for (auto& v : model.vertices)
		{
			v.pos.x += pos.x;
			v.pos.y += pos.y;
			v.pos.z += pos.z;
		}
		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
		AddStaticBind(std::make_unique<Sampler>(gfx));
		AddStaticBind(std::make_unique<Sampler>(gfx, 1u));
		AddStaticBind(std::make_unique<Sampler>(gfx, 2u));

		auto pvs = std::make_unique<VertexShader>(gfx, L"phongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));
		AddStaticBind(std::make_unique<PixelShader>(gfx, L"matPS.cso"));

		struct PSMeshMaterial
		{
			DirectX::XMFLOAT4 diffuseColor = { 0.65f,0.45f ,0.45f, 1.0f };
			DirectX::XMFLOAT4 specularColor = { 0.35f,0.35f ,0.40f, 1.0f };
			float specularIntensity = 0.0f;
			float specularPower = 10.0f;
			BOOL hasNormalMap = FALSE;
			BOOL hasDiffuseTexture = FALSE;
		} pmc;

		AddStaticBind(std::make_unique<PixelConstantBuffer<PSMeshMaterial>>(gfx, pmc, 1u));

		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "TANGENT" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "BINORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "NORMAL"  ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		};

		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		AddStaticBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));

		//-----------TESTING SHADOWING -------------//
		//AddStaticEffectBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
		//AddStaticEffectBind(std::make_unique<IndexBuffer>(gfx, model.indices));

		auto pevs = std::make_unique<VertexShader>(gfx, L"ShadowVS.cso");
		auto pevsbc = pevs->GetBytecode();
		AddStaticEffectBind(std::move(pevs));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> t_eied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
		};

		AddStaticEffectBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));
		AddStaticEffectBind(std::make_unique<InputLayout>(gfx, t_eied, pevsbc));
		//-------------------------------------------//
	}
	else
	{
		SetStaticIndexBuffer();
	}
	AddBind(std::make_unique<CBufferTransform>(gfx, *this));
	AddEffectBind(std::make_unique<CBufferTransform>(gfx, *this));

	dx::XMStoreFloat3x3(
		&mt,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}

void Terrain::Update(float dt, DirectX::XMFLOAT3 pos) noexcept
{	
}

DirectX::XMMATRIX Terrain::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat3x3(&mt) *
		DirectX::XMMatrixRotationRollPitchYaw(0, 0, 0) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> Terrain::GetBoundingBox() const noexcept
{
	return {};
}

DirectX::XMFLOAT3 Terrain::GetRotation() const noexcept
{
	return DirectX::XMFLOAT3();
}

