#include "ModelAi.h"
#include "Stencil.h"

ModelAi::ModelAi(Graphics& gfx, const std::string fileName, bool RHcoords, float scale)
	: mScale(scale)
{
	Assimp::Importer imp;
	using namespace DirectX;

	std::string path = "md5models\\boblampclean\\boblampclean.md5mesh";
	const auto pScene = imp.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder |
		0
		/*aiProcess_ConvertToLeftHanded*/
	);

	InitializeData(gfx, pScene);
}

void ModelAi::InitializeData(Graphics& gfx, const aiScene* pScene)
{
	using namespace DirectX;
	using namespace std;

	if (pScene->HasAnimations())
	{
		InitKeyMaps(*pScene->mAnimations[0]);
		InitFrameTime((float)pScene->mAnimations[0]->mDuration, (float)pScene->mAnimations[0]->mTicksPerSecond, 60u);
	}
	InitBoneMap(*pScene);

	const auto rootTF = pScene->mRootNode->mTransformation;

	XMStoreFloat4x4(&rootNodeTransform,
		XMMatrixTranspose(XMMATRIX(&rootTF.a1)
	));

	std::unordered_map<size_t, std::wstring> meshIndexToTexture;
	for (size_t i = 0; i < pScene->mNumMaterials; i++)
	{
		const auto& mat = pScene->mMaterials[i];
		if (mat->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString name;
			if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &name, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string x = name.C_Str();
				std::string path = "md5models\\boblampclean\\" + x;
				meshIndexToTexture.insert({ i, StringToWString(path)});
			}
		}
	}

	for (size_t i = 0; i < pScene->mNumMeshes; i++)
	{
		meshPtrs.push_back(MakeMesh(gfx, meshIndexToTexture, *pScene->mMeshes[i]));
	}

	pRoot = MakeNode(*pScene->mRootNode);
	MakeNodeTree(*pScene->mRootNode);
	boneToIndex.clear();
}

void ModelAi::Draw(Graphics& gfx, DirectX::XMMATRIX transform, float time)
{
	using namespace DirectX;
	
	for (size_t i = 0; i < meshPtrs.size(); i++)
	{
		meshPtrs[i]->Draw(gfx, XMLoadFloat4x4(&rootNodeTransform)*transform);
	}
}

void ModelAi::DrawEffects(Graphics& gfx, DirectX::XMMATRIX transform, float time)
{
	using namespace DirectX;

	for (size_t i = 0; i < meshPtrs.size(); i++)
	{
		meshPtrs[i]->DrawEffects(gfx, XMLoadFloat4x4(&rootNodeTransform)*transform);
	}
}

void ModelAi::UpdateModel(Graphics& gfx, DirectX::FXMMATRIX& transform, float time)
{
	using namespace DirectX;

	auto identity = XMMatrixIdentity();
	animationTime += mTicksPerFrame;
	if (time == 0.0f)
	{
		animationTime += mTicksPerFrame;
	}
	if (animationTime >= mAnimTicks)
	{
		animationTime = 0.0f;
	}

	XMStoreFloat3(&mBBoxPos, transform.r[3]);
	mBBoxPos.y += mBoundingBoxRadius;

	//pRoot->Draw(gfx, XMMatrixScaling(mScale, mScale, mScale) * identity, animationTime);
	mNodes[0]->Draw2(gfx, XMMatrixScaling(mScale, mScale, mScale) * identity, animationTime);
}

