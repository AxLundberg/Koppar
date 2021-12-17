#pragma once
#include "DrawableBase.h"

class Cylinder : public DrawableBase<Cylinder>
{
private:
	struct rotation
	{
		float roll;
		float pitch;
		float yaw;
	}rot;
	struct transformation
	{
		float x;
		float y;
		float z;
		float roll;
		float pitch;
		float yaw;
	}move;
public:
	Cylinder(Graphics& gfx,
		float posx, float posy, float posz,
		float roll, float pitch, float yaw,
		float scaleX, float scaleY, float scaleZ,
		DirectX::XMFLOAT3 color
		);
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
			std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;
private:
	//color
	DirectX::XMFLOAT3 color;
	// positional
	//float posx = 0.0f;
	//float posy = 0.0f;
	//float posz = 0.0f;
	float r = 0.0f;
	/*float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;*/
	float theta = 0.0f;
	float phi = 0.0f;
	float chi = 0.0f;
	// speed (delta/s)
	float droll = 0.0f;
	float dpitch = 0.0f;
	float dyaw = 0.0f;
	float dtheta = 0.0f;
	float dphi = 0.0f;
	float dchi = 0.0f;
	//scale
	DirectX::XMFLOAT3X3 modelTransform = {};
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleZ = 1.0f;
};