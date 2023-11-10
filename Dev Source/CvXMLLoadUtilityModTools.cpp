//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvXMLLoadUtilityModTools.cpp
//
//  AUTHOR:  Vincent Veldman  --  10/2007
//
//  PURPOSE: Group of functions to enable true Modular loading for Civilization 4 BtS
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvInfos.h"
#include "CvXMLLoadUtility.h"
#include "CvXMLLoadUtilityModTools.h"
#include <string> 
using namespace std;

//function to expand string arrays.
//i.e. original XML holds 60citynames for america
//you add 5new in a module, this one will append those 5
//to the array of 60(technically, it's the other way around,but who cares)
void CvXMLLoadUtilityModTools::StringArrayExtend(CvString **ppszListNew, int* pNumNewListElements,
												 CvString **ppszListOld, int iNumOldListElements,
												 CvString szDefaultListVal) const
{
	CvString* pszListNew;
	CvString* pszListOld;
	pszListNew = *ppszListNew;
	pszListOld = *ppszListOld;
	int m_iAddNumNames = 0;
	int iCount;
	int iNumNewListElements = *pNumNewListElements;

	for ( int j = 0; j < iNumNewListElements; j++ )
	{	
		iCount = 0;
		for ( int i = 0; i < iNumOldListElements; i++ )
		{
			if ( pszListNew[j] == pszListOld[i] )
			{
				break;
			}
			else 
			{
				iCount++;
			}
		}		
		if ( iCount == iNumOldListElements ) //it's not a dupe, we want to add it to the array
		{			
			m_iAddNumNames++;		// Nr of Elements to add to the new array
		}
		else  //set empty, don't want a dupe!
		{
			pszListNew[j] = szDefaultListVal;
		}
	}
	

	// Make a temp Array of the new size		
	CvString* m_paszTempList = new CvString[iNumOldListElements + m_iAddNumNames];	
	iCount = 0;

	//read values from previous infoclass object, we want them to be first...
	for ( int i = 0; i < iNumOldListElements; i++ )
	{	
		m_paszTempList[i] = pszListOld[i];	
		iCount++;
	}
	
	//read values from new infoclass object
	for ( int i = 0; i < iNumNewListElements; i++ )
	{
		if ( pszListNew[i] != szDefaultListVal)
		{
			m_paszTempList[iCount] = pszListNew[i];
			iCount++;
		}			
	}
	
	// set Number of array elements need for the additional list plus old list
	*pNumNewListElements = iNumOldListElements + m_iAddNumNames;
	// delete infoclass array ppszListNew
	if ( NULL != *ppszListNew )
	{
		SAFE_DELETE_ARRAY(*ppszListNew);
	}
	//new instance of infoclass array with new number of elements
	*ppszListNew = new CvString[*pNumNewListElements];
	pszListNew = *ppszListNew;

	// copy values from Temp array
	for ( int i = 0; i < *pNumNewListElements; i++ )
	{
		pszListNew[i] = m_paszTempList[i];
	}
	//Delete the temp array
	SAFE_DELETE_ARRAY(m_paszTempList);
}

bool CvXMLLoadUtilityModTools::isDuplicate(int iNumValueNew, int *ppiListNew, int iValueOld) const
{	
	for ( int j = 0; j < iNumValueNew; j++ )
	{	
		if ( ppiListNew[j] == iValueOld)
		{
			return true; 			
		}
	}	
	return false;
}

bool CvXMLLoadUtilityModTools::isDuplicate(int iNumValueNew, CvWString *ppiListNew, CvWString wValueOld) const
{	
	for ( int j = 0; j < iNumValueNew; j++ )
	{	
		if ( ppiListNew[j] == wValueOld)
		{
			return true; 			
		}
	}	
	return false;
}

void CvXMLLoadUtilityModTools::setLocationName( CvString *pszTextVal, const char* szDirName)
{
	std::string szFiraxisLoad = "NONE";
	if (szDirName == szFiraxisLoad) //non modular loading doesn't need this method
	{
		return;
	}
	CvString szModular = szDirName;
	CvString szTextVal = *pszTextVal;
	szModular = szModular + szTextVal;

	if (isExcludedFile(szTextVal)) // These are special files that are relative to the Directory of Civ4BeyondSword.exe
	{
		if ( isModularArt(szModular))
		{			
			szTextVal = gDLL->getModName();
			szTextVal += "Assets\\";
			szTextVal += szModular;
			writeThm(szTextVal);
		}
	}
	else
	{
		if ( isModularArt(szModular))
		{
			szTextVal = szModular;
		}
	}
	*pszTextVal = szTextVal;
}

