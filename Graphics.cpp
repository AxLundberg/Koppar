#include "Graphics.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "DepthStencil.h"

namespace dx = DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

std::mt19937 Graphics::s_Random;
std::uniform_int_distribution<std::mt19937::result_type> Graphics::s_distrib;

Graphics::Graphics(HWND hWnd, int width, int height)
	:
	width(width), height(height)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// device, f/b buff and swapchain creation
	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	);
	//access backbuffer (subresource) in swapchain
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;
	pSwap->GetBuffer(
		0,										//index buff('0' for backbuff) (subobject)
		__uuidof(ID3D11Resource),				//uuid of interface to recieve on subobject
		&pBackBuffer							//pointer to pointer to fill
	);

	pDevice->CreateRenderTargetView(
		pBackBuffer.Get(),						//resource to get view on
		nullptr,								//desc for additional targetview info
		&pTarget								//pointer to pointer to fill
	);

	//create depth stencil
	//D3D11_DEPTH_STENCIL_DESC dsd = {};
	//dsd.DepthEnable = TRUE;
	//dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//dsd.DepthFunc = D3D11_COMPARISON_LESS;

	//Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
	//pDevice->CreateDepthStencilState(&dsd, &pDSState);

	////bind depth state (output merger)
	//pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	//create depth stensil texture
	//Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	//D3D11_TEXTURE2D_DESC depthDesc = {};
	//depthDesc.Width = width;
	//depthDesc.Height = height;
	//depthDesc.MipLevels = 1u;
	//depthDesc.ArraySize = 1u;
	//depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;;
	//depthDesc.SampleDesc.Count = 1u;
	//depthDesc.SampleDesc.Quality = 0u;
	//depthDesc.Usage = D3D11_USAGE_DEFAULT;
	//depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//pDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthStencil  );

	////view creation for depth stencil texture
	//D3D11_DEPTH_STENCIL_VIEW_DESC desc_DSV = {};
	//desc_DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;;
	//desc_DSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//desc_DSV.Texture2D.MipSlice = 0u;
	//pDevice->CreateDepthStencilView(pDepthStencil.Get(), &desc_DSV, &pDSV);

	////bind that mofo (output merger)
	//pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

		//configure viewport
	D3D11_VIEWPORT vp[1] = {};
	vp[0].Width = (float)width;
	vp[0].Height = (float)height;
	vp[0].MinDepth = 0;
	vp[0].MaxDepth = 1;
	vp[0].TopLeftX = 0;
	vp[0].TopLeftY = 0;

	pContext->RSSetViewports(1u, vp);

	// init imgui d3d impl
	ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());
}

Graphics::~Graphics()
{
	ImGui_ImplDX11_Shutdown();
}

void Graphics::EndFrame()
{
	if (imguiEnabled)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	pSwap->Present(1u, 0u);
	//pSwap->Present(0u, 0u);
}

void Graphics::BeginFrame(float red, float green, float blue) noexcept
{
	// imgui begin frame
	if (imguiEnabled)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}
	ID3D11ShaderResourceView* const nulltex = nullptr;
	pContext->PSSetShaderResources(2, 1, &nulltex); //shadow map tex
	const float color[] = { red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

void Graphics::DrawIndexed(UINT count) noexcept
{
	pContext->DrawIndexed(count, 0u, 0u);
}
void Graphics::DrawIndexed(UINT count, UINT startIndexLoc, INT baseVertexLoc) noexcept
{
	pContext->DrawIndexed(count, startIndexLoc, baseVertexLoc);
}

void Graphics::DrawInstanced(UINT count, DirectX::XMFLOAT3 vertexpos) noexcept
{
	auto nParticles = 256u;
	pContext->DrawInstanced( nParticles, 1, 0, 0 );
}

void Graphics::BindSwapBuffer() noexcept
{
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);
}

UINT Graphics::GetWidth() const noexcept
{
	return width;
}

UINT Graphics::GetHeight() const noexcept
{
	return height;
}

void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
{
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}

void Graphics::SetCamera(DirectX::FXMMATRIX cam) noexcept
{
	camera = cam;
}

DirectX::XMMATRIX Graphics::GetCamera() const noexcept
{
	return camera;
}

void Graphics::SetCameraRot(DirectX::XMMATRIX camRot) noexcept
{
	cameraRot = camRot;
}
DirectX::XMMATRIX Graphics::GetCameraRot() const noexcept
{
	return cameraRot;
}

void Graphics::SetCameraPitchYawRoll(DirectX::XMFLOAT3 camPitchYawRoll)
{
	cameraPitchYawRoll = camPitchYawRoll;
}
DirectX::XMFLOAT3 Graphics::GetCameraPitchYawRoll() const noexcept
{
	return cameraPitchYawRoll;
}

