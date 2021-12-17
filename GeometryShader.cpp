#include "GeometryShader.h"

GeometryShader::GeometryShader(Graphics& gfx, const std::wstring& path)
{
	D3DReadFileToBlob(path.c_str(), &pBytecodeBlob);
	GetDevice(gfx)->CreateGeometryShader(pBytecodeBlob->GetBufferPointer(), pBytecodeBlob->GetBufferSize(), nullptr, &pGeometryShader);
	/*Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	D3DReadFileToBlob(path.c_str(), &pBlob);
	GetDevice(gfx)->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pGeometryShader);*/
}

void GeometryShader::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->GSSetShader(pGeometryShader.Get(), nullptr, 0u);
}

ID3DBlob* GeometryShader::GetBytecode() const noexcept
{
	return pBytecodeBlob.Get();
}
