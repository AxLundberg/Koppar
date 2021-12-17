#include "ModelData.h"
#include <DirectXMath.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


ModelData::ModelData(std::wstring filename, const bool isRHcoords, float scale)
	:
	filePath(L"objmodels\\" + filename + L"\\"),
	fileName(filename),
	isRH(isRHcoords),
	scale(scale)
{
	InitData();
}

void ModelData::InitData()
{
	std::wifstream inFile(filePath + fileName + L".obj");
	wchar_t buf[] = L"<<<CopperData>>>";
	UINT proceed = 0u;
	while (proceed < wcslen(buf) && buf[proceed] == inFile.get() )
		proceed++;
	while (inFile.get() != '\n' && inFile); //newline
	//check if file has data
	if (proceed == wcslen(buf)) 
	{
		ReadCopperData(inFile);
		InitMeshes(inFile);
		inFile.close();
	}
	else
	{
		inFile.seekg(0);
		WriteCopperData(inFile);
		inFile.close();
		InitData();
	}

	auto Reset = [](std::wstring& file)
	{
		std::wifstream inFile(file);
		std::wstring line;
		inFile >> line;
		UINT count = (line == L"<<<CopperData>>>") ? 1u : 0u;
		while (line != L"<<<EndOfCopperData>>>" && count) //go past CopperData
			std::getline(inFile, line);

		//create a temporary file
		char tmpFile[L_tmpnam_s];
		tmpnam_s(tmpFile, L_tmpnam_s);
		std::wofstream wTmpFile(tmpFile);

		//append remaining contents of the obj file to the temporary file
		while (std::getline(inFile, line))
			wTmpFile << line << L"\n";

		inFile.close();
		wTmpFile.close();

		//clear obj file and copy contents from the temporary file to the obj file.
		std::wofstream wFile(file, std::wofstream::out | std::wofstream::trunc);
		std::wifstream RtmpFile(tmpFile);
		while (std::getline(RtmpFile, line))
		{
			wFile << line << L"\n";
		}
		RtmpFile.close();
		wFile.close();
	};
	std::wstring file = filePath + fileName + L".obj";
	//Reset(file);
	size_t rootLeaves = (meshes.size() > 1) ? meshes.size()-1 : 0;
	meshTree.push_back((UINT)rootLeaves);
	for (size_t i = 0; i < rootLeaves; i++)
		meshTree.push_back((UINT)0);

	if (meshes.size())
	{
		SetBoundingBoxRadius();
	}
}

void ModelData::InitMeshes(std::wistream& file)
{
	Timer t(L"init meshes");
	struct V
	{
		UINT pos_i;
		UINT tc_i;
		UINT norm_i;
		V(UINT p, UINT tc, UINT n) : pos_i(p), tc_i(tc), norm_i(n) {}
	};
	std::vector<V> verts;
	verts.reserve(vertices.capacity());
	CprArray<int> checkV(vpos.capacity(), vtc.capacity());
	//std::vector<unsigned int> checkV(vpos.capacity()*vtc.capacity(), -1); //lookup to see if vertice already added
	int tindex = 0;
	std::wstring mtllib;
	int mesh = -1;
	while (file)
	{
		wchar_t pchar = file.get();
		switch (pchar)
		{
		case 'v':
			SetVert(file);
			break;
		case 'f':
			file.get();
			SetFace(file, verts, checkV, tindex);
			break;
		case 'o':
			mesh++;
			break;
		case 'g':
			mesh++;
			break;
		case 'm':
			while (file.get() != ' ');
			pchar = file.get();
			while (pchar != '\n')
			{
				mtllib += pchar;
				pchar = file.get();
			}
			//file >> mtllib;
			break;
		case 'u':
			UseLib(file, mesh);
			while (file.get() != '\n' && file);
			break;
		case '\n':
			break;
		default:
			while (file.get() != '\n' && file);
			break;
		}
	}
	
	for (size_t i = 0; i < verts.size(); i++)
		vertices.emplace_back( vpos[verts[i].pos_i], vtc[verts[i].tc_i], vnorm[verts[i].norm_i] );

	if (vnorm.size() <= 1)
		InitNormals();

	//Following code sets every meshes individual vertice/indice buffer
	UINT i = 0u;
	int* buf = new int[vertices.capacity()];
	for (auto& m : meshes)
	{
		std::memset(buf, -1, sizeof(*buf) * vertices.size());
		/*for (size_t i = 0; i < 3; i++, index++)
		{
			m.vertices.emplace_back(vertices[indices[index]].position, vertices[indices[index]].texCoord, vertices[indices[index]].normal);
			m.indices.emplace_back(i);
			buf[indices[index]] = i;
		}
		for (size_t i = 3, j = 3; i < m.vertices.capacity(); i++, index++)
		{
			if (buf[indices[index]] != -1)
			{
				m.indices.emplace_back(buf[indices[index]]);
			}
			else
			{
				m.indices.emplace_back(j);
				m.vertices.emplace_back(vertices[indices[index]].position, vertices[indices[index]].texCoord, vertices[indices[index]].normal);
			}
		}*/
		for (size_t k = 0, j = 0; k < m.indices.capacity(); k++, i++)
		{
			if (k<3 || buf[indices[i]] == -1)
			{
				buf[indices[i]] = (int)k;
				m.indices.emplace_back((int)j);
				m.vertices.emplace_back(vertices[indices[i]].position, vertices[indices[i]].texCoord, vertices[indices[i]].normal);
				j++;
			}
			else
				m.indices.emplace_back(m.indices[buf[indices[i]]]);
		}
	}
	delete[] buf;
	if (mtllib != L"")
		InitMaterial(mtllib);
}

