#pragma once
#include "platform.h"

struct MoviePlayerData;

class CUIVideoFFmpeg
{
public:
	CUIVideoFFmpeg() = default;
	CUIVideoFFmpeg(const char* aliasName);
	~CUIVideoFFmpeg();

	bool				Init(const char* pathToVideo);
	void				Terminate();

	void				Start();
	void				Stop();
	void				Rewind();

	bool				IsPlaying() const;

	// if frame is decoded this will update texture image
	void				Present();
	int					GetTextureId() const;
	int					GetWidth() const;
	int					GetHeight() const;

	void				SetTimeScale(float value);

	// Used to signal user when movie is completed. Not thread-safe
	bool				IsFinished() const { return m_isFinished; }

protected:
	int					Run();
	static int			ThreadFn(void* data);

	string				m_aliasName;

	void*				m_threadHandle{ nullptr };
	//ISoundSourcePtr		m_audioSrc;
	uint8*				m_frameBuffer{ nullptr };
	MoviePlayerData*	m_player{ nullptr };
	int					m_playerCmd{ 0 };
	int					m_textureId{ -1 };
	bool				m_isFinished{ true };
};