void ModelAi::InitKeyMaps(const aiAnimation& animation)
{
	using namespace DirectX;

	for (size_t ch_i = 0; ch_i < animation.mNumChannels; ch_i++)
	{
		const auto channel = animation.mChannels[ch_i];

		std::vector<mPosKey> posKeys;
		std::vector<mRotKey> rotKeys;
		std::vector<mScaleKey> scaKeys;
		posKeys.reserve(channel->mNumPositionKeys);
		rotKeys.reserve(channel->mNumRotationKeys);
		scaKeys.reserve(channel->mNumScalingKeys);

		for (size_t i = 0; i < posKeys.capacity(); i++)
		{
			const auto aiKeys = channel->mPositionKeys[i];

			posKeys.emplace_back((float)aiKeys.mTime, aiKeys.mValue);
		}
		for (size_t i = 0; i < scaKeys.capacity(); i++)
		{
			const auto aiKeys = channel->mScalingKeys[i];

			scaKeys.emplace_back((float)aiKeys.mTime, aiKeys.mValue);
		}
		for (size_t i = 0; i < rotKeys.capacity(); i++)
		{
			const auto aiKeys = channel->mRotationKeys[i];

			rotKeys.emplace_back(
				(float)aiKeys.mTime, aiKeys.mValue
			);
		}

		std::string nodeName = channel->mNodeName.C_Str();
		nameToPos.insert({ nodeName, posKeys });
		nameToRot.insert({ nodeName, rotKeys });
		nameToScale.insert({ nodeName, scaKeys });
	}
}

void ModelAi::InitBoneMap(const aiScene& pScene)
{
	using namespace DirectX;

	int boneIndex = 0;

	for (size_t i = 0; i < pScene.mNumMeshes; i++)
	{
		const auto mesh = pScene.mMeshes[i];

		for (size_t j = 0; j < mesh->mNumBones; j++)
		{
			const auto bone = mesh->mBones[j];
			const auto name = bone->mName.C_Str();

			if (boneToIndex.find(name) == boneToIndex.end()) // if bonemap does not contain the bone, add it.
			{
				XMFLOAT4X4 boneOffsetMat = {};
				XMStoreFloat4x4(&boneOffsetMat,
					XMMATRIX(&bone->mOffsetMatrix.a1));

				boneToIndex.insert({ name, boneIndex++ });
				mBoneOffsetMatrices.push_back(boneOffsetMat);
			}
		}
	}
	mVSBoneMatrices = mBoneOffsetMatrices;
}

void ModelAi::InitFrameTime(float nTicks, float ticksPerSecond, unsigned int fps)
{
	//should put values in container ~u_map<id, animation>
	mAnimTicks = nTicks;
	const auto duration = nTicks/ticksPerSecond;
	mTicksPerFrame = nTicks/(fps * duration);
}

std::unique_ptr<NodeAi<ModelAi>> ModelAi::MakeNode(const aiNode& node) 
{
	namespace dx = DirectX;

	dx::XMMATRIX transform = dx::XMMATRIX(&node.mTransformation.a1);
	int index = -1;
	std::string nodename = node.mName.C_Str();

	if (boneToIndex.find(nodename) != boneToIndex.end())
	{
		index = boneToIndex.at(nodename);
	}

	auto pNode = std::make_unique<NodeAi<typename ModelAi>>(transform, index, nodename, *this);

	for (UINT i = 0; i < node.mNumChildren; i++)
	{
		pNode->AddChild(MakeNode(*node.mChildren[i]));
	}

	return pNode;
}

void ModelAi::MakeNodeTree(const aiNode& node, const UINT nodeIndex)
{
	namespace dx = DirectX;

	if (nodeIndex == 0)
	{
		mNodes.push_back(MakeNodeAi(node));
	}

	const auto nNodes = (UINT)mNodes.size();

	for (UINT i = 0; i < node.mNumChildren; i++)
	{
		mNodes[nodeIndex]->AddChild(nNodes+i);
		mNodes.push_back(MakeNodeAi(*node.mChildren[i]));
	}
	for (UINT i = 0; i < node.mNumChildren; i++)
	{
		MakeNodeTree(*node.mChildren[i], nNodes+i);
	}
}

