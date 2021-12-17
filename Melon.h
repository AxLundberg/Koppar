#pragma once
#include "DrawableBase.h"

class Melon : public DrawableBase<Melon>
{
public:
	Melon(Graphics& gfx, float radius, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f });
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
			std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
private:
	DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
};