bool CvXMLLoadUtilityModTools::isExcludedFile(const char* szLocationName)
{
	CvString szLocationNameStripDot = szLocationName;
	CvString szLocationNameStripTemp = szLocationName;	

	string::size_type posDot = szLocationNameStripDot.find_last_of('.');

	if(posDot != string::npos) 
	{		
		//force lowercase for comparison
		int length = szLocationNameStripDot.size();
		for (int i = 0; i < length; ++i)
		{
			szLocationNameStripTemp[length - (i + 1)] = tolower(szLocationNameStripDot[i]);
		}
		string::size_type posDot = szLocationNameStripTemp.find_last_of('.');
		//delete everything after first dir
		szLocationNameStripTemp.erase(posDot);
		//compare
		if ( szLocationNameStripTemp == "mht" )  //if its a thema file, force static linking
		{
			return true;
		}
	}
	return false;
}

bool CvXMLLoadUtilityModTools::isCommaFile(CvString *pszTextVal, const char* szDirName)
{
	string::size_type posComma = (*pszTextVal).find_first_of(',');

	if(posComma != string::npos) //if no comma found at all, return false
	{
		CvString szTempLocation;
		CvString szAppend = " ";
		int iCountComma = 0;
		CvString szLocationNameStripComma;
		// Check how many comma's we have in the string and how many Button Files		

		szLocationNameStripComma = *pszTextVal;
		for ( int i = 0; i < szLocationNameStripComma.GetLength(); i++)
		{
			if (szLocationNameStripComma[i] == 44) // "," = 44 (ASCII)
			{
			  iCountComma++;
			}
		}

		// determine the append string at the end of the tag
		bool bContinue = true;
		szTempLocation = *pszTextVal;
		while ( bContinue)
		{
			posComma = szTempLocation.find_first_of(',');
			if(posComma != string::npos) //Prevent Null pointer deletion
			{
				szTempLocation = szTempLocation.substr(szTempLocation.find(",")+1);
				if (isdigit(szTempLocation[0]))  //We found the Append
				{					
					bContinue = false;
				}
			}
			else break;			
		}
		if (!bContinue )
		{
			szAppend = "," + szTempLocation;
		}

std::vector<CvString> vecButtonArtFile;
		// set Button Array
		// Array to hold the Button art files
		CvString szTempLocationSubstr;
		szTempLocation = *pszTextVal;
		while (true)
		{
			posComma = szTempLocation.find_first_of(',');
			if(posComma != string::npos) //Prevent Null pointer deletion
			{
				if (szTempLocation[0] == 44) // "," = 44 (ASCII)
				{
					szTempLocation = szTempLocation.substr(szTempLocation.find(",")+1);
				}
				szTempLocationSubstr = szTempLocation;
				posComma = szTempLocationSubstr.find_first_of(',');
				if(posComma != string::npos) //Prevent Null pointer deletion
				{
					szTempLocationSubstr.erase(posComma);
					if (!isdigit(szTempLocationSubstr[0]))
					{
						vecButtonArtFile.push_back(szTempLocationSubstr);
					}
					else break;
				}
				szTempLocation = szTempLocation.substr(szTempLocation.find(",")+1);
			}
			else if (szTempLocation.GetLength() >= 1)
			{
				if (!isdigit(szTempLocationSubstr[0]))
				{
					vecButtonArtFile.push_back(szTempLocationSubstr);
				}
				break;
			}
			else break;
		}		

		//Check if we need to modularize the files
		bool bNeedChange = false;
		CvString m_szFolderPath = GetProgramDir();		// Dir where the Civ4BeyondSword.exe is started from
		m_szFolderPath += gDLL->getModName();		// "Mods\Modname\"
		m_szFolderPath += "Assets/";
		m_szFolderPath += szDirName;			// "Modules\Modules\ModuleXXX" 
		for ( unsigned int i = 0; i < vecButtonArtFile.size(); i++)
		{
			szTempLocation = m_szFolderPath;
			szTempLocation += vecButtonArtFile[i];
			//set the Slash properly
			for ( int j = 0; j < szTempLocation.GetLength(); j++)
			{
				if ( szTempLocation[j] == 47)  // 47 = "/"
				{
					szTempLocation[j] = 92;  //92 = "\\", actually 1 backslash of course
				}
			}

			FILE *file = fopen(szTempLocation , "rb");
			if (file != NULL)
			{
				vecButtonArtFile[i] = szDirName + vecButtonArtFile[i];
				fclose(file);
				bNeedChange = true;
			}
		}

		// Now set the new tag string properly
		if (bNeedChange)
		{
			if (szLocationNameStripComma[0] == 44)
			{
				szTempLocation = ",";
			}
			else
			{
				szTempLocation = 0;
			}
			for ( unsigned int i = 0; i < vecButtonArtFile.size(); i++)
			{
				if (i != 0) szTempLocation += ",";
				szTempLocation += vecButtonArtFile[i];
			}
			if (szAppend[0] == 44)   // "," = 44 (ASCII)
			{
				szTempLocation += szAppend;
			}
			*pszTextVal = szTempLocation;
			return true;
		}	
	} 
	return false;
}

