#pragma once
#include "Bindable.h"

class VertexBuffer : public Bindable
{
public:
	template<class V>
	VertexBuffer(Graphics& gfx, const std::vector<V>& vertices)
		: stride(sizeof(V))
	{
		D3D11_BUFFER_DESC vb_d = {};
		vb_d.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vb_d.Usage = D3D11_USAGE_DEFAULT;
		vb_d.CPUAccessFlags = 0u;
		vb_d.MiscFlags = 0u;
		vb_d.ByteWidth = UINT(sizeof(V) * vertices.size());
		vb_d.StructureByteStride = sizeof(V);

		D3D11_SUBRESOURCE_DATA vsubr_d = {};
		vsubr_d.pSysMem = vertices.data();

		GetDevice(gfx)->CreateBuffer(&vb_d, &vsubr_d, &pVertexBuffer);
	}
	void Bind(Graphics& gfx) noexcept
	{
		const UINT offset = 0u;
		GetContext(gfx)->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
	}
protected:
	UINT stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
};