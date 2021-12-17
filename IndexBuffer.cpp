#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Graphics& gfx, const std::vector<int>& indices)
	: count((UINT)indices.size())
{
	D3D11_BUFFER_DESC ib_d = {};
	ib_d.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib_d.Usage = D3D11_USAGE_DEFAULT;
	ib_d.CPUAccessFlags = 0u;
	ib_d.MiscFlags = 0u;
	ib_d.ByteWidth = (UINT)(count * sizeof(int));
	ib_d.StructureByteStride = sizeof(int);

	D3D11_SUBRESOURCE_DATA isubr_d = {};
	isubr_d.pSysMem = indices.data();

	GetDevice(gfx)->CreateBuffer(&ib_d, &isubr_d, &pIndexBuffer);
}

IndexBuffer::IndexBuffer(Graphics& gfx, const std::vector<int>& indices, const std::vector<int>& subsetIndexStart)
	: count((UINT)indices.size())
{
	D3D11_BUFFER_DESC ib_d = {};
	ib_d.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib_d.Usage = D3D11_USAGE_DEFAULT;
	ib_d.CPUAccessFlags = 0u;
	ib_d.MiscFlags = 0u;
	ib_d.ByteWidth = (UINT)(count * sizeof(int));
	ib_d.StructureByteStride = sizeof(int);

	D3D11_SUBRESOURCE_DATA isubr_d = {};
	isubr_d.pSysMem = indices.data();

	GetDevice(gfx)->CreateBuffer(&ib_d, &isubr_d, &pIndexBuffer);

	for (UINT i = 0; i < subsetIndexStart.size(); i++)
	{
		subsetIndex.push_back(subsetIndexStart[i]);
	}
}

void IndexBuffer::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
}

UINT IndexBuffer::GetCount() const noexcept 
{
	return count;
}

std::vector<int> IndexBuffer::GetSubsetIndex() const noexcept
{
	return subsetIndex;
}