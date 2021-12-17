#pragma once

#include "BaseBindables.h"	
#include "BaseDrawables.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <unordered_map>

#include "Mnode.h"
#include "Mmesh.h"

class Mbase
{
public:
	Mbase(Graphics& gfx, const std::string fileName, bool RHcoords, float scale)
	{
		Assimp::Importer imp;
		//std::string path = "md5models\\boy.md5mesh";

		std::string path = "md5models\\boblampclean\\boblampclean.md5mesh";
		const auto pScene = imp.ReadFile(path,
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded
		);
		
		if (pScene->HasAnimations())
		{
			InitKeyMaps(pScene->mAnimations[0]);
		}

		InitBoneMap(pScene);

		/*auto roottransform = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(
			reinterpret_cast<const DirectX::XMFLOAT4X4*>(&pScene->mRootNode->mTransformation)
		));*/
		/*auto det = DirectX::XMMatrixDeterminant(roottransform);
		roottransform = DirectX::XMMatrixInverse(&det, roottransform);*/
		//DirectX::XMStoreFloat4x4(&inverseRoot, roottransform);

		//SOMETHING IN THE WATER
		auto asd = pScene->mRootNode->mTransformation.Transpose();
		asd.Inverse();
		auto roottransform = DirectX::XMMATRIX(&asd.a1);
		//auto roottransform = DirectX::XMMATRIX(&pScene->mRootNode->mTransformation.a1);
		DirectX::XMStoreFloat4x4(&inverseRoot, roottransform);
		//SOMETHING IN THE WATER

		InitTrees(pScene->mRootNode);

		for (size_t i = 0; i < nodeTree.size(); i++)
			nodeIdMap.insert({ nodeTree[i].name, i });

		for (size_t i = 0; i < boneTree.size(); i++)
			boneIdMap.insert({ boneTree[i].name, i });

		for (size_t i = 0; i < boneTree.size(); i++)
		{
			DirectX::XMFLOAT4X4 tmp = {};
			DirectX::XMFLOAT4X4 transp = {};
			DirectX::XMStoreFloat4x4(&tmp, boneTree[i].transform);
			DirectX::XMStoreFloat4x4(&transp, DirectX::XMMatrixTranspose(boneTree[i].transform));
			vsBoneMatrices.push_back(tmp);
			boneOffsetMatrices.push_back(tmp);
		}

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
			meshPtrs.push_back(MakeMesh(gfx, scale, *pScene->mMeshes[i]));

		pRoot = MakeNode(*pScene->mRootNode);

		//boneStringMap.clear();
	}
	//TEST FOR NOW
	void Draw(Graphics& gfx, DirectX::FXMMATRIX transform, float animation)
	{
		auto identity = DirectX::XMMatrixIdentity();
		
		if ((int)animationTime == 139) 
			animationTime = 0.0f;
		pRoot->Draw(gfx, identity, animationTime++);
		//pRoot->Draw(gfx, transform, animationTime++);
		/*for (size_t i = 0; i < meshPtrs.size(); i++)
		{
			meshPtrs[i]->Draw(gfx, transform, mMatrix);
		}*/
	}

	void Draw(Graphics& gfx, DirectX::FXMMATRIX transform) const
	{
		if (firstTime)
		{
			pRoot->Draw(gfx, DirectX::XMLoadFloat4x4(&inverseRoot));
			firstTime = false;
		}	
		else
		{
			if ((int)animationTime == 139) animationTime = 0.0f;
			/*if (++framedelay > 10)
			{
				animationTime++;
				framedelay = 0;
			}*/
			pRoot->Draw(gfx, DirectX::XMLoadFloat4x4(&inverseRoot), animationTime++);
		}
	}

	DirectX::XMFLOAT4X4& GetInverseRootMatrix() noexcept
	{
		return inverseRoot;
	}
	std::vector<DirectX::XMFLOAT4X4>& GetVSBoneMatrices() noexcept
	{
		return vsBoneMatrices;
	}
	DirectX::XMFLOAT4X4& GetVSBoneMatrix(int id) noexcept
	{
		return vsBoneMatrices[id];
	}
	std::vector<DirectX::XMFLOAT4X4>& GetBoneOffsetMatrices() noexcept
	{
		return boneOffsetMatrices;
	}
	DirectX::XMFLOAT4X4& GetBoneOffsetMatrix(int index) noexcept
	{
		return boneOffsetMatrices[index];
	}
private:
	void InitializeData(Graphics& gfx, const std::string fileName, float scale = 1.0f);
	void InitBoneMap(const aiScene* pScene);
	void InitTrees(aiNode* ainode, int parentindex = -1, int boneparentindex = -1);
	
