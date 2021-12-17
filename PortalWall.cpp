#include "PortalWall.h"
#include "BaseBindables.h"
#include "Plane.h"
#include "Stencil.h"

PortalWall::PortalWall(
	Graphics& gfx,
	DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot,
	float frameWidth, float frameHeight,
	float portalWidth, float portalHeight )
	:
	pos(pos), rotRPY(rot),
	frameX(frameWidth), frameY(frameHeight),
	portalX(portalWidth), portalY(portalHeight)
{
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{

		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 tangent;
			DirectX::XMFLOAT3 binormal;
			DirectX::XMFLOAT3 normal;
			DirectX::XMFLOAT2 texCoord;
		};

		auto model = Plane::MakePortalPlane<Vertex>(frameX, frameY, portalX, portalY);
		//auto backside = Plane::MakePortalPlane<Vertex>(fx, fy, px, py);
		
		auto frameXthick = (frameX-portalX)/(frameX*2.0f);
		auto frameYthick = (frameY-portalY)/(frameY*2.0f);
		
		model.vertices[0].texCoord  = { 0.0f,  1.0f  };
		model.vertices[1].texCoord  = { frameXthick, 1.0f - frameYthick };
		model.vertices[2].texCoord  = { 0.0f,  0.0f  };
		model.vertices[3].texCoord  = { frameXthick, frameYthick };
		model.vertices[4].texCoord  = { 1.0f, 0.0f };
		model.vertices[5].texCoord  = { 1.0f - frameXthick, frameYthick };
		model.vertices[6].texCoord  = { 1.0f, 1.0f };
		model.vertices[7].texCoord  = { 1.0f - frameXthick, 1.0f - frameYthick };
		model.vertices[8].texCoord  = { 0.0f,  1.0f };
		model.vertices[9].texCoord  = { frameXthick, 1.0f - frameYthick };
		model.vertices[10].texCoord = { 0.0f,  0.0f };
		model.vertices[11].texCoord = { frameXthick, frameYthick };
		model.vertices[12].texCoord = { 1.0f, 0.0f };
		model.vertices[13].texCoord = { 1.0f - frameXthick, frameYthick };
		model.vertices[14].texCoord = { 1.0f, 1.0f };
		model.vertices[15].texCoord = { 1.0f - frameXthick, 1.0f - frameYthick };

		std::vector<UINT> vCount(model.vertices.size(), 0u);

		//calculate tangent, bitangent and normal for every primitive
		auto TangentBitangentNormal = [&vCount, &model](size_t index) {
			//calculate tangent(T) & bitangent(B) for a triangle
			// Edge1 = delta_u1*T + delta_v1*B
			// Edge2 = delta_u2*T + delta_v2*B
			//
			// Matrix magic to solve for T & B
			//	_		     _	   _				  _		_			_
			// |			  |	  |					   |   |			 |
			// | E1(x1,y1,z1) |   | delta_u1  delta_v1 |   | T(x1,y1,z1) |
			// |			  | = |					   | * |			 |
			// | E2(x2,y2,z2) |   | delta_u2  delta_v2 |   | B(x1,y1,z1) |
			// |_			 _|	  |_				  _|   |_			_|
			//
			using namespace DirectX;
			const size_t v0 = (size_t)model.indices[index+2]; //vertices composing triangle 
			const size_t v1 = (size_t)model.indices[index+0]; //vertices composing triangle
			const size_t v2 = (size_t)model.indices[index+1]; //vertices composing triangle

			XMFLOAT3 e1 = {};  //edge1
			XMFLOAT3 e2 = {};  //edge2
			XMFLOAT2 dudv1 = {};  //Delta u and Delta v for edge1
			XMFLOAT2 dudv2 = {};  //Delta u and Delta v for edge2

			XMStoreFloat3(&e1, XMLoadFloat3(&model.vertices[v1].pos) - XMLoadFloat3(&model.vertices[v0].pos));
			dudv1.x = model.vertices[v1].texCoord.x - model.vertices[v0].texCoord.x;
			dudv1.y = model.vertices[v1].texCoord.y - model.vertices[v0].texCoord.y;

			XMStoreFloat3(&e2, XMLoadFloat3(&model.vertices[v2].pos) - XMLoadFloat3(&model.vertices[v0].pos));
			dudv2.x = model.vertices[v2].texCoord.x - model.vertices[v0].texCoord.x;
			dudv2.y = model.vertices[v2].texCoord.y - model.vertices[v0].texCoord.y;

			XMVECTOR tangent = {};
			XMVECTOR binormal = {};
			XMVECTOR normal = {};
			float determinant = dudv1.x*dudv2.y - dudv1.y*dudv2.x;

			tangent = XMVectorSet(1.0f/determinant * (dudv2.y * e1.x - dudv1.y * e2.x),
				1.0f/determinant * (dudv2.y * e1.y - dudv1.y * e2.y),
				1.0f/determinant * (dudv2.y * e1.z - dudv1.y * e2.z),
				0.0f
			);
			binormal = XMVectorSet(1.0f/determinant * (-dudv2.x * e1.x + dudv1.x * e2.x),
				1.0f/determinant * (-dudv2.x * e1.y + dudv1.x * e2.y),
				1.0f/determinant * (-dudv2.x * e1.z + dudv1.x * e2.z),
				0.0f
			);
			normal = XMVector3Cross(XMLoadFloat3(&e1), XMLoadFloat3(&e2));
			//add tangent, bitanget, and normal in each vertice of the triangle 
			//and keep count of the amount of vertices that uses the bi/tangent/normal for averaging
			for (size_t i = 0; i < 3; i++)
			{
				XMStoreFloat3(&model.vertices[model.indices[index+i]].tangent,
					XMLoadFloat3(&model.vertices[model.indices[index+i]].tangent) + tangent
				);
				XMStoreFloat3(&model.vertices[model.indices[index+i]].binormal,
					XMLoadFloat3(&model.vertices[model.indices[index+i]].binormal) + binormal
				);
				XMStoreFloat3(&model.vertices[model.indices[index+i]].normal,
					XMLoadFloat3(&model.vertices[model.indices[index+i]].normal) + normal
				);
				vCount[model.indices[index+i]]++;
			}
		};
		for (size_t i = 0; i < model.indices.size(); i += 3)
			TangentBitangentNormal(i);

		//average every vertices tangent, bitangent, and normal
		for (size_t i = 0; i < model.vertices.size(); i++)
		{
			dx::XMVECTOR tangent = dx::XMVectorSet((model.vertices[i].tangent.x   / vCount[i]), (model.vertices[i].tangent.y   / vCount[i]), (model.vertices[i].tangent.z   / vCount[i]), 0.0f);
			dx::XMVECTOR bitangent = dx::XMVectorSet((model.vertices[i].binormal.x / vCount[i]), (model.vertices[i].binormal.y / vCount[i]), (model.vertices[i].binormal.z / vCount[i]), 0.0f);
			dx::XMVECTOR normal = dx::XMVectorSet((model.vertices[i].normal.x    / vCount[i]), (model.vertices[i].normal.y    / vCount[i]), (model.vertices[i].normal.z    / vCount[i]), 0.0f);
			dx::XMVector3Normalize(tangent);
			dx::XMVector3Normalize(bitangent);
			dx::XMVector3Normalize(normal);
			dx::XMStoreFloat3(&model.vertices[i].tangent, tangent);
			dx::XMStoreFloat3(&model.vertices[i].binormal, bitangent);
			dx::XMStoreFloat3(&model.vertices[i].normal, normal);
		}

		std::wstring wstr = L"Textures\\crystal.jpg";
		std::wstring wstr2 = L"Textures\\crystal_n.jpg";

		AddStaticBind(std::make_unique<Texture>(gfx, wstr, 0u));
		AddStaticBind(std::make_unique<Texture>(gfx, wstr2, 1u));
		AddStaticBind(std::make_unique<Sampler>(gfx));

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"maneVS.cso");
		//AddStaticBind(std::make_unique<GeometryShader>(gfx, L"TextureGS.cso"));
		//AddStaticBind(std::make_unique<GeometryShader>(gfx, L"PhongGS.cso"));
		AddStaticBind(std::make_unique<PixelShader>(gfx, L"matPS.cso"));
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));

		struct ObjectCbuf
		{
			DirectX::XMFLOAT4 diffuseColor =  { 0.2f, 0.2f, 0.2f, 1.0f };
			DirectX::XMFLOAT4 specularColor = { 0.2f, 0.4f, 0.3f, 1.0f };
			float specularIntensity = 5.0f;
			float specularPower = 10.0f;
			BOOL hasNmap = TRUE;
			BOOL hasDiffuseTex = TRUE;
		} mat;

		AddStaticBind(std::make_unique<PixelConstantBuffer<ObjectCbuf>>(gfx, mat, 1u));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "TANGENT" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "BINORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "NORMAL"  ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		};

		AddStaticBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));

		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		//-----------TESTING SHADOWING -------------//
		AddStaticEffectBind(std::make_unique<VertexBuffer>(gfx, model.vertices));
		AddStaticEffectBind(std::make_unique<IndexBuffer>(gfx, model.indices));

		auto pevs = std::make_unique<VertexShader>(gfx, L"ShadowVS.cso");
		auto pevsbc = pevs->GetBytecode();
		AddStaticEffectBind(std::move(pevs));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> t_eied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
		};

		AddStaticEffectBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));
		AddStaticEffectBind(std::make_unique<InputLayout>(gfx, t_eied, pevsbc));
		AddStaticEffectBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		//-------------------------------------------//
	}
	else
	{
		SetStaticIndexBuffer();
		SetStaticEffectIndexBuffer();
	}

	AddBind(std::make_unique<CBufferTransform>(gfx, *this));
	AddEffectBind(std::make_unique<CBufferTransform>(gfx, *this));

	dx::XMStoreFloat3x3(
		&modelTransform,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}

