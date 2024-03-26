#include "Logic.h"
#include "BaseDrawables.h"
#include "Camera.h"

#include <algorithm>
#include <memory>
#include "MaMath.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


Logic::Logic()
	:
	window(1900, 1000, L"A Window"),
	light(window.Gfx())
{
	SetHeightMap();
	
	const auto projMat = DirectX::XMMatrixPerspectiveFovLH(1.0f, 19.0f / 10.0f, 0.15f, 500.0f);
	DirectX::XMStoreFloat4x4(&mProjectionMat, projMat);

	mCameras.AddCamera(std::make_unique<Camera>("Main Camera", projMat, DirectX::XMFLOAT3{ 0.0f, 75.0f, -35.0f }, PI/7, 0.0f));
	mCameras.AddCamera(std::make_unique<Camera>("Light Camera", projMat, DirectX::XMFLOAT3{ -10.0f, 10.0f, 20.0f }, PI/7, PI/1.25f));
	mCameras.BindCBufferCamera(window.Gfx(), "Light Camera");
	window.Gfx().SetProjection(DirectX::XMLoadFloat4x4(&mProjectionMat));
	set_t();
	class Factory
	{
	public:
		Factory(Graphics& gfx)
			:
			gfx(gfx)
		{}
		std::unique_ptr<Drawable> operator()()
		{
			using namespace DirectX;
			type++;
			switch (type)
			{
			case 1:	// X-axis
			{
				return std::make_unique<Cylinder>(gfx,
					XMFLOAT3{ 0.0f, 0.0f, 0.0f },
					XMFLOAT3{0.0f, PI / 2, 0.0f},
					XMFLOAT3{ 0.15f, 0.15f, 100.0f },
					XMFLOAT3{ 1.0f, 0.0f, 0.0f }
				);
			}
			case 2: // Y-axis
				return std::make_unique<Cylinder>(gfx,
					XMFLOAT3{ 0.0f, 0.0f, 0.0f},
					XMFLOAT3{ PI/2, 0.0f, 0.0f},
					XMFLOAT3{ .15f, 0.15f, 100.0f },
					XMFLOAT3{ 0.0f, 1.0f, 0.0f }
					);
			case 3: // Z-axis
				return std::make_unique<Cylinder>(gfx,
					XMFLOAT3{ 0.0f, 0.0f, 0.0f },			//pos
					XMFLOAT3{ 0.0f, 0.0f, 0.0f },			//rot (around x,y,z)
					XMFLOAT3{ 0.15f, 0.15f, 100.0f },		//scale
					XMFLOAT3{ 0.0f, 0.0f, 1.0f }
					);
			case 4:	// "x"
				return std::make_unique<Cylinder>(gfx,
					XMFLOAT3{ 11.5f, -1.0f, 0.0f },
					XMFLOAT3{PI/4, PI/2, 0.0f},
					XMFLOAT3{0.2f, 0.2f, 1.0f},
					XMFLOAT3{ 1.0f, 0.0f, 0.0f }
					);
			case 5:	// "x"
				return std::make_unique<Cylinder>(gfx,
					XMFLOAT3{ 11.5f, -1.0f, 0.0f },
					XMFLOAT3{ PI/4, -PI/2, 0.0f },
					XMFLOAT3{ 0.2f, 0.2f, 1.0f },
					XMFLOAT3{ 1.0f, 0.0f, 0.0f }
					);
			case 6:	// "z"
				return std::make_unique<Cylinder>(gfx,
					XMFLOAT3{ 0.0f, -.9f, 11.5f },
					XMFLOAT3{0.0f, PI/2, 0.0f },
					XMFLOAT3{0.2f, 0.2f, 1.0f }, 
					XMFLOAT3{ 0.0f, 0.0f, 1.0f }
					);
			case 7:	// "z"
				return std::make_unique<Cylinder>(gfx,
					XMFLOAT3{0.0f, -2.2f, 11.5f },
					XMFLOAT3{0.0f, PI/2, 0.0f },
					XMFLOAT3{ 0.2f, 0.2f, 1.0f },
					XMFLOAT3{ 0.0f, 0.0f, 1.0f }
					);
			case 8:	// "z"
				return std::make_unique<Cylinder>(gfx,
					XMFLOAT3{ 0.0f, -1.55f, 11.5f },
					XMFLOAT3{ -PI/5, PI/2, 0.0f },
					XMFLOAT3{ 0.2f, 0.2f, 1.0f },
					XMFLOAT3{ 0.0f, 0.0f, 1.0f }
					);
			default:
				assert(false && "bad drawable type in factory");
				return {};
				break;
			}
		}
	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		int type = 0;
	};
	using namespace DirectX;
	Factory f(window.Gfx());
	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, f);
	
	mPortals.push_back(std::make_unique<PortalWall>(window.Gfx(),
		XMFLOAT3{ 0.0f, 10.0f, 10.0f }, XMFLOAT3{ 0.0f, PI/2, 0.0f },
		20.0f, 20.0f, 10.0f, 10.0f,
		L"crystal"
		));
	mPortals.push_back(std::make_unique<PortalWall>(window.Gfx(),
		XMFLOAT3{ -20.0f, 0.0f, 20.0f }, XMFLOAT3{ PI/2, 0.0f, 0.0f },
		20.0f, 20.0f, 10.0f, 10.0f,
		L"crystal"
		));
	mPortals.push_back(std::make_unique<PortalWall>(window.Gfx(),
		XMFLOAT3{ -10.0f, 10.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f },
		20.0f, 20.0f, 10.0f, 10.0f,
		L"crystal"
		));
	auto nParticles = 256u;
	pSys.nParticles = nParticles;

	mSkyboxes.push_back(std::make_unique<Box>(window.Gfx(), L"Textures\\skybox_1.png"));
	

	//frustrum.AddDrawables(std::move(std::make_unique<PortalWall>(window.Gfx(),
	//	XMFLOAT3{ -10.0f, 10.0f, 0.01f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, 20.0f, 20.0f, 10.0f, 10.0f, L"crystal")));
	//frustrum.AddDrawables(std::move(std::make_unique<PortalWall>(window.Gfx(),
	//	XMFLOAT3{ -20.0f, 0.01f, 20.0f }, XMFLOAT3{ PI/2, 0.0f, 0.0f }, 20.0f, 20.0f, 10.0f, 10.0f, L"crystal")));
	//frustrum.AddDrawables(std::move(std::make_unique<PortalWall>(window.Gfx(),
	//	XMFLOAT3{ 0.01f, 10.0f, 10.0f  }, XMFLOAT3{ 0.0f, PI/2, 0.0f }, 20.0f, 20.0f, 10.0f, 10.0f, L"crystal" )));
	//frustrum.AddDrawables(std::move(std::make_unique<PortalWall>(window.Gfx(),
	//	XMFLOAT3{ -10.0f, 10.0f, -0.01f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, 20.0f, 20.0f, 10.0f, 10.0f, L"crystal")));
	//frustrum.AddDrawables(std::move(std::make_unique<PortalWall>(window.Gfx(),
	//	XMFLOAT3{ -20.0f, -0.01f, 20.0f }, XMFLOAT3{ PI/2, 0.0f, 0.0f }, 20.0f, 20.0f, 10.0f, 10.0f, L"crystal")));
	//frustrum.AddDrawables(std::move(std::make_unique<PortalWall>(window.Gfx(),
	//	XMFLOAT3{ -0.01f, 10.0f, 10.0f }, XMFLOAT3{ 0.0f, PI/2, 0.0f }, 20.0f, 20.0f, 10.0f, 10.0f, L"crystal")));

	//frustrum.AddModel(std::move(std::make_unique<Model>(window.Gfx(), L"Crate1", false, 1.0f, XMMatrixTranslation(12.0f, 12.0f, 12.0f))));
	//frustrum.AddModel(std::move(std::make_unique<Model>(window.Gfx(), L"Crate1", false, 1.0f, XMMatrixTranslation(-12.0f, 12.0f, 12.0f))));
	//frustrum.AddModel(std::move(std::make_unique<Model>(window.Gfx(), L"Crate1", false, 1.0f, XMMatrixTranslation(-12.0f, 12.0f, -12.0f))));
	const float trajectoryBallRadius = 0.2f;
	const XMFLOAT3 initLocation = { 0.0f, 0.0f, 0.0f };
	for (size_t i = 0; i < N_TRAJECTORIES; i++)
	{
		mTrajectoryStopIdx.push_back(0);
		mTrajectories.push_back({});
		const auto nTrajectorySpheres = N_TRAJECTORY_STEPS / (1 + i); // number of spheres in each trajectory
		for (size_t j = 0; j < nTrajectorySpheres; j++)
		{
			const DirectX::XMFLOAT3 trajectoryCol = { 1.f - 0.65f * j / nTrajectorySpheres, 0.2f, 0.2f };
			mTrajectories[i].push_back(std::make_unique<Ball>(window.Gfx(), trajectoryBallRadius, initLocation, trajectoryCol));
		}
		const DirectX::XMFLOAT3 collisionColor = { 0.f, 0.8f, 0.2f };
		mCollisionIndicators.push_back(std::make_unique<Ball>(window.Gfx(), trajectoryBallRadius + 0.1f, initLocation, collisionColor));
	}
}

