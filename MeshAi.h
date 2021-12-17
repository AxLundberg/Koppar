#pragma once

class MeshAi : public DrawableBase<MeshAi>
{
public:
	MeshAi(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> ptrsBindbles, std::vector<std::unique_ptr<Bindable>> ptrsEffectBindbles,
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
		for (auto& p_eb : ptrsEffectBindbles)
		{
			if (auto pi = dynamic_cast<IndexBuffer*>(p_eb.get()))
			{
				AddEffectIndexBuffer(std::unique_ptr<IndexBuffer>{ pi });
				p_eb.release();		//to not have 2 ptrs to same mem
			}
			else
			{
				AddEffectBind(std::move(p_eb));
			}
		}
		AddEffectBind(std::make_unique<CBufferBoneTransform>(gfx, *this));
		AddBind(std::make_unique<CBufferBoneTransform>(gfx, *this));
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX modelTransform) const noexcept
	{
		DirectX::XMStoreFloat4x4(&transform, modelTransform);
		Drawable::Draw(gfx);
	}
	void DrawEffects(Graphics& gfx, DirectX::FXMMATRIX modelTransform) const noexcept
	{
		DirectX::XMStoreFloat4x4(&transform, modelTransform);
		Drawable::DrawEffects(gfx, true, false, false);
	}
	DirectX::XMMATRIX GetTransformXM() const noexcept override
	{
		return DirectX::XMLoadFloat4x4(&transform);
	}
	void Update(int id, std::vector<DirectX::XMFLOAT4X4>& boneOffsetMatrices) noexcept
	{
	
	}
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override
	{
		return {};
	}
	DirectX::XMFLOAT3 GetRotation() const noexcept override
	{
		return DirectX::XMFLOAT3();
	}
	std::vector<DirectX::XMFLOAT4X4>* GetBoneTransformations() const noexcept
	{
		return &btm;
	}
private:
	mutable DirectX::XMFLOAT4X4 transform = {};
	std::vector<DirectX::XMFLOAT4X4>& btm;
	std::vector<DirectX::XMFLOAT4X4>& ogbtm;
};