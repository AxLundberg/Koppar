#include "Texture.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include "wincodec.h"

Texture::Texture(Graphics& gfx, std::wstring texFile, unsigned int slot) 
	: 
	slot(slot)
{

	 HRESULT hr = DirectX::CreateWICTextureFromFile(
		GetDevice(gfx),
		texFile.c_str(),
		nullptr, pTexView.GetAddressOf() //**texture, **textureview
	 );

	 /*if (texFile == L"Textures\\meteormap" || texFile == L"Textures\\map")
	 {
		 auto pxs = MakeNormalMap(150.0f, 150.0f, texFile);
		 int w = (int)pxs.first;
		 int h = (int)pxs.second;
		 int bpp = 4;
		 int w_h = w * h;
		 int* buf = new int[w_h];
		 for (size_t i = 0; i < h; i++)
		 {
			 for (size_t j = 0; j < w; j++)
			 {
				 int pixel = 0x00000000;
				 pixel = pixel + ((int)okenow[i*4*w+j*4]);
				 pixel = pixel + ((int)okenow[i*4*w+j*4+1] << 8);
				 pixel = pixel + ((int)okenow[i*4*w+j*4+2] << 16);
				 pixel = pixel + ((int)okenow[i*4*w+j*4+3] << 24);
				 buf[i*w+j] = pixel;
			 }
		 }
		 // setting up D3D11_SUBRESOURCE_DATA 
		 D3D11_SUBRESOURCE_DATA tbsd = {};
		 tbsd.pSysMem = (void*)buf;
		 tbsd.SysMemPitch = w * bpp;
		 //tbsd.SysMemSlicePitch = w * h * bpp; // Not needed since this is a 2d texture

		 // setting up D3D11_TEXTURE2D_DESC 
		 D3D11_TEXTURE2D_DESC tdesc = {};
		 tdesc.Width = w;
		 tdesc.Height = h;
		 tdesc.MipLevels = 1;
		 tdesc.ArraySize = 1;
		 tdesc.SampleDesc.Count = 1;
		 tdesc.SampleDesc.Quality = 0;
		 tdesc.Usage = D3D11_USAGE_DEFAULT;
		 tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		 tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		 tdesc.CPUAccessFlags = 0;
		 tdesc.MiscFlags = 0;
		 Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
		 // create the texture
		 GetDevice(gfx)->CreateTexture2D(&tdesc, &tbsd, &pTexture);

		 D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		 srvDesc.Format = tdesc.Format;
		 srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		 srvDesc.Texture2D.MostDetailedMip = 0;
		 srvDesc.Texture2D.MipLevels = 1;
		 GetDevice(gfx)->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pTexView);

		 delete[] buf;
	 }*/
	 if (FAILED(hr))
	 {   
         int w = 512;
         int h = 512;
         int bpp = 4;
         int w_h = w * h;
         int* buf = new int[w_h];

         // fill image with checkerboard ) 
         for (int i = 0; i < h; i++)
             for (int j = 0; j < w; j++)
             {
                 if ((i & 32) == (j & 32))
                     buf[i * w + j] = 0x00000000;
                 else
                     buf[i * w + j] = 0xffffffff;
             }

         // set up D3D11_SUBRESOURCE_DATA 
         D3D11_SUBRESOURCE_DATA tbsd = {};
         tbsd.pSysMem = (void*)buf;
         tbsd.SysMemPitch = w * bpp;

         // set up D3D11_TEXTURE2D_DESC 
         D3D11_TEXTURE2D_DESC tdesc = {};
         tdesc.Width = w;
         tdesc.Height = h;
         tdesc.MipLevels = 1;
         tdesc.ArraySize = 1;
         tdesc.SampleDesc.Count = 1;
         tdesc.SampleDesc.Quality = 0;
         tdesc.Usage = D3D11_USAGE_DEFAULT;
         tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
         tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
         tdesc.CPUAccessFlags = 0;
         tdesc.MiscFlags = 0;
         Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
         // create the texture
         GetDevice(gfx)->CreateTexture2D(&tdesc, &tbsd, &pTexture);

         D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
         srvDesc.Format = tdesc.Format;
         srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
         srvDesc.Texture2D.MostDetailedMip = 0;
         srvDesc.Texture2D.MipLevels = 1;
		 // create view
         GetDevice(gfx)->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pTexView);
        
         delete[] buf;
	 }
}

void Texture::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->PSSetShaderResources(slot, 1u, pTexView.GetAddressOf());
}

