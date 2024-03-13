#include "Cylinder.h"
#include "BaseBindables.h"
#include "Prism.h"
#include "Stencil.h"

Cylinder::Cylinder(Graphics& gfx,
	DirectX::XMFLOAT3 position,
	DirectX::XMFLOAT3 rotation,
	DirectX::XMFLOAT3 scale,
	DirectX::XMFLOAT3 color)
	:
	mPosition(position),
	mObjRotation(rotation),
	mScale(scale),
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

		auto model = Prism::Make<Vertex>();

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));


		/*struct PixelShaderConsts
		{
			dx::XMFLOAT3 color = color;
			float padding = {};
		} pcb2;

		AddStaticBind(std::make_unique<PixelConstantBuffer<PixelShaderConsts>>(gfx, pcb2, 1u));*/

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
	auto tf = dx::XMMatrixScaling(scale.x, scale.y, scale.z) *
		dx::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
		dx::XMMatrixTranslation(position.x, position.y, position.z);

	dx::XMStoreFloat3x3(&mModelTransform, tf);
}

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
		&mModelTransform,
		dx::XMMatrixScaling(scaleX, scaleY, scaleZ)
	);
}


void Cylinder::Update(float dt, DirectX::XMFLOAT3 pos) noexcept
{
	using namespace DirectX;
	mScale.x += mDeltaScale.x * dt;
	mScale.y += mDeltaScale.y * dt;
	mScale.z += mDeltaScale.z * dt;
	mObjRotation.x += mDeltaObjRotation.x * dt;
	mObjRotation.y += mDeltaObjRotation.y * dt;
	mObjRotation.z += mDeltaObjRotation.z * dt;
	mPointRotation.x = mDeltaPointRotation.x * dt;
	mPointRotation.y = mDeltaPointRotation.y * dt;
	mPointRotation.z = mDeltaPointRotation.z * dt;
	mPosition.x += mDeltaTranslation.x * dt;
	mPosition.y += mDeltaTranslation.y * dt;
	mPosition.z += mDeltaTranslation.z * dt;

	// Calculate the translation to move the object to the rotation origin and back
	XMVECTOR toRotOrigin = { mRotationOrigin.x - mPosition.x, mRotationOrigin.y - mPosition.y, mRotationOrigin.z - mPosition.z };
	XMMATRIX translationToOrigin = XMMatrixTranslationFromVector(toRotOrigin);
	XMMATRIX translationBack = XMMatrixTranslationFromVector(-toRotOrigin);
	// Calculate and combine the rotation matrices for rotations around X, Y, and Z axes
	XMMATRIX rotationX = XMMatrixRotationX(mPointRotation.x);
	XMMATRIX rotationY = XMMatrixRotationY(mPointRotation.y);
	XMMATRIX rotationZ = XMMatrixRotationZ(mPointRotation.z);
	XMMATRIX rotationMatrix = rotationX * rotationY * rotationZ;
	// Combine the transformations
	XMMATRIX transformMatrix = translationBack * rotationMatrix * translationToOrigin;
	// Apply the transformation to the object's position
	XMVECTOR newPosition = XMVector3TransformCoord(XMLoadFloat3(&mPosition), transformMatrix);
	// Update the position
	XMStoreFloat3(&mPosition, newPosition);
}

void Cylinder::SetPosition(DirectX::FXMVECTOR pos) noexcept
{
	DirectX::XMStoreFloat3(&mPosition, pos);
}
void Cylinder::SetRotation(DirectX::FXMVECTOR rot) noexcept
{
	DirectX::XMStoreFloat3(&mObjRotation, rot);
}
void Cylinder::SetScale(DirectX::FXMVECTOR scale) noexcept
{
	DirectX::XMStoreFloat3(&mScale, scale);
}
void Cylinder::SetDeltaTranslation(DirectX::FXMVECTOR dpos) noexcept
{
	DirectX::XMStoreFloat3(&mDeltaTranslation, dpos);
}
void Cylinder::SetDeltaObjRotation(DirectX::FXMVECTOR drot) noexcept
{
	DirectX::XMStoreFloat3(&mDeltaObjRotation, drot);
}
void Cylinder::SetDeltaPointRotation(DirectX::FXMVECTOR drot) noexcept
{
	DirectX::XMStoreFloat3(&mDeltaPointRotation, drot);
}
void Cylinder::SetPointRotationOrigin(DirectX::FXMVECTOR points) noexcept
{
	DirectX::XMStoreFloat3(&mRotationOrigin, points);
}
void Cylinder::SetDeltaScale(DirectX::FXMVECTOR dscale) noexcept
{
	DirectX::XMStoreFloat3(&mDeltaScale, dscale);
}


std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> Cylinder::GetBoundingBox() const noexcept
{
	return {};
}

DirectX::XMMATRIX Cylinder::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixScaling(mScale.x, mScale.y, mScale.z) *
		DirectX::XMMatrixRotationRollPitchYaw(mObjRotation.x, mObjRotation.y, mObjRotation.z) *
		DirectX::XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

}

DirectX::XMFLOAT3 Cylinder::GetRotation() const noexcept
{
	return { move.roll, move.pitch, move.yaw };
}