bool CvXMLLoadUtilityModTools::isModularArt(const char* szLocationName)
{
	//the passed is crap, we don't want to continue anything with it
	if (szLocationName == NULL || szLocationName == "" || szLocationName == "None" || szLocationName == "NONE" )
	{
		return false;
	}

	// Dir where the Civ4BeyondSword.exe is started from
	CvString m_szFolderPath = GetProgramDir();
	TCHAR szDirectory[MAX_PATH] = "";
	if(!::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory))
		return false;

	m_szFolderPath = szDirectory;
	// Safety measure - in case gDLL->getModName() doesn't deliver valid path...
	CvString szModDir;
	CvString szAssetsDir("Assets\\");
	const char* szModPath = gDLL->getModName();
	if (szModPath == NULL || szModPath == "")
	{
		szModDir = CvString("Mods\\WoC");
	}
	else
	{
		szModDir = CvString(szModPath);
	}
	
	m_szFolderPath.append("\\");
	m_szFolderPath.append(szModDir);		// "Mods\Modname\"
	m_szFolderPath.append(szAssetsDir);
	
	m_szFolderPath += szLocationName;		// where the tag points to, usually "Art\filename.xxx"

	FILE *file = fopen(m_szFolderPath.c_str(), "rb");

	if (file == NULL) 
	{
		return false;		
	}
	fclose(file);

	return true;
}

// Fully based on CString
CvString CvXMLLoadUtilityModTools::GetProgramDir()
{
	CvString szExeLocation = _pgmptr;
	if ( szExeLocation == NULL)
	{
		FAssertMsg(false, "Not running Stdlib Compatible Operating System?");
	}
	
	string::size_type posDot = szExeLocation.find_last_of('\\');

	if(posDot != string::npos) 
	{
		szExeLocation.erase(posDot + 1);
	}

	return szExeLocation;
}

CvString CvXMLLoadUtilityModTools::deleteFileName(const char* szDirName, const char szLocateChar)
{	
	CvString szDirNameStrip = szDirName;
	string::size_type pos = szDirNameStrip.find_last_of(szLocateChar);
	if(pos != string::npos) 
	{
		szDirNameStrip.erase(pos + 1);
	}
	else
	{
		FAssertMsg(false, "Couldn't find the directory slash")
	}

	return szDirNameStrip;
}

void CvXMLLoadUtilityModTools::writeThm(const char* szTextVal)
{	
	CvString tszTextVal = szTextVal;
	int posCut;
	string::size_type posBackSlash = tszTextVal.find_last_of('\\');
	string::size_type posSlashForw = tszTextVal.find_last_of('/');

	if (posBackSlash > posSlashForw)
	{
		posCut = posBackSlash;
	}
	else
	{
		posCut = posSlashForw;
	}

	if(posCut != string::npos) 
	{
		tszTextVal.erase(posCut);
	}
	else
	{
		FAssertMsg(false, "Something went wrong with the Theme file");
	}
	
	FILE *pFile = fopen(szTextVal, "w");

	//set the proper string to write to the Theme file	
	CvString szOutput = "resource_path\t";	
	szOutput.append("\"");
	szOutput.append(tszTextVal);
	szOutput.append("\";\n");
	szOutput.append("\n");
	szOutput.append("include\t");
	szOutput.append("\"");
	szOutput.append(tszTextVal);
	szOutput.append("\\Themes\\Civ4\\Civ4Theme.thm");	
	szOutput.append("\";\n");

	//write the file
	fputs(szOutput, pFile);
	fclose(pFile);
}
