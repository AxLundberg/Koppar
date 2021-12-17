#pragma once
#include "Bindable.h"



class TextureCube : public Bindable
{
public:
	TextureCube(Graphics& gfx, const std::wstring& path, UINT slot = 0);
	void Bind(Graphics& gfx) noexcept override;
private:
	unsigned int slot;
protected:
	std::wstring path;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTexView;
};
