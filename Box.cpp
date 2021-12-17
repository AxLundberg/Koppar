#include "Box.h"
#include "BaseBindables.h"
#include "Cube.h"
#include "Plane.h"
#include "Stencil.h"

Box::Box(Graphics& gfx)
{
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{

		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 norm;
		};
		 
		auto model = Cube::MakeIndependent<Vertex>();
		model.SetNormalsIndependentFlat();

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));
		AddStaticBind(std::make_unique<GeometryShader>(gfx, L"PhongGS.cso"));
		AddStaticBind(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
			{"BINORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
		};

		//AddStaticBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Write));
		AddStaticBind(std::make_unique<Sampler>(gfx));
		AddStaticBind(std::make_unique<Sampler>(gfx, 1u));
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		/*-------------TESTEING------------------*/

		model.Transform(DirectX::XMMatrixScaling(5.09f, 5.09f, 5.09f));
		AddStaticEffectBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
		AddStaticEffectBind(std::make_unique<IndexBuffer>(gfx, model.indices));

		auto pevs = std::make_unique<VertexShader>(gfx, L"ShadowVS.cso");
		auto pevsbc = pevs->GetBytecode();
		AddStaticEffectBind(std::move(pevs));

		AddStaticEffectBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));

		AddStaticEffectBind(std::make_unique<InputLayout>(gfx, ied, pevsbc));
		AddStaticEffectBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}
	else
	{
		SetStaticIndexBuffer();
	}

	AddBind(std::make_unique<CBufferTransform>(gfx, *this));
	AddEffectBind(std::make_unique<CBufferTransform>(gfx, *this));
	// model deformation transform (per instance, not stored as bind)
	dx::XMStoreFloat4x4(
		&modelTransform,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}

Box::Box(Graphics& gfx, const std::wstring& texturePath)
{
	using namespace DirectX;


	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 texCoord;
	};

	//auto model = Cube::MakeSkinnedIndependent<Vertex>();
	auto model = Cube::MakeSkinned<Vertex>();
	
	model.Transform(XMMatrixRotationRollPitchYaw(0.0f, PI, 0.0f));
	for (size_t i = 0; i < model.vertices.size(); i++)
	{
		std::swap(model.vertices[i].pos.x, model.vertices[i].pos.y);
	}
	AddBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
	if (!IsStaticInitialized())
	{
		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));
	}
	else
	{
		SetStaticIndexBuffer();
	}
	auto pvs = std::make_unique<VertexShader>(gfx, L"testVS.cso");
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));
	AddBind(std::make_unique<PixelShader>(gfx, L"testPS.cso"));
	AddBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Skybox));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	AddBind(std::make_unique<Texture>(gfx, texturePath, 0u));
	AddBind(std::make_unique<Sampler>(gfx, 1u));
	AddBind(std::make_unique<Sampler>(gfx, 2u));
	AddBind(std::make_unique<Sampler>(gfx, 3u));
	AddBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
	AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	

	AddBind(std::make_unique<CBufferSkyboxTransform>(gfx, 0u));
	//AddBind(std::make_unique<CBufferTransform>(gfx, *this));

	XMStoreFloat4x4(
		&modelTransform,
		XMMatrixScaling(5.0f, 5.0f, 5.0f)
	);
}

void Box::UpdateSky(Graphics& gfx, int skyID) noexcept
{
	
}

void Box::Update(float dt, DirectX::XMFLOAT3 ipos) noexcept
{
	auto xmmat = DirectX::XMMatrixTranslation(ipos.x, ipos.y, ipos.z);
	DirectX::XMStoreFloat4x4(&modelTransform, DirectX::XMMatrixTranslation(ipos.x, ipos.y, ipos.z));
}

DirectX::XMMATRIX Box::GetTransformXM() const noexcept
{
	namespace dx = DirectX;
	return dx::XMLoadFloat4x4(&modelTransform);
}

DirectX::XMFLOAT3 Box::GetRotation() const noexcept
{
	return {};
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> Box::GetBoundingBox() const noexcept
{
	return {};
}
