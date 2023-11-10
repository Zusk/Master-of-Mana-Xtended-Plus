// winamp.cpp

#include "CvGameCoreDLL.h"
#include "CvWinamp.h"

CvWinamp::CvWinamp()
{
	m_hwndWinamp = FindWindow("Winamp v1.x",NULL);
}

CvWinamp::~CvWinamp()
{
}

void CvWinamp::Play(void)
{
	if(m_hwndWinamp)
	{
		SendMessage(m_hwndWinamp, WM_COMMAND, WINAMP_BUTTON_PLAY, 0);
	}
}

void CvWinamp::Stop(void)
{
	if(m_hwndWinamp)
	{
		SendMessage(m_hwndWinamp, WM_COMMAND, WINAMP_BUTTON_STOP, 0);
	}
}

void CvWinamp::Prev(void)
{	
	if(m_hwndWinamp)
	{
		SendMessage(m_hwndWinamp, WM_COMMAND, WINAMP_BUTTON_PREV, 0);
	}
}

void CvWinamp::Next(void)
{
	if(m_hwndWinamp)
	{
		SendMessage(m_hwndWinamp, WM_COMMAND, WINAMP_BUTTON_NEXT, 0);
	}
}

void CvWinamp::Pause(void)
{
	if(m_hwndWinamp)
	{
		SendMessage(m_hwndWinamp, WM_COMMAND, WINAMP_BUTTON_PAUSE, 0);
	}
}

void CvWinamp::SetVolume(int vol)
{
	if(m_hwndWinamp)
	{
		SendMessage(m_hwndWinamp, WM_USER, vol, IPC_SETVOLUME);
	}
}

int CvWinamp::GetVolume()
{
	if(m_hwndWinamp)
	{
		return SendMessage(m_hwndWinamp, WM_USER, -666, IPC_SETVOLUME);
	}
	return -1;
}

int CvWinamp::GetTitleLength()
{
	if(m_hwndWinamp)
	{
		return (int)SendMessage(m_hwndWinamp, WM_USER, 1, IPC_GETOUTPUTTIME);
	}
	return -1;
}

int CvWinamp::GetTitleProgress()
{
	if(m_hwndWinamp)
	{
		return (int)SendMessage(m_hwndWinamp, WM_USER, 0, IPC_GETOUTPUTTIME);
	}
	return -1;
}

CvWString CvWinamp::GetTitleName()
{
	if(m_hwndWinamp)
	{
		char this_title[2048], *p;
		GetWindowText(m_hwndWinamp,this_title,sizeof(this_title));
		p = this_title+strlen(this_title)-8;
		while (p >= this_title)
		{
			if (!strnicmp(p,"- Winamp",8)) break;
			p--;
		}
		if (p >= this_title) p--;
		while (p >= this_title && *p == ' ') p--;
		*++p=0;
		
		p=this_title;
		while (*p != ' ') p++;
		p++;
		CvWString szTitle(p);
		//szTitle = szTitle.Left(szTitle.GetLength()-9);
		//szTitle = szTitle.Right(szTitle.GetLength()-szTitle.Find(" "));
		return szTitle;
	}
	return L"";
}