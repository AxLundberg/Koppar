#pragma once

#include "BaseBindables.h"	
#include "BaseDrawables.h"

#include "imgui.h"
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <unordered_map>

#include "NodeAi.h"
#include "MeshAi.h"

class ModelAi
{
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 tc;
		int bones[4] = { -1, -1, -1, -1 };
		float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};
	struct mPosKey
	{
		float mTime;
		aiVector3D mValue;
		mPosKey(float t, aiVector3D v) : mTime(t), mValue(v) {}
	};
	struct mScaleKey
	{
		float mTime;
		aiVector3D mValue;
		mScaleKey(float t, aiVector3D v) : mTime(t), mValue(v) {}
	};
	struct mRotKey
	{
		float mTime;
		aiQuaternion mValue;
		mRotKey(float t, aiQuaternion v) : mTime(t), mValue(v) {}
	};
public:
	ModelAi(Graphics& gfx, const std::string fileName, bool RHcoords, float scale);
	
	void UpdateModel(Graphics& gfx, DirectX::FXMMATRIX& transform, float milliSec);

	void Draw(Graphics& gfx, DirectX::XMMATRIX transform, float animation);

	void DrawEffects(Graphics& gfx, DirectX::XMMATRIX transform, float animation);

	void CalcNodeTransformation(DirectX::XMMATRIX& nodeTransform, std::string& nodeName, float animationTime);

	void SpawnControlWindow();

	std::vector<std::unique_ptr<NodeAi<ModelAi>>> mNodes;
	DirectX::XMFLOAT4X4 rootNodeTransform = {};
	std::vector<DirectX::XMFLOAT4X4> mBoneOffsetMatrices = {};
	std::vector<DirectX::XMFLOAT4X4> mVSBoneMatrices = {};
	float mBoundingBoxRadius = 0.0f;
	DirectX::XMFLOAT3 mBBoxPos = {};
private:

	std::unique_ptr<MeshAi> MakeMesh(Graphics& gfx, const std::unordered_map<size_t, std::wstring>& meshToTexture, const aiMesh& mesh);

	std::unique_ptr<NodeAi<ModelAi>> MakeNode(const aiNode& node);
	void MakeNodeTree(const aiNode& node, const UINT idx = 0u);
	std::unique_ptr<NodeAi<ModelAi>> MakeNodeAi(const aiNode& node);

	void InitializeData(Graphics& gfx, const aiScene* pScene);

	void InitIndices(const aiMesh& mesh, std::vector<int>& indices) const;

	void InitKeyMaps(const aiAnimation& animation);

	void InitBoneMap(const aiScene& pScene);

	void InitVerticesPosNorm(const aiMesh& mesh, std::vector<Vertex>& vertices);

	void InitVerticesBoneWeight(const aiMesh& mesh, std::vector<Vertex>& vertices) const;

	void InitNormals(std::vector<Vertex>& vertices, std::vector<int>& indices) const;

	void InitFrameTime(float nAnimFrames, float nTicksPerSecond, unsigned int cprFps = 60);

	void CalcInterpolatedTranslation(aiVector3D& outV, std::string& nodeName, float animationTime) const;
	void CalcInterpolatedRotation(aiQuaternion& outQ, std::string& nodeName, float animationTime) const;
	void CalcInterpolatedScaling(aiVector3D& outV, std::string& nodeName, float animationTime) const;

	NodeAi<ModelAi>& GetNode(std::string name)
	{

	}
private:

	std::unordered_map<std::string, std::vector<mScaleKey>> nameToScale;
	std::unordered_map<std::string, std::vector<mPosKey>> nameToPos;
	std::unordered_map<std::string, std::vector<mRotKey>> nameToRot;

	std::unordered_map<std::string, int> boneToIndex;

	std::vector<std::unique_ptr<MeshAi>> meshPtrs;
	std::unique_ptr<NodeAi<ModelAi>> pRoot;
	//ImGui garbo
	UINT mSelectedBone = 0;
	//will put these in container map for animation choice
	float mScale = 1.0f;
	mutable float animationTime = 0.0f;
	float mAnimTicks = 0;
	float mTicksPerFrame = 0.0f;
};
