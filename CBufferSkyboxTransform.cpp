#include "CbufferSkyboxTransform.h"

CBufferSkyboxTransform::CBufferSkyboxTransform(Graphics& gfx, unsigned int slot)
	:
	pVS_Cbuffer(std::make_unique<VertexConstantBuffer<Transform>>(gfx, slot))
{
}

void CBufferSkyboxTransform::Bind(Graphics& gfx) noexcept
{
	auto view = gfx.GetCamera();
	auto proj = gfx.GetProjection();

	const Transform tf =
	{
		DirectX::XMMatrixTranspose(view*proj)
	};

	pVS_Cbuffer->Update(gfx, tf);
	pVS_Cbuffer->Bind(gfx);
}

