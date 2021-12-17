#pragma once
#include "ConstantBuffers.h"
#include <DirectXMath.h>

class CBufferSkyboxTransform : public Bindable
{
private:
	struct Transform
	{
		DirectX::XMMATRIX viewProj;
	};
public:
	CBufferSkyboxTransform(Graphics& gfx, unsigned int slot);
	void Bind(Graphics& gfx) noexcept override;
private:
	std::unique_ptr<VertexConstantBuffer<Transform>> pVS_Cbuffer;
	const Camera* pCamera = nullptr;
};