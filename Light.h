#pragma once
#include "Graphics.h"
#include "Melon.h"
#include "ConstantBuffers.h"
#include "imgui.h"

class Light
{
public:
	Light(Graphics& gfx, float radius = 1.5f);
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Draw(Graphics& gfx) const noexcept;
	void Bind(Graphics& gfx, DirectX::FXMMATRIX view) const noexcept;
	void SetPosition(DirectX::XMFLOAT3) noexcept;
	void Revolve(float dt);
	DirectX::XMFLOAT3 GetPosition() const noexcept;
	void SetFocalPoint(DirectX::XMFLOAT3 newFocus) noexcept;
	DirectX::XMFLOAT3 GetFocalPoint() const noexcept;
private:
	const struct LightCBuf
	{
		DirectX::XMFLOAT3 pos;
		float padding0;
		DirectX::XMFLOAT3 ambient;
		float padding1;
		DirectX::XMFLOAT3 diffuseColor;
		float padding2;
		float diffuseIntensity;
		float attConst;
		float attLin;
		float attQuad;
	};
private:
	LightCBuf lightCB;
	DirectX::XMFLOAT3 focalPoint = { 0.0f, 0.0f, 0.0f };
	mutable Melon mesh;
	mutable PixelConstantBuffer<LightCBuf> cbuf;
};