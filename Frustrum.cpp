#include "Frustrum.h"

Frustrum::Frustrum()
{
	using namespace DirectX;

	mRooms.push_back(std::make_unique<Room>(XMFLOAT3{ 25.0f,  25.0f,  25.0f }, 25.0f));
	mRooms.push_back(std::make_unique<Room>(XMFLOAT3{ -25.0f,  25.0f,  25.0f }, 25.0f));
	mRooms.push_back(std::make_unique<Room>(XMFLOAT3{ -25.0f,  25.0f, -25.0f }, 25.0f));
	mRooms.push_back(std::make_unique<Room>(XMFLOAT3{ -25.0f, -25.0f,  25.0f }, 25.0f));

	const std::array<XMFLOAT3, 4> corners = { XMFLOAT3(0.0f, 5.0f,  5.0f),	//lowL
											  XMFLOAT3(0.0f, 15.0f, 5.0f),	//highL	
											  XMFLOAT3(0.0f, 5.0f,  15.0f),	//lowR
											  XMFLOAT3(0.0f, 15.0f, 15.0f)	//highR
	};

	mRooms[0].get()->AddPortal(*mRooms[1].get(), corners);

	const std::array<XMFLOAT3, 4> corners2 = { XMFLOAT3(-15.0f, 5.0f, 0.0f),
											   XMFLOAT3(-15.0f, 15.0f, 0.0f),
											   XMFLOAT3(-5.0f,  5.0f, 0.0f),
											   XMFLOAT3(-5.0f,  15.0f, 0.0f)
	};

	mRooms[1].get()->AddPortal(*mRooms[2].get(), corners2);

	const std::array<XMFLOAT3, 4> corners3 = { XMFLOAT3(-25.0f, 0.0f, 25.0f),
											   XMFLOAT3(-25.0f, 0.0f, 15.0f),
											   XMFLOAT3(-15.0f,  0.0f, 25.0f),
											   XMFLOAT3(-15.0f,  0.0f, 15.0f)
	};

	mRooms[1].get()->AddPortal(*mRooms[3].get(), corners3);

	auto test = mRooms[0].get()->GetAdjacentRoom(0).mPos;
}
void Frustrum::Update(const DirectX::FXMVECTOR& camPos, const DirectX::FXMMATRIX& viewprojection)
{
	UpdateCurrentRoom(camPos);
	if (mCurrRoomPtr != nullptr)
	{
		mCurrRoomPtr->UpdateFrustrum(camPos, viewprojection);
	}
}
void Frustrum::UpdateCurrentRoom(const DirectX::FXMVECTOR& camPos)
{
	if (mCurrRoomPtr == nullptr) //Check all rooms
	{
		return SetCurrRoom(camPos);
	}
	else if (mCurrRoomPtr->PointWithinRoom(camPos)) //Still in same room
	{
		return;
	}
	//Check adjacent rooms
	for (size_t i = 0; i < mCurrRoomPtr->mPortals.size(); i++)
	{
		auto& adjacentRoom = mCurrRoomPtr->GetAdjacentRoom(i);
		if (adjacentRoom.PointWithinRoom(camPos))
		{
			mCurrRoomPtr = &mCurrRoomPtr->GetAdjacentRoom(i);
			return;
		}
	}

	mCurrRoomPtr = nullptr; //was in no room
}
void Frustrum::SetCurrRoom(const DirectX::FXMVECTOR& camPos)
{
	for (size_t i = 0; i < mRooms.size(); i++)
	{
		if (mRooms[i].get()->PointWithinRoom(camPos))
		{
			mCurrRoomPtr = mRooms[i].get();
			return;
		}
	}
	mCurrRoomPtr = nullptr;
}
void Frustrum::DrawCells(Graphics& gfx)
{
	if (mCurrRoomPtr == nullptr)
	{
		return;
	}
	mCurrRoomPtr->DrawCell(gfx);
}
bool Frustrum::CullObject(const DirectX::FXMVECTOR& bBoxPos, const float& bBoxRadius) const
{
	if (mCurrRoomPtr == nullptr)
	{
		return false;
	}
	else
	{
		return mCurrRoomPtr->CullObject(bBoxPos, bBoxRadius);
	}

	return true;
}
bool Frustrum::CullObject(DirectX::FXMVECTOR&& bBoxPos, const float& bBoxRadius) const
{
	if (mCurrRoomPtr == nullptr)
	{
		return false;
	}
	else
	{
		return mCurrRoomPtr->CullObject(bBoxPos, bBoxRadius);
	}

	return true;
}

void Frustrum::ClearFrustrums()
{
	if (mCurrRoomPtr != nullptr)
	{
		mCurrRoomPtr->ClearFrustrum();
	}
}
const std::vector<std::unique_ptr<Room>>& Frustrum::GetRooms() const
{
	return mRooms;
}
const std::vector<DirectX::XMFLOAT4>& Frustrum::GetFrustrum(int id)
{
	return mRooms[id].get()->GetFrustrum();
}

void Frustrum::AddModel(std::unique_ptr<Model>&& obj)
{
	for (auto& r : mRooms)
	{
		if (r.get()->PointWithinRoom(obj.get()->GetBoundingBox().first))
		{
			r.get()->mModels.emplace_back(std::move(obj));
			break;
		}
	}
}

void Frustrum::AddDrawables(std::unique_ptr<Drawable>&& obj)
{
	for (auto& r : mRooms)
	{
		if (r.get()->PointWithinRoom(obj.get()->GetBoundingBox().first))
		{
			r.get()->mDrawables.emplace_back(std::move(obj));
			break;
		}
	}
}

void Frustrum::AddModelAi(const std::unique_ptr<ModelAi>& obj)
{	
	for (auto& r : mRooms)
	{
		/*if (r.get()->PointWithinRoom(obj.get()->GetBoundingBox().second))
		{
			r.get()->mModels.emplace_back(std::move(obj));
			break;
		}*/
	}
}
bool Frustrum::Inside()
{
	return mCurrRoomPtr != nullptr;
}
Frustrum::~Frustrum()
{
}