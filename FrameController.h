#pragma once
#include <array>
#include "BaseBindables.h"
#include "Graphics.h"

#include "Filter.h"
#include "DepthStencil.h"
#include "RenderTarget.h"

class FrameController
{
public:
	FrameController(Graphics& gfx)
		:
		ds(gfx, gfx.GetWidth(), gfx.GetHeight()),
		ds2(gfx, gfx.GetWidth(), gfx.GetHeight()),
		rt(gfx, gfx.GetWidth(), gfx.GetHeight()),
		mFilter(gfx)
	{
		mFilter.SetBilateralFilter(gfx, 0.1f, 0);
	}
	void BeginFrame(Graphics& gfx)
	{
		ds.Clear(gfx);
		ds2.Clear(gfx);
		rt.BindAsTarget(gfx, ds);
	}
	void ShadowPassEnd(Graphics& gfx)
	{
		gfx.BindSwapBuffer();
		ds.BindAsTexture(gfx, 2u);
	}
	void RenderPassBegin(Graphics& gfx)
	{
		rt.BindAsTarget(gfx, ds2);
	}
	void BindDSAsTexture(Graphics& gfx, UINT slot)
	{
		gfx.BindSwapBuffer();
		ds2.BindAsTexture(gfx, slot);
	}
	void BindRTAsTexture(Graphics& gfx, UINT slot)
	{
		gfx.BindSwapBuffer();
		rt.BindAsTexture(gfx, slot);
	}
	void Reset(Graphics& gfx, DirectX::XMFLOAT3 color)
	{
		rt.ClearRT(gfx, { color.x, color.y, color.z });
	}
	void SetFilter(Graphics& gfx, const std::string& filter)
	{
		mFilter.SetFilter(gfx, filter);
	}
	void UpdateFilter(Graphics& gfx, float sigma, int radius, float rSigma)
	{
		mFilter.UpdateGaussianKernel(gfx, sigma, (unsigned char)radius, rSigma);
	}
	void BindFilter(Graphics& gfx)
	{
		mFilter.Bind(gfx);
		rt.FilterPass1(gfx);
		mFilter.Bind2(gfx);
		rt.FilterPass2(gfx);
	}
public:
	DepthStencil ds;
	DepthStencil ds2;
	RenderTarget rt;
	Filter mFilter;
};