void ModelData::SetVert(std::wistream& file)
{
	wchar_t wchar = file.get();
	wchar_t valBuf[15] = {};
	float vals[3] = {};
	float tmp = 0.0f;
	switch (wchar)
	{
	case ' ': // "v " = position
		for (size_t i = 0, j = 0; i < 3; i++, j = 0)
		{
			while (wchar == ' ')
				wchar = file.get();
			while (wchar != ' ' && wchar != '\n')
			{
				valBuf[j] = wchar;
				wchar = file.get();
				j++;
			}
			vals[i] = wcstof(valBuf, 0);
			std::fill(std::begin(valBuf), std::end(valBuf), 0);
		}
		if (!isRH)
			vpos.emplace_back(DirectX::XMFLOAT3(vals[0]*scale, vals[1]*scale, vals[2]*scale));
		else
			vpos.emplace_back(DirectX::XMFLOAT3(vals[0]*scale, vals[2]*scale, -vals[1]*scale));
		break;
	case 't': // "vt" = texture coordinates
		for (size_t i = 0, j = 0; i < 2; i++, j = 0)
		{
			while (wchar == ' ' || wchar == 't')
				wchar = file.get();
			while (wchar != ' ' && wchar != '\n')
			{
				valBuf[j] = wchar;
				wchar = file.get();
				j++;
			}
			vals[i] = wcstof(valBuf, 0);
			std::fill(std::begin(valBuf), std::end(valBuf), 0);
		}
		vals[1] = (isRH) ? vals[1] : 1.0f-vals[1];
		vtc.emplace_back(DirectX::XMFLOAT2(vals[0], vals[1]));
		break;
	case 'n': // "vn" = normal
		for (size_t i = 0, j = 0; i < 3; i++, j = 0)
		{
			while (wchar == ' ' || wchar == 'n')
				wchar = file.get();
			while (wchar != ' ' && wchar != '\n')
			{
				valBuf[j] = wchar;
				wchar = file.get();
				j++;
			}
			vals[i] = wcstof(valBuf, 0);
			std::fill(std::begin(valBuf), std::end(valBuf), 0);
		}
		//vals[2] = (isRH) ? -vals[2] : vals[2];
		if (!isRH)
			vnorm.emplace_back(DirectX::XMFLOAT3(vals[0], vals[1], vals[2]));
		else
			vnorm.emplace_back(DirectX::XMFLOAT3(vals[0], vals[1], -vals[2]));
		break;
	default:
		break;
	}
	while (wchar!='\n' && file)
	{
		wchar = file.get();
	}
}
void ModelData::UseLib(std::wistream& file, int mesh_n)
{
	std::wstring compStr;
	file >> compStr;
	if (compStr == L"semtl")		//Make sure line specifies 'uselib'
	{
		std::wstring matName = L"";
		file >> matName;
		bool add = true;
		int i = -1;
		while (add && ++i < materials.size())
			add = (materials[i].matName == matName) ? false : true;
		if (add)
			materials.emplace_back(matName);
		meshes[mesh_n].material.matName = matName;
	}
};
void ModelData::SetColor(std::wistream& mtlFile, wchar_t& pchar, UINT& matIndex, std::vector<UINT>& mesh_index, bool& difColorSet)
{
	pchar = mtlFile.get(); //Diffuse Color is denoted by "Kd" Ambient Color by "Ka"
	if (pchar == 'd')
	{
		mtlFile >> materials[matIndex].difColor.x;
		mtlFile >> materials[matIndex].difColor.y;
		mtlFile >> materials[matIndex].difColor.z;
		for (auto& mi : mesh_index)
		{
			meshes[mi].material.diffColor.x = materials[matIndex].difColor.x;
			meshes[mi].material.diffColor.y = materials[matIndex].difColor.y;
			meshes[mi].material.diffColor.z = materials[matIndex].difColor.z;
		}
		difColorSet = true;
	}
	else if (pchar == 's')
	{
		mtlFile >> materials[matIndex].specColor.x;
		mtlFile >> materials[matIndex].specColor.y;
		mtlFile >> materials[matIndex].specColor.z;
		for (auto& mi : mesh_index)
		{
			meshes[mi].material.specColor.x = materials[matIndex].specColor.x;
			meshes[mi].material.specColor.y = materials[matIndex].specColor.y;
			meshes[mi].material.specColor.z = materials[matIndex].specColor.z;
		}
	}
	else if (pchar == 'a' && !difColorSet) //if no diffuse color has been set, set it to equal ambient color
	{
		mtlFile >> materials[matIndex].difColor.x;
		mtlFile >> materials[matIndex].difColor.y;
		mtlFile >> materials[matIndex].difColor.z;
		for (auto& mi : mesh_index)
		{
			meshes[mi].material.diffColor.x = materials[matIndex].difColor.x;
			meshes[mi].material.diffColor.y = materials[matIndex].difColor.y;
			meshes[mi].material.diffColor.z = materials[matIndex].difColor.z;
		}
	}
	while (mtlFile.get() != '\n' && mtlFile);
}
void ModelData::SetTransparency(std::wistream& mtlFile, wchar_t& pchar, UINT& matIndex, std::vector<UINT>& mesh_index)
{
	pchar = mtlFile.get();		// Transparency is denoted by "Tr" || "d " 
	float transp;
	mtlFile >> transp;

	materials[matIndex].difColor.w = (pchar == 'r') ? transp : 1.0f - transp; // if denoted by "d " transparency will be 1-transparency
	materials[matIndex].isTransparent = (materials[matIndex].difColor.w > 0);
	for (auto& mi : mesh_index)
	{
		meshes[mi].material.diffColor.w = (pchar == 'r') ? transp : 1.0f - transp;
		meshes[mi].material.isTransparent = (meshes[mi].material.diffColor.w > 0);
	}
}
void ModelData::SetDiffuseMap(std::wistream& mtlFile, UINT& matIndex, std::vector<UINT>& mesh_index)
{
	const UINT extLen = 4; // ".jpg"
	const UINT fileMaxSize = 40;
	wchar_t texBuff[fileMaxSize] = {};
	for (UINT i = 0, j = 0; i < fileMaxSize; i++, j++)
	{
		texBuff[j] = mtlFile.get();
		i = (texBuff[j] == '.') ? fileMaxSize-extLen : i; //if file extension reached only 3 chars remain
	}
	std::wstring texture = filePath + texBuff;
	bool add = true;
	UINT texIndex = 0;
	while (texIndex < texNames.size() && add) //Check if texture has already been read and set texIndex accordingly 
	{
		if (texNames[texIndex] == texture)
			add = false;
		else
			texIndex++;
	}
	if (add)
		texNames.push_back(texture);

	materials[matIndex].hasTexture = true;
	materials[matIndex].texIndex = texIndex;
	for (auto& mi : mesh_index)
	{
		meshes[mi].material.hasTexture = true;
		meshes[mi].material.texName = texture;
	}
	while (mtlFile.get() != '\n' && mtlFile);
}
void ModelData::SetNormalMap(std::wistream& mtlFile, UINT& matIndex, std::vector<UINT>& mesh_index)
{
	const UINT extLen = 4; // ".jpg"
	const UINT fileMaxSize = 40;
	wchar_t texBuff[fileMaxSize] = {};
	for (UINT i = 0, j = 0; i < fileMaxSize; i++, j++)
	{
		texBuff[j] = mtlFile.get();
		i = (texBuff[j] == '.') ? fileMaxSize-extLen : i; //if file extension reached only 3 chars remain
	}
	std::wstring nMapName = filePath + texBuff;
	bool add = true;
	UINT nMapIndex = 0;
	while (nMapIndex < nMapNames.size() && add) //Check if texture has already been read and set nMapIndex accordingly 
	{
		if (texNames[nMapIndex] == nMapName)
			add = false;
		else
			nMapIndex++;
	}
	if (add)
		nMapNames.push_back(nMapName);

	materials[matIndex].hasNormalMap = true;
	for (auto& mi : mesh_index)
	{
		meshes[mi].material.hasNormMap = true;
		meshes[mi].material.nMapName = nMapName;
	}
	while (mtlFile.get() != '\n' && mtlFile);
}

