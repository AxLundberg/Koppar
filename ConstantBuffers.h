#pragma once
#include "Bindable.h"

template<typename C>
class ConstantBuffer : public Bindable
{
public:
	ConstantBuffer(Graphics& gfx, const C& consts, unsigned int slot = 0)
		: slot(slot)
	{
		D3D11_BUFFER_DESC cb_d = {};
		cb_d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_d.Usage = D3D11_USAGE_DYNAMIC;						//Dynamic gives access to LOCK function
		cb_d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cb_d.MiscFlags = 0u;
		cb_d.ByteWidth = sizeof(consts);
		cb_d.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csubr_d = {};
		csubr_d.pSysMem = &consts;

		GetDevice(gfx)->CreateBuffer(&cb_d, &csubr_d, &pConstantBuffer);
	}

	ConstantBuffer(Graphics& gfx)
	{
		D3D11_BUFFER_DESC cb_d = {};
		cb_d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_d.Usage = D3D11_USAGE_DYNAMIC;						
		cb_d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cb_d.MiscFlags = 0u;
		cb_d.ByteWidth = sizeof(C);
		cb_d.StructureByteStride = 0u;

		GetDevice(gfx)->CreateBuffer(&cb_d, nullptr, &pConstantBuffer);
	}

	ConstantBuffer(Graphics& gfx, UINT slot)
		: slot(slot)
	{
		D3D11_BUFFER_DESC cb_d = {};
		cb_d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_d.Usage = D3D11_USAGE_DYNAMIC;
		cb_d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cb_d.MiscFlags = 0u;
		cb_d.ByteWidth = sizeof(C);
		cb_d.StructureByteStride = 0u;

		GetDevice(gfx)->CreateBuffer(&cb_d, nullptr, &pConstantBuffer);
	}

	void Update(Graphics& gfx, const C& consts)
	{
		D3D11_MAPPED_SUBRESOURCE msubr;
		GetContext(gfx)->Map(
			pConstantBuffer.Get(),
			0u,
			D3D11_MAP_WRITE_DISCARD,
			0u,
			&msubr
		);
		memcpy(msubr.pData, &consts, sizeof(consts));
		GetContext(gfx)->Unmap(pConstantBuffer.Get(), 0u);
	}

protected:
	unsigned int slot = 0u;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pBufferView;
};

template<typename C>
class VertexConstantBuffer : public ConstantBuffer<C>
{
	/*For inheritance of templated parent, explicit import*/
	using ConstantBuffer<C>::slot;
	using ConstantBuffer<C>::pConstantBuffer;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	void Bind(Graphics& gfx) noexcept override
	{
		GetContext(gfx)->VSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};

template<typename C>
class PixelConstantBuffer : public ConstantBuffer<C>
{
	/*For inheritance of templated parent, explicit import*/
	using ConstantBuffer<C>::slot;
	using ConstantBuffer<C>::pConstantBuffer;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	void Bind(Graphics& gfx) noexcept override
	{
		GetContext(gfx)->PSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};

template<typename C>
class GeometryConstantBuffer : public ConstantBuffer<C>
{
	/*For inheritance of templated parent, explicit import*/
	using ConstantBuffer<C>::slot;
	using ConstantBuffer<C>::pConstantBuffer;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	void Bind(Graphics& gfx) noexcept override
	{
		GetContext(gfx)->GSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};

template<typename C>
class ComputeConstantBuffer : public ConstantBuffer<C>
{
	/*For inheritance of templated parent, explicit import*/
	using ConstantBuffer<C>::slot;
	using ConstantBuffer<C>::pConstantBuffer;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	void Bind(Graphics& gfx) noexcept override
	{
		GetContext(gfx)->CSSetConstantBuffers(slot, 1u, pConstantBuffer.GetAddressOf());
	}
};