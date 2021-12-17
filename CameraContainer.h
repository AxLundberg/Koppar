#pragma once
#include <vector>
#include <memory>
#include "imgui.h"
#include "Camera.h"
#include "Graphics.h"
#include "CbufferCameraTransform.h"

class Camera;
class Graphics;

class CameraContainer
{
public:
	CameraContainer(Graphics& gfx)
	{
		pCameraCbuffer = std::make_unique<CBufferCameraTransform>(gfx, 1u);
	}
	void SpawnControlWindow()
	{
		if (ImGui::Begin("Cameras"))
		{
			if (ImGui::BeginCombo("", mCameraPtrs[currentCam]->GetName().c_str()))
			{
				for (int i = 0; i < std::size(mCameraPtrs); i++)
				{
					if (ImGui::Selectable(mCameraPtrs[i]->GetName().c_str(), (i == currentCam)))
					{
						currentCam = i;
					}
				}
				ImGui::EndCombo();
			}
			GetCamera().SpawnControlWindow();
		}
	}

	void Bind(Graphics& gfx) const
	{
		gfx.SetCamera(GetCamera().GetMatrix());
	}

	void Bind(Graphics& gfx, std::string cameraName) const
	{
		for (auto& c : mCameraPtrs)
		{
			if (c.get()->GetName() == cameraName)
			{
				gfx.SetCamera(c.get()->GetMatrix());
			}
		}
	}

	void BindCBufferCamera(Graphics& gfx, std::string cameraName) const
	{
		for (auto& c : mCameraPtrs)
		{
			if (c.get()->GetName() == cameraName)
			{
				pCameraCbuffer->SetCamera(c.get());
				pCameraCbuffer->Bind(gfx);
			}
		}
	}
	void UpdateCBufferCamera(Graphics& gfx) const noexcept
	{
		pCameraCbuffer->Update(gfx);
	}
	void UpdateCBufferCamera(Graphics& gfx, DirectX::XMFLOAT3 focalPoint) const noexcept
	{
		pCameraCbuffer->Update(gfx, focalPoint);
	}

	void AddCamera(std::unique_ptr<Camera> pCam) noexcept
	{
		mCameraPtrs.push_back(std::move(pCam));
	}

	Camera& GetCamera() const
	{
		return *mCameraPtrs[currentCam];
	}

	Camera& GetCamera(std::string cameraName) const
	{
		for (auto& c : mCameraPtrs)
		{
			if (c.get()->GetName() == cameraName)
			{
				return *c;
			}
		}
		return *mCameraPtrs[currentCam];
	}

	~CameraContainer()
	{}
private:
	std::vector<std::unique_ptr<Camera>> mCameraPtrs;
	std::unique_ptr<CBufferCameraTransform> pCameraCbuffer;
	int currentCam = 0;
};