#pragma once

#include <vector>
#include <array>
#include <DirectXMath.h>
#include "IndexedTriangleList.h"
#include "MaMath.h"

class Plane
{
public:
	template<class V>
	static IndexedTriangleList<V> MakeTesselated(int divisions_x, int divisions_y, float width, float height)
	{
		namespace dx = DirectX;
		assert(divisions_x >= 1);
		assert(divisions_y >= 1);

		/*constexpr float width = 25.0f;
		constexpr float height = 25.0f;*/
		const int nVertices_x = divisions_x + 1;
		const int nVertices_z = divisions_y + 1;
		int x_z = nVertices_x * nVertices_z;
		std::vector<V> vertices(x_z);

		{
			const float side_x = width / 2.0f;
			const float side_z = height / 2.0f;
			const float divisionSize_x = width / float(divisions_x);
			const float divisionSize_z = height / float(divisions_y);
			const auto bottomLeft = dx::XMVectorSet(-side_x, 0.0f, -side_z, 0.0f);

			for (int z = 0, i = 0; z < nVertices_z; z++)
			{
				const float z_pos = float(z) * divisionSize_z;
				for (int x = 0; x < nVertices_x; x++, i++)
				{
					const auto v = dx::XMVectorAdd(
						bottomLeft,
						dx::XMVectorSet(float(x) * divisionSize_x, 0.0f, z_pos, 0.0f)
					);
					dx::XMStoreFloat3(&vertices[i].pos, v);
				}
			}
		}

		std::vector<int> indices;
		int indices_n = divisions_x*divisions_y*6;
		indices.reserve( indices_n );
		{
			const auto vxy2i = [nVertices_x](size_t x, size_t y)
			{
				return (UINT)( x + (y*nVertices_x) );
			};
			for (size_t y = 0; y < divisions_y; y++)
			{
				for (size_t x = 0; x < divisions_x; x++)
				{
					const std::array<UINT, 4> indexArray =
					{ vxy2i(x,y),vxy2i(x + 1,y),vxy2i(x,y + 1),vxy2i(x + 1,y + 1) };
					indices.push_back(indexArray[0]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[3]);
				}
			}
		}

		return{ std::move(vertices),std::move(indices) };
	}

	template<class V>
	static IndexedTriangleList<V> Make(float width, float height)
	{
		return MakeTesselated<V>(1, 1, width, height);
	}

