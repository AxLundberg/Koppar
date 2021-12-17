#pragma once
#include "Graphics.h"

class Graphics;

class GraphicsResource
{
protected:
	ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept
	{
		return gfx.pContext.Get();
	}

	ID3D11Device* GetDevice(Graphics& gfx) noexcept
	{
		return gfx.pDevice.Get();
	}
};