float ImpactT(float y0, float v0, float angle)
{
	const float g = 9.8f;
	const float v0y = sinf(angle) * v0;
	const float a = g / 2;
	const float b = -v0y;
	const float c = 0 - y0;
	float p = b / a;
	float q = c / a;
	float t1 = -p/2 + sqrtf(((p * p) / 4) - q);
	float t2 = -p/2 - sqrtf(((p * p) / 4) - q);
	return t1 > t2 ? t1 : t2;
}

std::pair<float, float> ProjectileTrajectory(float initSpeed, float angle, float t)
{
	const float g = 9.8f;
	float initVelocityY = sinf(angle) * initSpeed;
	float initVelocityX = cosf(angle) * initSpeed;

	float height = initVelocityY * t - 0.5f * g * t * t;
	float length = initVelocityX * t;
	return { length, height };
}

std::pair<float, float> RollPitchFromTo(DirectX::XMVECTOR src, DirectX::XMVECTOR dest)
{
	// Calculate the vector from source to destination
	DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(dest, src);

	// Extract the components of the vector
	float nX, nY, nZ;
	nX = DirectX::XMVectorGetX(diff);  // Store X component of the vector
	nY = DirectX::XMVectorGetY(diff);  // Store Y component of the vector
	nZ = DirectX::XMVectorGetZ(diff);  // Store Z component of the
	// calculate rotational angles towards destination
	auto angleXZplane = atan2f(nX, nZ);
	auto x = sqrtf(nX * nX + nZ * nZ);
	auto y = nY;
	auto hyp = sqrtf(x * x + y + y);
	auto angleYXplane = atan2f(y, x);
	return { angleYXplane, angleXZplane };
}
void Logic::ImpulseP133()
{
	// frictionless impact
	using namespace DirectX;
	const float loft = deg_rad(mGC.loftAngle);

	XMVECTOR lineOfAction = { cosf(loft), sinf(loft), 0.f };

	const float vr = mGC.clubHeadVelocity;
	float impulse = (-vr * (mGC.collisionKoefficient + 1)) / ((1.f / mGC.ballMass) + (1.f / mGC.clubHeadMass));
	
	auto vBall = XMVectorDivide(XMVectorScale(lineOfAction, -impulse), XMVectorReplicate(mGC.ballMass));

	XMVECTOR vClubInit = { mGC.clubHeadVelocity, 0.f, 0.f };
	auto vClub = XMVectorAdd(vClubInit, XMVectorDivide(XMVectorScale(lineOfAction, impulse), XMVectorReplicate(mGC.clubHeadMass)));
	auto top = 0;

	// impact force
	auto test = XMVector3Length(vBall);
	float v_b = XMVectorGetX(XMVector3Length(vBall));
	float v_c = XMVectorGetX(XMVector3Length(vClub));

	float bKinetic = 0.5f * mGC.ballMass * v_b * v_b;
	float impactForce = bKinetic / mGC.collisionDuration;
	float cKinetic = 0.5f * mGC.clubHeadMass * mGC.clubHeadVelocity * mGC.clubHeadVelocity - 0.5f * mGC.clubHeadMass * v_c * v_c;
	float impf = cKinetic / mGC.collisionDuration;

	// friction bullshit
	float ff = mGC.ballMass * G;
}
float Logic::GolfLaunchVelocity(bool twoPiece)
{
	using namespace DirectX;
	float velocity = 0.f;
	float loftAngle = deg_rad(mGC.loftAngle);
	float vp = ((1 + mGC.collisionKoefficient) * mGC.clubHeadMass * mGC.clubHeadVelocity * cosf(loftAngle)) / (mGC.clubHeadMass + mGC.ballMass);
	float vn = (2 * mGC.clubHeadMass * mGC.clubHeadVelocity * sinf(loftAngle)) / (7 * (mGC.clubHeadMass + mGC.ballMass));
	float w = -1*(2 * mGC.clubHeadMass * mGC.clubHeadVelocity * sinf(loftAngle)) / (7 * mGC.ballRadius * (mGC.clubHeadMass + mGC.ballMass));
	float ballDiam = mGC.ballRadius * 2;
	float ww = -1 / mGC.ballRadius;
	float www = fabs(ww);
	float wwww = fabs(w);
	float wwwww = 7 / ballDiam;
	float wwwwww = 7 / mGC.ballRadius;
	XMVECTOR u1 = XMVectorScale({ cosf(loftAngle), sinf(loftAngle), 0.0f }, vp);
	XMVECTOR u2 = XMVectorScale({ sinf(loftAngle), -cosf(loftAngle), 0.0f }, vn);
	XMVECTOR u = XMVectorAdd(u1, u2);
	float v = XMVectorGetX(XMVector3Length(u));
	// Drag
	float cd = {};
	{
		if (twoPiece)
			cd = mGC.clubHeadVelocity < 65.f ? 0.53f - 0.0051f * mGC.clubHeadVelocity : 0.21f;
		else // ThreePiece
			cd = mGC.clubHeadVelocity < 60.f ? 0.73f - 0.0084f * mGC.clubHeadVelocity : 0.22f;
	}
	float fd = .5f * cd * mGC.airDensity * mGC.ballStreamArea * v * v;
	float fdx = -fd * XMVectorGetX(u) / v;
	float fdy = -fd * XMVectorGetY(u) / v;

	// Calculate C_m (very weird)
	
	//float cm = 0.05f * (sqrtf(1 + (0.31f * abs)164.f / v)) - 1); // 164 ???
	float cm = 0.05f * (sqrtf(1 + (0.31f * fabs(w) / v)) - 1);
	float fm = .5f * cm * mGC.airDensity * mGC.ballStreamArea * v * v;
	float fmx = -fm * XMVectorGetX(u) / v;
	float fmy = fm * XMVectorGetY(u) / v;
	float vBall = (mGC.clubHeadVelocity * (1 + mGC.collisionKoefficient)) / (1 + (mGC.ballMass / mGC.clubHeadMass));
	float vBalli = cosf(loftAngle) * cosf(loftAngle) * sinf(PI/2-loftAngle)*vBall;
	XMVECTOR v1 = { 0.f, -1.f, 0.f };
	XMVECTOR v2 = { XMVectorGetX(u) / v, 0.f, XMVectorGetY(u) / v };
	v2 = XMVectorScale(v2, fm);
	auto v3 = XMVector3Cross(v1, v2);


	//TEST
	XMVECTOR e_n = { sinf(loftAngle), 0.f, -cosf(loftAngle), 0.f };
	XMVECTOR e_p = { cosf(loftAngle), 0.f, sinf(loftAngle), 0.f };
	auto e_y = XMVector3Cross(e_n, e_p);
	
	// colli
	float cbv = ((1 + mGC.collisionKoefficient) * mGC.clubHeadMass) / (mGC.ballMass / mGC.clubHeadMass);
	cbv *= mGC.clubHeadVelocity;
	// BB
	{
		XMVECTOR V = { 71, 0, 0, 0 };
		XMVECTOR N = { 0.875f, 0.484f, 0.f, 0.f };
		auto res = XMVector2Dot(V, N);
		auto asda = 34;
	}

	return velocity;
}
void Logic::TestP117()
{
	using namespace DirectX;
	auto dotP = [](XMVECTOR v1, XMVECTOR v2) {
		return XMVectorGetX(XMVector3Dot(v1, v2));
	};
	auto crossP = [](XMVECTOR v1, XMVECTOR v2) {
		return XMVector3Cross(v1, v2);
	};
	auto vDiv = [](XMVECTOR v1, float denom) {
		return XMVectorScale(v1, 1.f/denom);
	};
	auto GetVelocity = [](XMVECTOR v1) {
		return XMVectorGetX(XMVector3Length(v1));
	};


	auto loft = deg_rad(mGC.loftAngle);

	XMVECTOR vBallBook, vClubBook, wClub, wBall;
	{
		XMVECTOR N = { cosf(loft), sinf(loft), 0.f }; // unit vector along line of impact
		//XMVECTOR v_r = { mGC.clubHeadVelocity, 0.f, 0.f }; // relative velocity club/ball
		XMVECTOR v_r = { cosf(loft) * mGC.clubHeadVelocity, sinf(loft) * mGC.clubHeadVelocity, 0.f}; // relative velocity club/ball
		
		float E = mGC.collisionKoefficient; // coefficient of restitution
		float m1 = mGC.clubHeadMass;
		float m2 = mGC.ballMass;
		XMVECTOR r1 = { cosf(deg_rad(mGC.clubCogAngle)) * mGC.clubCogDist, sinf(deg_rad(mGC.clubCogAngle)) * mGC.clubCogDist, 0.f };; // vector from club center of gravity to impact
		//XMVECTOR r1 = XMLoadFloat3(&mGC.clubCoGtoImpact); // vector from club center of gravity to impact
		XMVECTOR r2 = { -cosf(loft) * mGC.ballRadius, -sinf(loft)*mGC.ballRadius, 0.f }; // vector from ball center of gravity to impact
		float I1 = mGC.clubMOI;
		float I2 = m1 * mGC.ballRadius * mGC.ballRadius * 2.f / 5; // ball Moment of Inertia

		// calculate impulse
		float numerator = -(dotP(v_r, N) * (E + 1));
		float denom1 = 1 / m1 + 1 / m2;
		float denom2 = dotP(N, crossP(vDiv(crossP(r1, N), I1), r1));
		float denom3 = dotP(N, crossP(vDiv(crossP(r2, N), I2), r2));
		float impulse = numerator / (denom1 + denom2 + denom3);

		auto vClub = XMVectorAdd(v_r, vDiv(XMVectorScale(N, impulse), m1));
		auto vBall = vDiv(XMVectorScale(N, -impulse), m2);

		float impactForce = impulse / mGC.collisionDuration;
		auto nIF = XMVectorScale(N, impactForce); // normal impact force

		float u = mGC.frictionClubBall/impactForce; // ratio tangential friction to normal force
		XMVECTOR T = { sinf(loft), -cosf(loft), 0.f }; // unit tangent vector
		XMVECTOR Tbook = crossP(crossP(N, v_r), N);
		Tbook = XMVector3Normalize(Tbook);

		vClubBook = XMVectorAdd(v_r, vDiv(XMVectorAdd(XMVectorScale(N, impulse), XMVectorScale(T, u * impulse)), m1));
		vBallBook = vDiv(XMVectorAdd(XMVectorScale(N, -impulse), XMVectorScale(T, u * impulse)), m2);

		float I_cg = 0.f;
		{
			XMVECTOR clubCoGtoImpact = { mGC.clubCoGtoImpact.x, mGC.clubCoGtoImpact.y, mGC.clubCoGtoImpact.z };
			XMVECTOR ballCoGtoImpact = r2;
			auto d_c = XMVectorGetX(XMVector3Length(clubCoGtoImpact));

			I_cg = I1 + m1 * d_c * d_c + I2 + m2 * mGC.ballRadius * mGC.ballRadius;
		}

		wClub = vDiv(crossP(r1, XMVectorAdd(XMVectorScale(N, impulse), XMVectorScale(T, u * impulse))), I_cg);
		wBall = vDiv(crossP(r2, XMVectorAdd(XMVectorScale(N, -impulse), XMVectorScale(T, u * impulse))), I_cg);
	}

	XMVECTOR vBallFormula;
	{
		XMVECTOR e_n = { sinf(loft), -cosf(loft), 0.0f };
		XMVECTOR e_p = { cosf(loft), sinf(loft), 0.0f };

		float v_p = ((1 + mGC.collisionKoefficient) * mGC.clubHeadMass * mGC.clubHeadVelocity * cosf(loft)) / (mGC.clubHeadMass + mGC.ballMass);
		float v_n = (2 * mGC.clubHeadMass * mGC.clubHeadVelocity * sinf(loft)) / (7 * (mGC.clubHeadMass + mGC.ballMass));
		
		vBallFormula = XMVectorAdd(XMVectorScale(e_p, v_p), XMVectorScale(e_n, v_n));
	}

	float velocityBook = GetVelocity(vBallBook);
	float velocityFormula = GetVelocity(vBallFormula);
	auto w1 = wClub;
	auto w2 = wBall;
	auto rpm = XMVectorGetZ(wBall) * 60.f / 2 * PI;
	mGC.ballRPM = rpm;
	XMStoreFloat3(&mGC.ballVelocity, vBallBook);
	XMStoreFloat3(&mGC.ballVelocityFormula, vBallFormula);
}