std::unique_ptr<NodeAi<ModelAi>> ModelAi::MakeNodeAi(const aiNode& node)
{
	namespace dx = DirectX;

	dx::XMMATRIX transform = dx::XMMATRIX(&node.mTransformation.a1);
	int index = -1;
	std::string nodename = node.mName.C_Str();

	if (boneToIndex.find(nodename) != boneToIndex.end())
	{
		index = boneToIndex.at(nodename);
	}

	return std::make_unique<NodeAi<typename ModelAi>>(transform, index, nodename, *this);
}


std::unique_ptr<MeshAi> ModelAi::MakeMesh(Graphics& gfx, const std::unordered_map<size_t, std::wstring>& meshToTexture, const aiMesh& mesh)
{
	std::vector<int> indices;
	std::vector<Vertex> vertices;
	vertices.reserve(mesh.mNumVertices);
	indices.reserve((long long)mesh.mNumFaces * 3);

	InitVerticesPosNorm(mesh, vertices);
	InitVerticesBoneWeight(mesh, vertices);
	InitIndices(mesh, indices);

	if (!mesh.HasNormals())
	{
		InitNormals(vertices, indices);
	}

	std::vector<std::unique_ptr<Bindable>> bindablePtrs;
	bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, vertices));
	bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices));

	auto pvs = std::make_unique<VertexShader>(gfx, L"PhongSkinningVS.cso");
	auto pvsbc = pvs->GetBytecode();
	bindablePtrs.push_back(std::move(pvs));
	bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"matPS.cso"));

	struct PSMeshMaterial
	{
		DirectX::XMFLOAT4 diffuseColor = { 0.5f,0.5f ,0.5f, 1.0f };
		DirectX::XMFLOAT4 specularColor = { 0.1f,0.1f ,0.1f, 1.0f };
		float specularIntensity = 0.0f;
		float specularPower = 10.0f;
		BOOL hasNormalMap = FALSE;
		BOOL hasDiffuseTexture = TRUE;
	} pmc;

	bindablePtrs.push_back(std::make_unique<PixelConstantBuffer<PSMeshMaterial>>(gfx, pmc, 1u));

	std::wstring wstr = meshToTexture.at((size_t)mesh.mMaterialIndex);
	bindablePtrs.push_back(std::make_unique<Texture>(gfx, wstr, 0u));
	bindablePtrs.push_back(std::make_unique<Sampler>(gfx));
	bindablePtrs.push_back(std::make_unique<Sampler>(gfx, 1u));
	bindablePtrs.push_back(std::make_unique<Sampler>(gfx, 2u));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BLENDINDICES",0,DXGI_FORMAT_R32G32B32A32_SINT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BLENDWEIGHT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, ied, pvsbc));
	bindablePtrs.push_back(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));

	//-----------TESTING SHADOWING -------------//
	std::vector<std::unique_ptr<Bindable>> bindableEffectPtrs;

	auto pevs = std::make_unique<VertexShader>(gfx, L"PhongSkinningVS.cso");
	auto pevsbc = pevs->GetBytecode();
	bindableEffectPtrs.push_back(std::move(pevs));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> t_eied =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	};

	bindableEffectPtrs.push_back(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));
	bindableEffectPtrs.push_back(std::make_unique<InputLayout>(gfx, ied, pevsbc));
	//-------------------------------------------//

	return std::make_unique<MeshAi>(gfx, std::move(bindablePtrs), std::move(bindableEffectPtrs), mVSBoneMatrices, mBoneOffsetMatrices);
}

void ModelAi::InitIndices(const aiMesh& mesh, std::vector<int>& indices) const
{
	for (size_t i = 0; i < mesh.mNumFaces; i++)
	{
		indices.push_back(mesh.mFaces[i].mIndices[0]);
		indices.push_back(mesh.mFaces[i].mIndices[1]);
		indices.push_back(mesh.mFaces[i].mIndices[2]);
	}
}