	std::unique_ptr<Mnode<Mbase>> MakeNode(const aiNode& node)
	{
		namespace dx = DirectX;

		/*const auto transform = dx::XMMatrixTranspose(dx::XMLoadFloat4x4(
			reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
		));*/

		dx::XMMATRIX transform = dx::XMMATRIX(&node.mTransformation.a1);
		//transform = dx::XMMatrixTranspose(transform);

		std::vector<Mmesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);

		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			curMeshPtrs.push_back(meshPtrs.at(node.mMeshes[i]).get());
		}

		std::string nodename = node.mName.C_Str();
		auto index = -1;
		for (size_t i = 0; i < boneTree.size(); i++)
		{
			if (nodename == boneTree[i].name)
			{
				index = (int)i;
				break;
			}
		}

		auto pNode = std::make_unique<Mnode<typename Mbase>>(std::move(curMeshPtrs), transform, index, nodename, *this);

		for (UINT i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(MakeNode(*node.mChildren[i]));
		}
		count++;
		return pNode;
	}
	std::unique_ptr<Mmesh> MakeMesh(Graphics& gfx, float scale, const aiMesh& mesh)
	{
		namespace dx = DirectX;

		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 normal;
			int bones[4] = { -1, -1, -1, -1 };
			float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		};

		std::vector<Vertex> vertices;
		vertices.reserve(mesh.mNumVertices);

		InitVerticesPosNorm(mesh, vertices, scale);
		InitVerticesBoneWeight(mesh, vertices);

		/*
		//set pos and normal
		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			dx::XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };
			if (mesh.HasNormals())
			{
				normal = *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]);
			}

			vertices.push_back({ { mesh.mVertices[i].x * scale,
								   mesh.mVertices[i].y * scale,
								   mesh.mVertices[i].z * scale },
								   normal
			});
		}
		//set weight and boneid ( nodeID)
		for (size_t v_id = 0; v_id < vertices.size(); v_id++)
		{
			auto id = 0;

			for (size_t b_id = 0; b_id < mesh.mNumBones; b_id++)
			{
				if (id > 3) 
					break;

				const auto bonename = mesh.mBones[b_id]->mName.C_Str();
				const auto nodeId = nodeIdMap.at(bonename);
				const auto bone = &boneStringMap.at(bonename);
				//const auto boneId = &boneIdMap.at(bonename);
				for (size_t w_id = 0; w_id < bone->mNumWeights; w_id++)
				{
					const auto weightData = bone->mWeights[w_id];
					if (weightData.mVertexId == v_id )
					{
						vertices[v_id].bones[id] = (int)nodeId;
						vertices[v_id].weights[id++] = weightData.mWeight;
					}
				}
			}
		}
		*/

		std::vector<int> indices;
		indices.reserve((long long)mesh.mNumFaces * 3);

		InitIndices(mesh, indices);
		/*
		for (size_t i = 0; i < mesh.mNumFaces; i++)
		{
			indices.push_back(mesh.mFaces[i].mIndices[0]);
			indices.push_back(mesh.mFaces[i].mIndices[1]);
			indices.push_back(mesh.mFaces[i].mIndices[2]);
		}
		*/

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

		bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));
		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32B32A32_SINT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"BLENDWEIGHT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		};
		bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, ied, pvsbc));

		return std::make_unique<Mmesh>(gfx, std::move(bindablePtrs), vsBoneMatrices, boneOffsetMatrices);
	}
	
	template <typename T>	//set pos and normal
	void InitVerticesPosNorm(const aiMesh& mesh, std::vector<T>& vertices, float scale = 1.0f)
	{
		namespace dx = DirectX;

		for (unsigned int v_id = 0; v_id < mesh.mNumVertices; v_id++)
		{
			dx::XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };
			if (mesh.HasNormals())
			{
				normal = *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[v_id]);
			}

			vertices.push_back({ { mesh.mVertices[v_id].x * scale,
								   mesh.mVertices[v_id].y * scale,
								   mesh.mVertices[v_id].z * scale },
								   normal }
			);
		}
	}

	template <typename T>	//set bone reference and weight value
	void InitVerticesBoneWeight(const aiMesh& mesh, std::vector<T>& vertices)
	{
		for (size_t v_id = 0; v_id < vertices.size(); v_id++)
		{
			if (v_id != 0)
			{
				float blyat = vertices[v_id-1].weights[0] +
					vertices[v_id-1].weights[1]+
					vertices[v_id-1].weights[2]+
					vertices[v_id-1].weights[3];

				assert(blyat == 1.0f);
			}
			auto id = 0;

			for (size_t b_id = 0; b_id < mesh.mNumBones; b_id++)
			{
				if (id > 3)
					break;

				const auto bonename = mesh.mBones[b_id]->mName.C_Str();
				//const auto nodeId = nodeIdMap.at(bonename);
				//const auto vbone = &boneStringMap.at(bonename);
				const auto bone = mesh.mBones[b_id];
				const auto boneId = boneIdMap.at(bonename);
				for (size_t w_id = 0; w_id < bone->mNumWeights; w_id++)
				{
					const auto weightData = bone->mWeights[w_id];
					if (weightData.mVertexId == v_id)
					{
						//vertices[v_id].bones[id] = (int)nodeId;
						vertices[v_id].bones[id] = (int)boneId;
						if (vertices[v_id].bones[id] == 6)
						{
							float blyat = weightData.mWeight;
							int x = 0;
						}
						vertices[v_id].weights[id++] = weightData.mWeight;
					}
				}
			}
		}
	}

	template <typename T>	//if model does not have normals, calculate normals
	static void InitNormals(std::vector<T>& vertices, std::vector<int> indices)
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

	void InitIndices(const aiMesh& mesh, std::vector<int>& indices) const;

