#pragma once
#include <iostream>
#include <memory>

#include <chrono>
#include <debugapi.h>
#include <sstream>

class Timer
{
public:
	Timer() 
	{
		m_TimerStart = std::chrono::high_resolution_clock::now();
	}
	Timer(std::wstring&& name) : m_Name(std::move(name))
	{
		m_TimerStart = std::chrono::high_resolution_clock::now();
	}
	Timer(std::wstring&& name, std::wstring&& subName) : m_Name(std::move(name)), m_SubName(std::move(subName)), m_SubTimer(true)
	{
		m_TimerStart = std::chrono::high_resolution_clock::now();
	}
	void SubTimer(std::wstring&& name)
	{
		if (!m_SubTimer)
		{
			m_SubTimer = true;
			m_SubName = std::move(name);
			started = true;
			m_SubTimerStart = std::chrono::high_resolution_clock::now();
		}
		else
		{
			SubTimer();
			std::wstringstream wss;
			wss << L"\n" <<L" A subtimer called "<< m_SubName << L" Has already been initialized L" << "\n";
			OutputDebugStringW(wss.str().c_str());
		}
	}
	void SubTimer()
	{
		if (started)
		{
			auto subTimerStop = std::chrono::high_resolution_clock::now();
			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_SubTimerStart).time_since_epoch().count();
			auto stop = std::chrono::time_point_cast<std::chrono::microseconds>(subTimerStop).time_since_epoch().count();
			m_SubTime += stop - start;
			started = false;
		}
		else
		{
			started = true;
			m_SubTimerStart = std::chrono::high_resolution_clock::now();
		}
	}
	~Timer()
	{
		Stop();
	}

	void Stop()
	{
		auto timerStop = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_TimerStart).time_since_epoch().count();
		auto substart = (started) ? std::chrono::time_point_cast<std::chrono::microseconds>(m_SubTimerStart).time_since_epoch().count() : false;
		auto stop = std::chrono::time_point_cast<std::chrono::microseconds>(timerStop).time_since_epoch().count();
		auto delta = stop - start;
		auto milli = delta / 1000.0f;
		std::wostringstream wss;
		wss << L"\n" << m_Name << L" Took: " << delta << L" us" << L" (" << milli << L" ms)" << L"\n";
		OutputDebugStringW(wss.str().c_str());
		if (m_SubTimer)
		{
			if (started)
			{
				m_SubTime += stop - substart;
			}
			milli = (float)m_SubTime / 1000.0f;
			std::wostringstream wss;
			wss << L"\n" << m_SubName << L" Took: " << m_SubTime << L" us" << L" (" << milli << L" ms)" << L"\n";
			OutputDebugStringW(wss.str().c_str());
		}
	}
private:
	bool m_SubTimer = false;
	bool started = false;
	double m_SubTime = 0.0;
	std::wstring m_Name;
	std::wstring m_SubName;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_TimerStart;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_SubTimerStart;
};

template<typename T>
class CprArray {
	std::vector<T> _data;
	size_t rows, columns;
public:
	CprArray(size_t rows, size_t columns) : rows(rows), columns(columns), _data(rows* columns) {}

	CprArray(size_t rows, size_t columns, T init) : rows(rows), columns(columns), _data(rows* columns, init) {}

	T& operator()(size_t row, size_t column) {
		return _data[row * columns + column];
	}

	T const& operator()(size_t row, size_t column) const {
		return _data[row * columns + column];
	}
	size_t RowCount()
	{
		return rows;
	}
	size_t ColumnCount()
	{
		return columns;
	}
	~CprArray(){}
};
