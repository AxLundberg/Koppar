#pragma once
#include "Bindable.h"

class Texture : public Bindable
{
public:
	Texture(Graphics& gfx, std::wstring texFile, unsigned int slot);
	std::pair<UINT, UINT> MakeNormalMap(float width, float height, std::wstring filename);
	void Bind(Graphics& gfx) noexcept override;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTexView;
	std::vector<unsigned char> okenow;
	unsigned int slot = 0;
};