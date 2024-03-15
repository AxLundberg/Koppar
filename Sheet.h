#pragma once
#include "DrawableBase.h"

class Sheet : public DrawableBase<Sheet>
{
public:
	Sheet(Graphics& gfx, std::wstring vs, std::wstring ps, float width = 0.0f, float height = 0.0f);
	Sheet(Graphics& gfx,DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, float width, float height, std::wstring texture);
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
			std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;
	void SetPosition(DirectX::XMVECTOR) noexcept;

private:
	//model scale transformation
	DirectX::XMFLOAT3X3 modelTransform = {};
	//rotation
	struct
	{
		float roll {};
		float pitch {};
		float yaw {};
	}rot = {};
	//position
	DirectX::XMFLOAT3 pos = {};
	// size
	float width;
	float height;
};