void ModelData::InitMaterial(std::wstring& mtllib)
{
	std::wifstream mtlFile(filePath + mtllib);
	wchar_t pchar = {};
	UINT matIndex = 0;
	std::vector<UINT> mesh_index;
	mesh_index.reserve(meshes.size());

	std::wstring matName = L"";
	bool difColorSet = false;

	auto ProceedCheck = [&](std::wstring_view check)
	{
		bool proceed = true;
		for (size_t i = 0; i < check.length(); i++)
		{
			if (proceed)
				proceed = (pchar == check[i]) ? true : false;
			else
				i = check.length();

			pchar = mtlFile.get();
		}
		return proceed;
	};

	while (mtlFile)
	{
		pchar = mtlFile.get();

		while (pchar == ' ') pchar = mtlFile.get();

		switch (pchar)
		{
		case 'n':
			if (ProceedCheck(L"newmtl"))
			{
				mtlFile >> matName;
				for (int i = 0; i < materials.size(); i++)
				{
					matIndex = (matName == materials[i].matName) ? i : matIndex;
				}
				difColorSet = false;

				mesh_index.clear();
				for (size_t i = 0; i < meshes.size(); i++)
				{
					if (meshes[i].material.matName == matName)
						mesh_index.push_back((UINT)i);
				}
			}
			break;
		case 'N':
			if (ProceedCheck(L"Ns"))
			{
				mtlFile >> materials[matIndex].specPower;
				for (auto& mi : mesh_index)
				{
					meshes[mi].material.specularPower = materials[matIndex].specPower;
				}
			}
			break;
		case 'K':
			SetColor(mtlFile, pchar, matIndex, mesh_index, difColorSet);
			break;
		case 'm':
		{
			std::wstring line;
			mtlFile >> line;
			mtlFile.get();
			if (line == L"ap_Kd")
			{
				SetDiffuseMap(mtlFile, matIndex, mesh_index);
			}
			else if (line == L"ap_bump")
			{
				SetNormalMap(mtlFile, matIndex, mesh_index);
			}
		}
			break;
		case 't':
			SetTransparency(mtlFile, pchar, matIndex, mesh_index);
			break;
		case 'd':
			SetTransparency(mtlFile, pchar, matIndex, mesh_index);
			break;
		case '\n':
			break;
		default:
			while (mtlFile.get() != '\n' && mtlFile);
			break;
		}
	}
}