DirectX::XMFLOAT3 Graphics::GetCameraPosition() const noexcept
{
	return cameraPosition;
}

void Graphics::SetCameraPosition(DirectX::XMFLOAT3 camPosition) noexcept
{
	cameraPosition = camPosition;
}

void Graphics::EnableImgui() noexcept
{
	imguiEnabled = true;
}

void Graphics::DisableImgui() noexcept
{
	imguiEnabled = false;
}

bool Graphics::IsImguiEnabled() const noexcept
{
	return imguiEnabled;
}

void Graphics::init() noexcept
{
	s_Random.seed(std::random_device()());
}

float Graphics::random() noexcept
{
	return (float)s_distrib(s_Random)/(float)std::numeric_limits<uint32_t>::max();
}

float Graphics::random(float mean, float deviation) noexcept
{
	std::normal_distribution<float> n_distrib{ mean, deviation };
	return (float)n_distrib(s_Random);
}
//void Graphics::HelloTriangle( float angle, float x, float z)
//{
//	float w = 1200.0f;
//	float h = 800.0f;
//
//	struct Vertex
//	{
//		struct 
//		{
//			float x;
//			float y;
//			float z;
//			float w;
//		}pos;
//	};
//	      /*CREATE VERTEX BUFFER*/
//	//array of vertices
//	const Vertex vertices[] =
//	{
//		{ -0.4f, -0.4f, -0.4f, 1.0f }, 
//		{  0.4f, -0.4f, -0.4f, 1.0f },
//		{ -0.4f,  0.4f, -0.4f, 1.0f },
//		{  0.4f,  0.4f, -0.4f, 1.0f },
//
//		{  0.0f,  0.0f,  0.5f, 1.0f },
//
//		//{ -0.3f, -0.3f, 0.3f, 1.0f,
//		//0, 255, 0, 0, },
//		//{ 0.3f, -0.3f, 0.3f, 1.0f,
//		//0, 255, 0, 0, },
//		//{ -0.3f, 0.3f, 0.3f, 1.0f,
//		//0, 255, 0, 0, },
//		//{ 0.3f, 0.3f, 0.3f, 1.0f,
//		//0, 255, 0, 0, },
//	};
//
//	//create vertex buff
//	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
//
//	D3D11_BUFFER_DESC buf_d = {};
//	buf_d.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	buf_d.Usage = D3D11_USAGE_DEFAULT;
//	buf_d.CPUAccessFlags = 0u;
//	buf_d.MiscFlags = 0u;
//	buf_d.ByteWidth = sizeof(vertices);
//	buf_d.StructureByteStride = sizeof(Vertex);
//
//	D3D11_SUBRESOURCE_DATA subr_d = {};
//	subr_d.pSysMem = vertices;
//
//	pDevice->CreateBuffer(&buf_d, &subr_d, &pVertexBuffer);
//
//	//Bind vertex buffer to pipeline
//	const UINT stride = sizeof(Vertex);
//	const UINT offset = 0u;
//	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
//
//
//	/*CREATE INDEX BUFFER*/
//	//Index array
//	const u_short indices[] =
//	{
//		0,2,1, 2,3,1,
//
//		1,3,4, 3,2,4,
//		0,4,2, 0,1,4,
//		/*1,3,5, 3,7,5,
//		2,6,3, 3,6,7,
//		4,5,7, 4,7,6,
//		0,4,2, 2,4,6,
//		0,1,4, 1,5,4,*/
//	};
//
//	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
//
//	D3D11_BUFFER_DESC ibuf_d = {};
//	ibuf_d.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	ibuf_d.Usage = D3D11_USAGE_DEFAULT;
//	ibuf_d.CPUAccessFlags = 0u;
//	ibuf_d.MiscFlags = 0u;
//	ibuf_d.ByteWidth = sizeof(indices);
//	ibuf_d.StructureByteStride = sizeof(u_short);
//
//	D3D11_SUBRESOURCE_DATA isubr_d = {};
//	isubr_d.pSysMem = indices;
//
//	pDevice->CreateBuffer(&ibuf_d, &isubr_d, &pIndexBuffer);
//
//	//Bind index buffer to pipeline
//	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
//
//		/*CREATE CONSTANT BUFFER FOR TRANSFORMATION*/
//	struct ConstantBuffer
//	{
//		dx::XMMATRIX transform;
//	};
//
//	const ConstantBuffer cb =
//	{
//		{
//			dx::XMMatrixTranspose(
//				dx::XMMatrixRotationZ(angle) *
//				dx::XMMatrixRotationX(angle) *
//				dx::XMMatrixTranslation(x, 0.0f, z+4.0f) *
//				dx::XMMatrixPerspectiveFovLH( 1.0f, h/w, 0.1f, 10.0f)
//			)
//		}
//	};
//
//	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
//
//	D3D11_BUFFER_DESC cbuf_d = {};
//	cbuf_d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	cbuf_d.Usage = D3D11_USAGE_DYNAMIC;						//Dynamic gives access to LOCK function
//	cbuf_d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	cbuf_d.MiscFlags = 0u;
//	cbuf_d.ByteWidth = sizeof(cb);
//	cbuf_d.StructureByteStride = sizeof(0u);
//
//	D3D11_SUBRESOURCE_DATA csubr_d = {};
//	csubr_d.pSysMem = &cb;
//
//	pDevice->CreateBuffer(&cbuf_d, &csubr_d, &pConstantBuffer);
//
//	//Bind Constant buffer to pipeline
//	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());
//
//	struct ConstantBuffer2
//	{
//		struct 
//		{
//			float r;
//			float g;
//			float b;
//			float a;
//		}face_colors[5];
//	};
//	const ConstantBuffer2 cb2 =
//	{
//		{
//			{1.0f, 0.0f, 1.0f, 1.0f},
//			{1.0f, 0.0f, 1.0f, 1.0f},
//			{0.0f, 1.0f, 0.0f, 1.0f},
//			{1.0f, 1.0f, 0.0f, 1.0f},
//			{0.0f, 1.0f, 1.0f, 1.0f},
//		}
//	};
//	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer2;
//
//	D3D11_BUFFER_DESC cbuf_d2 = {};
//	cbuf_d2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	cbuf_d2.Usage = D3D11_USAGE_DEFAULT;						
//	cbuf_d2.CPUAccessFlags = 0u;
//	cbuf_d2.MiscFlags = 0u;
//	cbuf_d2.ByteWidth = sizeof(cb2);
//	cbuf_d2.StructureByteStride = sizeof(0u);
//
//	D3D11_SUBRESOURCE_DATA csubr_d2 = {};
//	csubr_d2.pSysMem = &cb2;
//
//	pDevice->CreateBuffer(&cbuf_d2, &csubr_d2, &pConstantBuffer2);
//
//	//Bind Constant buffer to pipeline
//	pContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());
//
//	//create pixelshader
//	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
//	//COM pointer to blob (binary blob of data)
//	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
//
//	D3DReadFileToBlob(L"PixelShader.cso", &pBlob);
//	pDevice->CreatePixelShader(
//		pBlob->GetBufferPointer(),
//		pBlob->GetBufferSize(),
//		nullptr,
//		&pPixelShader
//	);
//
//	//bind pixelshader
//	pContext->PSSetShader(
//		pPixelShader.Get(),
//		nullptr,
//		0u
//	);
//
//
//			/*Vertex shader creation*/
//	//COM pointer to shader object
//	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
//	D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
//
//	//Create vertex shader
//	pDevice->CreateVertexShader(
//		pBlob->GetBufferPointer(),  //void ptr to bytecode of shader
//		pBlob->GetBufferSize(),		//Size of buffer
//		nullptr,					//Linkage?
//		&pVertexShader);			//pointer to pointer to be filled
//
//	//bind vertex shader
//	pContext->VSSetShader(
//		pVertexShader.Get(), //ptr to vtexshader
//		nullptr,			 //for instancing?
//		0u					 //ditto
//	);
//
//	//Vertex input layout
//	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
//	const D3D11_INPUT_ELEMENT_DESC ied[] =
//	{
//		{ "POSITION",0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,0 },
//		//{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0},
//	};
//
//	pDevice->CreateInputLayout(
//		ied,
//		(UINT)std::size(ied),
//		pBlob->GetBufferPointer(),
//		pBlob->GetBufferSize(),
//		&pInputLayout
//	);
//
//	//Bind vertex layout
//	pContext->IASetInputLayout(pInputLayout.Get());
//
//	//Set primitive topology
//	pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
//	//pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
//
//	//configure viewport
//	D3D11_VIEWPORT vp[1];
//	vp[0].Width = w;
//	vp[0].Height = h;
//	vp[0].MinDepth = 0;
//	vp[0].MaxDepth = 1;
//	vp[0].TopLeftX = 0;
//	vp[0].TopLeftY = 0;
//
//
//	pContext->RSSetViewports(1u, vp);
//
//
//	pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u);
//
//}


