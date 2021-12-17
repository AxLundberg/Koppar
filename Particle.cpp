#include "Particle.h"
#include "BaseBindables.h"
#include "Plane.h"
#include "Stencil.h"


Particle::Particle(Graphics& gfx,
	DirectX::XMFLOAT4 pos, DirectX::XMFLOAT4 baseDir,
	DirectX::XMFLOAT3 dirVarianceA, DirectX::XMFLOAT3 dirVarianceB,
	DirectX::XMFLOAT3 size, DirectX::XMFLOAT2 initVelocity, DirectX::XMFLOAT2 velocity,
	UINT lifetime, UINT nParticles,
	std::wstring texture)
	:
	pSys{	pSys.dirAngleA = deg_rad(dirVarianceA.x),
			pSys.dirMeanAngleA = dirVarianceA.y,
			pSys.dirMeanOffsetA = dirVarianceA.z,
			pSys.dirAngleB = deg_rad(dirVarianceB.x),
			pSys.dirMeanAngleB = dirVarianceB.y,
			pSys.dirMeanOffsetB = dirVarianceB.z,
			pSys.size = {size.x, size.y},
			pSys.sizeVariance = size.z,
			pSys.velocity = velocity.x,
			pSys.velocityVariance = velocity.y,
			pSys.lifetime = lifetime,
			pSys.nParticles = nParticles },
	pInit{ pInit.emitterPos = {pos.x, pos.y, pos.z},
			pInit.emitterRadius = pos.w,
			pInit.initAngleAlpha = deg_rad(baseDir.x),
			pInit.initVarAlpha = deg_rad(baseDir.y),
			pInit.initAngleBeta = deg_rad(baseDir.z),
			pInit.initVarBeta = deg_rad(baseDir.w),
			pInit.initVelocity = initVelocity.x,
			pInit.initVelocityVariance = initVelocity.y },
	texture(texture),
	upPos({ pos.x, pos.y, pos.z })
{
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{
		InitializeParticles(gfx);

		std::wstring wstr = L"Textures\\gradiente.jpg";
		AddStaticBind(std::make_unique<Texture>(gfx, texture, 0u));
		AddStaticBind(std::make_unique<Sampler>(gfx));
		//AddStaticBind(std::make_unique<Sampler>(gfx));

		auto pvs = std::make_unique<VertexShader>(gfx, L"ParticleVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<ComputeShader>(gfx, L"ParticleCS.cso"));
		AddStaticBind(std::make_unique<ComputeTest>(gfx, initParticles));

		AddStaticBind(std::make_unique<GeometryShader>(gfx, L"ParticleGS.cso"));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"ParticlePS.cso"));
		AddStaticBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));

		//Particle constant buffer
		struct ParticleConstants
		{
			dx::XMFLOAT3 varianceConsts;
			float nParticles;
			dx::XMFLOAT3 emitterPos;
			float emitterRadius;
			dx::XMFLOAT2 initAlpha;
			dx::XMFLOAT2 initBeta;
			dx::XMFLOAT2 initVelocity;
			float size;
			float padding = 0.0f;
		} pcb{};

		pcb.varianceConsts = { pSys.dirAngleA, pSys.dirAngleB, pSys.velocityVariance };
		pcb.nParticles =	 (float)pSys.nParticles;
		pcb.emitterPos =	 { pInit.emitterPos.x, pInit.emitterPos.y, pInit.emitterPos.z };
		pcb.emitterRadius =  pInit.emitterRadius;
		pcb.initAlpha =		 { pInit.initAngleAlpha, pInit.initVarAlpha };
		pcb.initBeta =		 { pInit.initAngleBeta, pInit.initVarBeta };
		pcb.initVelocity =   { pInit.initVelocity, pInit.initVelocityVariance };
		pcb.size =			 pSys.size.x;

		AddStaticBind(std::make_unique<ComputeConstantBuffer<ParticleConstants>>(gfx, pcb));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		};

		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST));
	}
	else
	{
		if (initParticles.empty()) InitializeParticles(gfx);
		AddBind(std::make_unique<ComputeTest>(gfx, initParticles));
		AddBind(std::make_unique<Texture>(gfx, texture, 0u));
	}

	AddBind(std::make_unique<CBufferTransform>(gfx, *this, true));

	dx::XMStoreFloat3x3(
		&modelTransform,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}


