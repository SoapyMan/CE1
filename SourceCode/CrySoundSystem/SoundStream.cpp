#include "SoundStream.h"
#include <fmod.h>

CSoundStream::~CSoundStream()
{
	if (m_stream)
		FSOUND_Stream_Close(m_stream);
}

void CSoundStream::Terminate()
{
	if (m_stream)
	{
		Stop();
		FSOUND_Stream_Close(m_stream);
	}
	m_stream = nullptr;
	m_streamCb = nullptr;
}

void CSoundStream::Init(ISoundStreamCallback* streamCb)
{
	if (m_stream)
		Terminate();

	m_streamCb = streamCb;
	const int bufferSamples = 4096;
	int fsoundFlags = FSOUND_16BITS | FSOUND_SIGNED | FSOUND_2D;
	if (streamCb->GetChannels() == 2)
		fsoundFlags |= FSOUND_STEREO;
	else
		fsoundFlags |= FSOUND_MONO;

	const int sampleRate = streamCb->GetSampleRate();
	m_stream = FSOUND_Stream_Create(CSoundStream::Callback, bufferSamples * streamCb->GetChannels(), fsoundFlags, sampleRate, m_streamCb);
}

void CSoundStream::Play()
{
	m_channel = FSOUND_Stream_Play(FSOUND_FREE, m_stream);
}

void CSoundStream::Stop()
{
	FSOUND_Stream_Stop(m_stream);
}

signed char CSoundStream::Callback(FSOUND_STREAM* pStream, void* pBuffer, int nLength, void* nParam)
{
	if (!nParam)
		return FALSE;
	ISoundStreamCallback* streamer = reinterpret_cast<ISoundStreamCallback*>(nParam);
	return streamer->ReadSamples(pBuffer, nLength);
}