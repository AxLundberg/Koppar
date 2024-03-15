#pragma once
#include "DrawableBase.h"

class TestPlane : public DrawableBase<TestPlane>
{
public:
	TestPlane(Graphics& gfx, std::wstring vs, std::wstring ps, DirectX::FXMMATRIX tf, float width = 0.0f, float height = 0.0f);
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
			std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;
	void SetTransformFromPlane(DirectX::XMVECTOR plane);
	void SetPosition(DirectX::XMVECTOR pos);
private:
	//model scale transformation
	DirectX::XMFLOAT3X3 modelTransform = {};
	//rotation
	struct
	{
		float roll{};
		float pitch{};
		float yaw{};
	}rot = {};
	//position
	DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
	// size
	float width;
	float height;
};