#include "Mouse.h"

std::pair<int, int> Mouse::GetPos() const noexcept
{
	return { x,y };
}

int Mouse::GetPosX() const noexcept
{
	return x;
}

int Mouse::GetPosY() const noexcept
{
	return y;
}

bool Mouse::LeftPressed() const noexcept
{
	return left_pressed;
}

bool Mouse::RightPressed() const noexcept
{
	return right_pressed;
}

Mouse::Event Mouse::Read() noexcept
{
	if (buffer.size() > 0u)
	{
		Mouse::Event e = buffer.front();
		buffer.pop();
		return e;
	}
	else
	{
		return Mouse::Event();
	}
}

void Mouse::OnMouseMove( int newx, int newy ) noexcept
{
	x = newx;
	y = newy;
	buffer.push( Mouse::Event( Mouse::Event::Type::Move, *this ) );
	TrimBuffer();
}

void Mouse::OnLeftPressed(int x, int y) noexcept
{
	left_pressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::LeftP, *this));
	TrimBuffer();
}

void Mouse::OnLeftReleased(int x, int y) noexcept
{
	left_pressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::LeftR, *this));
	TrimBuffer();
}

void Mouse::OnRightPressed(int x, int y) noexcept
{
	right_pressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::RightP, *this));
	TrimBuffer();
}

void Mouse::OnRightReleased(int x, int y) noexcept
{
	right_pressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::RightR, *this));
	TrimBuffer();
}

void Mouse::Flush() noexcept
{
	buffer = std::queue<Event>();
}

void Mouse::TrimBuffer() noexcept
{
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}