void Particle::Update(float dt, DirectX::XMFLOAT3 pos) noexcept
{
	upPos.x = pos.x-pInit.emitterPos.x;
	upPos.y = pos.y-pInit.emitterPos.y;
	upPos.z = pos.z-pInit.emitterPos.z;
}

DirectX::XMMATRIX Particle::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat3x3(&modelTransform) *
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixTranslation(upPos.x, upPos.y, upPos.z);
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> Particle::GetBoundingBox() const noexcept
{
	return {};
}

DirectX::XMFLOAT3 Particle::GetRotation() const noexcept
{
	return DirectX::XMFLOAT3();
}

void Particle::InitializeParticles(Graphics& gfx) noexcept
{
	using namespace DirectX;
	gfx.init();
	auto VectorAngleOffset = [this, &gfx](float varAngle, float meanOffset, float meanAngle ) {
		if (meanOffset > 1.0f || meanOffset < -1.0f) meanOffset = 0.0f;
		float angle = {};
		float sideDecide = 2*gfx.random() - 1;
		if (sideDecide <= meanOffset)
		{
			float deviation = (varAngle + meanOffset*varAngle) / 3;
			angle = abs(gfx.random(0.0f, deviation)) + meanAngle;
		}
		else if (sideDecide > meanOffset)
		{
			float deviation = (varAngle - meanOffset*varAngle) / 3;
			angle = -abs(gfx.random(0.0f, deviation)) + meanAngle;
		}
		return angle;
	};
	auto RobustNormalize = [](XMVECTOR test)
	{
		using namespace DirectX;
		//DirectX::XMVECTOR temp = DirectX::XMVectorSet(xComponent, yComponent, zComponent, 0.0f);
		/*Alghoritm copy pasted from Microsoft*/
		// Compute the maximum absolute value component.
		XMVECTOR vAbs = XMVectorAbs(test);
		XMVECTOR max0 = XMVectorSplatX(vAbs);
		XMVECTOR max1 = XMVectorSplatY(vAbs);
		XMVECTOR max2 = XMVectorSplatZ(vAbs);
		max0 = XMVectorMax(max0, max1);
		max0 = XMVectorMax(max0, max2);

		// Divide by the maximum absolute component.
		XMVECTOR normalized = XMVectorDivide(test, max0);

		// Set to zero when the original length is zero.
		XMVECTOR mask = XMVectorNotEqual(g_XMZero, max0);
		normalized = XMVectorAndInt(normalized, mask);

		XMVECTOR t0 = XMVector3LengthSq(normalized);
		XMVECTOR length = XMVectorSqrt(t0);

		// Divide by the length to normalize.
		normalized = XMVectorDivide(normalized, length);

		// Set to zero when the original length is zero or infinity.  In the
		// latter case, this is considered to be an unexpected condition.
		normalized = XMVectorAndInt(normalized, mask);
		return normalized;
	};
	for (UINT i = 0; i < pSys.nParticles; i++)
	{
		ParticleData p = {};
		/*Randomize particle starting position*/
		//float ranRadius = gfx.random() * (pSys.emitterRadius-0.3f) + 0.3f;
		//float ranRadius = pSys.emitterRadius - abs(gfx.random(0.0f, pSys.emitterRadius/2.8f));
		//float ranRadius = pSys.emitterRadius - sq(gfx.random() * sqrtf(pSys.emitterRadius));
		float testX = gfx.random();
		float testY = gfx.random()*0.4f;
		while (testX < testY) {
			testX = gfx.random();
			testY = gfx.random();
		}
		float ranRadius = testX * pInit.emitterRadius;
		float ranAngle  = gfx.random()*2*PI;
		p.position = pInit.emitterPos;
		p.position.x += ranRadius * cosf(ranAngle);
		p.position.z += ranRadius * sinf(ranAngle);
		p.OGpos.x = p.position.x;
		p.OGpos.y = p.position.z;

		float alphaVar = (gfx.random()*2-1)*pInit.initVarAlpha;
		float betaVar  = (gfx.random()*2-1)*pInit.initVarBeta;
		p.direction = { cos(alphaVar+pInit.initAngleAlpha)*cos(betaVar+pInit.initAngleBeta),
						sin(betaVar+pInit.initAngleBeta),
						sin(alphaVar+pInit.initAngleAlpha) *cos(betaVar+pInit.initAngleBeta) };


		float ranVelo = pInit.initVelocity + (gfx.random()*2-1)*pInit.initVelocityVariance;
		p.velocity = ranVelo;
		p.time = i+1;
		p.size = 1.0f;
		
		initParticles.push_back(p);

	}
}

