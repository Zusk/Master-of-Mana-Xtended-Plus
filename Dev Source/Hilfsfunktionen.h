#pragma once

#ifndef HILFSFUNKTIONEN
#define HILFSFUNKTIONEN

void sortList(int* ListToSort, int* IDs, int iIndex);
void doAI_EconomyLog();

//New Class for Better Display - Sephi
class StringList
{
	public:
		StringList();
		~StringList();

		void append(CvWString newEntry);
		void newEntry(CvWString newEntry);
		CvWString get() const;
		bool isEmpty() const;

	private:
		bool m_bEmpty;
		bool m_bFirst;
		CvWString m_szText;
};

#endif

