#pragma once
#include "Graphics.h"
#include "MaMath.h"

class Camera
{
public:
	Camera(std::string name, DirectX::FXMMATRIX proj, DirectX::XMFLOAT3 pos, float pitch = 0, float yaw = 0) noexcept;
	DirectX::XMMATRIX GetMatrix() const noexcept;
	DirectX::XMMATRIX GetRotMatrix() const noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;
	DirectX::XMFLOAT3 GetPosition() const noexcept;
	DirectX::XMFLOAT3 GetRotation() const noexcept;
	DirectX::XMMATRIX GetLookAtMatrix(const DirectX::XMFLOAT3 point) const noexcept;
	const std::string& GetName() const noexcept;
	void SetPosition(const DirectX::FXMVECTOR newPos) noexcept;
	void SetPosition(const DirectX::XMFLOAT3 newPos) noexcept;
	void SetProjection(const DirectX::FXMMATRIX newProj) noexcept;
	void FPMove(DirectX::XMFLOAT3 translation) noexcept;
	void FPRotation(float x, float y, float width, float height) noexcept;
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void FPMoveGrav(DirectX::XMFLOAT3 translation, float height) noexcept;
private:
	std::string camName;
	DirectX::XMFLOAT3 camStartPos;
	DirectX::XMFLOAT4X4 projMatrix = {};
	float camStartPitch;
	float camStartYaw;
	DirectX::XMFLOAT3 camPos;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	float sensitivity = 0.001f;
	float prevx = 0.0f;
	float prevy = 0.0f;
};