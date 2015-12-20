/*
UserTracks.cc
Writes sa-ufiles.dat and sa-utrax.dat from file list
*/

#include <cstdio>
#include <cerrno>
#include <vector>
#include <string>
#include <stdexcept>

#include <Windows.h>
#include <ShlObj.h>

LPSTR myDocumentsPath=nullptr;
LPSTR uTraxPath=nullptr;
LPSTR uFilesPath=nullptr;

struct UTrax
{
	int startIndex;
	int stringLength;
	int audioType;
};

void __initMyDocumentsPath()
{
	if(myDocumentsPath) return;

	myDocumentsPath=new char[MAX_PATH];
	if(!SHGetSpecialFolderPathA(nullptr,myDocumentsPath,CSIDL_PERSONAL,false))
	{
		delete[] myDocumentsPath;
		throw std::runtime_error("Unable to get Documents folder");
	}

	uTraxPath=new char[MAX_PATH];
	uFilesPath=new char[MAX_PATH];
	memset(uTraxPath,0,MAX_PATH);
	memset(uFilesPath,0,MAX_PATH);
	strcpy(uTraxPath,myDocumentsPath);
	strcpy(uFilesPath,myDocumentsPath);

	strcat(uTraxPath,"\\GTA San Andreas User Files\\sa-utrax.dat");
	strcat(uFilesPath,"\\GTA San Andreas User Files\\sa-ufiles.dat");
}

errno_t UserTracksSave(std::vector<const char*> fileList)
{
	__initMyDocumentsPath();

	FILE* uTrax=fopen(uTraxPath,"wb");
	if(uTrax==nullptr) return errno;

	FILE* uFiles=fopen(uFilesPath,"w");
	if(uFiles==nullptr)
	{
		int r=errno;
		fclose(uTrax);
		return r;
	}

	for(auto i=fileList.begin();i!=fileList.end();i++)
	{
		char filePath[MAX_PATH];
		int temp;
		int extid=-1;

		strcpy(filePath,*i);
		
		// Detect audio type by extension
		{
			char* ext=strrchr(filePath,'.');
			if(!strcmpi(ext,".ogg")) extid=1;
			else if(!strcmpi(ext,".wav")) extid=2;
			else if(!strcmpi(ext,".mp3")) extid=3;
			else if(!strcmpi(ext,".aac") || !strcmpi(ext,".m4a")) extid=4;
		}

		// If we don't know the audio type, skip.
		if(extid==-1) continue;

		// The last backslash must be twice
		{
			char baseName[MAX_PATH];
			char* lastBackSlash=strrchr(filePath,'\\');
			if(lastBackSlash==nullptr)
			{
				fclose(uTrax);
				fclose(uFiles);
				return EINVAL;
			}

			strcpy(baseName,lastBackSlash+1);
			lastBackSlash[1]='\\';
			lastBackSlash[2]=0;
			strcat(filePath,baseName);
		}

		// OK, now write.
		int fLen=strlen(filePath);
		temp=ftell(uFiles);
		fwrite(&temp,4,1,uTrax);
		fwrite(filePath,1,fLen,uFiles);
		fwrite(&fLen,4,1,uTrax);
		fwrite(&extid,4,1,uTrax);
	}

	// Close file
	fclose(uTrax);
	fclose(uFiles);

	// No error
	return 0;
}

errno_t UserTracksSave(std::vector<std::string> fileList)
{
	std::vector<const char*> files;

	for(auto i=fileList.begin();i!=fileList.end();i++)
		files.push_back((*i).c_str());

	return UserTracksSave(files);
}

std::vector<std::string> UserTracksLoad()
{
	__initMyDocumentsPath();

	FILE* uTrax=fopen(uTraxPath,"rb");
	if(uTrax==nullptr) throw std::runtime_error("Cannot open sa-utrax.dat");

	FILE* uFiles=fopen(uFilesPath,"r");
	if(uFiles==nullptr)
	{
		fclose(uTrax);
		throw std::runtime_error("Cannot open sa-ufiles.dat");
	}

	UTrax uTrxStruct;
	std::vector<std::string> fileList;

	while(fread(&uTrxStruct,4,3,uTrax)==3)
	{
		char filePath[MAX_PATH];
		memset(filePath,0,MAX_PATH);

		fseek(uFiles,uTrxStruct.startIndex,SEEK_SET);
		// We don't want keep parsing while the sa-ufiles.dat reaches EOF
		if(fread(filePath,1,uTrxStruct.stringLength,uFiles)!=uTrxStruct.stringLength)
			break;

		// Convert the last 2 backslash to single backslash
		{
			char basename[MAX_PATH];
			char* backslash=strrchr(filePath,'\\');

			strcpy(basename,backslash+1);
			*backslash=0;
			strcat(filePath,basename);
		}

		fileList.push_back(std::string(filePath));
	}

	fclose(uTrax);
	fclose(uFiles);
	return fileList;
}