private:
	struct boneNode
	{
		std::string name;
		int parent_index;
		DirectX::XMMATRIX transform;
	};
	struct node
	{
		std::string name;
		int parent_index;
		bool hasBone;
		DirectX::XMMATRIX parent_tf;
	};
private:
	struct mPosKey
	{
		float mTime;
		DirectX::XMFLOAT3 mValue;
		mPosKey(float t, DirectX::XMFLOAT3 v) : mTime(t), mValue(v) {}
	};
	struct mScaleKey
	{
		float mTime;
		DirectX::XMFLOAT3 mValue;
		mScaleKey(float t, DirectX::XMFLOAT3 v) : mTime(t), mValue(v) {}
	};
	struct mRotKey
	{
		float mTime;
		DirectX::XMFLOAT4X4 mValue;
		mRotKey(float t, DirectX::XMFLOAT4X4 v) : mTime(t), mValue(v) {}
	};

	std::unordered_map<std::string, std::vector<mScaleKey>> nameToScale;
	std::unordered_map<std::string, std::vector<mPosKey>> nameToPos;
	std::unordered_map<std::string, std::vector<mRotKey>> nameToRot;

	void InitKeyMaps(aiAnimation* animation)
	{
		using namespace DirectX;

		for (size_t ch_i = 0; ch_i < animation->mNumChannels; ch_i++)
		{
			const auto channel = animation->mChannels[ch_i];

			std::vector<mPosKey> posKeys;
			std::vector<mRotKey> rotKeys;
			std::vector<mScaleKey> scaKeys;
			posKeys.reserve(channel->mNumPositionKeys);
			rotKeys.reserve(channel->mNumRotationKeys);
			scaKeys.reserve(channel->mNumScalingKeys);

			for (size_t i = 0; i < posKeys.capacity(); i++)
			{
				const auto aiKeys = channel->mPositionKeys[i];
				
				posKeys.emplace_back(
					(float)aiKeys.mTime,
					XMFLOAT3{aiKeys.mValue.x, aiKeys.mValue.y, aiKeys.mValue.z}
				);
			}
			for (size_t i = 0; i < scaKeys.capacity(); i++)
			{
				const auto aiKeys = channel->mScalingKeys[i];
				
				scaKeys.emplace_back(
					(float)aiKeys.mTime,
					XMFLOAT3{ aiKeys.mValue.x, aiKeys.mValue.y, aiKeys.mValue.z }
				);
			}
			for (size_t i = 0; i < rotKeys.capacity(); i++)
			{
				const auto aiKeys = channel->mRotationKeys[i];
				mQuat = aiKeys.mValue;
				XMMATRIX rotationM = XMMatrixRotationQuaternion(
					XMVectorSet(aiKeys.mValue.x, aiKeys.mValue.y, aiKeys.mValue.z, aiKeys.mValue.w));
				//rotationM = XMMatrixTranspose(rotationM);
				XMFLOAT4X4 mat2 = {};
				XMStoreFloat4x4(&mat2, rotationM);
				//auto aiRotMat = aiKeys.mValue.GetMatrix().Inverse();
				////aiRotMat.Transpose();
				//auto test = *reinterpret_cast<XMFLOAT3X3*>(&aiRotMat);
				//auto mat = XMLoadFloat3x3(&test);
				//XMFLOAT4X4 mat2 = {};
				//XMStoreFloat4x4(&mat2, mat);

				rotKeys.emplace_back(
					(float)aiKeys.mTime, mat2
				);
			}

			std::string nodeName = channel->mNodeName.C_Str();
			nameToPos.insert({ nodeName, posKeys });
			nameToRot.insert({ nodeName, rotKeys });
			nameToScale.insert({ nodeName, scaKeys });
		}
	}
