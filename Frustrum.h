#pragma once
#include "Room.H"
#include <DirectXMath.h>

class Frustrum
{
public:
	Frustrum();
	void Update(const DirectX::FXMVECTOR& camPos, const DirectX::FXMMATRIX& viewprojection);
	void UpdateCurrentRoom(const DirectX::FXMVECTOR& camPos);
	void SetCurrRoom(const DirectX::FXMVECTOR& camPos);
	void DrawCells(Graphics& gfx);
	bool CullObject(const DirectX::FXMVECTOR& bBoxPos, const float& bBoxRadius) const;
	bool CullObject(DirectX::FXMVECTOR&& bBoxPos, const float& bBoxRadius) const;
	const std::vector<std::unique_ptr<Room>>& GetRooms() const;
	const std::vector<DirectX::XMFLOAT4>& GetFrustrum(int id);
	void AddModel(std::unique_ptr<Model>&& obj);
	void AddModelAi(const std::unique_ptr<ModelAi>& obj);
	void AddDrawables(std::unique_ptr<Drawable>&& obj);
	void ClearFrustrums();
	bool Inside();
	~Frustrum();
private:
	Room* mCurrRoomPtr = nullptr;
	std::vector<std::unique_ptr<Room>> mRooms;
};

