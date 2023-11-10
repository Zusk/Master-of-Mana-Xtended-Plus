#ifndef CYWINAMP_H
#define CYWINAMP_H

class CvWinamp;

class CyWinamp
{
public:
	CyWinamp();
	CyWinamp(CvWinamp* pWinamp);
	CvWinamp* getWinamp() { return m_pWinamp;	}
	bool isNone() { return (m_pWinamp==NULL); }

	void Play();
	void Stop();
	void Prev();
	void Next();
	void Pause();
	void SetVolume(int vol);
	int GetVolume();
	int GetTitleProgress();
	int GetTitleLength();
	std::wstring GetTitleName();

protected:
	CvWinamp* m_pWinamp;
};

#endif