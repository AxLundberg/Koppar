#include "Ball.h"
#include "BaseBindables.h"
#include "Sphere.h"
#include "Stencil.h"

Ball::Ball(Graphics& gfx, 
	float radius,
	DirectX::XMFLOAT3 pos,
	DirectX::XMFLOAT3 color)
	:
	mRadius(radius),
	mPosition(pos),
	mColor(color)
{
	namespace dx = DirectX;
	if (!IsStaticInitialized())
	{
		auto pvs = std::make_unique<VertexShader>(gfx, L"ColorIndexVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<GeometryShader>(gfx, L"PhongGS.cso"));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"ColorIndexPS.cso"));

		struct Vertex
		{
			dx::XMFLOAT3 pos;
		};
		
		auto model = Sphere::Make<Vertex>();

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));

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

	struct PixelShaderConsts
	{
		dx::XMFLOAT3 color = {};
		float padding = {};
	} pcb2;

	pcb2.color = color;
	AddBind(std::make_unique<PixelConstantBuffer<PixelShaderConsts>>(gfx, pcb2, 1u));

	AddBind(std::make_unique<CBufferTransform>(gfx, *this));
	// model deformation transform (per instance, not stored as bind)
	dx::XMMATRIX scale = dx::XMMatrixScaling(radius, radius, radius);
	dx::XMMATRIX rot = dx::XMMatrixIdentity();
	dx::XMMATRIX translation = dx::XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

	dx::XMStoreFloat3x3(
		&mModelTransform,
		scale*rot*translation
	);
}


void Ball::Update(float dt, DirectX::XMFLOAT3 pos) noexcept
{

}

void Ball::SetPosition(DirectX::FXMVECTOR pos) noexcept
{
	DirectX::XMStoreFloat3(&mPosition, pos);
}
void Ball::SetRotation(DirectX::XMFLOAT3 pos) noexcept
{
	mPosition = pos;
}
DirectX::XMFLOAT3 Ball::GetPosition() noexcept
{
	return mPosition;
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> Ball::GetBoundingBox() const noexcept
{
	return {};
}

DirectX::XMMATRIX Ball::GetTransformXM() const noexcept
{
	//return DirectX::XMLoadFloat3x3(&modelTransform);
	return DirectX::XMLoadFloat3x3(&mModelTransform) *
		DirectX::XMMatrixRotationRollPitchYaw(mRotation.x, mRotation.y, mRotation.z) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
}

DirectX::XMFLOAT3 Ball::GetRotation() const noexcept
{
	return mRotation;
}