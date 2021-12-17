#include "TextureCube.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include "wincodec.h"
#include <vector>

TextureCube::TextureCube(Graphics& gfx, const std::wstring& path, UINT slot)
	:
	path(path),
	slot(slot)
{
	Microsoft::WRL::ComPtr<ID3D11Resource> pTexture;

    std::wstring wicpath = L"Textures\\skyboxdds.dds";

	//HRESULT hr = DirectX::CreateDDSTextureFromFile(
	//	GetDevice(gfx),
	//	wicpath.c_str(),
	//	nullptr, pTexView.GetAddressOf()
	//);

    HRESULT hr = DirectX::CreateWICTextureFromFile(
	GetDevice(gfx),
	wicpath.c_str(),
	nullptr, pTexView.GetAddressOf()
    );

    if (FAILED(hr))
    {
        int w = 512;
        int h = 512;
        int bpp = 4;
        int w_h = w * h;
        int* buf = new int[w_h];

        // fill image with checkerboard ) 
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
            {
                if ((i & 32) == (j & 32))
                    buf[i * w + j] = 0x00000000;
                else
                    buf[i * w + j] = 0xffffffff;
            }

        // set up D3D11_SUBRESOURCE_DATA 
        D3D11_SUBRESOURCE_DATA tbsd = {};
        tbsd.pSysMem = (void*)buf;
        tbsd.SysMemPitch = w * bpp;

        // set up D3D11_TEXTURE2D_DESC 
        D3D11_TEXTURE2D_DESC tdesc = {};
        tdesc.Width = w;
        tdesc.Height = h;
        tdesc.MipLevels = 1;
        tdesc.ArraySize = 1;
        tdesc.SampleDesc.Count = 1;
        tdesc.SampleDesc.Quality = 0;
        tdesc.Usage = D3D11_USAGE_DEFAULT;
        tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        tdesc.CPUAccessFlags = 0;
        tdesc.MiscFlags = 0;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
        // create the texture
        GetDevice(gfx)->CreateTexture2D(&tdesc, &tbsd, &pTexture);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = tdesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        // create view
        GetDevice(gfx)->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pTexView);

        delete[] buf;
    }
}

void TextureCube::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->PSSetShaderResources(slot, 1u, pTexView.GetAddressOf());
}
