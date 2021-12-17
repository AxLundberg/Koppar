#include "TestPlane.h"
#include "BaseBindables.h"
#include "Plane.h"

TestPlane::TestPlane(Graphics& gfx, std::wstring vs, std::wstring ps, DirectX::FXMMATRIX tf, float width, float height)
	:
	width(width), height(height)
{
	namespace dx = DirectX;

	struct Vertex
	{
		dx::XMFLOAT3 pos;
		dx::XMFLOAT2 tc;
	};

	int tessy = 1;
	int tessx = 1;
	auto model = Plane::MakeTesselated<Vertex>(tessx, tessy, width, height);
	for (auto& v : model.vertices)
	{
		const auto tmp = dx::XMVector3Transform(dx::XMLoadFloat3(&v.pos), tf);
		dx::XMStoreFloat3(&v.pos, tmp);
	}
	int blyat = 0;
	for (size_t i = 0; i <= tessy; i++)
	{
		float v = 1.0f - i * (1.0f/tessy);
		for (size_t j = 0; j <= tessx; j++)
		{
			float u = j * (1.0f/tessx);
			model.vertices[blyat++].tc = { u,v };
		}
	}
	AddBind(std::make_unique<Sampler>(gfx));
	AddBind(std::make_unique<Sampler>(gfx, 1u));
	AddBind(std::make_unique<Sampler>(gfx, 2u));

	AddBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
	AddIndexBuffer(std::make_unique<IndexBuffer>(gfx, model.indices));

	auto pvs = std::make_unique<VertexShader>(gfx, vs);
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));
	AddBind(std::make_unique<PixelShader>(gfx, ps));

	if (vs == L"shademeVS.cso")
	{

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		};

		AddBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}
	else
	{
		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
		};

		AddBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	AddBind(std::make_unique<CBufferTransform>(gfx, *this));

	dx::XMStoreFloat3x3(
		&modelTransform,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}

void TestPlane::Update(float dt, DirectX::XMFLOAT3 inpos) noexcept
{
	pos = inpos;
	rot.pitch += dt;
}

DirectX::XMMATRIX TestPlane::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat3x3(&modelTransform) *
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixRotationRollPitchYaw(sin(rot.roll), rot.pitch, rot.yaw) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> TestPlane::GetBoundingBox() const noexcept
{
	return {};
}

DirectX::XMFLOAT3 TestPlane::GetRotation() const noexcept
{
	return DirectX::XMFLOAT3();
}
