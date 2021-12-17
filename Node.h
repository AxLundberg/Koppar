#pragma once
#include <vector>
#include <directxmath.h>
#include "Mesh.h"

class Node
{
	friend class Model;
public:
	Node(std::vector<Mesh*> meshptrs, const DirectX::XMMATRIX& transform) noexcept
		:
		meshptrs(std::move(meshptrs))
	{
		DirectX::XMStoreFloat4x4(&this->transform, transform);
	}
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedtransform) const noexcept
	{
		const auto built = DirectX::XMLoadFloat4x4(&transform) * accumulatedtransform;
		for (const auto pm : meshptrs)
		{
			pm->Draw(gfx, built);
		}
		for (const auto& pc : childptrs)
		{
			pc->Draw(gfx, built);
		}
	}
	void DrawEffects(Graphics& gfx, DirectX::FXMMATRIX accumulatedtransform) const noexcept
	{
		const auto built = DirectX::XMLoadFloat4x4(&transform) * accumulatedtransform;
		for (const auto pm : meshptrs)
		{
			pm->DrawEffects(gfx, built);
		}
		for (const auto& pc : childptrs)
		{
			pc->DrawEffects(gfx, built);
		}
	}
private:
	void AddChild(std::unique_ptr<Node> pchild) noexcept
	{
		assert(pchild);
		childptrs.push_back(std::move(pchild));
	}
private:
	std::vector<std::unique_ptr<Node>> childptrs;
	std::vector<Mesh*> meshptrs;
	DirectX::XMFLOAT4X4 transform;
};