void ModelData::InitNormals()
{
	using namespace DirectX;

	std::vector<UINT> count(vertices.size());
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		/*The primitive*/
		auto& v0 = vertices[indices[i]];
		auto& v1 = vertices[indices[i + 1]];
		auto& v2 = vertices[indices[i + 2]];
		/*Vertex Position*/
		const auto p0 = XMLoadFloat3(&v0.position);
		const auto p1 = XMLoadFloat3(&v1.position);
		const auto p2 = XMLoadFloat3(&v2.position);
		/*Calculate normal*/
		const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));
		/*Store Normal*/
		XMStoreFloat3(&v0.normal, n);
		XMStoreFloat3(&v1.normal, n);
		XMStoreFloat3(&v2.normal, n);
	}
	return;
	for (size_t i = 0; i < indices.size(); i+=3)
	{
		//calculate normal of primitive
		const DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&vertices[indices[i+0]].position);
		const DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&vertices[indices[i+1]].position);
		const DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&vertices[indices[i+2]].position);
 
		XMVECTOR primitive_n = XMVector3Cross(p1 - p0, p2 - p0);
		if (isRH)
		{
			int blyat = 0;
			//primitive_n = XMVectorSet(XMVectorGetX(primitive_n), XMVectorGetY(primitive_n), XMVectorGetZ(primitive_n), 1.0f);
			//primitive_n = XMVector3Normalize(XMVector3Cross(p0 - p2, p1 - p2));
		}
		//add calculated normal to every vertice in the primitive and keep count of the amount of primitives that uses the vertice for normal averaging
		for (size_t j = 0; j < 3; j++)
		{
			DirectX::XMStoreFloat3(&vertices[indices[i+j]].normal,
				XMVector3Normalize(primitive_n)
			);
			count[indices[i+j]]++;
		}
	}
	//Normal averaging
	for (size_t i = 0; i < vertices.size(); i++)
	{
		float c = float(count[i]);
		DirectX::XMStoreFloat3(&vertices[i].normal,
			XMVector3Normalize(XMVectorSet(vertices[i].normal.x /c, vertices[i].normal.y /c, vertices[i].normal.z /c, 1.0f))
		);
	}
}

