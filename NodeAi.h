#pragma once
#include <vector>
#include <directxmath.h>

template<class T>
class NodeAi
{
	friend class ModelAi;
public:
	NodeAi( const DirectX::XMMATRIX& transform, int bneindex, std::string name, T& modelBase) noexcept
		:
		bti(bneindex), name(name), pModelBase(modelBase),
		rootTransform(modelBase.rootNodeTransform),
		boneOffsetMatrices(modelBase.mBoneOffsetMatrices),
		vsBoneMatrices(modelBase.mVSBoneMatrices)
	{
		DirectX::XMStoreFloat4x4(&mNodeTransform, transform);
	}
	const std::vector<UINT>& GetChildIndices()
	{
		return childNodeIndices;
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX parentTransform, float aniTime) noexcept
	{
		using namespace DirectX;
		
		auto ntf = XMLoadFloat4x4(&mNodeTransform);

		pModelBase.CalcNodeTransformation(ntf, name, aniTime);
		
		auto globalTF = parentTransform * ntf;

		XMMATRIX inputMatrix = XMMatrixScaling(mScale.x, mScale.y, mScale.z) *
							   XMMatrixRotationRollPitchYaw(mRot.x, mRot.y, mRot.z) *
							   XMMatrixTranslation(mTrans.x, mTrans.y, mTrans.z);

		globalTF = globalTF * inputMatrix;
		/*if (bti == 5)
		{
			globalTF = globalTF * XMMatrixScaling(2.0f, 2.0f, 2.0f);
		}
		if (name == "head")
		{
			globalTF = globalTF * XMMatrixScaling(2.0f, 2.0f, 2.0f);
		}*/
		if (bti != -1)
		{
			XMMATRIX bonebuilt = XMLoadFloat4x4(&rootTransform) * globalTF * XMLoadFloat4x4(&boneOffsetMatrices[bti]);
			XMStoreFloat4x4(&vsBoneMatrices[bti], bonebuilt);
		}

		for (const auto& pc : childNodeptrs)
		{
			pc->Draw(gfx, globalTF, aniTime);
		}
	}
	void Draw2(Graphics& gfx, DirectX::FXMMATRIX parentTransform, float aniTime) noexcept
	{
		using namespace DirectX;

		auto ntf = XMLoadFloat4x4(&mNodeTransform);

		pModelBase.CalcNodeTransformation(ntf, name, aniTime);

		auto globalTF = parentTransform * ntf;

		XMMATRIX inputMatrix = XMMatrixScaling(mScale.x, mScale.y, mScale.z) *
			XMMatrixRotationRollPitchYaw(mRot.x, mRot.y, mRot.z) *
			XMMatrixTranslation(mTrans.x, mTrans.y, mTrans.z);

		globalTF = globalTF * inputMatrix;

		if (bti != -1)
		{
			XMMATRIX bonebuilt = XMLoadFloat4x4(&rootTransform) * globalTF * XMLoadFloat4x4(&boneOffsetMatrices[bti]);
			XMStoreFloat4x4(&vsBoneMatrices[bti], bonebuilt);
		}

		for (const auto& i : childNodeIndices)
		{
			pModelBase.mNodes[i]->Draw2(gfx, globalTF, aniTime);
		}
	}
	void SetScale(DirectX::XMFLOAT3 scale)
	{
		mScale = scale;
	}
	void SetRotation(DirectX::XMFLOAT3 rotation)
	{
		mRot = rotation;
	}
	void SetTranslation(DirectX::XMFLOAT3 translation)
	{
		mTrans = translation;
	}
	std::string GetName()
	{
		return name;
	}
	int GetBoneIndex()
	{
		return bti;
	}
	std::vector<std::unique_ptr<NodeAi>>& GetChildren()
	{
		return childNodeptrs;
	}
	void SpawnControlWindow()
	{
		ImGui::Columns(2, nullptr, true);
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll",  &mRot.x, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", &mRot.y, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw",   &mRot.z, -180.0f, 180.0f);

		ImGui::Text("Translation");
		ImGui::SliderFloat("pos X", &mTrans.x, -10.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("pos Y", &mTrans.y, -10.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("pos Z", &mTrans.z, -10.0f, 10.0f, "%.1f");

		ImGui::Text("Scale");
		ImGui::SliderFloat("X", &mScale.x, -10.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("Y", &mScale.z, -10.0f, 10.0f, "%.1f");
		ImGui::SliderFloat("Z", &mScale.y, -10.0f, 10.0f, "%.1f");


		ImGui::NextColumn();
		if (ImGui::Button("reset orientation")) mRot = { 0.0f, 0.0f, 0.0f };
		if (ImGui::Button("reset roll"))  mRot.x = 0.0f;
		if (ImGui::Button("reset pitch")) mRot.y = 0.0f;
		if (ImGui::Button("reset yaw"))   mRot.z = 0.0f;
		if (ImGui::Button("reset translation")) mTrans = { 0.0f, 0.0f, 0.0f };
		if (ImGui::Button("reset t.x")) mTrans.x = 0.0f;
		if (ImGui::Button("reset t.y")) mTrans.y = 0.0f;
		if (ImGui::Button("reset t.z")) mTrans.z = 0.0f;
		if (ImGui::Button("reset scale")) mScale = { 0.0f, 0.0f, 0.0f };
		if (ImGui::Button("reset s.x")) mScale.x = 0.0f;
		if (ImGui::Button("reset s.y")) mScale.z = 0.0f;
		if (ImGui::Button("reset s.z")) mScale.y = 0.0f;

	}
private:
	void AddChild(std::unique_ptr<NodeAi> pchild) noexcept
	{
		assert(pchild);
		childNodeptrs.push_back(std::move(pchild));
	}
	void AddChild(UINT childIndex) noexcept
	{
		childNodeIndices.push_back(childIndex);
	}
private:
	std::vector<std::unique_ptr<NodeAi>> childNodeptrs;

	std::vector<UINT> childNodeIndices;

	mutable DirectX::XMFLOAT4X4 mNodeTransform;
	//imgui input transformations
	mutable DirectX::XMFLOAT3 mScale = { 1.0f, 1.0f, 1.0f };
	mutable DirectX::XMFLOAT3 mRot = { 0.0f, 0.0f, 0.0f };
	mutable DirectX::XMFLOAT3 mTrans = { 0.0f, 0.0f, 0.0f };

	DirectX::XMFLOAT4X4& rootTransform;
	std::vector<DirectX::XMFLOAT4X4>& vsBoneMatrices;
	std::vector<DirectX::XMFLOAT4X4>& boneOffsetMatrices;

	int mSelectedChild = -1;
	std::string name;
	int bti;
	T& pModelBase;
};
