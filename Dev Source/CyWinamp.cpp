// winamp.cpp

#include "CvGameCoreDLL.h"
#include "CyWinamp.h"
#include "CvWinamp.h"
#include "CyGlobalContext.h"

CyWinamp::CyWinamp() : m_pWinamp(NULL)
{
	m_pWinamp = &GC.getWinamp();
}

CyWinamp::CyWinamp(CvWinamp* pWinamp) : m_pWinamp(pWinamp)
{

}

void CyWinamp::Play(void)
{
	if (m_pWinamp)
        m_pWinamp->Play();
}

void CyWinamp::Stop(void)
{
	if (m_pWinamp)
        m_pWinamp->Stop();
}

void CyWinamp::Prev(void)
{
	if (m_pWinamp)
        m_pWinamp->Prev();
}

void CyWinamp::Next(void)
{
	if (m_pWinamp)
        m_pWinamp->Next();
}

void CyWinamp::Pause(void)
{
	if (m_pWinamp)
        m_pWinamp->Pause();
}

void CyWinamp::SetVolume(int vol)
{
	if (m_pWinamp)
        m_pWinamp->SetVolume(vol);
}

int CyWinamp::GetVolume()
{
	return m_pWinamp ? m_pWinamp->GetVolume() : -1;
}

int CyWinamp::GetTitleLength()
{
	return m_pWinamp ? m_pWinamp->GetTitleLength() : -1;
}

int CyWinamp::GetTitleProgress()
{
	return m_pWinamp ? m_pWinamp->GetTitleProgress() : -1;
}

std::wstring CyWinamp::GetTitleName()
{
	return m_pWinamp ? m_pWinamp->GetTitleName().GetCString() : L"";
}