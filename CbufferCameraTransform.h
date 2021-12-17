#pragma once
#include "ConstantBuffers.h"
#include "Camera.h"
#include "Drawable.h"
#include <DirectXMath.h>

class CBufferCameraTransform : public Bindable
{
private:
	struct Transform
	{
		DirectX::XMMATRIX viewProj;
	};
public:
	CBufferCameraTransform(Graphics& gfx, unsigned int slot);
	void Update(Graphics& gfx);
	void Update(Graphics& gfx, DirectX::XMFLOAT3& lookAtPoint);
	void SetCamera(const Camera* pCamera);
	void Bind(Graphics& gfx) noexcept override;
private:
	std::unique_ptr<VertexConstantBuffer<Transform>> pVS_Cbuffer;
	const Camera* pCamera = nullptr;
};