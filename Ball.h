#pragma once
#include "DrawableBase.h"

class Ball : public DrawableBase<Ball>
{
public:
	Ball(Graphics& gfx, float radius,
		DirectX::XMFLOAT3 pos,
		DirectX::XMFLOAT3 color
	);
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;
	void SetPosition(DirectX::FXMVECTOR pos) noexcept;
	void SetRotation(DirectX::XMFLOAT3 rot) noexcept;
private:
	//color
	DirectX::XMFLOAT3 mColor;
	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mRotation;
	DirectX::XMFLOAT3X3 mModelTransform = {};

	float mRadius;
};