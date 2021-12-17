#pragma once
#include "DrawableBase.h"

class PortalWall : public DrawableBase<PortalWall>
{
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormal;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoord;
	};
public:
	PortalWall(
		Graphics& gfx,
		DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot,
		float frameWidth, float frameHeight,
		float portalWidth, float portalHeight
	);
	PortalWall(
		Graphics& gfx,
		DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot,
		float frameWidth, float frameHeight,
		float portalWidth, float portalHeight,
		std::wstring texture
	);
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
			std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;
private:
	void CalculateBTN(std::vector<Vertex>& vertices, std::vector<int>& indices);
	//model scale transformation
	DirectX::XMFLOAT3X3 modelTransform = {};
	std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> mBoundingBox;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 rotRPY;
	// size
	float frameX;
	float frameY;
	float portalX;
	float portalY;
};