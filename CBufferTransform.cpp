#include "CBufferTransform.h"
#include "Cylinder.h"
#include "Particle.h"
#include <typeinfo>
#include "MaMath.h"

CBufferTransform::CBufferTransform(Graphics& gfx, const Drawable& parent)
	: parent(parent)
{
	if (!v_cbuf)
	{
		v_cbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx);
	}

}
CBufferTransform::CBufferTransform(Graphics& gfx, const Drawable& parent, bool geometry)
	: parent(parent)
{
	if (!v_cbuf)
	{
		v_cbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx);
	}

	if (!g_cbuf)
	{
		g_cbuf = std::make_unique<GeometryConstantBuffer<gsTransforms>>(gfx);
	}
}

void CBufferTransform::Bind(Graphics& gfx) noexcept
{

	const auto model = parent.GetTransformXM();
	const auto modelview = parent.GetTransformXM() * gfx.GetCamera();
	const auto modelviewprojection = parent.GetTransformXM() * gfx.GetCamera() * gfx.GetProjection();

	const Transforms tf =
	{
		DirectX::XMMatrixTranspose(model),
		DirectX::XMMatrixTranspose(modelview),
		DirectX::XMMatrixTranspose(modelviewprojection)
	};
	v_cbuf->Update(gfx, tf);
	v_cbuf->Bind(gfx);

	if (typeid(parent) == typeid(Particle))
	{
		const gsTransforms gstf =
		{
			gfx.GetCameraPosition(),
			DirectX::XMMatrixTranspose(model),
			DirectX::XMMatrixTranspose(modelviewprojection)
		};
		g_cbuf->Update(gfx, gstf);
		g_cbuf->Bind(gfx);
	}
	/*v_cbuf->Update(
		gfx,
		DirectX::XMMatrixTranspose(
			parent.GetTransformXM() *
			gfx.GetCamera() *
			gfx.GetProjection()
		)
	);
	v_cbuf->Bind(gfx);*/
}

std::unique_ptr<VertexConstantBuffer<CBufferTransform::Transforms>> CBufferTransform::v_cbuf;
//std::unique_ptr<PixelConstantBuffer<CBufferTransform::Transforms>> CBufferTransform::p_cbuf;
std::unique_ptr<GeometryConstantBuffer<CBufferTransform::gsTransforms>> CBufferTransform::g_cbuf;