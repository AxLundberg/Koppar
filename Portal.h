#pragma once
#include <array>
#include <DirectXMath.h>
#include <vector>

template <class T>
class Portal
{
	friend class Room;
public:
	Portal(T& r1, T& r2, const std::array<DirectX::XMFLOAT3, 4>& corners)
		:
		room1(r1), room2(r2)
	{

		mCorners.reserve(4);
		for (size_t i = 0; i < 4; i++)
		{
			mCorners.push_back(corners[i]);
		}
		lowL  = corners[0];
		highL = corners[1];
		lowR  = corners[2];
		highR = corners[3];
	
		mPortalCenter = { lowL.x + (highR.x - lowL.x)/2,
						  lowL.y + (highR.y - lowL.y)/2,
						  lowL.z + (highR.z - lowL.z)/2 
		};
	}
	~Portal()
	{
	}
	T& GetAdjacentRoom(T& fromRoom)
	{
		if (room1 == fromRoom)
		{
			return room2;
		}
		else
		{
			return room1;
		}
	}
	std::array<DirectX::FXMVECTOR, 4> GetCorners() const
	{
		 return std::array<DirectX::FXMVECTOR, 4> { DirectX::XMLoadFloat3(&mCorners[0]),
													 DirectX::XMLoadFloat3(&mCorners[1]),
													 DirectX::XMLoadFloat3(&mCorners[2]),
													 DirectX::XMLoadFloat3(&mCorners[3]) };
	}
	bool PortalWithinFrustrum(std::vector<DirectX::XMFLOAT4>& frustrum)
	{
		using namespace DirectX;
		auto portalPlane = XMPlaneFromPoints(XMLoadFloat3(&mCorners[0]), XMLoadFloat3(&mCorners[1]), XMLoadFloat3(&mCorners[2]));
		auto nPP = XMPlaneNormalize(portalPlane);
		auto unit = XMVectorSet(XMVectorGetX(nPP), XMVectorGetY(nPP), XMVectorGetZ(nPP), 1.0f);

		for (auto& c : mCorners)
		{
			auto check = XMLoadFloat3(&c) + unit;
			auto check2 = XMLoadFloat3(&c) - unit;
			auto asd = XMLoadFloat3(&c);
			if (PointWithinFrustrum(XMLoadFloat3(&c), frustrum) ||
				PointWithinFrustrum(XMLoadFloat3(&c) + 3*unit, frustrum) ||
				PointWithinFrustrum(XMLoadFloat3(&c) - 3*unit, frustrum))
			{
				return true;
			}
		}
		if (PointWithinFrustrum(XMLoadFloat3(&mPortalCenter), frustrum) ||
			PointWithinFrustrum(XMLoadFloat3(&mPortalCenter) + 3*unit, frustrum) ||
			PointWithinFrustrum(XMLoadFloat3(&mPortalCenter) - 3*unit, frustrum))
		{
			return true;
		}
		return false;
	}
	bool PointWithinFrustrum(DirectX::FXMVECTOR& point, std::vector<DirectX::XMFLOAT4>& frustrum)
	{
		using namespace DirectX;

		for (size_t i = 0; i < frustrum.size(); i++)
		{
			if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&frustrum[i]), point)) < 0.0f)
			{
				return false;
			}
		}
		return true;
	}
	bool PointWithinBoundingBox(DirectX::FXMVECTOR& point)
	{
		using namespace DirectX;

		auto nPP = XMPlaneNormalize(XMPlaneFromPoints(XMLoadFloat3(&mCorners[0]), XMLoadFloat3(&mCorners[1]), XMLoadFloat3(&mCorners[2])));
		auto unit = XMVectorSet(XMVectorGetX(nPP), XMVectorGetY(nPP), XMVectorGetZ(nPP), 1.0f);

		std::array<XMVECTOR, 6> bb = {};

		bb[0] = XMPlaneFromPoints(XMLoadFloat3(&mCorners[0]), XMLoadFloat3(&mCorners[0])+unit, XMLoadFloat3(&mCorners[1]));
		bb[1] = XMPlaneFromPoints(XMLoadFloat3(&mCorners[2])+unit, XMLoadFloat3(&mCorners[2]), XMLoadFloat3(&mCorners[3]));
		bb[2] = XMPlaneFromPoints(XMLoadFloat3(&mCorners[1]), XMLoadFloat3(&mCorners[1])+unit, XMLoadFloat3(&mCorners[3]));
		bb[3] = XMPlaneFromPoints(XMLoadFloat3(&mCorners[0])+unit, XMLoadFloat3(&mCorners[0]), XMLoadFloat3(&mCorners[2]));
		bb[4] = XMPlaneFromPoints(XMLoadFloat3(&mCorners[0])-6*unit, XMLoadFloat3(&mCorners[1])-6*unit, XMLoadFloat3(&mCorners[2])-6*unit);
		bb[5] = XMPlaneFromPoints(XMLoadFloat3(&mCorners[0])+6*unit, XMLoadFloat3(&mCorners[2])+6*unit, XMLoadFloat3(&mCorners[1])+6*unit);

		for (auto& plane : bb)
		{
			if (XMVectorGetX(XMPlaneDotCoord(plane, point)) < 0.0f)
			{
				return false;
			}
		}
		return true;
	}
private:
	DirectX::XMFLOAT3 lowL = {};
	DirectX::XMFLOAT3 highL = {};
	DirectX::XMFLOAT3 lowR = {};
	DirectX::XMFLOAT3 highR = {};
	std::vector<DirectX::XMFLOAT3> mCorners = {}; //lowL, highL, lowR, highR
	DirectX::XMFLOAT3 mPortalCenter = {};
	//DirectX::XMFLOAT3 mPortalNorm;
	//float mPortalRadius;
	T& room1;
	T& room2;
};