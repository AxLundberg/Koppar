#include "Filter.h"
#include "imgui.h"

Filter::Filter(Graphics& gfx)
	:
	k_cbuf(gfx, 2u)
{
	mFilters.insert({ "gaussian", std::make_pair(
									std::make_unique<ComputeShader>(gfx, L"gaussHorizontalCS.cso"),
									std::make_unique<ComputeShader>(gfx, L"gaussVerticalCS.cso")) }
	);
	mFilters.insert({ "bilateral", std::make_pair(
									std::make_unique<ComputeShader>(gfx, L"bilateralHorizontalCS.cso"),
									std::make_unique<ComputeShader>(gfx, L"bilateralVerticalCS.cso")) }
	);
}

void Filter::SetFilter(Graphics& gfx, const std::string& filter)
{
	if (filter == "gaussian")
	{
		SetGaussianFilter(gfx);
	}
	else if (filter == "bilateral")
	{
		SetBilateralFilter(gfx);
	}
}
void Filter::SetBilateralFilter(Graphics& gfx, float sigma, unsigned char radius, float rSigma)
{
	currentFilter = "bilateral";
	mKernel.radius = UINT(radius);
	std::fill(std::begin(mKernel.values), std::end(mKernel.values), 0.0f);

	UpdateGaussianKernel(gfx, sigma, radius, rSigma);
}

void Filter::SetGaussianFilter(Graphics& gfx, float sigma, unsigned char radius, float rSigma)
{
	currentFilter = "gaussian";
	mKernel.radius = UINT(radius);
	std::fill(std::begin(mKernel.values), std::end(mKernel.values), 0.0f);

	UpdateGaussianKernel(gfx, sigma, radius, rSigma);
}

void Filter::UpdateGaussianKernel(Graphics& gfx, float sigma, unsigned char radius, float rSigma)
{
	mKernel.radius = (UINT)radius;
	mKernel.rSigma = rSigma;
	const size_t r = (size_t)mKernel.radius;

	float kernelSum = 1.0f;
	mKernel.values[0] = 1.0f;

	for (int i = 1; i <= r; i++)
	{
		const float value = (float)expo(-sq(i)/(2.0f*sq(sigma)));
		mKernel.values[i] = value;
		kernelSum += 2 * value;
	}
	for (size_t i = 0; i <= r; i++)
	{
		mKernel.values[i] /= kernelSum;
	}
	k_cbuf.Update(gfx, mKernel);
	k_cbuf.Bind(gfx);
}

void Filter::Bind(Graphics& gfx)
{
	mFilters.at(currentFilter).first->Bind(gfx);
	k_cbuf.Bind(gfx);
}

void Filter::Bind2(Graphics& gfx)
{
	mFilters.at(currentFilter).second->Bind(gfx);
}

Filter::~Filter()
{
}