void Logic::BallControl()
{
	ImGui::End();

	if (ImGui::Begin("GolfBall"))
	{
		using namespace DirectX;
		using namespace std::string_literals;

		static bool imguiGolfConstants = false;
		imguiGolfConstants ^= ImGui::Button("Set golf constants");
		if (imguiGolfConstants)
		{
			static float ballMass = mGC.ballMass*1000, ballRadius = mGC.ballRadius*1000, clubHeadMass = mGC.clubHeadMass *1000;
			ImGui::SliderFloat("ball mass(gram)", &ballMass, 20.f, 60.f, "%.1f");
			ImGui::SliderFloat("ball radius(mm)", &ballRadius, 30.f, 60.f, "%.1f");
			ImGui::SliderFloat("club head mass(gram)", &clubHeadMass, 100.f, 600.f, "%.0f");
			ImGui::SliderFloat("loft angle(deg)", &mGC.loftAngle, 1.f, 20.f, "%.1f");
			ImGui::SliderFloat("air density(kg/m^3)", &mGC.airDensity, .8f, 2.f, "%.2f");
			ImGui::SliderFloat("club velocity(m/s)", &mGC.clubHeadVelocity, 1.f, 75.f, "%.1f");
			ImGui::SliderFloat("collisionKoefficient", &mGC.collisionKoefficient, 0.1f, 1.f, "%.2f");
			ImGui::SliderFloat("ground friction", &mGC.frictionGround, 0.05f, 1.f, "%.2f");
			ImGui::SliderFloat("frictionCoefficient", &mGC.frictionClubBall, 0.3f, .8f, "%.2f");
			ImGui::SliderFloat("collisionDuration", &mGC.collisionDuration, 0.001f, .01f, "%.3f");
			ImGui::SliderFloat("clubMOI(kg/m^2)", &mGC.clubMOI, .0001f, .0008f, "%.5f");

			static float clubCoGDist = mGC.clubCogDist*100.f;
			static float clubCoGAngle = mGC.clubCogAngle;
			ImGui::SliderFloat("clubCoGdist(cm)", &clubCoGDist, 0.5f, 10.f, "%.1f");
			ImGui::SliderFloat("clubCoGtoAnle(deg)", &clubCoGAngle, 0.0f, 90.f, "%.1f");

			mGC.clubCoGtoImpact.x = (clubCoGDist / 100.f) * cosf(deg_rad(clubCoGAngle));
			mGC.clubCoGtoImpact.y = (clubCoGDist / 100.f) * sinf(deg_rad(clubCoGAngle));

			ImGui::SliderFloat3("clubCoGtoImpact", (float*)&mGC.clubCoGtoImpact, 0.005f, 0.4f, "%.3f");
			mGC.ballMass = ballMass / 1000;
			mGC.ballRadius = ballRadius / 1000;
			mGC.clubCogDist = clubCoGDist / 100.f;
			mGC.clubCogAngle = clubCoGAngle;
			mGC.clubHeadMass = clubHeadMass / 1000;
			mGC.ballStreamArea = mGC.ballRadius * mGC.ballRadius * PI;
			GolfLaunchVelocity();
			ImpulseP133();
			if (ImGui::Button("Set club CoG on impactNormal")) clubCoGAngle = mGC.clubCogAngle;

			
			TestP117();
			// just print values on imgui
			ImGui::SliderFloat3("ball Velocity book", (float*)&mGC.ballVelocity, 0.0f, 1000.0f, "%.3f");
			ImGui::SliderFloat3("ball Velocity formula", (float*)&mGC.ballVelocityFormula, 0.0f, 1000.0f, "%.3f");
			ImGui::SliderFloat("ball launch rpm", &mGC.ballRPM, 0.0f, 1000.f, "%.1f");
		}

		static XMFLOAT3 ballPos, imguiBallPos = { 0.f, 0.f, 0.f };
		static XMVECTOR bp = XMLoadFloat3(&imguiBallPos);
		ImGui::Text("Ball position");
		ImGui::SliderFloat3("BallPos", reinterpret_cast<float*>(&imguiBallPos), -180.f, 180.f, "%.2f");

		if (ImGui::Button("Set Ball Position")) {
			SpawnGolfBall(imguiBallPos);
			ballPos = imguiBallPos;
			bp = XMLoadFloat3(&ballPos);
		}

		static XMFLOAT3 goalPos, imguiGoalPos = { 15.f, 15.f, 15.f };
		static XMVECTOR gp = XMLoadFloat3(&imguiGoalPos);
		ImGui::Text("Goal Position");
		ImGui::SliderFloat3("GoalPos", reinterpret_cast<float*>(&imguiGoalPos), -180.f, 180.f, "%.2f");

		if (ImGui::Button("Set Goal Position"))
		{
			SpawnGolfGoal(imguiGoalPos);
			goalPos = imguiGoalPos;
			gp = XMLoadFloat3(&imguiGoalPos);
		}

		// Get angles towards goal
		auto [roll, pitch] = RollPitchFromTo(bp, gp);
		// Set Aim
		static float rightLeft = 0.f, upDown = 0.f;
		ImGui::Text("Aim");
		ImGui::SliderFloat("RightLeft", &rightLeft, -180.0f, 180.0f);
		ImGui::SliderFloat("UpDown", &upDown, 0.0f, 90.0f);
		if (ImGui::Button("Aim Towards Goal"))
		{
			rightLeft = 0.f;
			upDown = -rad_deg(std::clamp(roll, -PI / 2, 0.f));;
		}

		// Apply aim
		auto rot = XMVECTOR{ -deg_rad(upDown), pitch + deg_rad(rightLeft), 0.f };
		mAim.get()->SetRotation(rot);

		// tmp init velocity slider
		static float initVelocity = 25.f;
		ImGui::Text("TmpInitVelocitySlider");
		ImGui::SliderFloat("initVelocity", &initVelocity, 0.0f, 90.0f);

		mRenderTrajectory ^= ImGui::Button("Trajectory");
		mRenderPathToGoal ^= ImGui::Button("GoalGuide");

		auto horizontalAngle = deg_rad(upDown);
		auto verticalAngle = pitch + deg_rad(rightLeft);
		auto xzPlaneLen = cosf(horizontalAngle);
		XMVECTOR direction = { xzPlaneLen * sinf(verticalAngle), sinf(horizontalAngle), xzPlaneLen * cosf(verticalAngle) };

		SetGoalPathGuide(bp, gp);
		SetTrajectories(bp, direction, initVelocity);
	}
}

