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
		Factory(Graphics& gfx, std::wstring heightmap, float divX, float divZ, float width, float height)
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
	Factory f(window.Gfx(), heightmap, divX, divZ, width, height);
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

	mSkyboxes.push_back(std::make_unique<Box>(window.Gfx(), L"Textures\\skybox_1.jpg"));
	mSkyboxes.push_back(std::make_unique<Box>(window.Gfx(), L"Textures\\skybox_2.jpg"));
	mSkyboxes.push_back(std::make_unique<Box>(window.Gfx(), L"Textures\\skybox_3.jpg"));
	mSkyboxes.push_back(std::make_unique<Box>(window.Gfx(), L"Textures\\skybox_4.jpg"));
	mSkyboxes.push_back(std::make_unique<Box>(window.Gfx(), L"Textures\\skybox_5.jpg"));
	mSkyboxes.push_back(std::make_unique<Box>(window.Gfx(), L"Textures\\skybox_6.jpg"));
	mSkyboxes.push_back(std::make_unique<Box>(window.Gfx(), L"Textures\\skybox_8.jpg"));

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
}

void Logic::BallControl()
{
	ImGui::End();

	if (ImGui::Begin("GolfBall"))
	{
		using namespace DirectX;
		using namespace std::string_literals;

		static float ballX = 0.f, ballY = 0.f, ballZ = 0.f;
		static XMFLOAT3 imguiBallPos = {0.f, 0.f, 0.f};
		static XMFLOAT3 ballPosition = { 0.f, 0.f, 0.f };
		static XMVECTOR bp = XMLoadFloat3(&ballPosition);
		ImGui::Text("BallPos");
		ImGui::SliderFloat3("BallPos", reinterpret_cast<float*>(&imguiBallPos), -180.f, 180.f, "%.2f");
		ImGui::SliderFloat("ballX", &ballX, -180.0f, 180.0f);
		ImGui::SliderFloat("ballY", &ballY, -180.0f, 180.0f);
		ImGui::SliderFloat("ballZ", &ballZ, -180.0f, 180.0f);

		if (ImGui::Button("SpawnBall")){
			ballPosition = imguiBallPos;
			bp = XMLoadFloat3(&ballPosition);
			mAim.get()->SetPosition(bp);
			mBall.get()->SetPosition(bp);
		}

		static XMFLOAT3 imguiGoalPos = { 15.f, 0.f, 0.f };
		static XMFLOAT3 goalPosition = imguiGoalPos;
		ImGui::Text("GoalPos");
		ImGui::SliderFloat3("GoalPos", reinterpret_cast<float*>(&imguiGoalPos), -180.f, 180.f, "%.2f");

		if (ImGui::Button("SetGoal"))
		{
			goalPosition = imguiGoalPos;
			XMVECTOR target = XMLoadFloat3(&goalPosition);
			XMVECTOR direction = XMVectorSubtract(target, bp);
			direction = XMVector3Normalize(direction);
			mAim.get()->SetDirection(direction);
			mGoal->SetPosition(target);
		}
		static float aimXZ = 0.f, aimY = 0.f;
		static float radius = 1000.f;
		ImGui::Text("Aim");
		ImGui::SliderFloat("angleXZ", &aimXZ, -180.0f, 180.0f);
		ImGui::SliderFloat("aimY", &aimY, -180.0f, 180.0f);
		if (ImGui::Button("AimGoal"))
		{
			aimXZ = 0.f;
			aimY = 0.f;
		}

		// calculate distance between ball and goal
		static float distance;
		{
			XMVECTOR gp = XMLoadFloat3(&goalPosition);
			XMVECTOR difference = XMVectorSubtract(bp, gp);
			XMVECTOR distanceSquared = XMVector3LengthSq(difference);
			distance = sqrtf(XMVectorGetX(distanceSquared));
		} 
		
		// calculate grounded angle
		static float newAimX = goalPosition.x, newAimZ = goalPosition.z;
		{
			float normX = goalPosition.x - ballX;
			float normZ = goalPosition.z - ballZ;
			float distBallToGoal = sqrtf((normX * normX) + (normZ * normZ));
			if (normZ == 0.f) // Division with zero guard
				normZ = 0.01f;
			float angleA = atan2f(normZ, normX);
			float angleB = deg_rad(aimXZ);
			float newX = cosf(angleA + angleB) * distBallToGoal;
			float newZ = sinf(angleA + angleB) * distBallToGoal;
			newAimX = ballX + newX;
			newAimZ = ballZ + newZ;
		}


		DirectX::XMVECTOR target = DirectX::XMVectorSet(newAimX, goalPosition.y, newAimZ, 0.0f);
		DirectX::XMVECTOR direction = DirectX::XMVectorSubtract(target, bp);
		direction = DirectX::XMVector3Normalize(direction);
		mAim.get()->SetDirection(direction);

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
		mBall->SetPosition(XMVECTOR{0.f, 0.f, 0.f});
		mBall->Draw(window.Gfx());
		mGoal->Draw(window.Gfx());
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
	{
		DirectX::XMVECTOR position = DirectX::XMVectorSet(1, 1, 1, 1); // (0, 10, 0)
		DirectX::XMVECTOR direction = DirectX::XMVectorSet(1, 1, 1, 0); // (1, 0, 0)
		DirectX::XMVECTOR scale = DirectX::XMVectorSet(1, 1, 1, 0); // (1, 1, 1)
		//mBall.get()->SetPosition(position);
		//mBall.get()->SetDirection(direction);
		mAim.get()->SetScale(scale);
		mAim.get()->Draw(window.Gfx());
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

		if (window.keyboard.KeyPressed(VK_SPACE) && cooldown_frame > 10u && campos.x < width && campos.y < height)
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
		if (abs(campos.x) >= width/2 - 5 or abs(campos.z) >= height/2 - 5)
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
			mCameras.GetCamera().FPMoveGrav(schmovement, GetHeight(mCameras.GetCamera("Main Camera").GetPosition(), divX, divZ, width, height));
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
	float vx = (divX<2) ? 2.0f : divX;
	float vy = (divZ<2) ? 2.0f : divZ;

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
			row.push_back(CalculateHeight2(posX, posY) / heightAttenuation);
		}
		height_map.push_back(row);
	}
}

