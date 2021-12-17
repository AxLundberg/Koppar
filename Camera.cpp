#include "Camera.h"
#include "imgui.h"

Camera::Camera(std::string name, DirectX::FXMMATRIX proj, DirectX::XMFLOAT3 pos, float pitch, float yaw) noexcept
    :
    camName(std::move(name)), camStartPos(pos), camStartPitch(pitch), camStartYaw(yaw)
{
    DirectX::XMStoreFloat4x4(&projMatrix, proj);
    Reset();
}

void Camera::SetProjection(DirectX::FXMMATRIX newProj) noexcept
{
    DirectX::XMStoreFloat4x4(&projMatrix, newProj);
}

void Camera::SetPosition(const DirectX::FXMVECTOR newPos) noexcept
{
    DirectX::XMStoreFloat3(&camPos, newPos);
};

void Camera::SetPosition(const DirectX::XMFLOAT3 newPos) noexcept
{
    camPos = newPos;
};

const std::string& Camera::GetName() const noexcept
{
    return camName;
}

DirectX::XMMATRIX Camera::GetRotMatrix() const noexcept
{
    return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f);
}

DirectX::XMFLOAT3 Camera::GetPosition() const noexcept
{
    return camPos;
}

DirectX::XMFLOAT3 Camera::GetRotation() const noexcept
{
    return { pitch, yaw, roll };
}

DirectX::XMMATRIX Camera::GetProjection() const noexcept
{
    return DirectX::XMLoadFloat4x4(&projMatrix);
}

DirectX::XMMATRIX Camera::GetMatrix() const noexcept
{
    DirectX::XMVECTOR posForward = DirectX::XMVector3Transform(
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
        DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)
    );
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3( &camPos);

    DirectX::XMVECTOR focalPoint = DirectX::XMVectorAdd(pos, posForward );
    return DirectX::XMMatrixLookAtLH(
        pos, focalPoint,
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    );
}

DirectX::XMMATRIX Camera::GetLookAtMatrix(const DirectX::XMFLOAT3 point) const noexcept
{
    using namespace DirectX;
    auto lh_mat = XMMatrixLookAtLH(XMLoadFloat3(&camPos), XMVectorSet(point.x, point.y, point.z, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    return lh_mat;
}


void Camera::FPMove(DirectX::XMFLOAT3 translation) noexcept
{
    //DirectX::XMVECTOR translate = DirectX::XMLoadFloat3(&translation);

    DirectX::XMStoreFloat3(&translation, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&translation),
        DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) /** DirectX::XMMatrixScaling(tjack,tjack,tjack)*/)
    );
    //DirectX::XMStoreFloat3(&translation, translate);

    camPos.x += translation.x;
    camPos.y += translation.y;
    camPos.z += translation.z;
}


void Camera::FPMoveGrav(DirectX::XMFLOAT3 translation, float height) noexcept
{
    DirectX::XMStoreFloat3(&translation, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&translation),
        DirectX::XMMatrixRotationRollPitchYaw(0.0f, yaw, 0.0f) /** DirectX::XMMatrixScaling(tjack,tjack,tjack)*/)
    );

    camPos.x += translation.x;
    camPos.y = height+6.0f;
    camPos.z += translation.z;
}

void Camera::FPRotation(float x, float y, float width, float height) noexcept
{
    float length = {};
    x = x - (width / 2);
    y = y - (height / 2);
    length = sqrtf(sq(x) + sq(y));
    length = length / (sqrtf(sq(width / 2) + sq(height / 2)));
    yaw += (x-prevx) * sensitivity;
    pitch = std::clamp(pitch + (y-prevy) * sensitivity, -PI * 0.495f, PI * 0.495f);

}

void Camera::SpawnControlWindow() noexcept
{
    ImGui::Text("Sensitivity");
    ImGui::SliderFloat("", &sensitivity, 0.002f, 0.001f, "%.5f");
    ImGui::Text("Position");
    ImGui::SliderFloat("x", &x, -180.0f, 180.0f, "%.1f");
    ImGui::SliderFloat("y", &y, -180.0f, 180.0f, "%.1f");
    ImGui::SliderFloat("z", &z, -180.0f, 180.0f, "%.1f");
    if (ImGui::Button("Set position"))
    {
        SetPosition(DirectX::XMVectorSet(x, y, z, 1.0f));
    }
    if (ImGui::Button("Reset"))
    {
        Reset();
    }
}

void Camera::Reset() noexcept
{
    camPos = camStartPos;
    pitch  = camStartPitch;
    yaw    = camStartYaw;
}
