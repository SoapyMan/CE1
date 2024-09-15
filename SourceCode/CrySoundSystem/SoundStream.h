#pragma once
#include "ISound.h"

struct FSOUND_STREAM;
class CSoundStream : public ISoundStream
{
public:
	~CSoundStream();

	void Init(ISoundStreamCallback* streamCb);
	void Terminate();

	void Play();
	void Stop();

	ISoundStreamCallback* GetCallback() const { return m_streamCb; }

	static signed char Callback(FSOUND_STREAM* pStream, void* pBuffer, int nLength, void* nParam);

private:
	ISoundStreamCallback* m_streamCb = nullptr;
	FSOUND_STREAM*	m_stream = nullptr;
	int				m_channel = -1;
};