#include "CbufferCameraTransform.h"

CBufferCameraTransform::CBufferCameraTransform(Graphics& gfx, unsigned int slot)
	:
	pVS_Cbuffer(std::make_unique<VertexConstantBuffer<Transform>>(gfx, slot))
{
	//pVS_Cbuffer = std::make_unique<VertexConstantBuffer<Transform>>(gfx, slot);
}
void CBufferCameraTransform::Update(Graphics& gfx)
{
	auto view = pCamera->GetMatrix();
	auto proj = pCamera->GetProjection();

	const Transform tf =
	{
		DirectX::XMMatrixTranspose(view*proj)
	};
	
	pVS_Cbuffer->Update(gfx, tf);
	pVS_Cbuffer->Bind(gfx);
}
void CBufferCameraTransform::Update(Graphics& gfx, DirectX::XMFLOAT3& lookAtPoint)
{
	auto view = pCamera->GetLookAtMatrix(lookAtPoint);
	auto proj = pCamera->GetProjection();

	auto campos = pCamera->GetPosition();

	const Transform tf =
	{
		DirectX::XMMatrixTranspose(view*proj)
	};

	pVS_Cbuffer->Update(gfx, tf);
	pVS_Cbuffer->Bind(gfx);
}

void CBufferCameraTransform::SetCamera(const Camera* pCam)
{
	pCamera = pCam;
}

void CBufferCameraTransform::Bind(Graphics& gfx) noexcept
{
	pVS_Cbuffer->Bind(gfx);
}

