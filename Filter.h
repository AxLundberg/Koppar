#pragma once
#include "BaseBindables.h"
#include <unordered_map>
#include "Graphics.h"
#include "ConstantBuffers.h"
#include "Sampler.h"
#include "imgui.h"

class Filter
{
private:
	const struct KernelCBuf
	{
		UINT radius = 0;
		float rSigma = 0.051f;
		float padd2;
		float padd3;
		float values[128] = {};
	};
public:
	Filter(Graphics& gfx);
	void SetFilter(Graphics& gfx, const std::string& filterName);
	void SetGaussianFilter(Graphics& gfx, float sigma = 0.01f, unsigned char radius = 0, float rSigma = 0.051f);
	void SetBilateralFilter(Graphics& gfx, float sigma = 0.01f, unsigned char radius = 0, float rSigma = 0.051f);
	void UpdateGaussianKernel(Graphics& gfx, float sigma, unsigned char radius, float rSigma);
	void Bind(Graphics& gfx);
	void Bind2(Graphics& gfx);
	~Filter();
private:
	KernelCBuf mKernel;
	std::string currentFilter = {};
	mutable ComputeConstantBuffer<KernelCBuf> k_cbuf;
	std::unordered_map<std::string, std::pair<std::unique_ptr<ComputeShader>, std::unique_ptr<ComputeShader>>> mFilters;
};