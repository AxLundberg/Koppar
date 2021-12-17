#include "ComputeShader.h"

ComputeShader::ComputeShader(Graphics& gfx, const std::wstring& path)
{
	D3DReadFileToBlob(path.c_str(), &pbcblob);
	GetDevice(gfx)->CreateComputeShader(pbcblob->GetBufferPointer(), pbcblob->GetBufferSize(), nullptr, &pComputeShader);
}

void ComputeShader::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->CSSetShader(pComputeShader.Get(), nullptr, 0u);
}

ID3DBlob* ComputeShader::GetBytecode() const noexcept
{
	return pbcblob.Get();
}

void ComputeShader::CSExecute(Graphics& gfx) noexcept
{
	GetContext(gfx)->Dispatch(1, 1, 1);
}