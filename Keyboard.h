#pragma once
#include <queue>	
#include <bitset>

class Keyboard
{
	friend class Window;
public:
	class Event
	{
	public:
		enum class Type
		{
			pressed,
			released,
			Invalid
		};
	private:
		Type type;
		unsigned char code;
	public:
		Event()
			:
			type(Type::Invalid),
			code(0u)
		{}
		Event(Type type, unsigned char code) noexcept
			:
			type(type),
			code(0u)
		{}
		bool IsPressed() const noexcept
		{
			return type == Type::pressed;
		}
		bool IsReleased() const noexcept
		{
			return type == Type::released;
		}
		bool IsValid() const noexcept
		{
			return type == Type::Invalid;
		}
		unsigned char GetCode() const noexcept
		{
			return code;
		}
	};
	Keyboard() = default;
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator = (const Keyboard&) = delete;
		/*KEY EVENTS*/
	bool KeyPressed(unsigned char keycode) const noexcept;
	Keyboard::Event ReadKey() noexcept;
	bool KeyEmpty() const noexcept;
	void FlushKey() noexcept;
		/*CHAR EVENTS*/
	char ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void FlushChar() noexcept;
	void Flush() noexcept;
		/*REPEAT CONTROL*/
	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
	bool AutorepeatIsEnabled() const noexcept;
private:
	void OnKeyPressed(unsigned char keycode) noexcept;
	void OnKeyReleased(unsigned char keycode) noexcept;
	void OnChar(char character) noexcept;
	void ClearState() noexcept;
	template<typename T>
	static void TrimBuffer(std::queue<T>& buffer) noexcept;
private:
	static constexpr unsigned int buffersize = 16u;
	static constexpr unsigned int nKeys = 256u;
	bool autorepeat = false;
	std::bitset<nKeys> keystates;
	std::queue<Event> keybuffer;
	std::queue<char> charbuffer;
};