size_t Logic::TrajectoryIndexBelowHeightMap(std::vector<std::unique_ptr<Ball>>& trajectory)
{
	using namespace DirectX;
	for (size_t i = 1; i < trajectory.size(); i++)
	{
		auto pos = trajectory[i].get()->GetPosition();
		auto terrainHeightAtPos = GetHeight(pos);
		// Check if trajectory collides with terrain
		if (pos.y < terrainHeightAtPos)
		{
			return i;
		}
	}
	return trajectory.size() - 1;
}


void Logic::SpawnGolfBall(DirectX::XMFLOAT3& location) 
{
	static constexpr float PEG_HEIGHT = 0.65f;
	location.y = GetHeight(location) + PEG_HEIGHT;
	auto ballPosition = DirectX::XMLoadFloat3(&location);
	mBall->SetPosition(ballPosition);
	mAim.get()->SetPosition(ballPosition);
}
void Logic::SpawnGolfGoal(DirectX::XMFLOAT3& location)
{
	location.y = GetHeight(location);
	auto goalPosition = DirectX::XMLoadFloat3(&location);
	mGoal->SetPosition(goalPosition);
}
void Logic::SetGoalPathGuide(DirectX::XMVECTOR bp, DirectX::XMVECTOR gp)
{
	using namespace DirectX;
	// set scale according to distance between ball and goal
	DirectX::XMVECTOR diff = XMVectorSubtract(gp, bp);
	float distance = XMVectorGetX(XMVector3Length(diff));
	mPathToGoal.get()->SetScale(XMVECTOR{ .1f, .1f, distance / 2.f });
	// rotate path towards goal
	auto [roll, pitch] = RollPitchFromTo(bp, gp);
	mPathToGoal.get()->SetRotation(XMVECTOR{ -roll, pitch, 0.f });
	// set position in the middle between ball pos and goal pos
	XMVECTOR pathPos = XMVectorAdd(bp, gp);
	pathPos = XMVectorScale(pathPos, 0.5f);
	mPathToGoal.get()->SetPosition(pathPos);
}
void Logic::SetTrajectory(std::vector<std::unique_ptr<Ball>>& trajectory, DirectX::FXMVECTOR initPos, DirectX::FXMVECTOR direction, float initialVelocity)
{
	using namespace DirectX;
	auto [horizontalAngle, verticalAngle] = RollPitchFromTo(initPos, XMVectorAdd(initPos, direction));
	float it = ImpactT(XMVectorGetY(initPos), initialVelocity, horizontalAngle);
	float timeStep = it / trajectory.size();
	for (size_t i = 0; i < trajectory.size(); i++)
	{
		auto [x, y] = ProjectileTrajectory(initialVelocity, horizontalAngle, i * timeStep);

		float newX = XMVectorGetX(initPos) + x * sinf(verticalAngle);
		float newY = XMVectorGetY(initPos) + y;
		float newZ = XMVectorGetZ(initPos) + x * cosf(verticalAngle);

		trajectory[i].get()->SetPosition({ newX, newY, newZ });
	}
}

