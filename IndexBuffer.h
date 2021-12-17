#pragma once
#include "Bindable.h"

class IndexBuffer : public Bindable
{
public:
	IndexBuffer(Graphics& gfx, const std::vector<int>& indices);
	IndexBuffer(Graphics& gfx, const std::vector<int>& indices, const std::vector<int>& subsetStartIndex);
	void Bind(Graphics& gfx) noexcept override;
	UINT GetCount() const noexcept;
	std::vector<int> GetSubsetIndex() const noexcept;
protected:
	UINT count;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	std::vector<int> subsetIndex = {};
};