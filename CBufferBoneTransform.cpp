#include "CBufferBoneTransform.h"
#include "BaseBindables.h"	
#include "BaseDrawables.h"
#include <typeinfo>
#include "MaMath.h"

CBufferBoneTransform::CBufferBoneTransform(Graphics& gfx, const Drawable& parent, UINT slot)
	: parent(parent)
{
	if (!v_cbuf)
	{
		v_cbuf = std::make_unique<VertexConstantBuffer<BoneTransforms>>(gfx, slot);
	}
}

void CBufferBoneTransform::Bind(Graphics& gfx) noexcept
{
	DirectX::XMMATRIX bs[256] = {};
	size_t nBones = {};
	
	if (auto mesh = dynamic_cast<const MeshAi*>(&parent))
	{
		nBones = mesh->GetBoneTransformations()->size();
		for (size_t i = 0; i < mesh->GetBoneTransformations()->size(); i++)
		{
			bs[i] = DirectX::XMLoadFloat4x4(&mesh->GetBoneTransformations()->at(i));
		}
	}
	
	const auto model = parent.GetTransformXM();
	const auto modelview = model * gfx.GetCamera();
	const auto modelviewprojection = model * gfx.GetCamera() * gfx.GetProjection();

	const BoneTransforms btf{ model, modelview, modelviewprojection, bs, nBones };

	v_cbuf->Update(gfx, btf);
	v_cbuf->Bind(gfx);
	
}

std::unique_ptr<VertexConstantBuffer<CBufferBoneTransform::BoneTransforms>> CBufferBoneTransform::v_cbuf;
