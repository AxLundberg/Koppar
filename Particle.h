#pragma once
#include "DrawableBase.h"

class Particle : public DrawableBase<Particle>
{
public:
	Particle(Graphics& gfx, DirectX::XMFLOAT4 pos, DirectX::XMFLOAT4 dir,
		DirectX::XMFLOAT3 dirVarianceA, DirectX::XMFLOAT3 dirVarianceB,
		DirectX::XMFLOAT3 size, DirectX::XMFLOAT2 initVelocity, DirectX::XMFLOAT2 velocity,
		UINT lifetime, UINT nParticles, std::wstring texture);
	void Update(float dt, DirectX::XMFLOAT3 pos) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
			std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> GetBoundingBox() const noexcept override;
	DirectX::XMFLOAT3 GetRotation() const noexcept override;

private:
	void InitializeParticles(Graphics& gfx) noexcept;
	//update model
	DirectX::XMFLOAT3 upPos = {};
	//model scale transformation
	DirectX::XMFLOAT3X3 modelTransform = {};
	std::wstring texture;
	struct particleSysData
	{
		float dirAngleA;
		float dirMeanAngleA;
		float dirMeanOffsetA;
		float dirAngleB;
		float dirMeanAngleB;
		float dirMeanOffsetB;
		DirectX::XMFLOAT2 size;
		float sizeVariance;
		float velocity;
		float velocityVariance;
		UINT nParticles;
		UINT lifetime;
	}pSys;

	struct initData
	{
		DirectX::XMFLOAT3 emitterPos;
		float emitterRadius;
		float initAngleAlpha;
		float initVarAlpha;
		float initAngleBeta;
		float initVarBeta;
		float initVelocity;
		float initVelocityVariance;
	}pInit;

	struct ParticleData
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 direction;
		DirectX::XMFLOAT2 OGpos;
		float size;
		float velocity;
		UINT time;
	};
	std::vector<ParticleData> initParticles = {};
};
