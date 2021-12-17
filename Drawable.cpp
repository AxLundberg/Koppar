#include "Drawable.h"
#include "IndexBuffer.h"
#include <cassert>
#include <typeinfo>
#include "BaseBindables.h"

void Drawable::Draw(Graphics& gfx) const noexcept
{
	for (auto& b : bindables)
	{
		b->Bind(gfx);
	}
	for (auto& b : GetStaticBinds())
	{
		b->Bind(gfx);
	}
	gfx.DrawIndexed(pIndexBuffer->GetCount());
}

void Drawable::DrawEffects(Graphics& gfx, bool vs, bool gs, bool ps) const noexcept
{
	if (!vs)
		gfx.GetContext()->VSSetShader(nullptr, nullptr, 0);
	if (!gs)
		gfx.GetContext()->GSSetShader(nullptr, nullptr, 0);
	if (!ps)
		gfx.GetContext()->PSSetShader(nullptr, nullptr, 0);
	
	std::string s = typeid(*this).name();
	if (s == "class MeshAi")
	{
		int blyat = 0;
	}
	
	auto nBound   = 0;

	for (auto& b : GetStaticBinds())
	{
		if (dynamic_cast<VertexBuffer*>(b.get()))
		{
			b->Bind(gfx);
			nBound++;
		}
		else if (dynamic_cast<IndexBuffer*>(b.get()))
		{
			b->Bind(gfx);
			nBound++;
		}
		else if (dynamic_cast<Topology*>(b.get()))
		{
			b->Bind(gfx);
			nBound++;
		}
		if (nBound == 3)
		{
			break;
		}
	}
	for (auto& b : bindables)
	{
		if (nBound == 3)
		{
			break;
		}
		if (dynamic_cast<VertexBuffer*>(b.get()))
		{
			b->Bind(gfx);
			nBound++;
		}
		else if (dynamic_cast<IndexBuffer*>(b.get()))
		{
			b->Bind(gfx);
			nBound++;
		}
		else if (dynamic_cast<Topology*>(b.get()))
		{
			b->Bind(gfx);
			nBound++;
		}
	}
	for (auto& b : effectBindables)
	{
		b->Bind(gfx);
	}
	for (auto& b : GetStaticEffectBinds())
	{
		b->Bind(gfx);
	}

	gfx.DrawIndexed(pIndexBuffer->GetCount());
}

void Drawable::DrawInstanced(Graphics& gfx) const noexcept
{

	for (auto& b : GetStaticBinds())
	{
		b->Bind(gfx);
	}
	for (auto& b : bindables)
	{
		b->Bind(gfx);
	}
	auto nParticles = 256u;
	gfx.DrawInstanced(nParticles, {} );
}

void Drawable::AddBind(std::unique_ptr<Bindable> bind) noexcept
{
	assert("*Must* use Addindexbuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer));
	bindables.push_back(std::move(bind));
}

void Drawable::AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept
{
	assert("Attempting to add indexbuffer agin" && pIndexBuffer == nullptr);
	pIndexBuffer = ibuf.get();
	bindables.push_back(std::move(ibuf));
}

void Drawable::AddEffectBind(std::unique_ptr<Bindable> bind) noexcept
{
	assert("*Must* use Addindexbuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer));
	effectBindables.push_back(std::move(bind));
}

void Drawable::AddEffectIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept
{
	assert("Attempting to add indexbuffer agin" && pEffectIndexBuffer == nullptr);
	pEffectIndexBuffer = ibuf.get();
	effectBindables.push_back(std::move(ibuf));
}