float Logic::GetHeight(DirectX::XMFLOAT3 pos, float divX, float divZ, float width, float height)
{
	float sideZ = height/2;
	float sideX = width/2;

	//camera positions correlated to heightmap position
	float sampleZ = (pos.z + sideZ)/(height/(divZ));
	float sampleX = (pos.x + sideX)/(width/(divX));

	//heightmap Sample 'coordinates'
	float lowZ  = floor(sampleZ);
	float highZ = floor(sampleZ + 1.0f);
	float lowX  = floor(sampleX);
	float highX = floor(sampleX + 1.0f);

	DirectX::XMVECTOR A = DirectX::XMVectorSet( highX, lowZ,  height_map[(int)lowZ][(int)highX], 0.0f );
	DirectX::XMVECTOR B = DirectX::XMVectorSet( lowX, highZ,  height_map[(int)highZ][(int)lowX], 0.0f );
	DirectX::XMVECTOR C = {};

	//Determine last point from heightmap to use in construction of plane
	if ((sampleZ - lowZ) >= (highX - sampleX))
	{
		C = DirectX::XMVectorSet( highX, highZ,  height_map[(int)highZ][(int)highX], 0.0f );
	}
	else
	{
		C = DirectX::XMVectorSet( lowX, lowZ,  height_map[(int)lowZ][(int)lowX], 0.0f );
	}
	DirectX::XMFLOAT4 plane;
	DirectX::XMStoreFloat4( &plane, DirectX::XMPlaneFromPoints(A, B, C) );

	//Return the Height value corresponding to position on the plane
	return (-plane.w - (sampleX*plane.x) - (sampleZ*plane.y))/plane.z;
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
