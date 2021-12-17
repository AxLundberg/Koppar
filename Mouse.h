#pragma once
#include <queue>

class Mouse
{
	friend class Window;
public:
	class Event
	{
	public:
		enum class Type
		{
			LeftP,
			LeftR,
			RightP,
			RightR,
			WheelU,
			WheelD,
			Move,
			Invalid
		};
	private:
		Type type;
		bool left_pressed;
		bool right_pressed;
		int x;
		int y;
	public:
		Event() noexcept
			:
			type(Type::Invalid),
			left_pressed(false),
			right_pressed(false),
			x(0),
			y(0)
		{}
		Event(Type type, const Mouse& parent) noexcept
			:
			type(type),
			left_pressed(parent.left_pressed),
			right_pressed(parent.right_pressed),
			x(parent.x),
			y(parent.y)
		{}
		bool IsValid() const noexcept
		{
			return type != Type::Invalid;
		}
		Type GetType() const noexcept
		{
			return type;
		}
		std::pair<int, int> GetPos() const noexcept
		{
			return { x,y };
		}
		int GetPosX() const noexcept 
		{
			return x;
		}
		int GetPosY() const noexcept
		{
			return y;
		}
		bool LeftPressed() const noexcept
		{
			return left_pressed;
		}
		bool RightPressed() const noexcept
		{
			return right_pressed;
		}
	};
public:
	Mouse() = default;
	Mouse(const Mouse&) = delete;
	Mouse& operator  = (const Mouse&) = delete;
	std::pair<int, int> GetPos() const noexcept;
	int GetPosX() const noexcept;
	int GetPosY() const noexcept;
	bool LeftPressed() const noexcept;
	bool RightPressed() const noexcept;
	Mouse::Event Read() noexcept;
	bool IsEmpty() const noexcept
	{
		return buffer.empty();
	}
	void Flush() noexcept;
private:
	void OnMouseMove(int x, int y) noexcept;
	void OnLeftPressed(int x, int y) noexcept;
	void OnRightPressed(int x, int y) noexcept;
	void OnLeftReleased(int x, int y) noexcept;
	void OnRightReleased(int x, int y) noexcept;
	//void OnWheelUp(int x, int y) noexcept;
	//void OnWheelDown(int x, int y) noexcept;
	void TrimBuffer() noexcept;
private:
	static constexpr unsigned int bufferSize = 16u;
	std::queue<Event> buffer;
	//state variables
	int x;
	int y;
	bool left_pressed = false;
	bool right_pressed = false;
};