public:
	/*DirectX::XMMATRIX*/void CalcNodeTransformation(DirectX::XMMATRIX& ntf, std::string nodeName, float _animationTime)
	{
		using namespace DirectX;
		if (nameToPos.find(nodeName) == nameToPos.end() ||
			nameToScale.find(nodeName) == nameToScale.end() ||
			nameToRot.find(nodeName) == nameToRot.end())
		{
			return;
		}
			
		XMMATRIX pos = {};
		XMMATRIX rot = {};
		XMMATRIX scale = {};

		if (nameToPos.find(nodeName) != nameToPos.end())
		{
			unsigned int i = 0;
			const auto& vec = nameToPos.at(nodeName);
			while (++i < vec.size()-1 && vec[i].mTime < _animationTime);

			pos = XMMatrixTranslation(vec[i-1].mValue.x, vec[i-1].mValue.y, vec[i-1].mValue.z);
		}
		if (nameToRot.find(nodeName) != nameToRot.end())
		{
			unsigned int i = 0;
			const auto& vec = nameToRot.at(nodeName);
			while (++i < vec.size()-1 && vec[i].mTime < _animationTime);

			rot = XMLoadFloat4x4(&vec[i-1].mValue);
		}
		if (nameToScale.find(nodeName) != nameToScale.end())
		{
			unsigned int i = 0;
			const auto& vec = nameToScale.at(nodeName);
			while (++i < vec.size()-1 && vec[i].mTime < _animationTime);

			scale = XMMatrixScaling(vec[i-1].mValue.x, vec[i-1].mValue.y, vec[i-1].mValue.z);
		}
		
		auto combined = scale * rot * pos; //SHOULD BE THIS ONE
		ntf = XMMatrixTranspose(combined);
		//auto combined = XMMatrixTranspose(scale) * XMMatrixTranspose(rot) * XMMatrixTranspose(pos);
		//ntf = XMMatrixTranspose(combined);
		//auto combined = scale * pos * rot ;
		//auto combined = pos * scale *  rot ;
		//auto combined = pos * rot   * scale;
		//auto combined = rot * pos   * scale;
		//auto combined = rot * scale * pos  ;
		//return combined;
	}
private:
	DirectX::XMFLOAT4X4 inverseRoot;
	std::vector<DirectX::XMFLOAT4X4> boneOffsetMatrices;
	std::vector<DirectX::XMFLOAT4X4> vsBoneMatrices;
	aiQuaternion mQuat;
	std::unordered_map<std::string, aiNodeAnim> nodeFrame;
	//std::unordered_map<std::string, animationFrame> frameData;
	//std::unordered_map<std::pair<std::string, unsigned int>, float> weightMap;
	std::unordered_map<std::string, aiBone> boneStringMap;
	std::unordered_map<std::string, size_t> boneIdMap;
	std::unordered_map<std::string, size_t> nodeIdMap;

	std::vector<boneNode> boneTree;
	std::vector<node> nodeTree;

	DirectX::XMFLOAT4X4 mMatrix;

	std::unique_ptr<Mnode<Mbase>> pRoot;
	std::vector<std::unique_ptr<Mmesh>> meshPtrs;
	int count = 0;
	mutable float animationTime = 0.0f;
	mutable float framedelay = 0.0f;
	mutable bool firstTime = true;
};
