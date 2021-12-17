#pragma once
#include "DrawableBase.h"

class Terrain : public DrawableBase<Terrain>
{
public:
	Terrain(Graphics& gfx, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot,
			int reach, float heightAttenuation, float divX, float divY,
			float width, float height, std::wstring filename);
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;
private:
	//model scale transform
	DirectX::XMFLOAT3X3 mt = {};
	//model rotation
	struct
	{
		float roll;
		float pitch;
		float yaw;
	}rot;
	//model translation
	DirectX::XMFLOAT3 pos = {};
	//granularity and size
	float divX;
	float divY;
	float width;
	float height;
};