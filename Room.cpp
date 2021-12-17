#include "Room.h"

Room::Room(DirectX::XMFLOAT3 position, float radius)
	: mPos(std::move(position)), mRadius(std::move(radius))
{
	mFrustrum.reserve(6);
}
Room::~Room()
{}
Room& Room::GetAdjacentRoom(size_t portalIndex)
{
	return mPortals[portalIndex].get()->GetAdjacentRoom(*this);
}

void Room::DrawCell(Graphics& gfx, Portal<Room>* fromPortal)
{
	for (auto& pm : mDrawables)
	{
		const auto bb = pm.get()->GetBoundingBox();
		if (!CullBoundingBox(DirectX::XMLoadFloat3(&bb.first), bb.second))
		{
			pm.get()->Draw(gfx);
		}
	}
	for (auto& pm : mModels)
	{
		const auto bb = pm.get()->GetBoundingBox();
		if (!CullBoundingBox(DirectX::XMLoadFloat3(&bb.first), bb.second.x))
		{
			pm.get()->Draw(gfx);
		}
	}
	/*for (auto& pm : mModelsAi)
	{
		const auto bb = pm.get()->GetBoundingBox();
		if (!CullBoundingBox(DirectX::XMLoadFloat3(&bb.second), bb.first.x))
		{
			pm.get()->Draw(gfx);
		}
	}*/
	for (auto& p : mPortals)
	{
		if (p.get() != fromPortal && !p.get()->GetAdjacentRoom(*this).mFrustrum.empty())
		{
			p.get()->GetAdjacentRoom(*this).DrawCell(gfx, p.get());
		}
	}
}

