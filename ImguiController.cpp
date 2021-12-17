#include "ImguiController.h"
#include "imgui.h"

ImguiController::ImguiController()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

ImguiController::~ImguiController()
{
	ImGui::DestroyContext();
}