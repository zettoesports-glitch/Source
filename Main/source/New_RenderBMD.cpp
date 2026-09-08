#include "StdAfx.h"
#include "New_RenderBMD.h"

#if jdk_shader_local330
#include "ZzzBMD.h"
#include "ZzzTexture.h"
#include "TextureScript.h"
#include "Utilities/Log/muConsoleDebug.h"
#include "Render/Model/BMDModernRuntime.h"
#include "Render/Model/BMDModernRuntimeProbe.h"

CGMNewRenderBMD* g_NewRenderBMD = NULL;

void CGMShaderBMD::Render(OGL330MODEL::RenderMeshVAO& r)
{
	BMDModernProbeMarkRenderEntry();

	BMD& rModel = *r.m_OldBMD;
	VAOMesh& rNewMesh = rModel.New_Meshs[r.m_IndexMesh];
	if (!rNewMesh.VAO)
		return;

	OGL330MODEL::UseShader(r.m_Shader);

	if ((r.m_FlagRender & RENDER_COLOR) == RENDER_COLOR)
	{
		if ((r.m_FlagRender & RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if ((r.m_FlagRender & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else
			DisableAlphaBlend();

		if ((r.m_FlagRender & RENDER_NODEPTH) == RENDER_NODEPTH)
			DisableDepthTest();

		DisableTexture();

		if (r.m_isAlpha < 0.99f)
			EnableAlphaTest();
	}
	else if ((r.m_FlagRender & RENDER_CHROME) == RENDER_CHROME ||
		(r.m_FlagRender & RENDER_CHROME2) == RENDER_CHROME2 ||
		(r.m_FlagRender & RENDER_CHROME3) == RENDER_CHROME3 ||
		(r.m_FlagRender & RENDER_CHROME4) == RENDER_CHROME4 ||
		(r.m_FlagRender & RENDER_CHROME5) == RENDER_CHROME5 ||
		(r.m_FlagRender & RENDER_CHROME7) == RENDER_CHROME7 ||
		(r.m_FlagRender & RENDER_METAL) == RENDER_METAL ||
		(r.m_FlagRender & RENDER_OIL) == RENDER_OIL)
	{
		glEnable(GL_TEXTURE_2D);
		BindTexture(r.m_TextureID);

		if ((r.m_FlagRender & RENDER_CHROME3) == RENDER_CHROME3
			|| (r.m_FlagRender & RENDER_CHROME4) == RENDER_CHROME4
			|| (r.m_FlagRender & RENDER_CHROME5) == RENDER_CHROME5
			|| (r.m_FlagRender & RENDER_CHROME7) == RENDER_CHROME7)
			EnableAlphaBlend();
		else if ((r.m_FlagRender & RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if ((r.m_FlagRender & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else if ((r.m_FlagRender & RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
			EnableLightMap();
		else if (r.m_isAlpha >= 0.99f)
			DisableAlphaBlend();
		else
			EnableAlphaTest();

		if ((r.m_FlagRender & RENDER_NODEPTH) == RENDER_NODEPTH)
			DisableDepthTest();
	}
	else if ((r.m_FlagRender & RENDER_TEXTURE) == RENDER_TEXTURE)
	{
		glEnable(GL_TEXTURE_2D);
		BindTexture(r.m_TextureID);

		if ((r.m_FlagRender & RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if ((r.m_FlagRender & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else if (r.m_isAlpha < 0.99f || Bitmaps[r.m_TextureID].Components == 4)
			EnableAlphaTest();
		else
			DisableAlphaBlend();

		if ((r.m_FlagRender & RENDER_NODEPTH) == RENDER_NODEPTH)
			DisableDepthTest();
	}
	else if ((r.m_FlagRender & RENDER_BRIGHT) == RENDER_BRIGHT)
	{
		EnableAlphaBlend();
		DisableTexture();
		DisableDepthMask();

		if ((r.m_FlagRender & RENDER_NODEPTH) == RENDER_NODEPTH)
			DisableDepthTest();
	}

	// First-light modern path. It is deliberately opt-in and narrow; unsupported
	// materials or any resource/encoding failure return false and immediately
	// continue through the unchanged legacy u_Bones renderer below. During an
	// isolated remote-player rollout, only the selected OBJECT render scope may
	// call TryRender; world objects/local Hero continue through this same legacy
	// shader path without being globally disabled.
	const bool modernAttempt =
		BMDModernAllowModernForCommand(r.m_Owner) && gBMDModernRuntime.IsEnabled();
	if (modernAttempt)
	{
		// Chrome4 uses the same material texture/skeleton plumbing as Chrome01,
		// but a different UV formula. Keep the original render flags for GL blend
		// state above, then present an immutable compatibility view to ModernBMD:
		// - flags alias to CHROME|BRIGHT so the current runtime accepts it;
		// - setting1.z==1.0 becomes the shader's Chrome4 sentinel via Wave.x;
		// - captured cos/sin/wave are copied into lightPosition.xyz so the delayed
		//   flush never recomputes animation phase from mutable WorldTime.
		OGL330MODEL::RenderMeshVAO modernCommand = r;
		const int flagsNoDepth = r.m_FlagRender & ~RENDER_NODEPTH;
		const bool chrome4Alias =
			flagsNoDepth == (RENDER_CHROME4 | RENDER_BRIGHT) &&
			r.m_meshUV.x == 0.0f && r.m_meshUV.y == 0.0f;
		if (chrome4Alias)
		{
			modernCommand.m_FlagRender =
				(r.m_FlagRender & RENDER_NODEPTH) | RENDER_CHROME | RENDER_BRIGHT;
			modernCommand.m_lightPosition.x = r.m_setting1.x;
			modernCommand.m_lightPosition.y = r.m_setting1.y;
			modernCommand.m_lightPosition.z = r.m_setting1.w;
		}

		if (gBMDModernRuntime.TryRender(modernCommand))
		{
			OGL330MODEL::InvalidateShaderCache();
			return;
		}

		// TryRender temporarily owns the GL program. Some failure paths occur only
		// after the modern program has been bound. Re-establish the legacy BMD
		// program explicitly before continuing the fallback path; otherwise the
		// shader cache can still say r.m_Shader is active while GL_CURRENT_PROGRAM
		// has already been reset by the modern attempt.
		OGL330MODEL::InvalidateShaderCache();
		OGL330MODEL::UseShader(r.m_Shader);
	}

	SendUniform(r.m_Shader, r.m_bodyLight, r.m_lightPosition, r.m_meshUV, r.m_setting1, r.m_setting2, r.m_isLight, (r.m_FlagRender & RENDER_SHADOWMAP), r.m_OldBMD->BodyOrigin);
	GLint alphaCutoffLocation = OGL330MODEL::GetAlphaCutoffUniform(r.m_Shader);
	if (alphaCutoffLocation >= 0)
	{
		const bool additive = (r.m_FlagRender & (RENDER_BRIGHT | RENDER_DARK | RENDER_LIGHTMAP)) != 0;
		const bool alphaTexture = r.m_TextureID >= 0 && Bitmaps[r.m_TextureID].Components == 4;
		glUniform1f(alphaCutoffLocation, (!additive && (r.m_isAlpha < 0.99f || alphaTexture)) ? 0.25f : 0.0f);
	}

	OGL330MODEL::SendBonePalette(r.m_Shader, r.m_BonePalette);

	glBindVertexArray(rNewMesh.VAO);
	glDrawRangeElements(GL_TRIANGLES, 0, rNewMesh.IndexCount - 1, rNewMesh.IndexCount, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

#endif