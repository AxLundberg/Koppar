#include "Cylinder.h"
#include "BaseBindables.h"
#include "Prism.h"
#include "Stencil.h"

Cylinder::Cylinder(Graphics& gfx,
	float posx, float posy, float posz,
	float roll, float pitch, float yaw,
	float scaleX, float scaleY, float scaleZ,
	DirectX::XMFLOAT3 color)
	:
	move{ posx, posy, posz,
		  roll, pitch, yaw },
	//posx(posx),
	//posy(posy),
	//posz(posz),
	rot{ roll,
	pitch,
	yaw },
	scaleX(scaleX),
	scaleY(scaleY),
	scaleZ(scaleZ),
	color(color)
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

		auto model = Prism::Make<Vertex>();

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));


		struct PixelShaderConsts
		{
			dx::XMFLOAT3 color = { 0.99f, 0.3f, 0.8f };
			float padding = {};
		} pcb2;

		AddStaticBind(std::make_unique<PixelConstantBuffer<PixelShaderConsts>>(gfx, pcb2, 1u));

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
	// model deformation transform (per instance, not stored as bind)
	dx::XMStoreFloat3x3(
		&modelTransform,
		dx::XMMatrixScaling(scaleX, scaleY, scaleZ)
	);
}


void Cylinder::Update(float dt, DirectX::XMFLOAT3 pos) noexcept
{
	rot.roll += droll * dt;
	rot.pitch += dpitch * dt;
	rot.yaw += dyaw * dt;
	theta += dtheta * dt;
	phi += dphi * dt;
	chi += dchi * dt;
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> Cylinder::GetBoundingBox() const noexcept
{
	return {};
}

DirectX::XMMATRIX Cylinder::GetTransformXM() const noexcept
{
	//return DirectX::XMLoadFloat3x3(&modelTransform);
	return DirectX::XMLoadFloat3x3(&modelTransform) *
		DirectX::XMMatrixRotationRollPitchYaw(move.roll, move.pitch, move.yaw) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixTranslation(move.x, move.y, move.z);
}

DirectX::XMFLOAT3 Cylinder::GetRotation() const noexcept
{
	return { move.roll, move.pitch, move.yaw };
}