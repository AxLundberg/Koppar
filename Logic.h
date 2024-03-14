#pragma once
#include "Window.h"
#include <math.h>
#include <chrono>
#include "ImguiController.h"

#include "Stencil.h"
#include "Frustrum.h"
#include "CameraContainer.h"
#include "Light.h"
#include "Model.h"
#include "ModelAi.h"
#include "Vector.h"
#include "Terrain.h"
#include "Particle.h"
#include "FrameController.h"
#include "TestPlane.h"

class Logic
{
public:
	Logic();
	int Run();
	void set_t();
	float check_t();
	void Change_ImGuiWnd() noexcept;
	void SetHeightMap() noexcept;
	float GetHeight(DirectX::XMFLOAT3 pos, float divX, float divY, float width, float height);
	void Control();
	~Logic();
private:
	void ShadowPass(DirectX::FXMMATRIX transform, DirectX::FXMMATRIX transform2);
	void RenderPass(DirectX::FXMMATRIX transform, DirectX::FXMMATRIX transform2);
	void DuFresne();
	void ImguiWindows();
	void BallControl();
private:
	//heightmap variables
	float divX = 499.0f;
	float divZ = 499.0f;
	float width = 400.0f;
	float height = 400.0f;
	float heightAttenuation = 10.0f;
	int mReach = 4;
	std::wstring heightmap = L"Textures\\map2";
	//misc
	ImguiController imgui;
	Window window;
	CameraContainer mCameras{ window.Gfx() };
	Frustrum frustrum;
	Light light;

	FrameController fc{ window.Gfx() };
	//std::vector<std::unique_ptr<class Model>> mBoxes = {};
	std::vector<std::unique_ptr<class Box>> mSkyboxes = {};
	std::vector<std::unique_ptr<class PortalWall>> mPortals = {};
	std::vector<std::unique_ptr<class Particle>> mParticles = {};
	//Vector vec{ window.Gfx(), {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 5 };