DirectX::XMVECTOR Logic::HeightMapIntersectionPlane(DirectX::XMVECTOR posAbove, DirectX::XMVECTOR posBelow)
{
	using namespace DirectX;
	static constexpr int granularity = 100;
	static constexpr float scaleIncrement = 1.f/granularity;

	DirectX::XMVECTOR diff = XMVectorSubtract(posBelow, posAbove);
	
	XMFLOAT3 posf3 = {};
	for (size_t i = 0; i < granularity; i++)
	{
		auto scaleFactor = i * scaleIncrement;
		const auto posCheck = XMVectorAdd(posAbove, XMVectorScale(diff, scaleFactor));
		XMStoreFloat3(&posf3, posCheck);
		auto heightAtPos = GetHeight(posf3);
		if (posf3.y < heightAtPos)
		{
			return GetHeightMapPlane(posf3);
		}
	}
	XMStoreFloat3(&posf3, posBelow);
	return GetHeightMapPlane(posf3);
}


void Logic::SetTrajectories(DirectX::FXMVECTOR initPos, DirectX::FXMVECTOR direction, float initialVelocity, int bounceIdx)
{
	using namespace DirectX;
	// recursive return
	if (std::isnan(XMVectorGetX(initPos)) || bounceIdx == N_TRAJECTORIES)
		return;

	auto& trajectory = mTrajectories[bounceIdx];

	SetTrajectory(trajectory, initPos, direction, initialVelocity);

	// store render stop (don't render trajectory below height map)
	auto idxBelowHeightMap = TrajectoryIndexBelowHeightMap(trajectory);
	mTrajectoryStopIdx[bounceIdx] = idxBelowHeightMap;

	// collision
	XMVECTOR intersectionPlane = {}, collisionPos = {};
	{
		auto posBelow = trajectory[idxBelowHeightMap].get()->GetPosition(); // first pos beneath heightmap
		auto posAbove = trajectory[idxBelowHeightMap - 1].get()->GetPosition(); // last pos above heightmap
		auto p1 = XMLoadFloat3(&posBelow);
		auto p2 = XMLoadFloat3(&posAbove);
		intersectionPlane = HeightMapIntersectionPlane(p2, p1);
		collisionPos = XMPlaneIntersectLine(intersectionPlane, p1, p2);
		mCollisionIndicators[bounceIdx]->SetPosition(collisionPos);
	}

	auto planeNormal = PlaneNormal(intersectionPlane);
	// recursive call
	return SetTrajectories(collisionPos, XMVector3Normalize(planeNormal), initialVelocity/2, ++bounceIdx);
}

DirectX::XMVECTOR Logic::PlaneNormal(DirectX::XMVECTOR plane)
{
	using namespace DirectX;
	XMVECTOR planeNormal;
	{
		planeNormal = XMVectorSet(XMVectorGetX(plane),
			XMVectorGetY(plane),
			XMVectorGetZ(plane),
			0.0f);
		planeNormal = DirectX::XMVector3Normalize(planeNormal);

		if (XMVectorGetY(planeNormal) > 0)
		{
			planeNormal = XMVectorMultiply(planeNormal, { 1.f, 1.f, 1.f });
		}
		else
		{
			planeNormal = XMVectorMultiply(planeNormal, { -1.f, -1.f, -1.f });
		}
	}
	return planeNormal;
}

void Logic::DuFresne()
{
	using namespace DirectX;
	static float df = check_t();
	std::wostringstream wostr;
	int y = window.moose.GetPosY();
	int x = window.moose.GetPosX();
	dt = check_t() - df;
	df = check_t();
	window.Gfx().BeginFrame(0.4f, 0.3f, 0.6f);
	wostr << L"XXX: " << x << "  YYY: " << y << " COUNT: " << count;
	window.SetWindowTitle(wostr.str());

	auto mainCamPos = mCameras.GetCamera("Main Camera").GetPosition();

	frustrum.Update(XMLoadFloat3(&mainCamPos), mCameras.GetCamera("Main Camera").GetMatrix() * window.Gfx().GetProjection());

	light.Bind(window.Gfx(), mCameras.GetCamera().GetMatrix());

	if(mCameras.GetCamera().GetName() == "Light Camera")
		light.SetPosition(mCameras.GetCamera("Light Camera").GetPosition());
	else
		mCameras.GetCamera("Light Camera").SetPosition(light.GetPosition());

	window.Gfx().SetCamera(mCameras.GetCamera().GetMatrix());
	window.Gfx().SetCameraRot(mCameras.GetCamera().GetRotMatrix());
	window.Gfx().SetCameraPitchYawRoll(mCameras.GetCamera().GetRotation());
	window.Gfx().SetCameraPosition(mCameras.GetCamera("Main Camera").GetPosition());

	const auto transform = DirectX::XMMatrixRotationRollPitchYaw(m_pos.roll, m_pos.pitch, m_pos.yaw) *
		DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
	const auto transform2 = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixTranslation(10.0f, -40.0f, 20.0f);

	fc.BeginFrame(window.Gfx());

	mGoal->Update(dt, {0.f, 0.f, 0.f});
	
	if (shadowPass)
	{
		ShadowPass(transform, transform2);
	}
	if (renderPass)
	{
		RenderPass(transform, transform2);
	}

	if (drawAxes)
	{
		for (UINT i = 0; i<drawables.size(); i++)
		{
			drawables[i].get()->Update(0.f, { 0.f, 0.f, 0.f });
			drawables[i].get()->Draw(window.Gfx());
		}
	}
	if (revolve)
	{
		light.Revolve(dt);
	}
	light.Draw(window.Gfx());

	if (drawDSTexture && !applyFilter)
	{
		fc.BindDSAsTexture(window.Gfx(), 2u);
		mDSSheet.get()->Draw(window.Gfx());
	}
	else if (!applyFilter)
	{
		fc.BindRTAsTexture(window.Gfx(), 2u);
		mRTSheet.get()->Draw(window.Gfx());
	}
	else if (renderPass && applyFilter)
	{
		fc.UpdateFilter(window.Gfx(), kSigmaFilter, radiusFilter, rSigmaFilter);
		fc.BindFilter(window.Gfx());
		mRTSheet.get()->Draw(window.Gfx());
	}
	
	// SOME GOLF rendered here to not be culled
	{
		mAim.get()->Draw(window.Gfx());
		mBall->Draw(window.Gfx());
		if (mRenderPathToGoal)
		{
			mPathToGoal.get()->Draw(window.Gfx());
		}
		if (mRenderTrajectory)
		{
			for (size_t i = 0; i < N_TRAJECTORIES; i++)
			{
				auto renderStopIdx = mTrajectoryStopIdx[i];
				auto& trajectory = mTrajectories[i];
				mCollisionIndicators[i].get()->Draw(window.Gfx());
				for (size_t j = 0; j < renderStopIdx; j++)
				{
					trajectory[j].get()->Draw(window.Gfx());
				}
			}
		}
		mGoal->Draw(window.Gfx());
	}
	
	light.SpawnControlWindow();
	mCameras.SpawnControlWindow();
	skelleBoi->SpawnControlWindow();
	BallControl();
	ImguiWindows();

	Control();

	frustrum.ClearFrustrums();
	fc.Reset(window.Gfx(), { 0.4f, 0.3f, 0.6f });
	//Show
	window.Gfx().EndFrame();
}