void Room::UpdateFrustrum(const DirectX::FXMVECTOR& camPos, DirectX::FXMMATRIX& viewprojection, Portal<Room>* fromPortal)
{
	if (fromPortal == nullptr)
	{
		UpdateFrustrumFromView(camPos, viewprojection);
	}
	else if (fromPortal->PointWithinBoundingBox(camPos))
	{
		mFrustrum = fromPortal->GetAdjacentRoom(*this).mFrustrum;
	}
	else
	{
		UpdateFrustrumThroughPortal(camPos, fromPortal);
	}

	for (auto& p : mPortals)
	{
		if (p.get() != fromPortal && (p.get()->PortalWithinFrustrum(mFrustrum) || p.get()->PointWithinBoundingBox(camPos)))
		{
			p.get()->GetAdjacentRoom(*this).UpdateFrustrum(camPos, viewprojection, p.get());
		}
	}
}
void Room::UpdateFrustrumThroughPortal(const DirectX::FXMVECTOR& camPos, Portal<Room>* fromPortal)
{
	using namespace DirectX;

	auto corners = fromPortal->GetCorners();
	auto portalPlane = XMPlaneFromPoints(corners[0], corners[1], corners[2]);

	XMVECTOR sidePlane1 = {}, sidePlane2 = {}, sidePlane3 = {}, sidePlane4 = {};

	auto nearPlane = XMLoadFloat4(&fromPortal->GetAdjacentRoom(*this).mFrustrum[0]);
	auto farPlane = XMLoadFloat4(&fromPortal->GetAdjacentRoom(*this).mFrustrum[1]);

	if (XMVectorGetX(XMPlaneDotCoord(portalPlane, camPos)) < 0.0f) //ccw side
	{
		sidePlane1 = XMPlaneFromPoints(camPos, corners[0], corners[1]);
		sidePlane2 = XMPlaneFromPoints(camPos, corners[3], corners[2]);
		sidePlane3 = XMPlaneFromPoints(camPos, corners[2], corners[0]);
		sidePlane4 = XMPlaneFromPoints(camPos, corners[1], corners[3]);
	}
	else
	{
		sidePlane1 = XMPlaneFromPoints(camPos, corners[1], corners[0]);
		sidePlane2 = XMPlaneFromPoints(camPos, corners[2], corners[3]);
		sidePlane3 = XMPlaneFromPoints(camPos, corners[0], corners[2]);
		sidePlane4 = XMPlaneFromPoints(camPos, corners[3], corners[1]);
	}

	XMFLOAT4 tmpVec = {};
	XMStoreFloat4(&tmpVec, XMPlaneNormalize(nearPlane));
	mFrustrum.emplace_back(tmpVec);
	XMStoreFloat4(&tmpVec, XMPlaneNormalize(farPlane));
	mFrustrum.emplace_back(tmpVec);
	XMStoreFloat4(&tmpVec, XMPlaneNormalize(sidePlane1));
	mFrustrum.emplace_back(tmpVec);
	XMStoreFloat4(&tmpVec, XMPlaneNormalize(sidePlane2));
	mFrustrum.emplace_back(tmpVec);
	XMStoreFloat4(&tmpVec, XMPlaneNormalize(sidePlane3));
	mFrustrum.emplace_back(tmpVec);
	XMStoreFloat4(&tmpVec, XMPlaneNormalize(sidePlane4));
	mFrustrum.emplace_back(tmpVec);
}
void Room::UpdateFrustrumFromView(const DirectX::FXMVECTOR& camPos, DirectX::FXMMATRIX& viewprojection, Portal<Room>* fromPortal)
{
	using namespace DirectX;

	XMFLOAT4X4 viewProjMat = {};
	XMStoreFloat4x4(&viewProjMat, viewprojection);

	XMFLOAT4 tmpVec = {};
	XMStoreFloat4(&tmpVec,
		XMPlaneNormalize(
			XMVectorSet(viewProjMat._14 + viewProjMat._13,
				viewProjMat._24 + viewProjMat._23,
				viewProjMat._34 + viewProjMat._33,
				viewProjMat._44 + viewProjMat._43
			)));
	mFrustrum.emplace_back(tmpVec);

	//far
	XMStoreFloat4(&tmpVec,
		XMPlaneNormalize(
			XMVectorSet(viewProjMat._14 - viewProjMat._13,
				viewProjMat._24 - viewProjMat._23,
				viewProjMat._34 - viewProjMat._33,
				viewProjMat._44 - viewProjMat._43
			)));
	mFrustrum.emplace_back(tmpVec);

	//left
	XMStoreFloat4(&tmpVec,
		XMPlaneNormalize(
			XMVectorSet(viewProjMat._14 + viewProjMat._11,
				viewProjMat._24 + viewProjMat._21,
				viewProjMat._34 + viewProjMat._31,
				viewProjMat._44 + viewProjMat._41
			)));
	mFrustrum.emplace_back(tmpVec);

	//right
	XMStoreFloat4(&tmpVec,
		XMPlaneNormalize(
			XMVectorSet(viewProjMat._14 - viewProjMat._11,
				viewProjMat._24 - viewProjMat._21,
				viewProjMat._34 - viewProjMat._31,
				viewProjMat._44 - viewProjMat._41
			)));
	mFrustrum.emplace_back(tmpVec);

	//top
	XMStoreFloat4(&tmpVec,
		XMPlaneNormalize(
			XMVectorSet(viewProjMat._14 - viewProjMat._12,
				viewProjMat._24 - viewProjMat._22,
				viewProjMat._34 - viewProjMat._32,
				viewProjMat._44 - viewProjMat._42
			)));
	mFrustrum.emplace_back(tmpVec);

	//Bottom
	XMStoreFloat4(&tmpVec,
		XMPlaneNormalize(
			XMVectorSet(viewProjMat._14 + viewProjMat._12,
				viewProjMat._24 + viewProjMat._22,
				viewProjMat._34 + viewProjMat._32,
				viewProjMat._44 + viewProjMat._42
			)));
	mFrustrum.emplace_back(tmpVec);
}
bool Room::CullObject(const DirectX::FXMVECTOR& bBoxPos, const float& r, Portal<Room>* fromPortal)
{
	bool retval = true;

	if (PointWithinRoom(bBoxPos))
	{
		return CullBoundingBox(bBoxPos, r);
	}

	for (auto& p : mPortals)
	{
		if (p.get() != fromPortal && retval &&
			!p.get()->GetAdjacentRoom(*this).mFrustrum.empty())
		{
			retval = p.get()->GetAdjacentRoom(*this).CullObject(bBoxPos, r, p.get());
		}
	}

	return retval;
}
bool Room::CullBoundingBox(const DirectX::FXMVECTOR& bBoxPos, const float& r)
{
	using namespace DirectX;
	//check corners of bounding Box
	{
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r, r, r, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r, r, -r, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r, -r, r, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r, -r, -r, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r, r, r, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r, r, -r, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r, -r, r, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r, -r, -r, 1.0f))))
			return false;
	}
	return true;

	//Alt. check corners of Bounding box
	for (signed char i = -1; i < 2; i += 2) {
		for (signed char j = -1; j < 2; j += 2) {
			for (signed char k = -1; k < 2; k += 2)
			{
				if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(i * r, j * r, k * r, 1.0f))))
				{
					return false;  //atleast one corner of bounding box was within frustrum
				}
			}
		}
	}
	return true;
}
bool Room::CullBoundingBox(const DirectX::FXMVECTOR& bBoxPos, const DirectX::XMFLOAT3& r)
{
	using namespace DirectX;
	//check corners and center of bounding Box
	{
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r.x, r.y, r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r.x, r.y, -r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r.x, -r.y, r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r.x, -r.y, -r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r.x, r.y, r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r.x, r.y, -r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r.x, -r.y, r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r.x, -r.y, -r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(0.0f, r.y, 0.0f, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(0.0f, -r.y, 0.0f, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(r.x, 0.0f, 0.0f, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(-r.x, 0.0f, 0.0f, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(0.0f, 0.0f, -r.z, 1.0f))))
			return false;
		if (PointWithinFrustrum(XMVectorAdd(bBoxPos, XMVectorSet(0.0f, 0.0f, r.z, 1.0f))))
			return false;
	}
	return true;
}
bool Room::PointWithinRoom(const DirectX::XMFLOAT3& point) const
{
	return !(point.x < (mPos.x - mRadius) || point.x >(mPos.x + mRadius) ||
		point.y < (mPos.y - mRadius) || point.y >(mPos.y + mRadius) ||
		point.z < (mPos.z - mRadius) || point.z >(mPos.z + mRadius)
		);
}
bool Room::PointWithinRoom(const DirectX::FXMVECTOR& point) const
{
	DirectX::XMFLOAT3 fPoint = {};
	DirectX::XMStoreFloat3(&fPoint, point);
	return !(fPoint.x < (mPos.x - mRadius) || fPoint.x >(mPos.x + mRadius) ||
		fPoint.y < (mPos.y - mRadius) || fPoint.y >(mPos.y + mRadius) ||
		fPoint.z < (mPos.z - mRadius) || fPoint.z >(mPos.z + mRadius)
		);
}
bool Room::PointWithinRoom(const DirectX::FXMVECTOR&& point) const
{
	DirectX::XMFLOAT3 fPoint = {};
	DirectX::XMStoreFloat3(&fPoint, point);
	return !(fPoint.x < (mPos.x - mRadius) || fPoint.x >(mPos.x + mRadius) ||
		fPoint.y < (mPos.y - mRadius) || fPoint.y >(mPos.y + mRadius) ||
		fPoint.z < (mPos.z - mRadius) || fPoint.z >(mPos.z + mRadius)
		);
}
bool Room::PointWithinFrustrum(const DirectX::FXMVECTOR& point) const
{
	using namespace DirectX;

	for (size_t i = 0; i < mFrustrum.size(); i++)
	{
		if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&mFrustrum[i]), point)) < 0.0f)
		{
			return false;
		}
	}
	return true;
}
void Room::AddPortal(Room& otherRoom, const std::array<DirectX::XMFLOAT3, 4>& corners)
{
	using namespace DirectX;

	mPortals.push_back(std::make_shared<Portal<Room>>(*this, otherRoom, corners));
	otherRoom.AddPortal(mPortals.back());
}
void Room::AddPortal(std::shared_ptr<Portal<Room>> portal)
{
	mPortals.push_back(portal);
}
const std::vector<DirectX::XMFLOAT4>& Room::GetFrustrum() const
{
	return mFrustrum;
}
void Room::ClearFrustrum()
{
	mFrustrum.clear();
	for (auto& p : mPortals)
	{
		if (!p.get()->GetAdjacentRoom(*this).mFrustrum.empty())
		{
			p.get()->GetAdjacentRoom(*this).ClearFrustrum();
		}
	}
}