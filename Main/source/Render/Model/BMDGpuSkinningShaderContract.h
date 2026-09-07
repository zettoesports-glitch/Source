#pragma once

namespace Render
{
	// GPU skinning contract shared by the VAO layout and the vertex shader.
	// Current Season 5.2 BMD data has one bone per vertex, so influence 0 is
	// populated with weight 1.0 and influences 1..3 remain zero.
	struct BMDGpuSkinningLayout
	{
		static const unsigned int PositionLocation = 0;
		static const unsigned int NormalLocation = 1;
		static const unsigned int TexCoordLocation = 2;
		static const unsigned int BoneIndexLocation = 3;
		static const unsigned int BoneWeightLocation = 4;
		static const unsigned int BoneUBOBinding = 2;
		static const unsigned int MaxBones = 200;
	};
}
