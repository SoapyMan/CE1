//-------------------------------------------------------------------------------------------------
// Author: Marcio Martins
//
// Purpose:
//  - A Bink Video Control
//
// History:
//  - [8/8/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#ifndef UIVIDEOPANEL_H
#define UIVIDEOPANEL_H 

#define UICLASSNAME_VIDEOPANEL			"UIVideoPanel"

#include "UIWidget.h"
#include "UISystem.h"
#include "UIVideoFFmpeg.h"

class CUISystem;


class CUIVideoPanel : public CUIWidget, public _ScriptableEx<CUIVideoPanel>
{
public:

	UI_WIDGET(CUIVideoPanel)

		CUIVideoPanel();
	~CUIVideoPanel();

	CUISystem* GetUISystem() { return m_pUISystem; }

	string GetClassName();

	LRESULT Update(uint iMessage, WPARAM wParam, LPARAM lParam);	//AMD Port
	int Draw(int iPass);

	int LoadVideo(const string& szFileName, bool bSound);

	int ReleaseVideo();
	int Play();
	int Stop();
	int Pause(bool bPause = 1);
	int IsPlaying();
	int IsPaused();

	int SetVolume(int iTrackID, float fVolume);
	int SetPan(int iTrackID, float fPan);

	int SetFrameRate(int iFrameRate);

	int EnableVideo(bool bEnable = 1);
	int EnableAudio(bool bEnable = 1);

	int OnError(const char* szError);
	int OnFinished();

	static void InitializeTemplate(IScriptSystem* pScriptSystem);

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int LoadVideo(IFunctionHandler* pH);
	int ReleaseVideo(IFunctionHandler* pH);

	int Play(IFunctionHandler* pH);
	int Stop(IFunctionHandler* pH);
	int Pause(IFunctionHandler* pH);

	int IsPlaying(IFunctionHandler* pH);
	int IsPaused(IFunctionHandler* pH);

	int SetVolume(IFunctionHandler* pH);
	int SetPan(IFunctionHandler* pH);

	int SetFrameRate(IFunctionHandler* pH);

	int EnableVideo(IFunctionHandler* pH);
	int EnableAudio(IFunctionHandler* pH);

	string					m_szVideoFile;

	CUIVideoFFmpeg			m_videoPlayer;
	bool					m_bKeepAspect;
	bool					m_bLooping;
	bool					m_bPaused;
	bool					m_bFinished;
	UISkinTexture			m_pOverlay;
	int*					m_pSwapBuffer;
};

#endif