	template<class V>
	static IndexedTriangleList<V> MakeTerrain(float divx, float divy, float size_width, float size_height, std::wstring filename)
	{
		namespace dx = DirectX;

		filename = filename + L".bmp";
		const wchar_t* file = filename.c_str();
		FILE* filePtr;
		int error;
		unsigned int count;
		BITMAPFILEHEADER bitmapFileHeader;
		BITMAPINFOHEADER bitmapInfoHeader;
		int imageSize;
		unsigned char* bitmapImage;


		if (_wfopen_s(&filePtr, file, L"r+b")!=0)
		{
			error = 1;
			//error handling
		}
		//count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
		if (fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1)
		{
			error = 1;
			//error handling
		}

		//count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
		if (fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1)
		{
			error = 1;
			//error handling
		}

		float m_terrainWidth = (float)bitmapInfoHeader.biWidth;
		float m_terrainHeight = (float)bitmapInfoHeader.biHeight;

		// Calculate the size of the bitmap image data.(chars)
		imageSize = int(bitmapFileHeader.bfSize-bitmapFileHeader.bfOffBits);

		// Allocate memory for the bitmap image data.
		//bitmapImage = (unsigned char*)malloc(sizeof(unsigned char) * imageSize);
		bitmapImage =  new unsigned char[imageSize];
		if (!bitmapImage)
		{
			error = 1;
			//error handling
		}
		// Move to the beginning of the bitmap data.
		fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
		// Read in the bitmap image data.
		count = (unsigned int)fread(bitmapImage, 1, imageSize, filePtr);
		if (count != imageSize)
		{
			error = 1;
			//error handling
		}
	
		assert(divx >= 1);
		assert(divy >= 1);

		const int nVertices_x = (int)divx + 1;
		const int nVertices_y = (int)divy + 1;
		int x_y = nVertices_x * nVertices_y;
		std::vector<V> vertices(x_y);

		const float side_x = size_width / 2.0f;
		const float side_y = size_height / 2.0f;
		const float divisionSize_x = size_width / float(divx);
		const float divisionSize_y = size_height / float(divy);

		int cpp		  = bitmapInfoHeader.biBitCount/8;															//chars per pixel
		float c_width = (m_terrainWidth-1)*cpp;																	//chars per line (-1 since start at 0)
		DWORD stride  = ((((bitmapInfoHeader.biWidth * bitmapInfoHeader.biBitCount) + 31) & ~31) >> 3);			//chars to traverse entire line ('char x' line Y --> 'char x' line Y+1)
		int padding	  = (int)stride % (int)m_terrainWidth;														//padding per line (char)
		float xstep	  = (m_terrainWidth-1)/divx;																//char sample step(x)
		float ystep	  = ((m_terrainHeight-1)/divy);																//char sample step(y)												
		
		float bitmapIndex	  = 0.0f;
		const auto bottomLeft = dx::XMVectorSet(-side_x, 0.0f, -side_y, 0.0f);

		for (int y = 0, i = 0; y < nVertices_y; y++)
		{
			bitmapIndex = floor(ystep*y)*stride;
			const float y_pos = float(y) * divisionSize_y;
			for (int x = 0; x < nVertices_x; x++, i++)
			{
				int pixel = int(bitmapIndex + floor(x*xstep)*cpp);
				float height = bitmapImage[pixel];
				const auto v = dx::XMVectorAdd(
					bottomLeft,
					dx::XMVectorSet(float(x) * divisionSize_x, height/10.0f, y_pos, 0.0f)
				);
				bitmapImage[pixel+1] = (unsigned char)((height));
				dx::XMStoreFloat3(&vertices[i].pos, v);
			}
		}
		rewind(filePtr);
		fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
		fwrite(bitmapImage, 1, imageSize, filePtr);
		rewind(filePtr);
		fseek(filePtr, bitmapFileHeader.bfOffBits+(65944), SEEK_SET);
		unsigned char asdsdsd = fgetc(filePtr);
		fseek(filePtr, bitmapFileHeader.bfOffBits+(65945), SEEK_SET);
		asdsdsd = fgetc(filePtr);
		//free( bitmapImage);
		delete[] bitmapImage;
		// Close the file.
		error = fclose(filePtr);
		if (error != 0)
		{
			error = 1;
			//error handling
		}

		int size = (int)(divx * divy * 6);
		std::vector<int> indices;
		indices.reserve(size);
		{
			const auto vxy2i = [nVertices_x](size_t x, size_t y)
			{
				return (UINT)(x + (y*nVertices_x));
			};
			for (size_t y = 0; y < divy; y++)
			{
				for (size_t x = 0; x < divx; x++)
				{
					const std::array<UINT, 4> indexArray =
					{ vxy2i(x,y),vxy2i(x + 1,y),vxy2i(x,y + 1),vxy2i(x + 1,y + 1) };
					indices.push_back(indexArray[0]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[3]);
				}
			}
		}
		return{ std::move(vertices), std::move(indices) };
	}

