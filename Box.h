#pragma once
#include "DrawableBase.h"
#include "IndexBuffer.h"

class Box : public DrawableBase<Box>
{
public:
	Box(Graphics& gfx);
	Box(Graphics& gfx, const std::wstring& texturePath);
	void UpdateSky(Graphics& gfx, int skyID) noexcept;
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
private:
	std::vector<std::unique_ptr<Bindable>> effects;
	bool useEffect = false;
	//box rotation
	float r;
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	// model transform
	DirectX::XMFLOAT4X4 modelTransform = {};
};