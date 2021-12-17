#pragma once
#include "ConstantBuffers.h"
#include "Drawable.h"
#include <DirectXMath.h>

class CBufferTransform : public Bindable
{
private:
	struct Transforms
	{
		DirectX::XMMATRIX model;
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX modelViewProj;	
	};
	struct gsTransforms
	{
		DirectX::XMFLOAT3 campos;
		DirectX::XMMATRIX model;
		DirectX::XMMATRIX modelViewProj;
	};
public:
	CBufferTransform(Graphics& gfx, const Drawable& parent);
	CBufferTransform(Graphics& gfx, const Drawable& parent, bool geometry);
	void Bind(Graphics& gfx) noexcept override;
private:
	static std::unique_ptr<VertexConstantBuffer<Transforms>> v_cbuf;
	static std::unique_ptr<GeometryConstantBuffer<gsTransforms>> g_cbuf;
	const Drawable& parent;
};