void ModelData::ReadCopperData(std::wistream& file)
{
	UINT cprLines = 0u;
	wchar_t pchar = file.get();
	while (pchar != '<') //count how many lines of copperData
	{
		cprLines += 1u;
		while (file.get()!='\n'); //newline
		pchar = file.get();
	}
	file.clear();
	file.seekg(0);
	while (file.get() != '\n');

	std::vector<UINT> cprData(2*(size_t)cprLines-4);			// <<<COPPER DATA FORMAT>>>
	for (size_t i = 0; i < cprData.size()-1; i++)				// [0]   = Vertices:      x
	{															// [1-3] = vPos/vTc/vnorm x y z
		while (file.get() != ' ');								// [4]   = Indices:       x
		file >> cprData[i];										// [5]   = textures:      x
		if (i == 1)												// [6]   = materials:     x
		{														// [7]   = meshes:		  x
			file >> cprData[++i];								// [8+]  = v/i per mesh   x y
			file >> cprData[++i];
		}
		else if (i > 7)
			file >> cprData[++i];
		while (file.get()!='\n' && file);
	}
	vpos.reserve(cprData[1]);
	vtc.reserve(cprData[2]);
	vnorm.reserve(cprData[3]);
	indices.reserve(cprData[4]);
	texNames.reserve(cprData[5]);
	materials.reserve(cprData[6]);
	meshes.reserve(cprData[7]);
	UINT vtices = 0;
	for (size_t i = 0, j = 8, k = 8; i < cprData[7]; i++, j++, k++)
	{
		vtices += cprData[j];
		meshes.emplace_back(cprData[j++], cprData[++k]);
	}
	vertices.reserve(vtices);
	//make sure default tc/norm exists
	if (!vnorm.capacity())
		vnorm.push_back(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	if (!vtc.capacity())
		vtc.push_back(DirectX::XMFLOAT2(0.0f, 0.0f));
}

void ModelData::WriteCopperData(std::wistream& modelFile)
{
	//variables to store data in.
	std::wstring meshMatLib = L"";

	wchar_t pchar = {};
	UINT nMeshes = 0;
	while (modelFile) // calculate number of meshes first
	{
		while (pchar != '\n' && modelFile) pchar = modelFile.get();
		pchar = modelFile.get();
		if (pchar == 'o' /*|| pchar == 'g'*/)
			nMeshes++;
	}
	modelFile.clear();
	modelFile.seekg(0);

	//SetFace lambda "global" variables
	UINT nVertices = 0, vtot = 0;
	std::vector<UINT> vpos;
	std::vector<UINT> vtc;
	std::vector<UINT> vnorm;

	//SetFace counts how many vertices are created
	auto SetFace = [&vtot, &nVertices, &vpos, &vtc, &vnorm](std::wstring lLine)
	{
		struct V
		{
			UINT posIndex;
			UINT texCoordIndex;
			UINT normIndex;
		};
		std::vector<V> face = {}; //vector holding this face's vertex indices
		std::wstringstream lwss(lLine);
		std::vector<std::wstring> vVertexString;
		for (UINT i = 0; i < lLine.length(); i++)
		{
			if (lLine[i] == ' ' && i != lLine.length()-1)  //each 'space' represents a new vertice except if line ends with a 'space'
			{
				std::wstring vData; //holds vertex Indices in string format e.g: L"2/2/1" - (pos/TC/Norm)
				lwss >> vData;
				UINT whatPart = 0u; //used to keep track of what part of the vertice the number represent ( 0/1/2 = pos/tc/norm)
				UINT posIndex = 0u, normIndex = 0u, texCoordIndex = 0u; //default values of indexes. e.g if no texcoord is given (vData = "pos//norm") TCindex will be set to 0.
				std::wstring valueStr = L"";
				//Parse the vertex string and fill face vector. Vertex Data structure = PosIndex/TCindex/NormIndex
				for (UINT j = 0; j < vData.length(); j++)
				{
					//add next character to valueStr until end of string or until character is a '/', then cast the valueStr to int and set value to correct index
					if (vData[j] != '/')
						valueStr += vData[j];
					if (vData[j] == '/' || j == vData.length()-1)
					{
						std::wstringstream wssToInt(valueStr);
						switch (whatPart)
						{
						case 0: //pos
							wssToInt >> posIndex;
							if (posIndex>0) posIndex--;
							break;
						case 1: //tc
							wssToInt >> texCoordIndex;
							if (texCoordIndex>0) texCoordIndex--;
							break;
						case 2: //norm
							wssToInt >> normIndex;
							if (normIndex>0) normIndex--;
							break;
						default:
							break;
						}
						valueStr = L"";
						whatPart++;
					}
				}
				V temp = { posIndex, texCoordIndex, normIndex };
				face.push_back(temp);
			}
		}
		//Face vector now holds the vertices that it is composed of 
		for (UINT i = 0; i < face.size(); i++)
		{
			//check to see if this vertice already exists and set this vertex's index accordingly
			bool add = true;
			UINT vIndex = 0;
			while (vIndex < vtot && add)
			{
				if (face[i].posIndex != vpos[vIndex] || face[i].texCoordIndex != vtc[vIndex])
					vIndex++;
				else
					add = false;
			}
			if (add)
			{
				nVertices++; //mesh vertice count
				vtot++;	     //total vertices
				vpos.push_back(face[i].posIndex);
				vtc.push_back(face[i].texCoordIndex);
				vnorm.push_back(face[i].normIndex);
			}
		}
	};
	std::vector<UINT> m_nVertices;
	m_nVertices.reserve(nMeshes);
	UINT m_Vs = 0;
	UINT m_Is = 0;

	while (modelFile)
	{
		while (modelFile && pchar != '\n') //newline
			pchar = modelFile.get();
		pchar = modelFile.get();
		switch (pchar)
		{
		case 'm':    //mtllib - material library filename
			if (meshMatLib == L"")
				modelFile >> meshMatLib;
			if (meshMatLib == L"tllib")
				modelFile >> meshMatLib;
			break;
		case 'f': //count number of vertices in the current mesh
		{
			UINT fv = 0;
			std::wstring line;
			std::getline(modelFile, line);
			for (size_t i = 0; i < line.length(); i++)
			{
				fv += (line[i] == ' ') ? 1u : 0u;
			}
			SetFace(line);
			pchar = '\n';
			m_Is += (fv>3) ? (3*fv)-6 : fv;
		}
			break;
		case 'o':    //o if not first mesh, store mesh vertice & indice count and reset the count
			if (modelFile.get() == ' ' && m_Is)
			{
				m_Vs = nVertices;
				nVertices = 0;
				m_nVertices.push_back(m_Vs);
				m_nVertices.push_back(m_Is);
				m_Is = 0, m_Vs = 0;
			}
			break;
		case 'g':    //o if not first mesh, store mesh vertice & indice count and reset the count
			if (modelFile.get() == ' ' && m_Is)
			{
				m_Vs = nVertices;
				nVertices = 0;
				m_nVertices.push_back(m_Vs);
				m_nVertices.push_back(m_Is);
				m_Is = 0, m_Vs = 0;
			}
			break;
		default:
			break;
		}
	}

	if (m_Is) //save last mesh values
	{
		m_Vs = nVertices;
		nVertices = 0u;
		m_nVertices.push_back(m_Vs);
		m_nVertices.push_back(m_Is);
	}
	modelFile.clear();
	modelFile.seekg(0);


	std::vector<std::wstring> meshMaterials;
	UINT nVpos = 0;
	UINT nVtc = 0;
	UINT nVnorm = 0;
	UINT nMaterials = 0;
	UINT nTextures = 0;
	UINT nIndices = 0;

	while (modelFile) // Parse the .obj file
	{
		std::wstring compStr = L"";
		std::wstring line = L"";
		std::getline(modelFile, line);
		std::wstringstream wss(line);
		switch (line[0])
		{
		case 'm':    //mtllib - material library filename
			wss >> compStr;
			if (compStr == L"mtllib")
				wss >> meshMatLib;
			break;
		case 'v':	// Vertex
		{
			line.erase(0, 1);
			float x = 0.0f, y = 0.0f, z = 0.0f;
			wss >> x >> y >> z;
			if (line[0] == ' ') nVpos++;
			else if (line[0] == 't') nVtc++;
			else if (line[0] == 'n') nVnorm++;
		}
		break;
		case 'f':	// f - this line defines a face of the model
		{
			line.erase(0, 1);
			SetFace(line);
			UINT fv = 0;
			for (UINT i = 0; i < line.length(); i++)
				fv += (line[i] == ' ' && i != line.length()-1) ? 1u : 0u;
			nIndices += (fv>3) ? (3*fv)-6 : fv;
		}
		break;
		case 'u':    //usemtl - which material to use
			wss >> compStr;
			if (compStr == L"usemtl")		//Make sure line specifies 'uselib'
			{
				std::wstring matName = L"";
				wss >> matName;
				bool add = true;
				for (auto& mat : meshMaterials)
					add = (mat == matName) ? false : add;
				if (add)
					meshMaterials.push_back(matName);
			}
			break;
		default:
			break;
		}
	}
	nMaterials = (UINT)meshMaterials.size();
	modelFile.clear();

	//move on to material
	std::wifstream matFile(filePath + meshMatLib);
	if (matFile)
	{
		while (matFile)
		{
			std::wstring matline = L"";
			std::wstring matcompStr = L"";
			std::getline(matFile, matline);
			std::wstringstream mwss(matline);

			switch (matline[0])
			{
			case 'm': //Get the diffuse map (texture)
				mwss >> matcompStr;
				if (matcompStr == L"map_Kd")
				{
					std::wstring texName = L"";
					for (UINT i = 7u, j = 7u; i < matline.length(); i++, j++) // start at 7 to go past "map_Kd "
					{
						texName += matline[j];
						if (matline[j] == '.')	// at this point only file extension remains (3 more characters)
							i = (UINT)matline.length()-4;
					}
					//check if this texture has already been loaded
					bool add = true;
					for (auto& tex : texNames)
						add = (tex == texName) ? false : add;
					if (add)
						texNames.push_back(texName);
				}
				break;
			default:
				break;
			}
		}
		nTextures = (UINT)texNames.size();
		matFile.close();
	}

	modelFile.clear();
	modelFile.seekg(0);
	//create a temporary file
	char tmpFile[L_tmpnam_s];
	tmpnam_s(tmpFile, L_tmpnam_s);
	std::wofstream wTmpFile(tmpFile);

	//Write how many vertices/indices/meshes/textures/materials that the model is composed of at the top of the temporary file
	wTmpFile << L"<<<CopperData>>>\n";
	wTmpFile << L"#Vertices:  " << vtot << std::endl;
	wTmpFile << L"#vPos/vTc/vNorm:   " << nVpos << L" " << nVtc << L" " << nVnorm << std::endl;
	wTmpFile << L"#Indices:   " << nIndices << std::endl;
	wTmpFile << L"#textures:  " << nTextures << std::endl;
	wTmpFile << L"#materials: " << nMaterials << std::endl;
	wTmpFile << L"#meshes:    " << nMeshes << std::endl;
	for (UINT i = 0, j = 0, k = 1; i < nMeshes; i++, j += 2, k += 2)
		wTmpFile << L"#mesh_" << i << L"    " << m_nVertices[j] << L" " <<  m_nVertices[k] << std::endl;
	wTmpFile << L"<<<EndOfCopperData>>>\n";

	//append contents of the obj file to the temporary file
	std::wstring line = L"";
	while (std::getline(modelFile, line))
	{
		wTmpFile << line << L"\n";
	}
	modelFile.clear();
	modelFile.seekg(0);
	wTmpFile.close();

	//clear obj file and copy contents from the temporary file to the obj file.
	std::wofstream wFile(filePath + fileName + L".obj", std::wofstream::out | std::wofstream::trunc);
	std::wifstream RtmpFile(tmpFile);
	while (std::getline(RtmpFile, line))
	{
		wFile << line << L"\n";
	}
	RtmpFile.close();
	wFile.close();
	std::remove(tmpFile);
	modelFile.clear();
	modelFile.seekg(0);
}