#pragma once
#include "DrawableBase.h"

#include <vector>
#include <array>
#include "IndexedObjTriangleList.h"
#include "MaMath.h"

class ModelData
{
public:
	ModelData() = default;
	ModelData(std::wstring filename, const bool isRHcoords, float scale);
	~ModelData() = default;
private:
	void InitData();
	void WriteCopperData(std::wistream& modelfile);
	void ReadCopperData(std::wistream& file);
	void InitMeshes(std::wistream& file);
	void InitNormals();
	void InitMaterial(std::wstring& mtllib);
	template <typename Vi>
	void SetFace(std::wistream& file, std::vector<Vi>& verts, CprArray<int>& checkV, int& index)
	{
		/*Reads line starting with 'f', checks if vertices has already been loaded and fills up the Indice vector for the model*/
		wchar_t valBuf[5] = {};
		wchar_t wchar = file.get();
		int faceBuf[15] = { 0 };
		UINT fvn = 0;			//fvn keeps track of which vertice in the face
		UINT part = 0;			//vertice format (0=pos/1=tc/2=norm) 

		while (wchar != '\n' && file) //read face line
		{
			UINT  vbi = 0;		//vbi = valBuf index,
			while (wchar != ' ' && wchar != '/' && wchar != '\n') //Read next value to valbuf
			{
				valBuf[vbi++] = wchar;
				wchar = file.get();
			}
			faceBuf[fvn*3+part] = (wcstol(valBuf, 0, 10)) ? wcstol(valBuf, 0, 10)-1 : 0; //store read value in facebuf ('-1' since *.obj start at 1)
			if (wchar == '/') //same vertice, update part
			{
				part++;
				wchar = file.get();
			}
			else if (wchar == ' ') //new vertice, reset part, increment face vertex index
			{
				part = 0;
				fvn++;
				wchar = file.get();
			}
			std::fill(std::begin(valBuf), std::end(valBuf), 0);
		}
		fvn++; // increment number of vertices did not fulfill "else if clause"

		UINT first = {}, last = {};
		for (size_t i = 0; i < fvn; i++) //for every vertice on this line
		{
			bool add = true;
			int faceVindex = 0;
			int alreadyAdded = checkV(faceBuf[i*3], faceBuf[i*3+1]);

			if (!alreadyAdded)
			{
				faceVindex = index++;
				checkV(faceBuf[i*3], faceBuf[i*3+1]) = index;
			}
			else
			{
				faceVindex = --alreadyAdded;
				add = false;
			}
			if (add)
			{
				verts.emplace_back(faceBuf[i*3], faceBuf[i*3+1], faceBuf[i*3+2]);
			}
			if (i < 2)
			{
				if (i == 0) 
					first = faceVindex;
				else 
					last = faceVindex;
			}
			else
			{
				indices.emplace_back(first);
				indices.emplace_back(last);
				indices.emplace_back(faceVindex);
				last = faceVindex;
			}
		}
	}
	void SetVert(std::wistream& file);
	void UseLib(std::wistream& file, int mesh_n);
	void SetColor(std::wistream& mtlFile, wchar_t& pchar, UINT& matIndex, std::vector<UINT>& mesh_index, bool& difColorSet);
	void SetTransparency(std::wistream& mtlFile, wchar_t& pchar, UINT& matIndex, std::vector<UINT>& mesh_index);
	void SetDiffuseMap(std::wistream& mtlFile, UINT& matIndex, std::vector<UINT>& mesh_index);
	void SetNormalMap(std::wistream& mtlFile, UINT& matIndex, std::vector<UINT>& mesh_index);
private:
	std::wstring filePath;
	std::wstring fileName;
	bool isRH;
	float scale;
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoord;
		DirectX::XMFLOAT3 normal;
		Vertex() : position({ 0.0f, 0.0f, 0.0f }), texCoord({ 0.0f, 0.0f }), normal({ 0.0f, 0.0f, 0.0f }) {}

		Vertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 texCoord, DirectX::XMFLOAT3 normal)
			: position(position), texCoord(texCoord), normal(normal) {}
	};
	struct SurfaceMaterial
	{
		std::wstring matName;
		DirectX::XMFLOAT4 difColor;
		DirectX::XMFLOAT4 specColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		float specPower = 0.0f;
		UINT texIndex;
		UINT nMapIndex = {};
		bool hasTexture;
		bool hasNormalMap = false;
		bool isTransparent;
		SurfaceMaterial()
			:
			matName(L""), hasTexture(false), isTransparent(false), texIndex(0), difColor(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)) {}
		SurfaceMaterial(std::wstring& name)
			:
			matName(name), hasTexture(false), isTransparent(false), texIndex(0), difColor(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)) {}
		SurfaceMaterial(std::wstring& name, bool tex, bool transp, UINT texIndex, DirectX::XMFLOAT4 difc)
			:
			matName(name), hasTexture(tex), isTransparent(transp), texIndex(texIndex), difColor(difc)
		{}
	};
	struct Material
	{
		std::wstring  matName = L"";
		std::wstring  texName = L"";
		std::wstring nMapName = L"";
		DirectX::XMFLOAT4 diffColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4 specColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		float specularPower = 0.0f;
		bool hasTexture = false;
		bool hasNormMap = false;
		bool isTransparent = false;
	};
public:
	std::vector<Vertex> vertices;
	std::vector<int> indices = {};
	std::vector<std::wstring> texNames;
	std::vector<std::wstring> nMapNames;
private:
	void SetBoundingBoxRadius()
	{
		auto radius = 0.0f;
		auto max = meshes[0].vertices[0].pos;
		auto min = meshes[0].vertices[0].pos;

		for (auto& m : meshes)
		{
			for (auto& v : m.vertices)
			{
				max.x = v.pos.x < max.x ? max.x : v.pos.x;
				max.y = v.pos.y < max.y ? max.y : v.pos.y;
				max.z = v.pos.z < max.z ? max.z : v.pos.z;
				min.x = v.pos.x > min.x ? min.x : v.pos.x;
				min.y = v.pos.y > min.y ? min.y : v.pos.y;
				min.z = v.pos.z > min.z ? min.z : v.pos.z;
			}
		}
		radius = (std::max(std::max(max.x-min.x, max.y - min.y), max.z-min.z))/2;

		if (radius > boundingBoxRadius)
		{
			boundingBoxRadius = radius;
		}
	}
	std::vector<SurfaceMaterial> materials;
	std::vector<DirectX::XMFLOAT3> vpos;
	std::vector<DirectX::XMFLOAT2> vtc;
	std::vector<DirectX::XMFLOAT3> vnorm;
	
	struct Mesh
	{
		struct Vertx
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 tc;
			DirectX::XMFLOAT3 norm;
			Vertx(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 texCoord, DirectX::XMFLOAT3 normal)
				: pos(position), tc(texCoord), norm(normal) {}
		};
		std::vector<Vertx> vertices;
		std::vector<int> indices;
		Material material;
		//unsigned int materialRef;//can be removed
		//unsigned int textureRef;//can be remoevd

		Mesh(unsigned int nVertices, unsigned int nIndices) /*: materialRef(0u), textureRef(0u)*/
		{
			vertices.reserve(nVertices);
			indices.reserve(nIndices);
		}
	};
public:
	float boundingBoxRadius = 0.0f;
	std::vector<Mesh> meshes;
	std::vector<UINT> meshTree;
};