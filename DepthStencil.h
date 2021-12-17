#pragma once
#include "Graphics.h"
#include "GraphicsResource.h"
#include "Sheet.h"

class DepthStencil : public GraphicsResource
{
	friend class RenderTarget;
	friend class Graphics;
public:
	DepthStencil(Graphics& gfx, UINT width, UINT height)
		:
		width(width), height(height)
	{
		// create depth stensil texture
		D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1u;
		descDepth.ArraySize = 1u;
		descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
		descDepth.SampleDesc.Count = 1u;
		descDepth.SampleDesc.Quality = 0u;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags =  D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
		GetDevice(gfx)->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

		// create depth stensil view on the dsTexture
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		GetDevice(gfx)->CreateDepthStencilView(pDepthStencil.Get(), &dsvDesc, &pDepthStencilView);

		// create shader resource view on the dsTexture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		//srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		GetDevice(gfx)->CreateShaderResourceView(pDepthStencil.Get(), &srvDesc, &pDT_TextureView);
	}

	void BindAsDepthStencil(Graphics& gfx) noexcept
	{
		GetContext(gfx)->OMSetRenderTargets(0, nullptr, pDepthStencilView.Get());
	}

	void BindAsTexture(Graphics& gfx, UINT slot)
	{
		GetContext(gfx)->PSSetShaderResources(slot, 1, pDT_TextureView.GetAddressOf());
	}

	void Clear(Graphics& gfx) noexcept
	{
		GetContext(gfx)->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pDT_TextureView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
	unsigned int width;
	unsigned int height;
};