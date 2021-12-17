#include "Light.h"
#include "imgui.h"

Light::Light(Graphics& gfx, float radius)
	: mesh(gfx, radius), cbuf(gfx)
{
	Reset();
}

void Light::SpawnControlWindow() noexcept
{
	if (ImGui::Begin("Light"))
	{
		static char imguiBuffah[128] = { 0 };
		static char imguiBuffah2[128] = { 0 };
		auto Proceed = [](std::string_view check, char buff[])
		{
			wchar_t pchar = {};
			bool proceed = true;
			for (size_t i = 0; i < check.length(); i++)
			{
				pchar = buff[i];
				if (proceed)
					proceed = (pchar == check[i]);
				else
					i = check.length();
			}
			return proceed;
		};
		ImGui::Text( "Position" );
		ImGui::SliderFloat( "X",&lightCB.pos.x,-60.0f,160.0f );
		ImGui::SliderFloat( "Y",&lightCB.pos.y,-60.0f,160.0f );
		ImGui::SliderFloat( "Z",&lightCB.pos.z,-60.0f,160.0f );
		ImGui::InputText("Update by input", imguiBuffah2, IM_ARRAYSIZE(imguiBuffah));
		if (ImGui::Button("Update Position"))
		{
			float val = {};
			char test[128];
			std::stringstream sstr(imguiBuffah2);
			sstr >> test >> val;

			switch (imguiBuffah2[0])
			{
			case 'x':
				if (Proceed("x", imguiBuffah2)) lightCB.pos.x = val;
				break;
			case 'y':
				if (Proceed("y", imguiBuffah2)) lightCB.pos.y = val;
				break;
			case 'z':
				if (Proceed("z", imguiBuffah2)) lightCB.pos.z = val;
				break;
			default:
				break;
			}
			std::fill(std::begin(imguiBuffah2), std::end(imguiBuffah2), 0);
		}
		ImGui::Text( "Intensity/Color" );
		ImGui::SliderFloat( "Intensity",&lightCB.diffuseIntensity,0.01f,2.0f,"%.2f");
		ImGui::ColorEdit3( "Diffuse Color",&lightCB.diffuseColor.x );
		ImGui::ColorEdit3( "Ambient",&lightCB.ambient.x );
		
		ImGui::Text( "Falloff" );
		ImGui::SliderFloat( "Constant",&lightCB.attConst,0.0f,1.0f,"%.2f");
		ImGui::SliderFloat( "Linear",&lightCB.attLin,0.0001f,1.0f,"%.4f");
		ImGui::SliderFloat( "Quadratic",&lightCB.attQuad,0.0000001f,0.001f,"%.7f");
		ImGui::InputText("update by input", imguiBuffah, IM_ARRAYSIZE(imguiBuffah));
		if (ImGui::Button("Update"))
		{

			float val = {};
			char test[128];
			std::stringstream sstr(imguiBuffah);
			sstr >> test >> val;

			switch (imguiBuffah[0])
			{
			case 'c':
				if (Proceed("const", imguiBuffah)) lightCB.attConst = val;
				break;
			case 'l':
				if (Proceed("linear", imguiBuffah)) lightCB.attLin = val;
				break;
			case 'q':
				if (Proceed("quad", imguiBuffah)) lightCB.attQuad = val;
				break;
			default:
				break;
			}
			std::fill(std::begin(imguiBuffah), std::end(imguiBuffah), 0);
		}
		ImGui::Text("FocalPoint (x/y/z)");
		ImGui::SliderFloat3("", &focalPoint.x, -100.0f, 100.0f, "%.0f");
		if( ImGui::Button( "Reset" ) )
		{
			Reset();
		}
	}
	ImGui::End();
}

void Light::Reset() noexcept
{
	lightCB = {
		{ 0.0f, 27.0f, -35.0f } ,0.0f,	// pos + padding
		{ 0.39f, 0.47f, 0.48f } ,0.0f,	// ambient + padding
		{ 0.83f, 0.67f, 0.15f } ,0.0f,	// diffuse + padding
		1.0f,							// diffuse intesity
		1.0f,							// attenuation constant
		0.0698f,						// attenuation linear
		0.0002f,						// attenuation quad
	};
	focalPoint = { 0.0f, 0.0f, 0.0f };
}

void Light::Draw(Graphics& gfx) const noexcept 
{
	mesh.SetPos(lightCB.pos);
	mesh.Draw(gfx);
}

void Light::Bind(Graphics& gfx, DirectX::FXMMATRIX view) const noexcept 
{
	auto cpy = lightCB;
	const auto pos = DirectX::XMLoadFloat3(&lightCB.pos);
	DirectX::XMStoreFloat3(&cpy.pos, DirectX::XMVector3Transform(pos, view));
	cbuf.Update( gfx, cpy );
	cbuf.Bind(gfx);
}

DirectX::XMFLOAT3 Light::GetPosition() const noexcept
{
	return lightCB.pos;
}

void Light::SetPosition(DirectX::XMFLOAT3 newPos) noexcept
{
	lightCB.pos = newPos;
}

DirectX::XMFLOAT3 Light::GetFocalPoint() const noexcept
{
	return focalPoint;
}

void Light::SetFocalPoint(DirectX::XMFLOAT3 newFocus) noexcept
{
	focalPoint = newFocus;
}

void Light::Revolve(float dt)
{
	auto rot = DirectX::XMMatrixRotationRollPitchYaw(0.0f, deg_rad(dt), 0.0f);
	auto tmp = DirectX::XMLoadFloat3(&lightCB.pos);
	DirectX::XMStoreFloat3(&lightCB.pos, DirectX::XMVector3Transform(tmp, rot));
}