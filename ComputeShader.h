#pragma once
#include "Bindable.h"

class ComputeShader : public Bindable
{
public:
	ComputeShader(Graphics& gfx, const std::wstring& path);
	void Bind(Graphics& gfx) noexcept override;
	ID3DBlob* GetBytecode() const noexcept;
	void CSExecute(Graphics& gfx) noexcept;
private:
	Microsoft::WRL::ComPtr<ID3DBlob> pbcblob;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> pComputeShader;
};
