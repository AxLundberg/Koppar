#pragma once
#include <random>

class Rander
{
public:
	static void init()
	{
		s_Random.seed(std::random_device()());
	}
	static float random()
	{
		return (float)s_distrib(s_Random)/(float)std::numeric_limits<uint32_t>::max();
	}
private:
	static std::mt19937 s_Random;
	static std::uniform_int_distribution<std::mt19937::result_type> s_distrib;
};
std::mt19937 Rander::s_Random;
std::uniform_int_distribution<std::mt19937::result_type> Rander::s_distrib;