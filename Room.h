#pragma once
#include "Portal.h"
#include "Drawable.h"
#include "Model.h"
#include "ModelAi.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>

class Room
{
	friend class Frustrum;
public:
	Room(DirectX::XMFLOAT3 position, float radius);
	Room& GetAdjacentRoom(size_t portalIndex);
	friend bool operator==(const Room& lhs, const Room& rhs)
	{
		return lhs.mPos.x == rhs.mPos.x && lhs.mPos.y == rhs.mPos.y && lhs.mPos.z == rhs.mPos.z;
	}
	const std::vector<DirectX::XMFLOAT4>& GetFrustrum() const;
	~Room();
private:
	void DrawCell(Graphics& gfx, Portal<Room>* fromPortal = nullptr);
	void UpdateFrustrum(const DirectX::FXMVECTOR& camPos, DirectX::FXMMATRIX& viewprojection, Portal<Room>* fromPortal = nullptr);
	void UpdateFrustrumThroughPortal(const DirectX::FXMVECTOR& camPos, Portal<Room>* fromPortal);
	void UpdateFrustrumFromView(const DirectX::FXMVECTOR& camPos, DirectX::FXMMATRIX& viewprojection, Portal<Room>* fromPortal = nullptr);
	bool CullObject(const DirectX::FXMVECTOR& bBoxPos, const float& r, Portal<Room>* fromPortal = nullptr);
	bool CullBoundingBox(const DirectX::FXMVECTOR& bBoxPos, const float& r);
	bool CullBoundingBox(const DirectX::FXMVECTOR& bBoxPos, const DirectX::XMFLOAT3& bBoxRadii);
	bool PointWithinRoom(const DirectX::XMFLOAT3& point) const;
	bool PointWithinRoom(const DirectX::FXMVECTOR& point) const;
	bool PointWithinRoom(const DirectX::FXMVECTOR&& point) const;
	bool PointWithinFrustrum(const DirectX::FXMVECTOR& point) const;
	void AddPortal(Room& otherRoom, const std::array<DirectX::XMFLOAT3, 4>& corners);
	void AddPortal(std::shared_ptr<Portal<Room>> portal);

	void ClearFrustrum();
public:
	DirectX::XMFLOAT3 mPos;
	float mRadius;
private:
	std::vector<std::unique_ptr<Drawable>> mDrawables;
	std::vector<std::unique_ptr<Model>> mModels;
	std::vector<std::unique_ptr<ModelAi>> mModelsAi;
	std::vector<std::shared_ptr<Portal<Room>>> mPortals;
	std::vector<DirectX::XMFLOAT4> mFrustrum;
};