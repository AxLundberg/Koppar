#pragma once
#include "Bindable.h"
#include <Algorithm>

class ComputeTest : public Bindable
{
public:
	template<class C>
	ComputeTest(Graphics& gfx, const std::vector<C>& input)
		: stride(sizeof(C)), inputSize((UINT)input.size())
	{
		/*---Input subresource data---*/
		D3D11_SUBRESOURCE_DATA in_subr_d = {};
		in_subr_d.pSysMem = input.data();

		D3D11_BUFFER_DESC inOutDesc = {};
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		/*set stucturedbuffer input desc*/
		inOutDesc.Usage = D3D11_USAGE_DEFAULT;
		inOutDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		inOutDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		inOutDesc.CPUAccessFlags = 0;
		inOutDesc.ByteWidth = inputSize * stride;
		inOutDesc.StructureByteStride = stride;

		/*Set structuredbuffer SRV desc*/
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.Flags = 0;
		srvDesc.BufferEx.FirstElement = 0;
		srvDesc.BufferEx.NumElements = inputSize;

		/*Create VSinput buffer & view*/
		GetDevice(gfx)->CreateBuffer(&inOutDesc, &in_subr_d, &pVSInputBuf);

		/*set output uav desc*/
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = inputSize;

		/*Create output buffer & view*/
		GetDevice(gfx)->CreateBuffer(&inOutDesc, nullptr, &pOutputBuf);
		GetDevice(gfx)->CreateUnorderedAccessView(pOutputBuf.Get(), &uavDesc, &pOutputUAV);
		GetDevice(gfx)->CreateShaderResourceView(pOutputBuf.Get(), &srvDesc, &pVSInputSRV);

		/*Set consume and append buffer description*/
		D3D11_BUFFER_DESC conAppDesc = {};
		conAppDesc.Usage = D3D11_USAGE_DEFAULT;
		conAppDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		conAppDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		conAppDesc.CPUAccessFlags = 0;
		conAppDesc.ByteWidth = inputSize * stride;
		conAppDesc.StructureByteStride = stride;

		GetDevice(gfx)->CreateBuffer(&conAppDesc, &in_subr_d, &pConsumeBuf);
		GetDevice(gfx)->CreateBuffer(&conAppDesc, nullptr, &pAppendBuf);

		GetDevice(gfx)->CreateUnorderedAccessView(pConsumeBuf.Get(), &uavDesc, &pConsumeUAV);
		GetDevice(gfx)->CreateUnorderedAccessView(pAppendBuf.Get(), &uavDesc, &pAppendUAV);

		SetNoise(gfx);
		Set1DLookup(gfx, 0.1f, 256);

		testptr[0] = inputSize;
		testptr[1] = 0;
		UAVptr[0] = pAppendUAV.Get();
		UAVptr[1] = pConsumeUAV.Get();
		UAVptr[2] = pOutputUAV.Get();
		SRVptr[0] = pNoiseTexSRV.Get();
		SRVptr[1] = pLookupTexSRV.Get();
		//SRVptr[2] = pVSInputSRV.Get();
	}
	void Bind(Graphics& gfx) noexcept
	{
		std::swap(UAVptr[0], UAVptr[1]);

		GetContext(gfx)->CSSetUnorderedAccessViews(0u, 3u, UAVptr.get(), testptr.get());

		GetContext(gfx)->CSSetShaderResources(0u, 2u, SRVptr.get());

		GetContext(gfx)->Dispatch(1, 1, 1);

		ID3D11UnorderedAccessView* pNullUAV[3] = { nullptr };
		ID3D11ShaderResourceView* pNullSRV[2] = { nullptr };

		GetContext(gfx)->CSSetUnorderedAccessViews(0u, 3u, pNullUAV, nullptr);
		GetContext(gfx)->CSSetShaderResources(0u, 2u, pNullSRV);

		GetContext(gfx)->VSSetShaderResources(0u, 1u, pVSInputSRV.GetAddressOf());
		//MapBuffer(gfx, inputSize, stride, pOutputBuf); //debugging
	}
	void MapBuffer(Graphics& gfx, UINT inputSize, UINT stride, const Microsoft::WRL::ComPtr<ID3D11Resource>& pResource)
	{
		D3D11_BUFFER_DESC memDesc = {};
		ZeroMemory(&memDesc, sizeof(D3D11_BUFFER_DESC));
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Microsoft::WRL::ComPtr<ID3D11Buffer> pCopyBuffer;
		struct ParticleData
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 direction;
			DirectX::XMFLOAT2 OGpos;
			float size;
			float velocity;
			UINT time;
		};

		//gfx.Desc().BufferSetCopyStructured(memDesc, inputSize, stride);
		memDesc.Usage = D3D11_USAGE_STAGING;
		memDesc.BindFlags = 0;
		memDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		memDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		memDesc.ByteWidth = inputSize * stride;
		memDesc.StructureByteStride = stride;
		GetDevice(gfx)->CreateBuffer(&memDesc, nullptr, &pCopyBuffer);

