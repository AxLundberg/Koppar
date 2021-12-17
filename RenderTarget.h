#pragma once
#include "Graphics.h"
#include "Sampler.h"
#include "GraphicsResource.h"
#include "DepthStencil.h"

//class DepthStencil;

class RenderTarget : public GraphicsResource
{
public:
	RenderTarget(Graphics& gfx, UINT width, UINT height)
		:
		width(width), height(height)
	{
		{
			// create texture resource
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
			GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &pTexture);
			
			// create the resource view on the texture
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			GetDevice(gfx)->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pTextureView);

			//create unordered access view
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = textureDesc.Format;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			GetDevice(gfx)->CreateUnorderedAccessView(pTexture.Get(), &uavDesc, pTextureUAView.GetAddressOf());

			// create the target view on the texture
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = textureDesc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
			GetDevice(gfx)->CreateRenderTargetView(pTexture.Get(), &rtvDesc, &pTargetView);
		}

		// create filter texture resource
		D3D11_TEXTURE2D_DESC filterTexDesc = {};
		filterTexDesc.Width = width;
		filterTexDesc.Height = height;
		filterTexDesc.MipLevels = 1;
		filterTexDesc.ArraySize = 1;
		filterTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		filterTexDesc.SampleDesc.Count = 1;
		filterTexDesc.SampleDesc.Quality = 0;
		filterTexDesc.Usage = D3D11_USAGE_DEFAULT;
		filterTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		filterTexDesc.CPUAccessFlags = 0;
		filterTexDesc.MiscFlags = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pFilterTexture;
		GetDevice(gfx)->CreateTexture2D(&filterTexDesc, nullptr, &pFilterTexture);

		//create render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtDesc = {};
		rtDesc.Format = filterTexDesc.Format;
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;
		GetDevice(gfx)->CreateRenderTargetView(pFilterTexture.Get(), &rtDesc, pFilterTargetView.GetAddressOf());

		//create shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = filterTexDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		GetDevice(gfx)->CreateShaderResourceView(pFilterTexture.Get(), &srvDesc, pFilterTexView.GetAddressOf());

		//create unordered access view
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = filterTexDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		GetDevice(gfx)->CreateUnorderedAccessView(pFilterTexture.Get(), &uavDesc, pFilterUAView.GetAddressOf());

	}
	void ClearRT(Graphics& gfx, DirectX::XMFLOAT3&& color)
	{
		const float col[4] = { color.x, color.y, color.z, 1.0f };
		GetContext(gfx)->ClearRenderTargetView(pTargetView.Get(), col);
	}
	void BindAsTexture(Graphics& gfx, UINT slot) noexcept
	{
		GetContext(gfx)->PSSetShaderResources(slot, 1, pTextureView.GetAddressOf());
	}
	void FilterPass1(Graphics& gfx)
	{
		gfx.BindSwapBuffer();
		GetContext(gfx)->CSSetShaderResources(0, 1, pTextureView.GetAddressOf()); //read from
		GetContext(gfx)->CSSetUnorderedAccessViews(0, 1, pFilterUAView.GetAddressOf(), nullptr); //write to

		GetContext(gfx)->Dispatch(2, (UINT)height, 1);

		ID3D11UnorderedAccessView* pNullUAV[1] = { nullptr };
		ID3D11ShaderResourceView* pNullSRV[1] = { nullptr };
		GetContext(gfx)->CSSetUnorderedAccessViews(0u, 1u, pNullUAV, nullptr);
		GetContext(gfx)->CSSetShaderResources(0u, 1u, pNullSRV);

	}
	void FilterPass2(Graphics& gfx)
	{
		GetContext(gfx)->CSSetShaderResources(0, 1, pFilterTexView.GetAddressOf()); //read from
		GetContext(gfx)->CSSetUnorderedAccessViews(0, 1, pTextureUAView.GetAddressOf(), nullptr); //write to

		GetContext(gfx)->Dispatch((UINT)width, 2, 1);

		ID3D11UnorderedAccessView* pNullUAV[1] = { nullptr };
		GetContext(gfx)->CSSetUnorderedAccessViews(0u, 1u, pNullUAV, nullptr);

		GetContext(gfx)->PSSetShaderResources(2u, 1, pTextureView.GetAddressOf()); //bind texture
	}

	void BindUAV(Graphics& gfx, UINT slot) noexcept
	{
		GetContext(gfx)->CSSetUnorderedAccessViews(slot, 1, pFilterUAView.GetAddressOf(), nullptr);
	}

	void BindAsTarget(Graphics& gfx) noexcept
	{
		GetContext(gfx)->OMSetRenderTargets(1, pTargetView.GetAddressOf(), nullptr);
	}

	void BindAsTarget(Graphics& gfx, const DepthStencil& depthStencil) noexcept
	{
		GetContext(gfx)->OMSetRenderTargets(1, pTargetView.GetAddressOf(), depthStencil.pDepthStencilView.Get());
	}

private:
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pFilterUAView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pFilterTexView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pFilterTargetView;

	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pTextureUAView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetView;

	unsigned int width;
	unsigned int height;
};