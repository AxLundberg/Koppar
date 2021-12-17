#pragma once
#include "Winhead.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <vector>
#include "MaMath.h"
#include <cmath>
#include <memory>
#include <random>

#include <random>
#include "Helpers.h"
#include "Camera.h"

class DepthStencil;

class Graphics
{
	//friend class Bindable;
	friend class GraphicsResource;
public:
	Graphics(HWND hWnd, int width, int height);
	Graphics(const Graphics&) = delete;
	Graphics& operator = (const Graphics&) = delete;
	~Graphics();
	void EndFrame();
	void BeginFrame(float red, float green, float blue) noexcept;
	void DrawIndexed(UINT count) noexcept;
	void DrawIndexed(UINT count, UINT startIndexLoc, INT baseVertexLoc) noexcept;
	void DrawInstanced(UINT count, DirectX::XMFLOAT3 vertexpos) noexcept;

	void BindSwapBuffer() noexcept;
	UINT GetWidth() const noexcept;
	UINT GetHeight() const noexcept;

	/*--Below is likely to change--*/
	void SetProjection(DirectX::FXMMATRIX proj) noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;
	void SetCamera(DirectX::FXMMATRIX cam) noexcept;
	DirectX::XMMATRIX GetCamera() const noexcept;
	void SetCameraRot(DirectX::XMMATRIX camRot) noexcept;
	DirectX::XMMATRIX GetCameraRot() const noexcept;
	void SetCameraPitchYawRoll(DirectX::XMFLOAT3 camRot);
	DirectX::XMFLOAT3 GetCameraPitchYawRoll() const noexcept;
	void SetCameraPosition(DirectX::XMFLOAT3 camPosition) noexcept;
	DirectX::XMFLOAT3 GetCameraPosition() const noexcept;
	void EnableImgui() noexcept;
	void DisableImgui() noexcept;
	bool IsImguiEnabled() const noexcept;
	static void init() noexcept;
	static float random() noexcept;
	static float random(float mean, float deviation) noexcept;
	ID3D11DeviceContext* GetContext()
	{
		return pContext.Get();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
	bool imguiEnabled = true;
private:
	UINT width;
	UINT height;
	static std::mt19937 s_Random;
	static std::uniform_int_distribution<std::mt19937::result_type> s_distrib;
	DirectX::XMMATRIX camera = {};
	DirectX::XMMATRIX cameraRot = {};
	DirectX::XMFLOAT3 cameraPitchYawRoll = {};
	DirectX::XMFLOAT3 cameraPosition = {};
	DirectX::XMMATRIX projection = {};
};