		GetContext(gfx)->CopyResource(pCopyBuffer.Get(), pResource.Get());
		HRESULT hr = GetContext(gfx)->Map(pCopyBuffer.Get(), 0u, D3D11_MAP_READ, 0u, &mappedResource);
		if (SUCCEEDED(hr))
		{
			ParticleData* data = reinterpret_cast<ParticleData*>(mappedResource.pData);
			std::vector<ParticleData> mapdata;
			for (UINT i = 0; i < inputSize; i++)
			{
				mapdata.push_back(data[i]);
			}
			std::vector<UINT> debugcount(256);
			for (UINT i = 0; i < inputSize; i++)
			{
				UINT index = (UINT)mapdata[i].direction.z;
				debugcount[index]++;
			}
			UINT count = 0;
			for (UINT i = 0; i < 256; i++)
			{
				count += debugcount[i];
			}
			GetContext(gfx)->Unmap(pCopyBuffer.Get(), 0u);
		}
	}
	void MapBufferCount(Graphics& gfx, UINT inputSize, UINT stride, Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>& pUAV)
	{
		D3D11_BUFFER_DESC memDesc = {};
		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		Microsoft::WRL::ComPtr<ID3D11Buffer> pCopyBuffer;

		//gfx.Desc().BufferSetCopyStructureCount(memDesc);
		memDesc.Usage = D3D11_USAGE_STAGING;
		memDesc.BindFlags = 0;
		memDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		memDesc.MiscFlags = 0;
		memDesc.ByteWidth = 4;
		memDesc.StructureByteStride = NULL;
		GetDevice(gfx)->CreateBuffer(&memDesc, nullptr, &pCopyBuffer);

		GetContext(gfx)->CopyStructureCount(pCopyBuffer.Get(), 0u, pUAV.Get());
		HRESULT hr = GetContext(gfx)->Map(pCopyBuffer.Get(), 0u, D3D11_MAP_READ, 0u, &mappedResource);
		if (SUCCEEDED(hr))
		{
			UINT* data = reinterpret_cast<UINT*>(mappedResource.pData);
			UINT first = data[0];
			GetContext(gfx)->Unmap(pCopyBuffer.Get(), 0u);
		}
	}
	void SetNoise(Graphics& gfx) noexcept
	{
		gfx.init();
		UINT* RGBA = new UINT[256];     //Array of RGBA counts to make sure all values (0-255) appear same amount of times in all channels
		for (UINT i = 0; i < 256u; i++) //fill each channel with a counter of 256(0xff)
		{
			//RGBA[i] = 0x40404040;
			RGBA[i] = 0xffffffff;
		}
		/*Lambda function to calculate index value*/
		const auto calcIndex = [RGBA](unsigned char rR, unsigned char rG, unsigned char rB, unsigned char rA)
		{
			while (!(RGBA[rR] & 0xff000000)) rR++;
			while (!(RGBA[rG] & 0x00ff0000)) rG++;		//Find non empty counter in each respective color channel
			while (!(RGBA[rB] & 0x0000ff00)) rB++;
			while (!(RGBA[rA] & 0x000000ff)) rA++;
			RGBA[rR] -= 0x01000000;
			RGBA[rG] -= 0x00010000;						//decrement channel counter
			RGBA[rB] -= 0x00000100;
			RGBA[rA] -= 0x00000001;
			return (rR << 24 | rG << 16 | rB << 8 | rA); //return value of each channel in 32bit parameter ( 0xRRGGBBAA )
		}; 

		UINT w = 256u;
		UINT h = 128u;
		UINT w_h = w*h;
		UINT* noise = new UINT[UINT((size_t)w_h + 4*(size_t)w)];
		//Fill the texture
		//Fill first 128*128 pixels with RGBA values
		for (UINT i = 0; i < h; i++)
		{
			for (UINT j = 0; j < w; j++)
			{
				unsigned char rRed = (unsigned char)(gfx.random()*255u+1u);
				unsigned char rGreen = (unsigned char)(gfx.random()*255u+1u);
				unsigned char rBlue = (unsigned char)(gfx.random()*255u+1u);
				unsigned char rAlpha = (unsigned char)(gfx.random()*255u+1u);
				noise[i*w + j] = (UINT)calcIndex(rRed, rGreen, rBlue, rAlpha);
			}
		}
		/*Fill last 4 rows in the texture with variance values for direction angle and velocity*/
		for (UINT i = 0; i < w; i++)
		{
			float alphaVariance = gfx.random()*2 - 1.0f;
			UINT f = *(UINT*)&alphaVariance;
			noise[w_h+i] = f;
		}
		for (UINT i = 0; i < w; i++)
		{
			float betaVariance = gfx.random()*2 - 1.0f;
			UINT f = *(UINT*)&betaVariance;
			noise[w_h + w + i] = f;
		}
		for (UINT i = 0; i < w; i++)
		{
			float velocityVariance = gfx.random()*2 - 1.0f;
			UINT f = *(UINT*)&velocityVariance;
			noise[w_h + 2*w + i] = f;
		}
		for (UINT i = 0; i < w; i++)
		{
			float randVal = gfx.random()*2 - 1.0f;
			UINT f = *(UINT*)&randVal;
			noise[w_h + 3*w + i] = f;
		}

		D3D11_SUBRESOURCE_DATA t_subr_d = {};
		D3D11_TEXTURE2D_DESC tDesc = {};
		D3D11_SHADER_RESOURCE_VIEW_DESC tSrvDesc = {};
		UINT bpp = sizeof(UINT);
		// set D3D11_SUBRESOURCE_DATA 
		t_subr_d.pSysMem = (void*)noise;
		t_subr_d.SysMemPitch = w * bpp;
		t_subr_d.SysMemSlicePitch = 0;

		// set D3D11_TEXTURE2D_DESC 
		tDesc.Width = w;
		tDesc.Height = h+4;
		tDesc.MipLevels = 1;
		tDesc.ArraySize = 1;
		tDesc.SampleDesc.Count = 1;
		tDesc.SampleDesc.Quality = 0;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.Format = DXGI_FORMAT_R32_UINT;
		tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tDesc.CPUAccessFlags = 0;
		tDesc.MiscFlags = 0;
		
		// set D3D11_SHADER_RESOURCE_VIEW
		tSrvDesc.Format = tDesc.Format;
		tSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		tSrvDesc.Texture2D.MostDetailedMip = 0;
		tSrvDesc.Texture2D.MipLevels = 1;

		GetDevice(gfx)->CreateTexture2D(&tDesc, &t_subr_d, &pNoiseTexture);
		GetDevice(gfx)->CreateShaderResourceView(pNoiseTexture.Get(), &tSrvDesc, &pNoiseTexSRV);
		delete[] RGBA;
		delete[] noise;
	}
	void Set1DLookup(Graphics& gfx, float deviation, UINT size) {
		float* buf = new float[size];
		float mean = 0.0f;
		gfx.init();
		int byat = 0;
		int byaeet = 0;
		//Fill texture
		for (UINT i = 0; i < size; i++)
		{
			float index = gfx.random( 0.0f, deviation );
			buf[i] = index;
			if (abs(index)>0.1f)
			{
				byaeet++;
			}
			if (abs(index)>0.2f)
			{
				byat++;
			}
			//if (index>-1.0f && index<-0.01f || index > 0.01f && index < 1.0f) i++;
		}

		// setting up D3D11_SUBRESOURCE_DATA 
		D3D11_SUBRESOURCE_DATA tbsd = {};
		tbsd.pSysMem = (void*)buf;

		// setting up D3D11_TEXTURE1D_DESC 
		D3D11_TEXTURE1D_DESC tdesc = {};
		tdesc.Width = size;
		tdesc.MipLevels = 1;
		tdesc.ArraySize = 1;
		tdesc.Usage = D3D11_USAGE_DEFAULT;
		tdesc.Format = DXGI_FORMAT_R32_FLOAT;
		tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tdesc.CPUAccessFlags = 0;
		tdesc.MiscFlags = 0;
		// create the texture
		GetDevice(gfx)->CreateTexture1D(&tdesc, &tbsd, &pLookupTexture);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = tdesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		srvDesc.Texture1D.MostDetailedMip = 0;
		srvDesc.Texture1D.MipLevels = 1;
		GetDevice(gfx)->CreateShaderResourceView(pLookupTexture.Get(), &srvDesc, &pLookupTexSRV);

		delete[] buf;
	}
protected:
	std::unique_ptr<ID3D11UnorderedAccessView*[]> UAVptr{ new ID3D11UnorderedAccessView*[3]{ } };
	std::unique_ptr<ID3D11ShaderResourceView*[]> SRVptr{ new ID3D11ShaderResourceView*[2]{ } };
	std::unique_ptr<UINT[]> testptr{ new UINT[3]{ 0u, 0u, 0u} };
	UINT stride;
	UINT inputSize;

	Microsoft::WRL::ComPtr<ID3D11Buffer> pVSInputBuf;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pVSInputSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pOutputBuf;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pOutputUAV;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pNoiseTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pNoiseTexSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture1D> pLookupTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pLookupTexSRV;

	Microsoft::WRL::ComPtr<ID3D11Buffer> pConsumeBuf;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pConsumeUAV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pAppendBuf;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pAppendUAV;

	//for debugging purposes
	/*
	Microsoft::WRL::ComPtr<ID3D11Buffer> pCopyBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pInputRes;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pOutputRes;
	UINT debug = 0;*/
	
};