PortalWall::PortalWall(
	Graphics& gfx,
	DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot,
	float frameWidth, float frameHeight,
	float portalWidth, float portalHeight,
	std::wstring texture)
	:
	pos(std::move(pos)), rotRPY(std::move(rot)),
	frameX(frameWidth), frameY(frameHeight),
	portalX(portalWidth), portalY(portalHeight)
{
	namespace dx = DirectX;

	mBoundingBox.first = pos;
	dx::XMStoreFloat3(&mBoundingBox.second,
		dx::XMVector3Transform(dx::XMVectorSet(frameX/2, frameY/2, 5.0f, 1.0f), dx::XMMatrixRotationRollPitchYaw(rotRPY.x, rotRPY.y, rotRPY.z))
	);

	if (!IsStaticInitialized())
	{

		auto model = Plane::MakePortalPlane<PortalWall::Vertex>(frameX, frameY, portalX, portalY);

		auto frameXthick = (frameX-portalX)/(frameX*2.0f);
		auto frameYthick = (frameY-portalY)/(frameY*2.0f);

		model.vertices[0].texCoord = { 0.0f,  1.0f };
		model.vertices[1].texCoord = { frameXthick, 1.0f - frameYthick };
		model.vertices[2].texCoord = { 0.0f,  0.0f };
		model.vertices[3].texCoord = { frameXthick, frameYthick };
		model.vertices[4].texCoord = { 1.0f, 0.0f };
		model.vertices[5].texCoord = { 1.0f - frameXthick, frameYthick };
		model.vertices[6].texCoord = { 1.0f, 1.0f };
		model.vertices[7].texCoord = { 1.0f - frameXthick, 1.0f - frameYthick };
		model.vertices[8].texCoord = { 0.0f,  1.0f };
		model.vertices[9].texCoord = { frameXthick, 1.0f - frameYthick };
		model.vertices[10].texCoord = { 0.0f,  0.0f };
		model.vertices[11].texCoord = { frameXthick, frameYthick };
		model.vertices[12].texCoord = { 1.0f, 0.0f };
		model.vertices[13].texCoord = { 1.0f - frameXthick, frameYthick };
		model.vertices[14].texCoord = { 1.0f, 1.0f };
		model.vertices[15].texCoord = { 1.0f - frameXthick, 1.0f - frameYthick };

		//calculate tangent, bitangent and normal for every primitive
		std::vector<UINT> vCount(model.vertices.size(), 0u);
		auto TangentBitangentNormal = [&vCount, &model](size_t index) {
			//calculate tangent(T) & bitangent(B) for a triangle
			// Edge1 = delta_u1*T + delta_v1*B
			// Edge2 = delta_u2*T + delta_v2*B
			//
			// Matrix magic to solve for T & B
			//	_		     _	   _				  _		_			_
			// |			  |	  |					   |   |			 |
			// | E1(x1,y1,z1) |   | delta_u1  delta_v1 |   | T(x1,y1,z1) |
			// |			  | = |					   | * |			 |
			// | E2(x2,y2,z2) |   | delta_u2  delta_v2 |   | B(x1,y1,z1) |
			// |_			 _|	  |_				  _|   |_			_|
			//
			using namespace DirectX;
			const size_t v0 = (size_t)model.indices[index+2]; //vertices composing triangle 
			const size_t v1 = (size_t)model.indices[index+0]; //vertices composing triangle
			const size_t v2 = (size_t)model.indices[index+1]; //vertices composing triangle

			XMFLOAT3 e1 = {};  //edge1
			XMFLOAT3 e2 = {};  //edge2
			XMFLOAT2 dudv1 = {};  //Delta u and Delta v for edge1
			XMFLOAT2 dudv2 = {};  //Delta u and Delta v for edge2

			XMStoreFloat3(&e1, XMLoadFloat3(&model.vertices[v1].pos) - XMLoadFloat3(&model.vertices[v0].pos));
			dudv1.x = model.vertices[v1].texCoord.x - model.vertices[v0].texCoord.x;
			dudv1.y = model.vertices[v1].texCoord.y - model.vertices[v0].texCoord.y;

			XMStoreFloat3(&e2, XMLoadFloat3(&model.vertices[v2].pos) - XMLoadFloat3(&model.vertices[v0].pos));
			dudv2.x = model.vertices[v2].texCoord.x - model.vertices[v0].texCoord.x;
			dudv2.y = model.vertices[v2].texCoord.y - model.vertices[v0].texCoord.y;

			XMVECTOR tangent = {};
			XMVECTOR binormal = {};
			XMVECTOR normal = {};
			float determinant = dudv1.x*dudv2.y - dudv1.y*dudv2.x;

			tangent = XMVectorSet(1.0f/determinant * (dudv2.y * e1.x - dudv1.y * e2.x),
				1.0f/determinant * (dudv2.y * e1.y - dudv1.y * e2.y),
				1.0f/determinant * (dudv2.y * e1.z - dudv1.y * e2.z),
				0.0f
			);
			binormal = XMVectorSet(1.0f/determinant * (-dudv2.x * e1.x + dudv1.x * e2.x),
				1.0f/determinant * (-dudv2.x * e1.y + dudv1.x * e2.y),
				1.0f/determinant * (-dudv2.x * e1.z + dudv1.x * e2.z),
				0.0f
			);
			normal = XMVector3Cross(XMLoadFloat3(&e1), XMLoadFloat3(&e2));
			//add tangent, bitanget, and normal in each vertice of the triangle 
			//and keep count of the amount of vertices that uses the bi/tangent/normal for averaging
			for (size_t i = 0; i < 3; i++)
			{
				XMStoreFloat3(&model.vertices[model.indices[index+i]].tangent,
					XMLoadFloat3(&model.vertices[model.indices[index+i]].tangent) + tangent
				);
				XMStoreFloat3(&model.vertices[model.indices[index+i]].binormal,
					XMLoadFloat3(&model.vertices[model.indices[index+i]].binormal) + binormal
				);
				XMStoreFloat3(&model.vertices[model.indices[index+i]].normal,
					XMLoadFloat3(&model.vertices[model.indices[index+i]].normal) + normal
				);
				vCount[model.indices[index+i]]++;
			}
		};
		for (size_t i = 0; i < model.indices.size(); i += 3)
			TangentBitangentNormal(i);

		//average every vertices tangent, bitangent, and normal
		for (size_t i = 0; i < model.vertices.size(); i++)
		{
			dx::XMVECTOR tangent = dx::XMVectorSet((model.vertices[i].tangent.x   / vCount[i]), (model.vertices[i].tangent.y   / vCount[i]), (model.vertices[i].tangent.z   / vCount[i]), 0.0f);
			dx::XMVECTOR bitangent = dx::XMVectorSet((model.vertices[i].binormal.x / vCount[i]), (model.vertices[i].binormal.y / vCount[i]), (model.vertices[i].binormal.z / vCount[i]), 0.0f);
			dx::XMVECTOR normal = dx::XMVectorSet((model.vertices[i].normal.x    / vCount[i]), (model.vertices[i].normal.y    / vCount[i]), (model.vertices[i].normal.z    / vCount[i]), 0.0f);
			dx::XMVector3Normalize(tangent);
			dx::XMVector3Normalize(bitangent);
			dx::XMVector3Normalize(normal);
			dx::XMStoreFloat3(&model.vertices[i].tangent, tangent);
			dx::XMStoreFloat3(&model.vertices[i].binormal, bitangent);
			dx::XMStoreFloat3(&model.vertices[i].normal, normal);
		}
		std::wstring wstr =  L"Textures\\" + texture + L".jpg";
		std::wstring wstr2 = L"Textures\\" + texture + L"_n.jpg";

		/*std::wstring wstr = L"Textures\\ground.jpg";
		std::wstring wstr2 = L"Textures\\ground.jpg";*/

		AddStaticBind(std::make_unique<Texture>(gfx, wstr, 0u));
		AddStaticBind(std::make_unique<Texture>(gfx, wstr2, 1u));
		AddStaticBind(std::make_unique<Sampler>(gfx));
		AddStaticBind(std::make_unique<Sampler>(gfx, 1u));

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"maneVS.cso");
		AddStaticBind(std::make_unique<PixelShader>(gfx, L"matPS.cso"));
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));

		struct ObjectCbuf
		{
			DirectX::XMFLOAT4 diffuseColor = { 0.2f, 0.2f, 0.2f, 1.0f };
			DirectX::XMFLOAT4 specularColor = { 0.2f, 0.4f, 0.3f, 1.0f };
			float specularIntensity = 8.01f;
			float specularPower = 5.0f;
			BOOL hasNmap = TRUE;
			BOOL hasDiffuseTex = TRUE;
		} mat;

		AddStaticBind(std::make_unique<PixelConstantBuffer<ObjectCbuf>>(gfx, mat, 1u));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "TANGENT" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "BINORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "NORMAL"  ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		};

		AddStaticBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));

		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		//-----------TESTING SHADOWING -------------//

		auto pevs = std::make_unique<VertexShader>(gfx, L"ShadowVS.cso");
		auto pevsbc = pevs->GetBytecode();
		AddStaticEffectBind(std::move(pevs));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> t_eied =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
		};

		AddStaticEffectBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Off));
		AddStaticEffectBind(std::make_unique<InputLayout>(gfx, t_eied, pevsbc));
		AddStaticEffectBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		//-------------------------------------------//
	}
	else
	{
		SetStaticIndexBuffer();
	}

	AddEffectBind(std::make_unique<CBufferTransform>(gfx, *this));
	AddBind(std::make_unique<CBufferTransform>(gfx, *this));

	dx::XMStoreFloat3x3(
		&modelTransform,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}