	template<class V>
	static IndexedTriangleList<V> MakeHeightMap(float w, float h, float vx, float vy, float heightAttenuation, int reach, std::wstring filename)
	{
		namespace dx = DirectX;

		filename = filename + L".bmp";
		const wchar_t* file = filename.c_str();
		FILE* filePtr;
		bool error = false;
		BITMAPFILEHEADER bitmapFileHeader;
		BITMAPINFOHEADER bitmapInfoHeader;
		int imageSize;
		auto Error = [&filename, &error](std::wstring message)
		{
			message += filename;
			MessageBox(0, message.c_str(),
				L"Error", MB_OK);
			if (!error) error = true;
		};

		if (_wfopen_s(&filePtr, file, L"r+b")!=0)
			Error(L"Could not open ");
		if (fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1)
			Error(L"Could not read bitmap file header of  ");
		if (fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1)
			Error(L"Could not read bitmap info header of  ");

		imageSize = int(bitmapFileHeader.bfSize-bitmapFileHeader.bfOffBits); // Calculate the byte size of the bitmap image data
		if (!imageSize)
			Error(L"ImageSize is equal to zero for file ");

		unsigned char* bitmapImage = new unsigned char[imageSize];	// Move to the beginning of the bitmap data.
		fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);		// Read in the bitmap image pixel data.
		fread(bitmapImage, 1, imageSize, filePtr);
		fclose(filePtr);

		UINT bpp = bitmapInfoHeader.biBitCount/8;		//bytes per pixel
		UINT npx_w = bitmapInfoHeader.biWidth;			//picture number of pixel wide												
		UINT npx_h = bitmapInfoHeader.biHeight;			//picture number of pixel high
		DWORD stride = ((((bitmapInfoHeader.biWidth * bitmapInfoHeader.biBitCount) + 31) & ~31) >> 3);	//chars per line

		CprArray<unsigned char> pxs(npx_h, (size_t)npx_w*3u);		// RGB color channel array to store normal in 
		CprArray<unsigned char> pxs_A(npx_h, npx_w);				// alpha channel array to store height value
		auto CalculateNormalMap = [&pxs, &pxs_A, &w, &h](size_t row, size_t col)
		{
			using namespace DirectX;										
			size_t c_h = pxs_A.RowCount()-1;									
			size_t c_w = pxs_A.ColumnCount()-1;								
			float scaleX = float(w/c_w);									
			float scaleY = float(h/c_h);											// 			   Sobel style
			//sample surrounding pixels height value (if there is one)				//    _|________|________|________|_
			unsigned char x0 = {}, x1 = {}, y0 = {}, y1 = {};						//     |        |        |        |
			unsigned char xy0 = {}, xy1 = {}, xy2 = {}, xy3 = {};					//     |  xy2   |   y1   |   xy3  |
			//Sample neighbooring pixels height values (if there is one)			//    _|________|________|________|_
			x0 =  (col)			? pxs_A(row, col-1) : pxs_A(row, col);				// 	   |        |        |        |
			x1 =  (col+1 < c_w) ? pxs_A(row, col+1) : pxs_A(row, col);				//     |   x0   |  pix   |   x1   |
			y0 =  (row)		    ? pxs_A(row-1, col) : pxs_A(row, col);				//    _|________|________|________|_
			y1 =  (row < c_h)   ? pxs_A(row+1, col) : pxs_A(row, col);				//     |        |        |        |
			xy0 = (col && row)			 ? pxs_A(row-1, col-1) : pxs_A(row, col);	//     |  xy0   |   y0   |   xy1  |
			xy1 = (col+1<c_w && row)	 ? pxs_A(row-1, col+1) : pxs_A(row, col);	//    _|________|________|________|_
			xy2 = (col && row<c_h)		 ? pxs_A(row+1, col-1) : pxs_A(row, col);	// 	   |		|		 |		  |
			xy3 = (col+1<c_w && row<c_h) ? pxs_A(row+1, col+1) : pxs_A(row, col);	// 
			//calculate normal and translate value range to 0-255
			float n_x = scaleX * float(2*(x0-x1) + (xy0-xy1) + (xy2-xy3));
			float n_y = scaleY * float(2*(y0-y1) + (xy0-xy2) + (xy2-xy3));
			XMVECTOR pixel_n = XMVector3Normalize(XMVectorSet(n_x, n_y, 1.0f, 0.0f));
			unsigned char r = (unsigned char)((XMVectorGetX(pixel_n) + 1)/2 * 255);
			unsigned char g = (unsigned char)((XMVectorGetY(pixel_n) + 1)/2 * 255);
			unsigned char b = (unsigned char)((XMVectorGetZ(pixel_n) + 1)/2 * 255);
			//save the normal components in the color channels array. 
			pxs(row, col + 0) = r;
			pxs(row, col + 1) = g;
			pxs(row, col + 2) = b;
		};

