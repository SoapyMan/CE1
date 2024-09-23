// ---------------------------------------------------------------------------------------------
//	Crytek CryENGINE source code
//	History:
//	- Created by Michael Glueck
// ---------------------------------------------------------------------------------------------
#pragma once

class GLMInfo
{
protected:
	//info for one particular patch 
	typedef struct GLMInfoPatch
	{
		uint uiOffsetX;
		uint uiOffsetY;
		uint uiWidth;
		uint uiHeight;
	}GLMInfoPatch;

	//info struct for one GLM
	typedef struct GLMInfoMesh
	{
		CString GLMName;
		uint uiLMIndex;
		std::vector<GLMInfoPatch> vPatchInfos;
	}GLMInfoMesh;



};