std::pair<UINT, UINT> Texture::MakeNormalMap(float w, float h, std::wstring filename)
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

	int reach = 0;

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

	CprArray<unsigned char> pxs(npx_h, (size_t)npx_w*3u);		// RGB color channel array to store normal in 
	CprArray<unsigned char> pxs_A(npx_h, npx_w);				// alpha channel array to store height value
	CprArray<unsigned char> hm(npx_h, npx_w);					// height map

	auto CalculateMap = [&pxs, &pxs_A, &w, &h](size_t row, size_t col)
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
		x0 = (col) ? pxs_A(row, col-1) : pxs_A(row, col);						// 	   |        |        |        |
		x1 = (col+1 < c_w) ? pxs_A(row, col+1) : pxs_A(row, col);				//     |   x0   |  pix   |   x1   |
		y0 = (row) ? pxs_A(row-1, col) : pxs_A(row, col);						//    _|________|________|________|_
		y1 = (row < c_h) ? pxs_A(row+1, col) : pxs_A(row, col);					//     |        |        |        |
		xy0 = (col && row) ? pxs_A(row-1, col-1) : pxs_A(row, col);				//     |  xy0   |   y0   |   xy1  |
		xy1 = (col+1<c_w && row) ? pxs_A(row-1, col+1) : pxs_A(row, col);		//    _|________|________|________|_
		xy2 = (col && row<c_h) ? pxs_A(row+1, col-1) : pxs_A(row, col);			// 	   |		|		 |		  |
		xy3 = (col+1<c_w && row<c_h) ? pxs_A(row+1, col+1) : pxs_A(row, col);	// 
		//calculate normal and translate value range to 0-255
		float n_x = scaleX * float(2*(x0-x1) + (xy0-xy1) + (xy2-xy3));
		float n_y = scaleY * float(2*(y0-y1) + (xy0-xy2) + (xy2-xy3));
		XMVECTOR pixel_n = XMVector3Normalize(XMVectorSet(n_x, n_y, 1.0f, 0.0f));
		unsigned char r = (unsigned char)((XMVectorGetX(pixel_n) + 1)/2 * 255);
		unsigned char g = (unsigned char)((XMVectorGetY(pixel_n) + 1)/2 * 255);
		unsigned char b = (unsigned char)((XMVectorGetZ(pixel_n) + 1)/2 * 255);
		//save the normal components in the color channels array. 
		pxs(row, col*3 + 0) = r;
		pxs(row, col*3 + 1) = g;
		pxs(row, col*3 + 2) = b;
	};
	auto CalculateHeight2 = [&pxs_A, &weightGrid, &reach](int pix_x, int pix_y)
	{
		int cols = (int)pxs_A.ColumnCount()-1;
		int rows = (int)pxs_A.RowCount()-1;

		int& r = reach;
		int west = (pix_x<r) ? pix_x : r;
		int east = (pix_x+r) > cols ? (cols-pix_x) : r;
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
	
	for (size_t i = 0; i < npx_h; i++) //Load color and alpha pixel arrays with image pixel data
	{
		for (size_t j = 0; j < npx_w; j++)
		{
			UINT bmi_i = UINT(i*stride+j*bpp);
			pxs_A(i, j) = bitmapImage[bmi_i];
			/*for (size_t k = 0; k < 3; k++)
				pxs(i, j*3+k) = bitmapImage[bmi_i];*/
		}
	}
	for (size_t i = 0; i < npx_h; i++) //Load color and alpha pixel arrays with image pixel data
	{
		for (size_t j = 0; j < npx_w; j++)
		{
			unsigned char height = (unsigned char)CalculateHeight2((int)j, (int)i);
			pxs_A(i, j) = height;
		}
	}
	for (size_t i = 0; i < npx_h; i++) //calculate and store normal data in R/G/B array
	{
		for (size_t j = 0; j < npx_w; j++)
			CalculateMap(i, j);
	}
	delete[] bitmapImage;

	size_t size = size_t(npx_h)*size_t(npx_w)*4;
	okenow.reserve(size);
	for (size_t i = 0; i < npx_h; i++)
	{
		for (size_t j = 0; j < npx_w; j++)
		{
			for (size_t k = 0; k < 3; k++)
				okenow.push_back(pxs(  npx_h-1-i, j*3+k));
			okenow.push_back(pxs_A(npx_h-1-i, j));
		}
	}
	return std::pair<UINT, UINT>(npx_w, npx_h);
}