void Logic::RenderPass(DirectX::FXMMATRIX transform, DirectX::FXMMATRIX transform2)
{
	using namespace DirectX;

	fc.RenderPassBegin(window.Gfx());
	frustrum.DrawCells(window.Gfx());

	if (drawDSScreen)
		mTestPlane.get()->Draw(window.Gfx());
	if (drawTest)
		mTestPlane2.get()->Draw(window.Gfx());

	if (sky)
	{
		const auto tmp = sky-1;
		mSkyboxes[tmp].get()->Draw(window.Gfx());
	}
	if (drawSkelleboi)
	{
		if (!shadowPass)
		{
			skelleBoi.get()->UpdateModel(window.Gfx(), transform, 0.1f);
		}
		skelleBoi.get()->Draw(window.Gfx(), transform, 0.1f);

		if (!shadowPass)
		{
			skelleBoi2.get()->UpdateModel(window.Gfx(), XMMatrixTranslation(5.0f, 6.0f, 5.0f), 0.0f);
		}
		skelleBoi2.get()->Draw(window.Gfx(), XMMatrixTranslation(5.0f, 6.0f, 5.0f), 0.1f);
	}
	if (drawBoxes)
	{
	}
	if (drawCottage)
	{
		//cottage.get()->Draw(window.Gfx(), transform2);
	}
	if (drawTerrain)
	{
		window.Gfx().GetContext()->GSSetShader(nullptr, nullptr, 0);
		mTerrain.get()->Draw(window.Gfx());
	}
	if (drawPticle)
	{
		//mParticle.get()->Update(0.0f, light.GetPosition());
		//mParticle.get()->DrawInstanced(window.Gfx());
		window.Gfx().GetContext()->GSSetShader(nullptr, nullptr, 0);
	}
	if (drawNormMaps)
	{
		for (size_t i = 0; i < mPortals.size(); i++)
		{
			mPortals[i]->Draw(window.Gfx());
		}
	}
}

void Logic::ShadowPass(DirectX::FXMMATRIX transform, DirectX::FXMMATRIX transform2)
{
	if (lightLookAt)
	{
		window.Gfx().SetCamera(mCameras.GetCamera("Light Camera").GetLookAtMatrix(light.GetFocalPoint()));
		mCameras.UpdateCBufferCamera(window.Gfx(), light.GetFocalPoint());
	}
	else
	{
		window.Gfx().SetCamera(mCameras.GetCamera("Light Camera").GetMatrix());
		mCameras.UpdateCBufferCamera(window.Gfx());
	}

	using namespace DirectX;
	//"portals"
	if (drawBoxes)
	{
		auto drawBox = [&](size_t i) {
			//q1<->q2
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(0.0f, 4.5f, 4.5f+i));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(0.0f, 15.5f, 4.5f+i));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(0.0f, 4.5f+i, 4.5f));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(0.0f, 4.5f+i, 15.5f));
			////q2<->q3
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-4.5f-i, 4.5f, 0.0f));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-4.5f-i, 15.5f, 0.0f));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-4.5f, 4.5f+i, 0.0f));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-15.5f, 4.5f+i, 0.0f));
			////q2<->(-q2)
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-14.5f-i, 0.0f, 14.5f));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-14.5f-i, 0.0f, 25.5f));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-14.5f, 0.0f, 14.5f+i));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-25.5f, 0.0f, 14.5f+i));
		};

		for (size_t i = 0; i < 12; i++){
			drawBox(i);
		}

		auto drawQuadrantBoxes = [&](size_t& i, size_t& j, size_t& k) {

			auto x = i * 8.0f, y = j * 8.0f, z = k * 8.0f;
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(x, y, z));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(x, y, -z));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-x, y, z));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-x, -y, z));
			//mBox.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(-x, y, -z));
		};

		for (size_t i = 1; i <= 2; i++){
			for (size_t j = 1; j < 3; j++){
				for (size_t k = 1; k <= 2; k++)
				{
					drawQuadrantBoxes(i, j, k);
				}
			}
		}
	}
	if (drawCottage)
	{
		//cottage.get()->DrawEffects(window.Gfx(), transform2);
	}
	if (drawSkelleboi)
	{
		skelleBoi.get()->UpdateModel(window.Gfx(), transform, 0.1f);
		skelleBoi.get()->DrawEffects(window.Gfx(), transform, 0.1f);
		skelleBoi2.get()->UpdateModel(window.Gfx(), XMMatrixTranslation(5.0f, 6.0f, 5.0f), 0.0f);
		skelleBoi2.get()->DrawEffects(window.Gfx(), XMMatrixTranslation(5.0f, 6.0f, 5.0f), 0.0f);
	}
	if (drawTerrain)
	{
		window.Gfx().GetContext()->GSSetShader(nullptr, nullptr, 0);
		mTerrain.get()->DrawEffects(window.Gfx(), true, false, false);
	}
	if (drawNormMaps)
	{
		for (size_t i = 0; i < mPortals.size(); i++)
		{
			mPortals[i]->DrawEffects(window.Gfx(), true, false, false);
		}
	}
	fc.ShadowPassEnd(window.Gfx());
	window.Gfx().SetCamera(mCameras.GetCamera().GetMatrix());
}

