#pragma once

class Mmesh : public DrawableBase<Mmesh>
{
public:
	Mmesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> ptrsBindbles,
		std::vector<DirectX::XMFLOAT4X4>& bonematrices, std::vector<DirectX::XMFLOAT4X4>& ogbtm)
		: 
		btm(bonematrices), ogbtm(ogbtm)
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

		AddBind(std::make_unique<CBufferBoneTransform>(gfx, *this, 32));
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX modelTransform) const noexcept
	{
		//auto identity = DirectX::XMMatrixIdentity();
		DirectX::XMStoreFloat4x4(&transform, modelTransform);
		Drawable::Draw(gfx);
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX modelTransform, DirectX::XMFLOAT4X4 mMatrix) const noexcept
	{
		mMat = mMatrix;
		//auto identity = DirectX::XMMatrixIdentity();

		DirectX::XMStoreFloat4x4(&transform, modelTransform);
		Drawable::Draw(gfx);
	}
	DirectX::XMMATRIX GetTransformXM() const noexcept override
	{
		return DirectX::XMLoadFloat4x4(&transform);
	}
	void Update(int id, std::vector<DirectX::XMFLOAT4X4>& boneOffsetMatrices) noexcept
	{

	}
	DirectX::XMMATRIX GetBoundingBox() const noexcept override
	{
		if (count == (int)btm.size())
			count = 0;

		//return DirectX::XMLoadFloat4x4(&ogbtm[count++]);
		return DirectX::XMLoadFloat4x4(&btm[count++]);
	
	}
	DirectX::XMFLOAT3 GetRotation() const noexcept override
	{
		return DirectX::XMFLOAT3();
	}
	std::vector<DirectX::XMFLOAT4X4> GetBoneTransformations() const noexcept
	{
		return btm;
	}
private:
	mutable DirectX::XMFLOAT4X4 transform = {};
	//mutable std::vector<DirectX::XMFLOAT4X4> matrices = {};
	mutable DirectX::XMFLOAT4X4 mMat = {};
	mutable int count = 0;
	std::vector<DirectX::XMFLOAT4X4>& btm;
	std::vector<DirectX::XMFLOAT4X4>& ogbtm;
};