		for (size_t i = 0; i < npx_h; i++) //Load color and alpha pixel arrays with image pixel data
		{
			for (size_t j = 0; j < npx_w; j++)
			{
				UINT bmi_i = UINT(i*stride+j*bpp);
				pxs_A(i, j) = bitmapImage[bmi_i];
				for (size_t k = 0; k < 3; k++)
					pxs(i, j*3+k) = bitmapImage[bmi_i];
			}
		}
		for (size_t i = 0; i < npx_h; i++) //calculate and store normal data in R/G/B array
		{
			for (size_t j = 0; j < npx_w; j++)
				CalculateNormalMap(i, j);
		}
		delete[] bitmapImage;

		//With the pixel data will make the terrain mesh
		if (vx<2) vx = 2;
		if (vy<2) vy = 2;
		
		CprArray<float> weightGrid((size_t)reach*2+1, (size_t)reach*2+1);
		float centDist = (weightGrid.ColumnCount() >> 1) > 0 ? float(weightGrid.ColumnCount()) : 1.0f;
		float factor = sqrtf(centDist*centDist+centDist*centDist);
		for (float i = 0; i < weightGrid.RowCount(); i++)
		{
			for (float j = 0; j < weightGrid.ColumnCount(); j++)
			{
				float value = abs(centDist - i)*abs(centDist - i)+abs(centDist - j)*abs(centDist - j);
				value = 1.01f - sqrtf(value) * 1.0f/factor;
				weightGrid((size_t)i, (size_t)j) = value;
			}
		}
		auto CalculateHeight2 = [&pxs_A, &weightGrid, &reach](int pix_x, int pix_y)
		{
			int cols = (int)pxs_A.ColumnCount()-1;
			int rows = (int)pxs_A.RowCount()-1;

			int& r = reach;
			int west  = (pix_x<r) ?  pix_x : r;
			int east  = (pix_x+r) > cols ? (cols-pix_x) : r;
			int south = (pix_y<r) ? pix_y : r;
			int north = (pix_y+r) > rows ? (rows-pix_y) : r;

			float weightedHeight = 0.0f;
			float weight = 0.0f;
			for (int i = -west; i < east+1; i++)
			{
				int min = ((r-abs(i)) > south) ? south : r-abs(i);
				int max = ((r-abs(i)) > north) ? north : r-abs(i);
				for (int j = -min; j < max+1; j++)
				{
					weightedHeight += pxs_A(pix_y+j, pix_x+i) * weightGrid(j+r, i+r);
					weight += weightGrid(j+r, i+r);
				}
			}
			return weightedHeight / weight;
		};

		std::vector<V> vertices(size_t(vx)*size_t(vy));
		DirectX::XMVECTOR bottomLeft = DirectX::XMVectorSet(-w/2.0f, 0.0f, -h/2.0f, 0.0f);
		for (size_t iy = 0; iy < vy; iy++)
		{
			float posY = iy*h/(vy-1);
			for (size_t ix = 0; ix < vx; ix++)
			{
				float posX = ix*w/(vx-1);

				int pix_x = (int)floor(posX*(npx_w-1.0f)/w); //translate vertice x-pos to pixel grid
				int pix_y = (int)floor(posY*(npx_h-1.0f)/h); //translate vertice y-pos to pixel grid
				float height = CalculateHeight2(pix_x, pix_y);
				const auto v = dx::XMVectorAdd(
					bottomLeft,
					dx::XMVectorSet(posX, height/heightAttenuation, posY, 0.0f)
					);
				dx::XMStoreFloat3(&vertices[iy*size_t(vx)+ix].pos, v);
				float tc_u = ix/(vx-1);
				float tc_v = 1.0f - iy/(vy-1);
				vertices[iy*size_t(vx)+ix].texCoord.u = tc_u;
				vertices[iy*size_t(vx)+ix].texCoord.v = tc_v;
			}
		}
		size_t sizee = (size_t(vx)-1)*(size_t(vy)-1)*6;
		std::vector<int> indices;
		indices.reserve(sizee);
		size_t what = indices.size();
		{
			const auto vxy2i = [vx](size_t x, size_t y)
			{
				return (UINT)(x + (y*vx));
			};
			for (size_t y = 0; y < vy-1; y++)
			{
				for (size_t x = 0; x < vx-1; x++)
				{
					const std::array<UINT, 4> indexArray =
					{ vxy2i(x,y),vxy2i(x + 1,y),vxy2i(x,y + 1),vxy2i(x + 1,y + 1) };
					indices.push_back(indexArray[0]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[3]);
				}
			}
		}