void Logic::ImguiWindows()
{
	ImGui::End();
	if (ImGui::Begin("Model"))
	{
		using namespace std::string_literals;

		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll", &m_pos.roll, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", &m_pos.pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &m_pos.yaw, -180.0f, 180.0f);

		ImGui::Text("Position");
		ImGui::SliderFloat("X", &m_pos.x, -100.0f, 100.0f);
		ImGui::SliderFloat("Y", &m_pos.y, -100.0f, 100.0f);
		ImGui::SliderFloat("Z", &m_pos.z, -100.0f, 100.0f);

		static char imguiBuffah[128] = { 0 };
		ImGui::Text("update by input");
		ImGui::InputText("",imguiBuffah, IM_ARRAYSIZE(imguiBuffah));
		if (ImGui::Button("Update"))
		{
			auto Proceed = [](std::string_view check)
			{
				wchar_t pchar = {};
				bool proceed = true;
				for (size_t i = 0; i < check.length(); i++)
				{
					pchar = imguiBuffah[i];
					if (proceed) proceed = (pchar == check[i]) ? true : false;
					else i = check.length();
				}
				return proceed;
			};
			float val = {};
			char test[128];
			std::stringstream sstr(imguiBuffah);
			sstr >> test >> val;

			switch (imguiBuffah[0])
			{
			case 'r':
				if (Proceed("roll")) m_pos.roll = deg_rad(val);
				break;
			case 'p':
				if (Proceed("pitch")) m_pos.pitch = deg_rad(val);
				break;
			case 'y':
				if (Proceed("yaw")) m_pos.yaw = deg_rad(val);
				else m_pos.y = val;
				break;
			case 'x':
				m_pos.x = val;
				break;
			case 'z':
				m_pos.z = val;
				break;
			default:
				break;
			}
			std::fill(std::begin(imguiBuffah), std::end(imguiBuffah), 0);
		}
	}
	ImGui::End();
	if (ImGui::Begin("Drawables"))
	{
		ImGui::Checkbox("Axes", &drawAxes);
		ImGui::Checkbox("Terrain", &drawTerrain);
		ImGui::Checkbox("Boxes", &drawBoxes);
		ImGui::Checkbox("Particles", &drawPticle);
		ImGui::Checkbox("NormalMaps", &drawNormMaps);
		ImGui::Checkbox("Cottage", &drawCottage);
		ImGui::Checkbox("AnimationModel", &drawSkelleboi);
	}
	ImGui::End();
	if (ImGui::Begin("Misc"))
	{
		ImGui::Checkbox("Render pass", &renderPass);
		ImGui::Checkbox("Shadow pass", &shadowPass);
		ImGui::Checkbox("LightLookAt", &lightLookAt);
		ImGui::Checkbox("LightDepthStencil", &drawDSScreen);
		ImGui::Checkbox("DepthStencil", &drawDSTexture);
		ImGui::Checkbox("Filter", &applyFilter);
		ImGui::Checkbox("Revolve", &revolve);
		ImGui::SliderAngle("dt", &dt, -75.0f, 75.0f, "%.1f");
		if (ImGui::Button("SetGaussian"))
		{
			applyFilter ^= true;
			fc.SetFilter(window.Gfx(), "gaussian");
		}
		if (ImGui::Button("SetBilateral"))
		{
			applyFilter ^= true;
			fc.SetFilter(window.Gfx(), "bilateral");
		}
		if (ImGui::Button("SetSky"))
		{
			sky++;
			sky = sky % 8;
		}
	}
	ImGui::End();

	float camx = window.Gfx().GetCameraPosition().x;
	float camy = window.Gfx().GetCameraPosition().y;
	float camz = window.Gfx().GetCameraPosition().z;

	ImGui::End();
	if (ImGui::Begin("misc data"))
	{
		ImGui::SliderFloat("Sim Speed factor", &speed, -10.0f, 10.0f);
		ImGui::SliderFloat("Flying Speed factor", &fly_v, 0.0f, 10.0f);
		ImGui::SliderFloat("Grounded Speed factor", &ground_v, 0.0f, 10.0f);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Status: %s", show_demo_window ? "ON" : "OFF");
		ImGui::Text("POSITION");
		ImGui::SliderFloat("X", &camx, -100.0f, 100.0f);
		ImGui::SliderFloat("Y", &camy, -100.0f, 100.0f);
		ImGui::SliderFloat("Z", &camz, -100.0, 100.0f);
		ImGui::SliderFloat("range sigma", &rSigmaFilter, 0.0f, 1.0f, "%.5f");
		ImGui::SliderFloat("Kernel sigma", &kSigmaFilter, 0.0f, 100.0f);
		ImGui::SliderInt("Kernel radius", &radiusFilter, 0, 127);
		ImGui::Checkbox("ShadowPtcle", &shdwptcle);
	}
	ImGui::End();
}

void Logic::Control()
{
	static UINT cooldown_frame = 0u;
	cooldown_frame++;
	static bool glue = false;
	auto campos = mCameras.GetCamera("main camera").GetPosition();
	if (!window.cursor)
	{
		mCameras.GetCamera().FPRotation((float)window.moose.GetPosX(), (float)window.moose.GetPosY(), 1200, 900);

		if (window.keyboard.KeyPressed(VK_SPACE) && cooldown_frame > 10u && campos.x < MAP_WIDTH && campos.y < MAP_HEIGHT)
		{
			cooldown_frame = 0u;
			glue ^= true;
			Change_ImGuiWnd();
		}
	}
	if (window.keyboard.KeyPressed('C') && !glue && cooldown_frame > 10u)
	{
		cooldown_frame = 0u;
		window.ChangeCursor();
		window.SetWindowTitle(L"CONRTOLRA");
	}
	if ((window.keyboard.KeyPressed('W') || window.keyboard.KeyPressed('A') || window.keyboard.KeyPressed('S') || window.keyboard.KeyPressed('D')))
	{
		if (abs(campos.x) >= MAP_WIDTH/2 - 5 or abs(campos.z) >= MAP_HEIGHT/2 - 5)
		{
			glue = false;
		}
		DirectX::XMFLOAT3 schmovement = { 0.0f, 0.0f, 0.0f };
		if (window.keyboard.KeyPressed('W'))
			schmovement.z += (glue) ? ground_v : fly_v;
		if (window.keyboard.KeyPressed('A'))
			schmovement.x -= (glue) ? ground_v : fly_v;
		if (window.keyboard.KeyPressed('S'))
			schmovement.z -= (glue) ? ground_v : fly_v;
		if (window.keyboard.KeyPressed('D'))
			schmovement.x += (glue) ? ground_v : fly_v;
		if (glue)
			mCameras.GetCamera().FPMoveGrav(schmovement, GetHeight(mCameras.GetCamera("Main Camera").GetPosition()));
		else
			mCameras.GetCamera().FPMove(schmovement);
	}	
}

