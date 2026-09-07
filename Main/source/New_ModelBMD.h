#pragma once

#include <memory>

#if jdk_shader_local330
class BMD;
class OBJECT;

namespace OGL330
{
	bool IsShader();
	void SetShaderState(bool Enable);
}

// Runtime safety guard for shared BMD assets. NPC/monster instances are kept on
// the legacy renderer until the modern per-instance transform contract has full
// parity. Implemented in Render/Model/BMDModernObjectGuard.cpp.
bool BMDModernShouldForceLegacyObject(const OBJECT* object);

namespace OGL330MODEL
{
	typedef struct _mvec3
	{
		float x, y, z;
		_mvec3() : x(0.f), y(0.f), z(0.f) {}
	}mvec3;

	typedef struct _mvec4
	{
		float x, y, z, w;
		_mvec4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
	}mvec4;

	void Init();
	void ConvertOldMeshToVaoMesh(GLuint iModel, bool DelMesh = true);
	void UseShader(GLuint shaderID);
	void UnUseShader();
	void InvalidateShaderCache();
	void BeginUniformBatch();
	void EndUniformBatch();
	GLint GetBonesUniform(GLuint shaderID);
	GLint GetAlphaCutoffUniform(GLuint shaderID);
	bool SendBonePalette(GLuint shaderID, const std::shared_ptr<std::vector<float> >& palette);
	void SendUniform(GLuint shaderID, const mvec4& bodyLight, const mvec4& lightPosition, const mvec4& meshUV, const mvec4& setting1, const mvec4& setting2, const bool enableLight, const bool shadow, vec3_t vBodyOrigin);
	void SetTargetRender(OBJECT*);
}

class CGMNewRenderBMD;

namespace OGL330MODEL
{
	struct RenderMeshVAO
	{
		BMD* m_OldBMD;
		int		m_IndexMesh;
		int		m_TextureID;
		int		m_FlagRender;
		int		m_ConstNumber;
		bool	m_isLight;
		GLuint  m_Shader;
		float	m_isAlpha;
		mvec3	m_isColor;
		mvec4   m_setting1;
		mvec4   m_setting2;
		mvec4   m_bodyLight;
		mvec4   m_meshUV;
		mvec4   m_lightPosition;
		std::shared_ptr<std::vector<float> > m_BonePalette;

		// Immutable transform snapshot for the modern generated-shader path.
		// m_BonePalette stays byte-for-byte compatible with the legacy u_Bones
		// renderer; the modern runtime removes these baked model transforms from
		// that captured palette and sends them through the official instance
		// attributes instead.
		bool	m_ModernTranslate;
		float	m_ModernBodyScale;
		mvec3	m_ModernBodyOrigin;
	public:
		RenderMeshVAO()
		{
			m_OldBMD = NULL;
			m_IndexMesh = m_TextureID = m_FlagRender = -1;
			m_ConstNumber = 0;
			m_isLight = false;
			m_Shader = -1;
			m_isAlpha = 1.f;
			m_isColor.x = m_isColor.y = m_isColor.z = 1.f;
			m_ModernTranslate = false;
			m_ModernBodyScale = 1.f;
			m_ModernBodyOrigin.x = m_ModernBodyOrigin.y = m_ModernBodyOrigin.z = 0.f;
		}
	};
	typedef std::vector<RenderMeshVAO> MeshVAO;

	class CGMMeshShader
	{
	private:
		MeshVAO	    m_Data;
		bool		m_Lock;
		bool		m_Enabled;
		int			m_BatchDepth;

		vec3_t		m_vLightPosOrg;
		vec3_t		m_vLightDirOrg;

		vec3_t		m_vLightPos;
		vec3_t		m_vLightDir;

		bool		m_Transfrom;
		float* m_finalBone;
		std::shared_ptr<std::vector<float> > m_CurrentBonePalette;

		CGMMeshShader(const CGMMeshShader&);
		CGMMeshShader& operator=(const CGMMeshShader&);

		bool IsAlpha(int iType);

		void MakeShaderType(int iShaderType, bool enableLight, bool bAlphaNoUse, float BlendU, float BlendV, RenderMeshVAO& r);

	public:
		CGMMeshShader();
		~CGMMeshShader();

		void SetHighLight(bool bHighLight = false, bool bBattleCastle = false);

		inline bool Enabled() { return m_Enabled; }
		inline void Toggle(bool b) { m_Enabled = b; }
		inline void Lock(bool b) { m_Lock = b; }
		inline bool IsBatching() const { return m_BatchDepth > 0; }
		inline bool HasPendingMeshes() const { return !m_Data.empty(); }
		inline void BeginBatch() { ++m_BatchDepth; m_Lock = true; }
		inline bool EndBatch()
		{
			if (m_BatchDepth > 0)
			{
				--m_BatchDepth;
			}
			m_Lock = (m_BatchDepth > 0);
			return !m_Lock;
		}
		inline void	SetTransfrom(bool b) { m_Transfrom = b; }
		inline bool GetTransfrom() { return m_Transfrom; }

		inline void SetLightPosition(vec3_t vPos, vec3_t vDir)
		{
			VectorCopy(vPos, m_vLightPos); VectorCopy(vDir, m_vLightDir);
		}

		void AddBoneTransform(BMD* model, float(*BoneMatrix)[3][4], bool trans);
		inline float* GetfinalBone() { return m_finalBone; }
		void AddMeshCommand(BMD* pSrc, int idx, int RFlag, float Alpha, int BlendMesh, float Light, float BlendU, float BlendV, int Texture);
		void FlushAllMesh();
		void Release();
	public:
		static CGMMeshShader* Instance() {
			static CGMMeshShader sInstance;
			return &sInstance;
		};
	};
}


class rRenderLayOut
{
private:
	bool m_ForcedLegacy;
	bool m_PreviousShaderState;

public:
	rRenderLayOut(OBJECT* pObj)
		: m_ForcedLegacy(false)
		, m_PreviousShaderState(false)
	{
		// NPC/monster BMD objects are shared assets with mutable per-instance
		// state. Until the modern path snapshots every instance transform, render
		// them entirely through the known-good legacy path. This also prevents a
		// hover/highlight pass from switching an invisible modern NPC back to the
		// legacy renderer only while the mouse is over it.
		if (pObj != NULL && BMDModernShouldForceLegacyObject(pObj))
		{
			m_PreviousShaderState = OGL330::IsShader();
			if (m_PreviousShaderState)
			{
				if (GMMeshShader->HasPendingMeshes())
					GMMeshShader->FlushAllMesh();
				OGL330::SetShaderState(false);
				m_ForcedLegacy = true;
			}
		}

		OGL330MODEL::SetTargetRender(pObj);
	}

	~rRenderLayOut()
	{
		OGL330MODEL::SetTargetRender(NULL);
		if (m_ForcedLegacy)
			OGL330::SetShaderState(m_PreviousShaderState);
	}
};

#define GMMeshShader (OGL330MODEL::CGMMeshShader::Instance())

#endif
