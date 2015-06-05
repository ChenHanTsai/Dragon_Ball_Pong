#pragma once

#include <xaudio2.h>

class AudioData
{
public:

	WAVEFORMATEXTENSIBLE wfx;
	XAUDIO2_BUFFER buffer;
	XAUDIO2_BUFFER_WMA wmabuffer;

	AudioData(void);
	~AudioData(void);

	// Load a Wave file
	HRESULT LoadWave(LPCTSTR strFileName);
	// Load an Xwma file created with xWMAencode.exe
	HRESULT LoadXwma(LPCTSTR strFileName);
};

