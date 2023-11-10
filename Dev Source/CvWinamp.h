#ifndef CVWINAMP_H
#define CVWINAMP_H

#pragma once

#define IPC_GETOUTPUTTIME	105
#define IPC_SETVOLUME		122
#define WINAMP_BUTTON_PREV	40044
#define WINAMP_BUTTON_PLAY	40045
#define WINAMP_BUTTON_PAUSE	40046
#define WINAMP_BUTTON_STOP	40047
#define WINAMP_BUTTON_NEXT	40048

class CvWinamp
{
public:
	DllExport CvWinamp();
	DllExport virtual ~CvWinamp();
	DllExport void Play();
	DllExport void Stop();
	DllExport void Prev();
	DllExport void Next();
	DllExport void Pause();
	DllExport void SetVolume(int vol);
	DllExport int GetVolume();
	DllExport int GetTitleProgress();
	DllExport int GetTitleLength();
	DllExport CvWString GetTitleName();

	HWND m_hwndWinamp;
};

#endif