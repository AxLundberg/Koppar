#pragma once
#include <vector>
#include <directxmath.h>

template<class T>
class Mnode
{
	friend class Mbase;
public:
	Mnode(std::vector<Mmesh*> meshptrs, const DirectX::XMMATRIX& transform, int bneindex, std::string name, T& parent) noexcept
		:
		meshptrs(std::move(meshptrs)), bti(bneindex), name(name), pParent(parent),
		invRootMatrix(parent.GetInverseRootMatrix()),
		boneOffsetMatrices(parent.GetBoneOffsetMatrices()),
		vsBoneMatrices(parent.GetVSBoneMatrices())
	{
		DirectX::XMStoreFloat4x4(&nodeTransform, transform);
		DirectX::XMStoreFloat4x4(&ogNodeTransform, transform);
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX pTransform) const noexcept
	{
		using namespace DirectX;

		XMMATRIX built = {};

		built = XMLoadFloat4x4(&nodeTransform) * pTransform;
		XMMATRIX bonebuilt = {};

		if (bti != -1)
		{
			bonebuilt = XMLoadFloat4x4(&boneOffsetMatrices[bti]) * built * XMLoadFloat4x4(&invRootMatrix);
			XMStoreFloat4x4(&vsBoneMatrices[bti], bonebuilt);
		}

		for (const auto pm : meshptrs)
		{
			pm->Draw(gfx, built);
		}
		for (const auto& pc : childNodeptrs)
		{
			pc->Draw(gfx, built);
		}
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX parentTransform, float aniTime) const noexcept
	{
		using namespace DirectX;
		/*XMFLOAT4X4 asd = {};
		asd._11 = 1.0f;
		asd._22 = 1.0f;
		asd._33 = 1.0f;
		asd._44 = 1.0f;
		auto gettransformation = XMLoadFloat4x4(&asd);
		XMMATRIX invrootmat = XMLoadFloat4x4(&invroot);*/

		auto ntf = XMLoadFloat4x4(&ogNodeTransform);
		
		pParent.CalcNodeTransformation(ntf, name, aniTime);

		auto globalTF = ntf * parentTransform;

		if (bti != -1)
		{
			XMMATRIX bonebuilt = XMLoadFloat4x4(&boneOffsetMatrices[bti]) * globalTF * XMLoadFloat4x4(&invRootMatrix);
			XMStoreFloat4x4(&vsBoneMatrices[bti], bonebuilt);
		}

		for (const auto pm : meshptrs)
		{
			auto identity = DirectX::XMMatrixIdentity();
			/*if (name == "origin")
				globalTF = XMLoadFloat4x4(&ogNodeTransform)* (identity * XMLoadFloat4x4(&invRootMatrix));*/
			pm->Draw(gfx, globalTF);
			//pm->Draw(gfx, XMLoadFloat4x4(&nodeTransform));
		}
		for (const auto& pc : childNodeptrs)
		{
			pc->Draw(gfx, globalTF, aniTime);
		}
	}
private:
	void AddChild(std::unique_ptr<Mnode> pchild) noexcept
	{
		assert(pchild);
		childNodeptrs.push_back(std::move(pchild));
	}
private:
	std::vector<Mmesh*> meshptrs;


	std::vector<std::unique_ptr<Mnode>> childNodeptrs;

	mutable DirectX::XMFLOAT4X4 nodeTransform;
	mutable DirectX::XMFLOAT4X4 ogNodeTransform;

	DirectX::XMFLOAT4X4& invRootMatrix;

	std::vector<DirectX::XMFLOAT4X4>& vsBoneMatrices;
	std::vector<DirectX::XMFLOAT4X4>& boneOffsetMatrices;

	std::string name;
	int bti;
	T& pParent;
};
