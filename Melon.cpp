#include "Melon.h"
#include "BaseBindables.h"
#include "Sphere.h"
#include "Stencil.h"

Melon::Melon(Graphics& gfx, float radius, DirectX::XMFLOAT3 position)
	: pos({ position.x, position.y, position.z })
{
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{
		auto pvs = std::make_unique<VertexShader>(gfx, L"ColorIndexVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"ColorIndexPS.cso"));

		struct Vertex
		{
			dx::XMFLOAT3 pos;
		};

		auto model = Sphere::MakeTesselated<Vertex>(25, 50);
		// deform vertices of model by linear transformation
		model.Transform(dx::XMMatrixScaling(radius, radius, radius));

		AddBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

		AddIndexBuffer(std::make_unique<IndexBuffer>(gfx, model.indices));

		struct PixelShaderConstants
		{
			dx::XMFLOAT3 color = { 0.99f, 0.8f, 0.3f };
			float padding = {};
		} cb2;
		AddStaticBind(std::make_unique<PixelConstantBuffer<PixelShaderConstants>>(gfx, cb2, 1u));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddStaticBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}
	else
	{
		SetStaticIndexBuffer();
	}

	AddBind(std::make_unique<CBufferTransform>(gfx, *this));
}

void Melon::Update(float dt, DirectX::XMFLOAT3 pos) noexcept
{

}

void Melon::SetPos(DirectX::XMFLOAT3 pos) noexcept
{
	this->pos = pos;
}

DirectX::XMMATRIX Melon::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> Melon::GetBoundingBox() const noexcept
{
	return {};
}

DirectX::XMFLOAT3 Melon::GetRotation() const noexcept
{
	return {};
}
