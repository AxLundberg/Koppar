#pragma once
#include <math.h>
#include <stdlib.h>
#include <string>

constexpr float PI = 3.14159265f;
constexpr double PI_D = 3.1415926535897932;

constexpr float EUL = 2.71828183f;
constexpr double EUL_D = 2.7182818284590452;

template <typename T>
constexpr auto sq(const T& x)
{
	return x * x;
}

// get hypothenuse
template <typename T>
constexpr auto pyth(const T& x, const T& y)
{
	return sqrtf(sq(x) + sq(y));
}

// e^(x)
template <typename T>
constexpr auto expo(const T& x)
{
	return (T)pow((T)EUL_D, x);
}

template<typename T>
T wrap_angle(T theta)
{
	const T twoPI  = (T)2 * (T)PI_D;
	const T modded = (T)fmod(theta, twoPI);
	return (modded > (T)PI_D) ?
		(modded - (T)2.0 * (T)PI_D) :
		modded;
}

template<typename T>
constexpr T interpolate(const T& src, const T& dst, float alpha)
{
	return src + (dst - src) * alpha;
}

template<typename T>
constexpr T deg_rad(T deg)
{
	return deg * PI / (T)180.0;
}

template<typename T>
constexpr T rad_deg(T rad)
{
	return rad * 180.0f / (T)PI;
}

static std::wstring StringToWString(const std::string& str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wide = new wchar_t[len + 1];
	memset(wide, '\0', sizeof(wchar_t) * (len + 1));
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, len);
	std::wstring w_str(wide);
	delete[] wide;
	return w_str;
}