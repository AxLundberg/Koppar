#pragma once
#include "ConstantBuffers.h"
#include "Drawable.h"
#include <DirectXMath.h>

class CBufferBoneTransform : public Bindable
{
private:
	struct BoneTransforms
	{
		DirectX::XMMATRIX model = {};
		DirectX::XMMATRIX modelview = {};
		DirectX::XMMATRIX modelviewprojection = {};
		DirectX::XMMATRIX bones[256] = {};
		
		BoneTransforms(DirectX::XMMATRIX m, DirectX::XMMATRIX mv, DirectX::XMMATRIX mvp, DirectX::XMMATRIX bone[256], size_t nBones)
			: model(DirectX::XMMatrixTranspose(m)), modelview(DirectX::XMMatrixTranspose(mv)), modelviewprojection(DirectX::XMMatrixTranspose(mvp))
		{
			for (size_t i = 0; i < nBones; i++)
			{
				bones[i] = bone[i];
			}
		}
	};
public:
	CBufferBoneTransform(Graphics& gfx, const Drawable& parent, UINT slot = 0u);
	void Bind(Graphics& gfx) noexcept override;
private:
	static std::unique_ptr<VertexConstantBuffer<BoneTransforms>> v_cbuf;
	const Drawable& parent;
};