		std::vector<UINT> vCount(vertices.size(), 0u);
		auto TangentBitangentNormal = [&vCount, &vertices, &indices](size_t index) {
			//calculate tangent(T) & bitangent(B) for a triangle
			// Edge1 = delta_u1*T + delta_v1*B
			// Edge2 = delta_u2*T + delta_v2*B
			//
			// Matrix magic to solve for T & B
			//	_		     _	   _				  _		_			_
			// |			  |	  |					   |   |			 |
			// | E1(x1,y1,z1) |   | delta_u1  delta_v1 |   | T(x1,y1,z1) |
			// |			  | = |					   | * |			 |
			// | E2(x2,y2,z2) |   | delta_u2  delta_v2 |   | B(x1,y1,z1) |
			// |_			 _|	  |_				  _|   |_			_|
			//
			using namespace DirectX;
			const size_t v0 = (size_t)indices[index+0]; //vertices composing triangle 
			const size_t v1 = (size_t)indices[index+1]; //vertices composing triangle
			const size_t v2 = (size_t)indices[index+2]; //vertices composing triangle

			XMFLOAT3 e1	   = {};  //edge1
			XMFLOAT2 dudv1 = {};  //Delta u and Delta v for edge1
			XMFLOAT3 e2	   = {};  //edge2
			XMFLOAT2 dudv2 = {};  //Delta u and Delta v for edge2

			XMStoreFloat3(&e1, XMLoadFloat3(&vertices[v1].pos) - XMLoadFloat3(&vertices[v0].pos));
			dudv1.x = vertices[v1].texCoord.u - vertices[v0].texCoord.u;
			dudv1.y = vertices[v1].texCoord.v - vertices[v0].texCoord.v;

			XMStoreFloat3(&e2, XMLoadFloat3(&vertices[v2].pos) - XMLoadFloat3(&vertices[v0].pos));
			dudv2.x = vertices[v2].texCoord.u - vertices[v0].texCoord.u;
			dudv2.y = vertices[v2].texCoord.v - vertices[v0].texCoord.v;
			 
			XMVECTOR tangent   = {};
			XMVECTOR bitangent = {};
			XMVECTOR normal    = {};
			float determinant = dudv1.x*dudv2.y - dudv1.y*dudv2.x;


			tangent	  = XMVectorSet( 1.0f/determinant * (dudv2.y * e1.x - dudv1.y * e2.x),
									 1.0f/determinant * (dudv2.y * e1.y - dudv1.y * e2.y),
									 1.0f/determinant * (dudv2.y * e1.z - dudv1.y * e2.z),
									 0.0f
								);
			bitangent = XMVectorSet( 1.0f/determinant * (-dudv2.y * e1.x + dudv1.x * e2.x),
									 1.0f/determinant * (-dudv2.y * e1.y + dudv1.x * e2.y),
									 1.0f/determinant * (-dudv2.y * e1.z + dudv1.x * e2.z),
									 0.0f
								);
			normal    = XMVector3Cross(XMLoadFloat3(&e1), XMLoadFloat3(&e2));

			//add tangent, bitanget, and normal in each vertice of the triangle 
			//and keep count of the amount of vertices that uses the bi/tangent/normal for averaging
			for (size_t i = 0; i < 3; i++)
			{
				XMStoreFloat3(&vertices[indices[index+i]].tangent,
					XMLoadFloat3(&vertices[indices[index+i]].tangent) + tangent
				);
				XMStoreFloat3(&vertices[indices[index+i]].bitangent,
					XMLoadFloat3(&vertices[indices[index+i]].bitangent) + bitangent
				);
				XMStoreFloat3(&vertices[indices[index+i]].normal,
					XMLoadFloat3(&vertices[indices[index+i]].normal) + normal
				);
				vCount[indices[index+i]]++;
			}
		};