void Logic::SetHeightMap() noexcept
{
	namespace dx = DirectX;

	std::wstring filepath = heightmap + L".bmp";
	const wchar_t* file = filepath.c_str();
	FILE* filePtr;
	bool error = false;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize;
	auto Error = [&filepath, &error](std::wstring message)
	{
		message += filepath;
		MessageBox(0, message.c_str(),
			L"Error", MB_OK);
		if (!error) error = true;
	};

	if (_wfopen_s(&filePtr, file, L"r+b")!=0)
		Error(L"Could not open ");
	if (fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1)
		Error(L"Could not read bitmap file header of  ");
	if (fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1)
		Error(L"Could not read bitmap info header of  ");

	imageSize = int(bitmapFileHeader.bfSize-bitmapFileHeader.bfOffBits); // Calculate the byte size of the bitmap image data
	if (!imageSize)
		Error(L"ImageSize is equal to zero for file ");

	unsigned char* bitmapImage = new unsigned char[imageSize];	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);		// Read in the bitmap image pixel data.
	fread(bitmapImage, 1, imageSize, filePtr);
	fclose(filePtr);

	UINT bpp = bitmapInfoHeader.biBitCount/8;		//bytes per pixel
	UINT npx_w = bitmapInfoHeader.biWidth;			//picture number of pixel wide												
	UINT npx_h = bitmapInfoHeader.biHeight;			//picture number of pixel high
	DWORD stride = ((((bitmapInfoHeader.biWidth * bitmapInfoHeader.biBitCount) + 31) & ~31) >> 3);	//bytes per line

	CprArray<unsigned char> pxs_A(npx_h, npx_w); // alpha channel array to store height value

	for (size_t i = 0; i < npx_h; i++) //Load color and alpha pixel arrays with image pixel data
	{
		for (size_t j = 0; j < npx_w; j++)
		{
			UINT bmi_i = UINT(i*stride+j*bpp);
			pxs_A(i, j) = bitmapImage[bmi_i];
		}
	}
	delete[] bitmapImage;

	// Create height map with pixel data
	float vx = (MAP_DIV_X<2) ? 2.0f : MAP_DIV_X;
	float vy = (MAP_DIV_Z<2) ? 2.0f : MAP_DIV_Z;

	//Kernel radii
	int reach = mReach;

	CprArray<float> weightGrid((size_t)reach*2+1, (size_t)reach*2+1);
	float centDist = (weightGrid.ColumnCount() >> 1) > 0 ? float(weightGrid.ColumnCount()) : 1.0f;
	float factor = 1.4142f * centDist;
	for (float i = 0; i < weightGrid.RowCount(); i++)
	{
		for (float j = 0; j < weightGrid.ColumnCount(); j++)
		{
			float value = abs(centDist - i)*abs(centDist - i)+abs(centDist - j)*abs(centDist - j);
			value = 1.01f - sqrtf(value) * 1.0f/factor;
			weightGrid((size_t)i, (size_t)j) = value;
		}
	}
	auto CalculateHeight2 = [&pxs_A, &weightGrid, &reach](int pix_x, int pix_y)
	{
		int cols = (int)pxs_A.ColumnCount()-1;
		int rows = (int)pxs_A.RowCount()-1;

		int& r = reach;
		int north = (pix_y+r) > rows ? (rows-pix_y) : r;
		int east = (pix_x+r) > cols ? (cols-pix_x) : r;
		int south = (pix_y<r) ? pix_y : r;
		int west = (pix_x<r) ? pix_x : r;

		float weightedHeight = 0.0f;
		float weight = 0.0f;
		for (int i = -west; i < east+1; i++)
		{
			int min = ((r-abs(i)) > south) ? south : r-abs(i);
			int max = ((r-abs(i)) > north) ? north : r-abs(i);
			for (int j = -min; j < max+1; j++)
			{
				weightedHeight += pxs_A(pix_y+j, pix_x+i) * weightGrid(j+r, i+r);
				weight += weightGrid(j+r, i+r);
			}
		}
		return weightedHeight / weight;
	};
	
	height_map.reserve((size_t)vy);
	for (size_t i = 0; i < vy; i++) 
	{
		std::vector<float> row;
		row.reserve((size_t)vx);
		int posY = int(i*npx_h/vy);
		for (size_t j = 0; j < vx; j++)
		{
			int posX = int(j*npx_w/vx);
			row.push_back(CalculateHeight2(posX, posY) / MAP_HEIGHT_ATTENUATION);
		}
		height_map.push_back(row);
	}
}

DirectX::XMVECTOR Logic::GetHeightMapPlane(DirectX::XMFLOAT3 pos)
{
	float sideZ = MAP_HEIGHT / 2;
	float sideX = MAP_WIDTH / 2;
	// heightmap positions correlated to input pos
	float mapZ = pos.z + sideZ;
	float mapX = pos.x + sideX;
	// normalized value from 0.0 - 1.0
	float mapNZ = mapZ / MAP_HEIGHT;
	float mapNX = mapX / MAP_WIDTH;
	// sample points of height map
	float sampleZ = mapNZ * MAP_DIV_Z;
	float sampleX = mapNX * MAP_DIV_X;
	//heightmap Sample 'coordinates'
	float lowZ = floor(sampleZ);
	float highZ = floor(sampleZ + 1.0f);
	float lowX = floor(sampleX);
	float highX = floor(sampleX + 1.0f);
	// convert from sample -> world pos
	float w_lowZ = lowZ * (MAP_HEIGHT / MAP_DIV_Z) - (MAP_HEIGHT / (2 * MAP_DIV_Z)) - sideZ;
	float w_highZ = highZ * (MAP_HEIGHT / MAP_DIV_Z) - (MAP_HEIGHT / (2 * MAP_DIV_Z)) - sideZ;
	float w_lowX = lowX * (MAP_WIDTH / MAP_DIV_X) - (MAP_WIDTH / (2 * MAP_DIV_X)) - sideX;
	float w_highX = highX * (MAP_WIDTH / MAP_DIV_X) - (MAP_WIDTH / (2 * MAP_DIV_X)) - sideX;

	// create a plane from three points
	DirectX::XMVECTOR A = DirectX::XMVectorSet(w_highX, height_map[(int)lowZ][(int)highX], w_lowZ, 0.0f);
	DirectX::XMVECTOR B = DirectX::XMVectorSet(w_lowX, height_map[(int)highZ][(int)lowX], w_highZ, 0.0f);
	DirectX::XMVECTOR C = {};

	//Determine last point from heightmap to use in construction of plane
	if ((sampleZ - lowZ) >= (highX - sampleX))
	{
		C = DirectX::XMVectorSet(w_highX, height_map[(int)highZ][(int)highX], w_highZ, 0.0f);
	}
	else
	{
		C = DirectX::XMVectorSet(w_lowX, height_map[(int)lowZ][(int)lowX], w_lowZ, 0.0f);
	}
	return DirectX::XMPlaneFromPoints(A, B, C);
}


float Logic::GetHeight(DirectX::XMFLOAT3 pos)
{
	float sideZ = MAP_HEIGHT / 2;
	float sideX = MAP_WIDTH / 2;
	// heightmap positions correlated to input pos
	float mapZ = pos.z + sideZ;
	float mapX = pos.x + sideX;
	// normalized value from 0.0 - 1.0
	float mapNZ = mapZ / MAP_HEIGHT;
	float mapNX = mapX / MAP_WIDTH;
	// sample points of height map
	float sampleZ = mapNZ * MAP_DIV_Z;
	float sampleX = mapNX * MAP_DIV_X;

	DirectX::XMFLOAT4 plane;
	DirectX::XMStoreFloat4(&plane, GetHeightMapPlane(pos));

	//Return the Height value corresponding to position on the plane
	const float heightValue = (-plane.w - (pos.x * plane.x) - (pos.z * plane.z)) / plane.y;
	return heightValue;
}

int Logic::Run()
{
	while (true)
	{
		if (const auto m = Window::ProcessMessages())
		{
			return *m;
		}
		DuFresne();
	}
}

void Logic::set_t()
{
	//std::chrono::steady_clock::time_point ref = here;
	here = std::chrono::steady_clock::now();
}

float Logic::check_t()
{
	return std::chrono::duration<float>(std::chrono::steady_clock::now() - here).count();
}

void Logic::Change_ImGuiWnd() noexcept
{
	if (show_demo_window)
	{
		show_demo_window = false;
	}
	else
	{
		show_demo_window = true;
	}
}

Logic::~Logic()
{
}
