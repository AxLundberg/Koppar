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
	for (size_t i = 0; i < N_TRAJECTORY_STEPS; i++)
	{
		const DirectX::XMFLOAT3 col = { 1.f - 0.65f * i/N_TRAJECTORY_STEPS, 0.2f, 0.2f};
		mTrajectory.push_back(std::make_unique<Ball>(window.Gfx(), trajectoryBallRadius, initLocation, col));
	}
	for (size_t i = 0; i < N_COLLISION_INDICATORS; i++)
	{
		const DirectX::XMFLOAT3 col = { 0.f, 0.8f, 0.2f };
		mCollisionIndicators.push_back(std::make_unique<Ball>(window.Gfx(), trajectoryBallRadius, initLocation, col));
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
	auto angleYXplane = -atan2f(y, x);
	return { angleYXplane, angleXZplane };
}

void Logic::BallControl()
{
	ImGui::End();

	if (ImGui::Begin("GolfBall"))
	{
		using namespace DirectX;
		using namespace std::string_literals;

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

		SetGoalPathGuide(bp, gp);
		SetTrajectory(bp, deg_rad(upDown), pitch + deg_rad(rightLeft), initVelocity);
	}
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
	mPathToGoal.get()->SetRotation(XMVECTOR{ roll, pitch, 0.f });
	// set position in the middle between ball pos and goal pos
	XMVECTOR pathPos = XMVectorAdd(bp, gp);
	pathPos = XMVectorScale(pathPos, 0.5f);
	mPathToGoal.get()->SetPosition(pathPos);
}
void Logic::SetTrajectory(DirectX::FXMVECTOR ballPos, float horizontalAngle, float turnAngle, float initialVelocity)
{
	using namespace DirectX;
	float angle = horizontalAngle;
	float it = ImpactT(XMVectorGetY(ballPos), initialVelocity, angle);
	float timeStep = it / N_TRAJECTORY_STEPS;
	size_t nStep = N_TRAJECTORY_STEPS;
	for (size_t i = 0; i < nStep; i++)
	{
		auto [x, y] = ProjectileTrajectory(initialVelocity, angle, i * timeStep);

		float dist = sqrtf(x * x + y * y);
		float newX = XMVectorGetX(ballPos) + x * sinf(turnAngle);
		float newY = XMVectorGetY(ballPos) + y;
		float newZ = XMVectorGetZ(ballPos) + x * cosf(turnAngle);

		mTrajectory[i].get()->SetPosition({ newX, newY, newZ });
	}
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
		XMVECTOR A = { -5.f, 5.f, 0.f };
		XMVECTOR B = { -5.f, 5.f, -5.f };
		XMVECTOR C = { 0.f, 5.f, -5.f };
		auto plane = XMPlaneFromPoints(C, B, A);
		mGolfPlane->SetTransformFromPlane(plane);
		mGolfPlane->Draw(window.Gfx());

		mAim.get()->Draw(window.Gfx());
		mBall->Draw(window.Gfx());
		if (mRenderPathToGoal)
		{
			mPathToGoal.get()->Draw(window.Gfx());
		}
		if (mRenderTrajectory)
		{
			mTrajectory[0].get()->Draw(window.Gfx());
			
			for (size_t i = 1; i < mTrajectory.size(); i++)
			{
				auto pos = mTrajectory[i].get()->GetPosition();
				auto terrainHeightAtPos = GetHeight(pos);
				// Check if trajectory collides with terrain
				if (pos.y < terrainHeightAtPos)
				{
					auto pos2 = mTrajectory[i - 1].get()->GetPosition();
					auto intersectionPlane = GetHeightMapPlane(pos);
					auto p2 = XMLoadFloat3(&pos2);
					auto p1 = XMLoadFloat3(&pos);
					auto res = XMPlaneIntersectLine(intersectionPlane, p1, p2);
					// render collision ball at impact location
					mCollisionIndicators[0]->SetPosition(res);
					mCollisionIndicators[0]->Draw(window.Gfx());
					break; // only want to render the path that is above the terrain, so we break
				}
				mTrajectory[i].get()->Draw(window.Gfx());
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
