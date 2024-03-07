#pragma once
class Vecmesh : public DrawableBase<Vecmesh>
{
public:
	Vecmesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> ptrsBindbles, DirectX::XMFLOAT4 pos, int id) : bpos(pos), id(id)
	{
		if (!IsStaticInitialized())
		{
			AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}
		for (auto& p_b : ptrsBindbles)
		{
			if (auto pi = dynamic_cast<IndexBuffer*>(p_b.get()))
			{
				AddIndexBuffer(std::unique_ptr<IndexBuffer>{ pi });
				p_b.release();		//to not have 2 ptrs to same mem
			}
			else
			{
				AddBind(std::move(p_b));
			}
		}

		AddBind(std::make_unique<CBufferTransform>(gfx, *this));
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noexcept
	{
		using namespace DirectX;
		//accumulatedTransform = accumulatedTransform/* * XMMatrixTranslation(bpos.x, bpos.y, bpos.z)*/;
		DirectX::XMStoreFloat4x4(&transform, accumulatedTransform* XMMatrixTranslation(bpos.x, bpos.y, bpos.z));
		Drawable::Draw(gfx);
	}
	DirectX::XMMATRIX GetTransformXM() const noexcept override
	{
		namespace dx = DirectX;

		return DirectX::XMLoadFloat4x4(&transform);
	}
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept
	{
		namespace dx = DirectX;

	}
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override
	{
		return {};
	}
	DirectX::XMFLOAT3 GetRotation() const noexcept override
	{
		return DirectX::XMFLOAT3();
	}
private:
	mutable int id = 0;
	mutable DirectX::XMFLOAT4X4 transform = {};
	mutable DirectX::XMFLOAT4X4 bto = {};
	mutable DirectX::XMFLOAT4X4 invbto = {};
	mutable DirectX::XMFLOAT4X4 btransform = {};
	mutable DirectX::XMFLOAT4 bpos = {};
};