void ModelAi::InitNormals(std::vector<Vertex>& vertices, std::vector<int>& indices) const
{
	using namespace DirectX;

	std::vector<UINT> count(vertices.size());

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		//calculate normal of primitive
		const DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&vertices[indices[i+0]].position);
		const DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&vertices[indices[i+1]].position);
		const DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&vertices[indices[i+2]].position);

		XMVECTOR face_normal = XMVector3Cross(p1 - p0, p2 - p0);

		//add calculated normal to every vertice in the primitive and keep count of the amount of primitives that uses the vertice for normal averaging
		for (size_t j = 0; j < 3; j++)
		{
			const auto index = indices[i+j];
			DirectX::XMStoreFloat3(&vertices[index].normal,
				(DirectX::XMLoadFloat3(&vertices[index].normal) + face_normal)
			);
			count[index]++;
		}
	}
	//Normal averaging
	for (size_t i = 0; i < vertices.size(); i++)
	{
		auto nCount = float(count[i]);
		DirectX::XMStoreFloat3(&vertices[i].normal,
			XMVector3Normalize((DirectX::XMLoadFloat3(&vertices[i].normal) / XMVectorSet(nCount, nCount, nCount, 1.0f)))
		);
	}
}

void ModelAi::InitVerticesBoneWeight(const aiMesh& mesh, std::vector<Vertex>& vertices) const
{
	for (size_t v_id = 0; v_id < vertices.size(); v_id++)
	{
		auto id = 0;

		for (size_t b_id = 0; b_id < mesh.mNumBones; b_id++)
		{
			if (id > 3)
				break;

			std::string bonename = mesh.mBones[b_id]->mName.C_Str();

			const auto bone = mesh.mBones[b_id];
			//const auto boneId = boneIdMap.at(bonename);
			const auto boneId = boneToIndex.at(bonename);
			for (size_t w_id = 0; w_id < bone->mNumWeights; w_id++)
			{
				const auto weightData = bone->mWeights[w_id];
				if (weightData.mVertexId == v_id)
				{
					vertices[v_id].bones[id] = (int)boneId;
					vertices[v_id].weights[id++] = weightData.mWeight;
				}
			}
		}
	}
}

void ModelAi::InitVerticesPosNorm(const aiMesh& mesh, std::vector<Vertex>& vertices)
{
	namespace dx = DirectX;

	for (unsigned int v_id = 0; v_id < mesh.mNumVertices; v_id++)
	{
		dx::XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };
		if (mesh.HasNormals())
		{
			normal = *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[v_id]);
		}
		dx::XMFLOAT2 tc = { 0.0f, 0.0f };

		if (mesh.HasTextureCoords(0))
		{
			tc = *reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][v_id]);
		}
		
		auto largest = std::max(std::max(abs(mesh.mVertices[v_id].x), abs(mesh.mVertices[v_id].y)), abs(mesh.mVertices[v_id].z));
		if (largest*mScale/2 > mBoundingBoxRadius)
		{
			mBoundingBoxRadius = largest*mScale/2;
		}
		vertices.push_back({ { mesh.mVertices[v_id].x,
							   mesh.mVertices[v_id].y,
							   mesh.mVertices[v_id].z },
							   normal,
							   tc }
		);
	}
}

void ModelAi::CalcNodeTransformation(DirectX::XMMATRIX& nodeTransform, std::string& nodeName, float animTime)
{
	using namespace DirectX;

	if (nameToPos.find(nodeName) == nameToPos.end() ||
		nameToScale.find(nodeName) == nameToScale.end() ||
		nameToRot.find(nodeName) == nameToRot.end())
	{
		return;
	}

	aiVector3D scale;
	CalcInterpolatedScaling(scale, nodeName, animTime);
	XMMATRIX scaling = XMMatrixScaling(scale.x, scale.y, scale.z);

	aiQuaternion qRot;
	CalcInterpolatedRotation(qRot, nodeName, animTime);
	XMMATRIX rotation = XMMatrixRotationQuaternion(XMVectorSet(qRot.x, qRot.y, qRot.z, qRot.w));

	aiVector3D transl;
	CalcInterpolatedTranslation(transl, nodeName, animTime);
	XMMATRIX translation = XMMatrixTranslation(transl.x, transl.y, transl.z);

	nodeTransform = scaling * rotation * translation;
	nodeTransform = XMMatrixTranspose(nodeTransform);
}