void PortalWall::CalculateBTN(std::vector<Vertex>& vertices, std::vector<int>& indices)
{
	//calculate tangent(T) & bitangent(B) for a triangle
			// Edge1 = delta_u1*T + delta_v1*B
			// Edge2 = delta_u2*T + delta_v2*B
			//
			// Matrix magic to solve for T & B
			//	_		     _	   _				  _		_			_
			// |			  |	  |					   |   |			 |
			// | E1(x1,y1,z1) |   | delta_u1  delta_v1 |   | T(x1,y1,z1) |
			// |			  | = |					   | * |			 |
			// | E2(x2,y2,z2) |   | delta_u2  delta_v2 |   | B(x1,y1,z1) |
			// |_			 _|	  |_				  _|   |_			_|
			//
	using namespace DirectX;

	std::vector<UINT> vCount(vertices.size(), 0u);
	for (size_t index = 0; index < vertices.size(); index++)
	{
		const size_t v0 = (size_t)indices[index+2]; //vertices composing triangle 
		const size_t v1 = (size_t)indices[index+0]; //vertices composing triangle
		const size_t v2 = (size_t)indices[index+1]; //vertices composing triangle

		XMFLOAT3 e1 = {};  //edge1
		XMFLOAT3 e2 = {};  //edge2
		XMFLOAT2 dudv1 = {};  //Delta u and Delta v for edge1
		XMFLOAT2 dudv2 = {};  //Delta u and Delta v for edge2

		XMStoreFloat3(&e1, XMLoadFloat3(&vertices[v1].pos) - XMLoadFloat3(&vertices[v0].pos));
		dudv1.x = vertices[v1].texCoord.x - vertices[v0].texCoord.x;
		dudv1.y = vertices[v1].texCoord.y - vertices[v0].texCoord.y;

		XMStoreFloat3(&e2, XMLoadFloat3(&vertices[v2].pos) - XMLoadFloat3(&vertices[v0].pos));
		dudv2.x = vertices[v2].texCoord.x - vertices[v0].texCoord.x;
		dudv2.y = vertices[v2].texCoord.y - vertices[v0].texCoord.y;

		XMVECTOR tangent = {};
		XMVECTOR binormal = {};
		XMVECTOR normal = {};
		float determinant = dudv1.x*dudv2.y - dudv1.y*dudv2.x;

		tangent = XMVectorSet(1.0f/determinant * (dudv2.y * e1.x - dudv1.y * e2.x),
			1.0f/determinant * (dudv2.y * e1.y - dudv1.y * e2.y),
			1.0f/determinant * (dudv2.y * e1.z - dudv1.y * e2.z),
			0.0f
		);
		binormal = XMVectorSet(1.0f/determinant * (-dudv2.x * e1.x + dudv1.x * e2.x),
			1.0f/determinant * (-dudv2.x * e1.y + dudv1.x * e2.y),
			1.0f/determinant * (-dudv2.x * e1.z + dudv1.x * e2.z),
			0.0f
		);
		normal = XMVector3Cross(XMLoadFloat3(&e1), XMLoadFloat3(&e2));
		//add tangent, bitanget, and normal in each vertice of the triangle 
		//and keep count of the amount of vertices that uses the bi/tangent/normal for averaging
		for (size_t i = 0; i < 3; i++)
		{
			XMStoreFloat3(&vertices[indices[index+i]].tangent,
				XMLoadFloat3(&vertices[indices[index+i]].tangent) + tangent
			);
			XMStoreFloat3(&vertices[indices[index+i]].binormal,
				XMLoadFloat3(&vertices[indices[index+i]].binormal) + binormal
			);
			XMStoreFloat3(&vertices[indices[index+i]].normal,
				XMLoadFloat3(&vertices[indices[index+i]].normal) + normal
			);
			vCount[indices[index+i]]++;
		}
	}
	//average every vertices tangent, bitangent, and normal
	for (size_t i = 0; i < vertices.size(); i++)
	{
		XMVECTOR tangent = XMVectorSet((vertices[i].tangent.x   / vCount[i]), (vertices[i].tangent.y   / vCount[i]), (vertices[i].tangent.z   / vCount[i]), 0.0f);
		XMVECTOR bitangent = XMVectorSet((vertices[i].binormal.x / vCount[i]), (vertices[i].binormal.y / vCount[i]), (vertices[i].binormal.z / vCount[i]), 0.0f);
		XMVECTOR normal = XMVectorSet((vertices[i].normal.x    / vCount[i]), (vertices[i].normal.y    / vCount[i]), (vertices[i].normal.z    / vCount[i]), 0.0f);
		XMVector3Normalize(tangent);
		XMVector3Normalize(bitangent);
		XMVector3Normalize(normal);
		XMStoreFloat3(&vertices[i].tangent, tangent);
		XMStoreFloat3(&vertices[i].binormal, bitangent);
		XMStoreFloat3(&vertices[i].normal, normal);
	}
}

void PortalWall::Update(float dt, DirectX::XMFLOAT3 inpos) noexcept
{
	pos.x = inpos.x;
	pos.y = inpos.y;
	pos.z = inpos.z;
}

DirectX::XMMATRIX PortalWall::GetTransformXM() const noexcept
{
	return 	DirectX::XMMatrixRotationRollPitchYaw(rotRPY.x, rotRPY.y, rotRPY.z) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> PortalWall::GetBoundingBox() const noexcept
{
	return mBoundingBox;
}

DirectX::XMFLOAT3 PortalWall::GetRotation() const noexcept
{
	return DirectX::XMFLOAT3();
}