		//calculate tangent, bitangent and normal for every primitive
		for (size_t i = 0; i < indices.size(); i+=3)
			TangentBitangentNormal(i);
		
		//average every vertices tangent, bitangent, and normal
		for (size_t i = 0; i < vertices.size(); i++)
		{
			dx::XMVECTOR tangent   = dx::XMVectorSet( (vertices[i].tangent.x   / vCount[i]), (vertices[i].tangent.y   / vCount[i]), (vertices[i].tangent.z   / vCount[i]), 0.0f );
			dx::XMVECTOR bitangent = dx::XMVectorSet( (vertices[i].bitangent.x / vCount[i]), (vertices[i].bitangent.y / vCount[i]), (vertices[i].bitangent.z / vCount[i]), 0.0f );
			dx::XMVECTOR normal    = dx::XMVectorSet( (vertices[i].normal.x    / vCount[i]), (vertices[i].normal.y    / vCount[i]), (vertices[i].normal.z    / vCount[i]), 0.0f );
			dx::XMVector3Normalize(tangent);
			dx::XMVector3Normalize(bitangent);
			dx::XMVector3Normalize(normal);
			dx::XMStoreFloat3(&vertices[i].tangent, tangent);
			dx::XMStoreFloat3(&vertices[i].bitangent, bitangent);
			dx::XMStoreFloat3(&vertices[i].normal, normal);
		}

		return{ std::move(vertices), std::move(indices) };
	}
	template<class V>
	static IndexedTriangleList<V> MakePortalPlane(float frameX, float frameY, float portalX, float portalY)
	{
		namespace dx = DirectX;

		std::vector<V> vertices(16);
		auto fX = frameX;
		auto pX = portalX;
		auto fY = frameY;
		auto pY = portalY;

		// 'front'
		vertices[0].pos = { -fX/2.0f, -fY/2.0f, 0.0f };
		vertices[1].pos = { -pX/2.0f, -pY/2.0f, 0.0f };
		vertices[2].pos = { -fX/2.0f,  fY/2.0f, 0.0f };
		vertices[3].pos = { -pX/2.0f,  pY/2.0f, 0.0f };
		vertices[4].pos = {  fX/2.0f,  fY/2.0f, 0.0f };
		vertices[5].pos = {  pX/2.0f,  pY/2.0f, 0.0f };
		vertices[6].pos = {  fX/2.0f, -fY/2.0f, 0.0f };
		vertices[7].pos = {  pX/2.0f, -pY/2.0f, 0.0f };
		//'back'
		for (size_t i = 0; i < 8; i++)
		{
			auto v = dx::XMVector3Transform(dx::XMLoadFloat3(&vertices[i].pos), dx::XMMatrixRotationRollPitchYaw(PI, 0.0f, 0.0f));
			dx::XMStoreFloat3(&vertices[i+8].pos, v);
		}

		return{
			std::move(vertices),{
				0,2,1, 2,3,1,
				2,4,3, 4,5,3,
				4,6,5, 6,7,5,
				6,0,7, 0,1,7,

				8,10,9, 10,11,9,
				10,12,11, 12,13,11,
				12,14,13, 14,15,13,
				14,8,15, 8,9,15,
			}
		};
	}
};