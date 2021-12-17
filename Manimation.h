#pragma once
#pragma once

#include "BaseBindables.h"	
#include "BaseDrawables.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <unordered_map>


class Manimation
{
public:
	Manimation(Graphics& gfx, const std::string fileName)
	{
		Assimp::Importer imp;

		std::string path = "md5models\\boblampclean\\boblampclean.md5mesh";
		//std::string path = "md5models\\boy.md5mesh";
		const auto pScene = imp.ReadFile(path,
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded
		);

		for (size_t i = 0; i < pScene->mNumAnimations; i++)
		{
			pScene->mAnimations[i]->mName.C_Str();
		}
	}
private:

};