void ModelAi::CalcInterpolatedScaling(aiVector3D& outV, std::string& nodeName, float animationTime) const
{
	if (nameToScale.at(nodeName).size() == 1)
	{
		outV = nameToScale.at(nodeName)[0].mValue;
		return;
	}
	unsigned int i = 0;
	const auto& vec = nameToScale.at(nodeName);
	while (++i < vec.size()-1 && vec[i].mTime < animationTime);

	aiVector3D _V1 = vec[i-1].mValue;
	aiVector3D _V2 = vec[i].mValue;

	float _t1 = vec[i-1].mTime;
	float _t2 = vec[(i % vec.size())].mTime;
	float deltaT = _t2 - _t1;
	float factorV1 = (animationTime - _t1) / deltaT;
	float factorV2 = 1.0f - factorV1;

	outV.x = (factorV1 * _V1.x) + (factorV2 * _V2.x);
	outV.y = (factorV1 * _V1.y) + (factorV2 * _V2.y);
	outV.z = (factorV1 * _V1.z) + (factorV2 * _V2.z);
}

void ModelAi::CalcInterpolatedTranslation(aiVector3D& outV, std::string& nodeName, float animationTime) const
{
	if (nameToPos.at(nodeName).size() == 1)
	{
		outV = nameToPos.at(nodeName)[0].mValue;
		return;
	}
	unsigned int i = 0;
	const auto& vec = nameToPos.at(nodeName);
	while (++i < vec.size()-1 && vec[i].mTime < animationTime);

	aiVector3D _V1 = vec[i-1].mValue;
	aiVector3D _V2 = vec[i].mValue;

	float _t1 = vec[i-1].mTime;
	float _t2 = vec[(i % vec.size())].mTime;
	float deltaT = _t2 - _t1;
	float factorV1 = (animationTime - _t1) / deltaT;
	float factorV2 = 1.0f - factorV1;

	outV.x = (factorV1 * _V1.x) + (factorV2 * _V2.x);
	outV.y = (factorV1 * _V1.y) + (factorV2 * _V2.y);
	outV.z = (factorV1 * _V1.z) + (factorV2 * _V2.z);
}

void ModelAi::CalcInterpolatedRotation(aiQuaternion& outQ, std::string& nodeName, float animationTime) const
{
	if (nameToRot.at(nodeName).size() == 1)
	{
		outQ = nameToRot.at(nodeName)[0].mValue;
		return;
	}
	unsigned int i = 0;
	const auto& vec = nameToRot.at(nodeName);
	while (++i < vec.size()-1 && vec[i].mTime < animationTime);

	aiQuaternion _Q1 = vec[i-1].mValue;
	aiQuaternion _Q2 = vec[i].mValue;

	float _t1 = vec[i-1].mTime;
	float _t2 = vec[(i % vec.size())].mTime;
	float deltaT = _t2 - _t1;
	float factor = (animationTime - _t1) / deltaT;

	assert(factor >= 0.0f && factor <= 1.0f);

	aiQuaternion::Interpolate(outQ, _Q1, _Q2, factor); // SLERP
	outQ = outQ.Normalize();
}

void ModelAi::SpawnControlWindow()
{
	if (ImGui::Begin("Bones"))
	{
		//pRoot->SpawnControlWindow();
		if (ImGui::BeginCombo("", mNodes[mSelectedBone]->GetName().c_str()))
		{
			for (int i = 0; i < std::size(mNodes); i++)
			{
				if (ImGui::Selectable(mNodes[i]->GetName().c_str(), (i == mSelectedBone)))
				{
					mSelectedBone = i;
				}
			}
			ImGui::EndCombo();
		}
		mNodes[mSelectedBone]->SpawnControlWindow();
	}
}