//	filename = filename + L".bmp";
//	const wchar_t* file = filename.c_str();
//	FILE* filePtr;
//	bool error = false;
//	BITMAPFILEHEADER bitmapFileHeader;
//	BITMAPINFOHEADER bitmapInfoHeader;
//	int imageSize;
//	auto Error = [&filename, &error](std::wstring message)
//	{
//		//create message
//		message += filename;
//		MessageBox(0, message.c_str(),    //display message
//			L"Error", MB_OK);
//		if (!error) error = true;
//	};
//
//	if (_wfopen_s(&filePtr, file, L"r+b")!=0)
//		Error(L"Could not open ");
//	if (fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1)
//		Error(L"Could not read bitmap file header of  ");
//	if (fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1)
//		Error(L"Could not read bitmap info header of  ");
//
//	imageSize = int(bitmapFileHeader.bfSize-bitmapFileHeader.bfOffBits); // Calculate the byte size of the bitmap image data
//	if (!imageSize)
//		Error(L"ImageSize is equal to zero for file ");
//
//	unsigned char* bitmapImage = new unsigned char[imageSize];	// Move to the beginning of the bitmap data.
//	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);		// Read in the bitmap image data.
//	fread(bitmapImage, 1, imageSize, filePtr);
//	fclose(filePtr);
//
//	UINT bpp = bitmapInfoHeader.biBitCount/8;		//bytes per pixel
//	UINT p_w = bitmapInfoHeader.biWidth;			//pixel width												
//	UINT p_h = bitmapInfoHeader.biHeight;			//pixel height
//	DWORD stride = ((((bitmapInfoHeader.biWidth * bitmapInfoHeader.biBitCount) + 31) & ~31) >> 3);	//chars per line
//
//	CprArray<unsigned char> pxs(p_h, (size_t)p_w*4u);		//4 channel pixel array RGBA
//	auto CalculateMap = [&pxs, &w, &h](size_t row, size_t col)
//	{
//		using namespace DirectX;										//				Sobel style
//		//Get surrounding pixel values									//     _|________|________|________|_
//		unsigned char x0 = {}, x1 = {}, y0 = {}, y1 = {};				//      |        |        |        |
//		size_t c_h = pxs.RowCount()-1;									//      |  xy2   |   y1   |   xy3  |
//		size_t c_w = pxs.ColumnCount()-1;								//     _|________|________|________|_
//		float scaleX = float(w/c_h);									//		|        |        |        |
//		float scaleY = float(h/c_h);									//		|   x0   |  pix   |   x1   |
//		//sample surrounding pixels height value (if there is one)		//	   _|________|________|________|_
//		x0 = (col)		   ? pxs(row  , col-1) : pxs(row, col+3);		//      |        |        |        |
//		x1 = (col+4 < c_w) ? pxs(row  , col+7) : pxs(row, col+3);		//      |  xy0   |   y0   |   xy1  |
//		y0 = (row)		   ? pxs(row-1, col+3) : pxs(row, col+3);		//     _|________|________|________|_
//		y1 = (row < c_h)   ? pxs(row+1, col+3) : pxs(row, col+3);		//		|		 |		  |		   |
//		//calculate normal and translate to byte values
//		unsigned char xy0 = {}, xy1 = {}, xy2 = {}, xy3 = {};
//		xy0 = (col && row)           ? pxs(row-1, col-1) : pxs(row, col+3);
//		xy1 = (col+4<c_w && row)     ? pxs(row-1, col+7) : pxs(row, col+3);
//		xy2 = (col && row<c_h)       ? pxs(row+1, col-1) : pxs(row, col+3);
//		xy3 = (col+4<c_w && row<c_h) ? pxs(row+1, col+7) : pxs(row, col+3);
//		float n_x = scaleX * float(2*(x0-x1) + (xy0-xy1) + (xy2-xy3));
//		float n_y = scaleY * float(2*(y0-y1) + (xy0-xy2) + (xy2-xy3));
//		XMVECTOR pixel_n = XMVector3Normalize(XMVectorSet(n_x, n_y, 1.0f, 0.0f));
//		unsigned char r = unsigned char(((XMVectorGetX(pixel_n) + 1)/ 2) * 255);
//		unsigned char g = unsigned char(((XMVectorGetY(pixel_n) + 1)/ 2) * 255);
//		unsigned char b = unsigned char(((XMVectorGetZ(pixel_n) + 1)/ 2) * 255);
//		//save the normal components in color channels. 
//		pxs(row, col + 0) = r;
//		pxs(row, col + 1) = g;
//		pxs(row, col + 2) = b;
//		//sample 'extra' surrounding pixels height value 
//		//unsigned char xy0 = {}, xy1 = {}, xy2 = {}, xy3 = {};
//		//xy0 = (column && row < c_h )		  ? pxs(row+1, column-cpp) : pxs(row, column);
//		//xy1 = (column && row)				  ? pxs(row-1, column-cpp) : pxs(row, column);
//		//xy2 = (row && column < c_w+cpp)		  ? pxs(row-1, column+cpp) : pxs(row, column);
//		//xy3 = (row < c_h && column < c_w+cpp) ? pxs(row+1, column+cpp) : pxs(row, column);
//	};
//
//	for (size_t i = 0; i < p_h; i++) //Load pixel array with image pixel data
//	{
//		for (size_t j = 0; j < p_w; j++)
//		{
//			UINT bmi_i = UINT(i*stride+j*bpp);
//			for (size_t k = 0; k < 4; k++)
//				pxs(i, j*4+k) = bitmapImage[bmi_i];
//		}
//	}
//	for (size_t i = 0; i < p_h; i++) //Calculate normal and HeightMap
//	{
//		for (size_t j = 0; j < p_w; j++)
//		{
//			CalculateMap(i, j*4);
//		}
//	}
//	delete[] bitmapImage;
//	UINT size = UINT(4000000);
//	okenow.reserve(size);
//	for (size_t i = 0; i < p_h; i++)
//	{
//		for (size_t j = 0; j < 4000; j++)
//		{
//			okenow.push_back(pxs(999-i,j));
//		}
//	}
//	return std::pair<UINT, UINT>(1000, 1000);
//}