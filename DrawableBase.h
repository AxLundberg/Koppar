#pragma once
#include "Drawable.h"
#include "IndexBuffer.h"

template<class T>
class DrawableBase : public Drawable
{
protected:
	bool IsStaticInitialized() const noexcept
	{
		return !staticBinds.empty();
	}

	void AddStaticBind(std::unique_ptr<Bindable> bind) noexcept
	{
		if (typeid(*bind) != typeid(IndexBuffer))
		{
			staticBinds.push_back(std::move(bind));
		}
		else if (typeid(*bind) == typeid(IndexBuffer))
		{
			auto temp = dynamic_cast<IndexBuffer*>(bind.get());
			pIndexBuffer = temp;
			staticBinds.push_back(std::move(bind));
		}
	}

	void SetStaticIndexBuffer() noexcept
	{
		for (const auto& ib : staticBinds)
		{
			if (typeid(*ib) == typeid(IndexBuffer))
			{
				const auto temp = static_cast<IndexBuffer*>(ib.get());
				pIndexBuffer = temp;
			}
		}
	}

	const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept override
	{
		return staticBinds;
	}
protected:

	bool IsStaticEffectInitialized() const noexcept
	{
		return !staticEffectBinds.empty();
	}

	void AddStaticEffectBind(std::unique_ptr<Bindable> bind) noexcept
	{
		if (typeid(*bind) != typeid(IndexBuffer))
		{
			staticEffectBinds.push_back(std::move(bind));
		}
		else if (typeid(*bind) == typeid(IndexBuffer))
		{
			auto temp = dynamic_cast<IndexBuffer*>(bind.get());
			pEffectIndexBuffer = temp;
			staticEffectBinds.push_back(std::move(bind));
		}
	}
	void SetStaticEffectIndexBuffer() noexcept
	{
		for (const auto& ib : staticEffectBinds)
		{
			if (typeid(*ib) == typeid(IndexBuffer))
			{
				const auto temp = static_cast<IndexBuffer*>(ib.get());
				pEffectIndexBuffer = temp;
			}
		}
	}
	const std::vector<std::unique_ptr<Bindable>>& GetStaticEffectBinds() const noexcept override
	{
		return staticEffectBinds;
	}
private:
	static std::vector<std::unique_ptr<Bindable>> staticBinds;
	static std::vector<std::unique_ptr<Bindable>> staticEffectBinds;
};

template<class T>
std::vector<std::unique_ptr<Bindable>> DrawableBase<T>::staticBinds;
template<class T>
std::vector<std::unique_ptr<Bindable>> DrawableBase<T>::staticEffectBinds;