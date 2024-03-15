#include "Sheet.h"
#include "BaseBindables.h"
#include "Stencil.h"
#include "Plane.h"

Sheet::Sheet(Graphics& gfx, std::wstring vs, std::wstring ps, float width, float height)
	:
	width(width), height(height)
{
	namespace dx = DirectX;

	if (!IsStaticInitialized())
	{

		std::vector<dx::XMFLOAT2> vertices = {  
			dx::XMFLOAT2{ -1, 1 },
			dx::XMFLOAT2{  1, 1 },
			dx::XMFLOAT2{ -1,-1 },
			dx::XMFLOAT2{  1,-1 } 
		};

		std::vector<int> indices = { 0,1,2,1,3,2 };

		AddStaticBind(std::make_unique<Sampler>(gfx));

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));
		AddStaticBind(std::make_unique<IndexBuffer>(gfx, indices));
	}
	else
	{
		SetStaticIndexBuffer();
	}
	auto pvs = std::make_unique<VertexShader>(gfx, vs);
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));
	AddBind(std::make_unique<PixelShader>(gfx, ps));

	const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};

	//AddBind(std::make_unique<Stencil>(gfx, Stencil::Mode::Write));
	AddBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
	AddBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	AddBind(std::make_unique<CBufferTransform>(gfx, *this));

	dx::XMStoreFloat3x3(
		&modelTransform,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}


Sheet::Sheet(
	Graphics& gfx,
	DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot,
	float width, float height,
	std::wstring texture)
	:
	pos(pos), rot{ rot.x, rot.y, rot.z },
	width(width), height(height)
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
			struct
			{
				float u;
				float v;
			} texCoord;
		};
		
		int tessy = 1;
		int tessx = 1;
		auto model = Plane::MakeTesselated<Vertex>(tessx,tessy,width,height);
		int blyat = 0;
		for (size_t i = 0; i <= tessy; i++)
		{
			float v = 1.0f - i * (1.0f/tessy);
			for (size_t j = 0; j <= tessx; j++)
			{
				float u = j * (1.0f/tessx);
				model.vertices[blyat++].texCoord = { u,v };
			}
		}

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
			dudv1.x = model.vertices[v1].texCoord.u - model.vertices[v0].texCoord.u;
			dudv1.y = model.vertices[v1].texCoord.v - model.vertices[v0].texCoord.v;

			XMStoreFloat3(&e2, XMLoadFloat3(&model.vertices[v2].pos) - XMLoadFloat3(&model.vertices[v0].pos));
			dudv2.x = model.vertices[v2].texCoord.u - model.vertices[v0].texCoord.u;
			dudv2.y = model.vertices[v2].texCoord.v - model.vertices[v0].texCoord.v;

			XMVECTOR tangent = {};
			XMVECTOR binormal = {};
			XMVECTOR normal = {};
			float determinant = dudv1.x*dudv2.y - dudv1.y*dudv2.x;

			tangent	  = XMVectorSet(1.0f/determinant * (dudv2.y * e1.x - dudv1.y * e2.x),
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

		//calculate tangent, bitangent and normal for every primitive
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
		
		std::wstring wstr = L"Textures\\ground.jpg";
		AddStaticBind(std::make_unique<Texture>(gfx, texture, 1u));
		AddStaticBind(std::make_unique<Sampler>(gfx));
		AddStaticBind(std::make_unique<Sampler>(gfx, 1u));

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

	/*	auto pvs = std::make_unique<VertexShader>(gfx, L"textureVS.cso");
		AddStaticBind(std::make_unique<PixelShader>(gfx, L"texturePS.cso"));*/
		auto pvs = std::make_unique<VertexShader>(gfx, L"maneVS.cso");
		AddStaticBind(std::make_unique<PixelShader>(gfx, L"manePS.cso"));
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "TANGENT" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "BINORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "NORMAL"  ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		};

		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));
		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	}
	else
	{
		SetStaticIndexBuffer();
	}
	AddBind(std::make_unique<CBufferTransform>(gfx, *this));

	dx::XMStoreFloat3x3(
		&modelTransform,
		dx::XMMatrixScaling(1.0f, 1.0f, 1.0f)
	);
}

void Sheet::SetPosition(DirectX::XMVECTOR p) noexcept
{
	DirectX::XMStoreFloat3(&pos, p);
}

void Sheet::Update(float dt, DirectX::XMFLOAT3 pos) noexcept
{
	/*roll += droll * dt;
	pitch += dpitch * dt;
	yaw += dyaw * dt;
	theta += dtheta * dt;
	phi += dphi * dt;
	chi += dchi * dt;*/
	/*rot.yaw = rot.yaw + rot.yaw*dt;
	rot.pitch = rot.pitch + rot.pitch*dt;*/
	//rot.roll += dt;
	rot.pitch += dt;
}

DirectX::XMMATRIX Sheet::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat3x3(&modelTransform) *
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f) *
		DirectX::XMMatrixRotationRollPitchYaw(sin(rot.roll), rot.pitch, rot.yaw) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> Sheet::GetBoundingBox() const noexcept
{
	return {};
}

DirectX::XMFLOAT3 Sheet::GetRotation() const noexcept
{
	return DirectX::XMFLOAT3();
}