	bool mRenderTrajectory = false;
	std::vector<std::unique_ptr<Ball>> mTrajectory = {};
	std::unique_ptr<Ball> mBall = std::make_unique<Ball>(window.Gfx(), 5.f, DirectX::XMFLOAT3{ 5.0f, 0.0f, 0.0f}, DirectX::XMFLOAT3{ 0.8f, 0.8f, 0.8f});
	std::unique_ptr<Ball> mTest2 = std::make_unique<Ball>(window.Gfx(), 5.f, DirectX::XMFLOAT3{ 5.0f, 0.0f, 0.0f}, DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f});
	std::unique_ptr<Vector> mAim = std::make_unique<Vector>(window.Gfx(), DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f }, 4);
	std::unique_ptr<Cylinder> mGoal = std::make_unique<Cylinder>(window.Gfx(),
		DirectX::XMFLOAT3{ 11.5f, 1.0f, 0.0f },
		DirectX::XMFLOAT3{ PI / 2, 0.f, 0.0f },
		DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f },
		DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f }
	);

	std::unique_ptr<Sheet> mRTSheet = std::make_unique<Sheet>(window.Gfx(), L"fullscreenVS.cso", L"fullscreenPS.cso" );
	std::unique_ptr<Sheet> mDSSheet = std::make_unique<Sheet>(window.Gfx(), L"depthVS.cso", L"depthPS.cso" );
	//std::unique_ptr<Model> cottage = std::make_unique<Model>(window.Gfx(), L"cottage2_obj", false, 1.0f );
	std::unique_ptr<ModelAi> skelleBoi = std::make_unique<ModelAi>(window.Gfx(), "Boblampclean", false, 0.1f);
	std::unique_ptr<ModelAi> skelleBoi2 = std::make_unique<ModelAi>(window.Gfx(), "Boblampclean", false, 0.1f );
	//std::unique_ptr<Model> mBox = std::make_unique<Model>(window.Gfx(), L"Crate1", false, 0.5f );
	std::unique_ptr<Terrain> mTerrain = std::make_unique<Terrain>( window.Gfx(), DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f },
		mReach, heightAttenuation, divX, divZ,  width, height, heightmap);

	std::unique_ptr<TestPlane> mTestPlane = std::make_unique<TestPlane>( window.Gfx(), L"vecVS.cso", L"vecPS.cso", DirectX::XMMatrixTranslation(0.0f, 25.0f, -15.0f), 19.0f, 10.0f );
	std::unique_ptr<TestPlane> mTestPlane2 = std::make_unique<TestPlane>( window.Gfx(), L"shademeVS.cso", L"shademePS.cso", DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f), 50.0f, 50.0f );
	struct particleSysData
	{
		DirectX::XMFLOAT4 emitter{ 0.0f, 60.0f, 0.0f, 15.0f };			//Particle emitter position and size. XYZ = position,  W = emitter Radius
		DirectX::XMFLOAT4 particleInitDir{ 0.0f, 180.0f, 60.0f, 20.0f };//Particle starting direction. X = alpha, Y = alpha variance, Z = Beta, W = Betavariance
		DirectX::XMFLOAT3 dirVarianceAlpha{ 5.0f, 0.0f, 0.0f };			//Particle direction update parameters. X = Alpha angle variation, Y = Mean Angle offset, Z = Mean offset (from basevector) Z-major 
		DirectX::XMFLOAT3 dirVarianceBeta{ 2.0f, 0.0f, 0.0f };			//Particle direction update parameters (Beta).
		DirectX::XMFLOAT3 size{ 0.8f, 1.0f, 0.5f };						//Particle size factor 
		DirectX::XMFLOAT2 initVelocity{ 0.1f, 0.05f };					//X = initvelocity, Y = initvariance
		DirectX::XMFLOAT2 velocity{ 0.1f, 0.005f };						//X = velocity, Y = variance
		UINT particleLifetime{128};
		UINT nParticles{256};
		std::wstring texture = L"Textures\\gradiente.jpg";
	}pSys;
	/*std::unique_ptr<Particle> mParticle = std::make_unique<Particle>(window.Gfx(),
					pSys.emitter, pSys.particleInitDir,
					pSys.dirVarianceAlpha, pSys.dirVarianceBeta,
					pSys.size, pSys.initVelocity, pSys.velocity,
					pSys.particleLifetime, pSys.nParticles,
					pSys.texture );*/
	//Mbase boi{ window.Gfx(), "boy", false, 1.0f };
	//Model hand{ window.Gfx(), L"rigged_hand_obj", false, 50.0f };
	//Model planeRH{ window.Gfx(), L"testPlaneRH", true, 10.0f };
	//Model planeLH{ window.Gfx(), L"testPlaneLH", false, 10.0f };
	//Model house{ window.Gfx(), L"house", false, 0.1f };
	struct
	{
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 6.0f;
		float z = 0.0f;
	} m_pos;
private:
	std::vector<std::unique_ptr<class Drawable>> drawables;
	std::vector<std::unique_ptr<class Drawable>> instancedDrawables;
	std::vector<std::vector<float>> height_map;
	std::chrono::steady_clock::time_point here;
private:
	DirectX::XMFLOAT4X4 mProjectionMat = {};
	static constexpr size_t nDrawables = 8; 
	int count = 0;
	bool grav = false;
	bool show_demo_window = true;
	//speeed
	float speed    = 0.2f;
	float ground_v = 0.1f;
	float fly_v	   = 0.52f;
	//imgui Drawables variables
	bool drawBoxes = false;
	bool drawNormMaps = false;
	bool drawPticle = false;
	bool drawSkelleboi = false;
	bool drawCottage = false;
	bool drawTerrain = false;
	bool drawAxes = true;
	bool drawTest = false;
	bool drawTestbox = false;
	//imgui misc variables
	bool drawDSScreen = false;
	bool drawDSTexture = false;
	bool shadowPass = true;
	bool renderPass = true;
	bool lightLookAt = true;
	bool revolve = true;
	bool applyFilter = false;
	bool changeSky = false;
	bool shdwptcle = false;
	int sky = 1;
	float dt = 0.1f;
	//filter sigma
	float kSigmaFilter = 0.0f;
	float rSigmaFilter = 0.051f;
	int radiusFilter = 0;
	DirectX::XMFLOAT3 focalPoint{ 0.0f, 0.0f, 0.0f };
	//time variable
	float ms;
};
