#pragma once
#include <functional>
#include "platform.h"

struct MoviePlayerData;
struct ISoundStream;
class CMovieAudioSource;

class CUIVideoFFmpeg
{
public:
	using FinishFunc = std::function<void()>;

	CUIVideoFFmpeg() = default;
	CUIVideoFFmpeg(const char* aliasName);
	~CUIVideoFFmpeg();

	bool				Init(const char* pathToVideo, bool needSound);
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
	FinishFunc			m_onFinished;

protected:
	int					Run();
	static int			ThreadFn(void* data);

	string				m_aliasName;

	void*				m_threadHandle{ nullptr };
	uint8*				m_frameBuffer{ nullptr };
	MoviePlayerData*	m_player{ nullptr };
	ISoundStream*		m_audioStream{ nullptr };
	CMovieAudioSource*	m_audioSrc{ nullptr };
	int					m_playerCmd{ 0 };
	int					m_textureId{ -1 };
};