#pragma once
#include "Graphics.h"
#include <DirectXMath.h>


class Drawable
{
	template<class T>
	friend class DrawableBase;
public:
	Drawable() = default;
	Drawable(const Drawable&) = delete;
	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
	virtual 		std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept = 0;
	virtual DirectX::XMFLOAT3 GetRotation() const noexcept = 0;
	void Draw(Graphics& gfx) const noexcept;
	void DrawEffects(Graphics& gfx, bool vs, bool gs, bool ps) const noexcept;
	void DrawInstanced(Graphics& gfx) const noexcept;
	virtual void Update(float dt, DirectX::XMFLOAT3 pos) noexcept {}/* = 0;*/
	virtual ~Drawable() = default;
protected:
	void AddBind(std::unique_ptr<class Bindable> bind) noexcept;
	void AddIndexBuffer(std::unique_ptr<class IndexBuffer> ibuf) noexcept;
private:
	virtual const std::vector<std::unique_ptr<class Bindable>>& GetStaticBinds() const noexcept = 0;
protected:
	void AddEffectBind(std::unique_ptr<class Bindable> bind) noexcept;
	void AddEffectIndexBuffer(std::unique_ptr<class IndexBuffer> ibuf) noexcept;
private:
	virtual const std::vector<std::unique_ptr<class Bindable>>& GetStaticEffectBinds() const noexcept = 0;
private:
	const IndexBuffer* pIndexBuffer = nullptr;
	const IndexBuffer* pEffectIndexBuffer = nullptr;
	std::vector<std::unique_ptr<class Bindable>> bindables;
	std::vector<std::unique_ptr<class Bindable>> effectBindables;
};