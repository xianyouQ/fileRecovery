

// BisheclassStruct.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "iostream"
#include "windows.h"	
#include "vector"
#include "stdlib.h"
#include "string"
#include "list"

using namespace std;

class FileTime{
public:
	int Hour;
	int Min;
	int Sec;
	FileTime(){}
	FileTime(BYTE Time_0E,BYTE Time_0F){
		WORD Time = ((WORD) (Time_0F)<<8)+(WORD)Time_0E;
		Sec = (int) (Time&0x001f);
		Min = (int) ((Time&0x07e0)>>5);
		Hour = (int)((Time&0xf800)>>11);

	}
};
class FileDate{
public:
	int Year;
	int Mon;
	int Day;
	FileDate(){}
	FileDate(BYTE Date_10,BYTE Date_11){
		WORD Date = ((WORD)(Date_11)<<8)+(WORD)Date_10;
		Day = (int)(Date&0x001f);
		Mon = (int)((Date&0x01e0)>>5);
		Year = (int)((Date&0xfe00)>>9)+1980;

	}



};

class NTFSAttribute{
public:
	DWORD AttributeType;
	DWORD AttributeBytes;
	BYTE AttributeBoolAlwaysIn;
	NTFSAttribute(){}
	NTFSAttribute(DWORD AttributeType,DWORD AttributeBytes,BYTE AttributeBoolAlwaysIn){
		this->AttributeBoolAlwaysIn = AttributeBoolAlwaysIn;
		this->AttributeBytes = AttributeBytes;
		this->AttributeType = AttributeType;

	}


};


class NTFSStandardInfo{
public:
	
	SYSTEMTIME  BulidTime;
	SYSTEMTIME LastEditTime;
	SYSTEMTIME MFTLastTime;
	SYSTEMTIME LastAccessTime;
	NTFSStandardInfo(){}
	NTFSStandardInfo(LARGE_INTEGER  BulidTime,LARGE_INTEGER LastEditTime,LARGE_INTEGER MFTLastTime,LARGE_INTEGER LastAccessTime){
		FILETIME BulidTimeFT ;
		BulidTimeFT.dwHighDateTime = BulidTime.HighPart;
		BulidTimeFT.dwLowDateTime = BulidTime.LowPart;
		FileTimeToSystemTime(&BulidTimeFT,&this->BulidTime);
		FILETIME LastEditTimeFT ;
		LastEditTimeFT.dwHighDateTime = LastEditTime.HighPart;
		LastEditTimeFT.dwLowDateTime = LastEditTime.LowPart;
		FileTimeToSystemTime(&LastEditTimeFT,&this->LastEditTime);
		FILETIME MFTLastTimeFT ;
		MFTLastTimeFT.dwHighDateTime = MFTLastTime.HighPart;
		MFTLastTimeFT.dwLowDateTime = MFTLastTime.LowPart;
		FileTimeToSystemTime(&MFTLastTimeFT,&this->MFTLastTime);
		FILETIME LastAccessTimeFT ;
		LastAccessTimeFT.dwHighDateTime = LastAccessTime.HighPart;
		LastAccessTimeFT.dwLowDateTime = LastAccessTime.LowPart;
		FileTimeToSystemTime(&LastAccessTimeFT,&this->LastAccessTime);
	
	}



};


class NTFSFileName{
public:
	
	LARGE_INTEGER ParentMFTIndex;

	wstring FileName;
	NTFSFileName(){}
    NTFSFileName(LARGE_INTEGER ParentMFTIndex,wstring FileName){
	this->ParentMFTIndex = ParentMFTIndex;
	
	this->FileName.append(FileName);
	}
	

};
class CN{
public:
LARGE_INTEGER BeginCluster;
LARGE_INTEGER Length;
CN(){
	BeginCluster.QuadPart = 0;
	Length.QuadPart = 0;
}

};

class MFTData{
public:
	BYTE AlwaysIn;
	//BYTE Data[1024];
	vector<CN> DataLCN;
	CN DataVCN; 
	bool isNull(){
		if(AlwaysIn ==3)
			return true;
		else
			return false;
	}
	MFTData(){
		 AlwaysIn = 3;
	    // memset(Data,0,1024);
	}
	MFTData(BYTE *Buffer){
		this->AlwaysIn = Buffer[0x08];
		if(Buffer[0x08] ==0x00){
			WORD ContentPadding = ((WORD)Buffer[0x15]<<8)+(WORD)Buffer[0x14];
			DWORD ContentLength = ((DWORD)Buffer[0x13]<<24)+((DWORD)Buffer[0x12]<<16)+((DWORD)Buffer[0x11]<<8)+(DWORD)Buffer[0x10];
			if(ContentLength > 1024){
				cout<<""<<endl;
			}
			//memset(Data,0,1024);
			//memcpy_s(Data,1024,Buffer+ContentPadding,ContentLength);

		}
		else{
			WORD ContentPadding = ((WORD)Buffer[0x21]<<8)+(WORD)Buffer[0x20];

				DataVCN.BeginCluster.QuadPart = ((long long)Buffer[0x17]<<56)+((long long)Buffer[0x16]<<48)+
					((long long)Buffer[0x15]<<40)+((long long)Buffer[0x14]<<32)
				+((long long)Buffer[0x13]<<24)+((long long)Buffer[0x12]<<16)+((long long)Buffer[0x11]<<8)+((long long)Buffer[0x10]);
			
			DataVCN.Length.QuadPart = ((long long)Buffer[0x1f]<<56)+((long long)Buffer[0x1e]<<48)+
					((long long)Buffer[0x1d]<<40)+((long long)Buffer[0x1c]<<32)
				+((long long)Buffer[0x1b]<<24)+((long long)Buffer[0x1a]<<16)+((long long)Buffer[0x19]<<8)+((long long)Buffer[0x18]);
			BYTE FirstBYTE = Buffer[ContentPadding];
		
			while(FirstBYTE != 0){
				int Low = FirstBYTE&0x0f;
				int High = (FirstBYTE&0xf0)>>4;
				CN DataCN;
				for(int i = 0 ;i < Low ; i++){
					
					DataCN.Length.QuadPart = DataCN.Length.QuadPart+((long long)Buffer[ContentPadding+i+1]<<(8*i));
				}
				for(int i = 0; i < High;i++){
					DataCN.BeginCluster.QuadPart = DataCN.BeginCluster.QuadPart+((long long)Buffer[ContentPadding+Low+i+1]<<(8*i));
				}
				if(DataLCN.size() > 0) 
					DataCN.BeginCluster.QuadPart = DataCN.BeginCluster.QuadPart + DataLCN.at(DataLCN.size()-1).BeginCluster.QuadPart;
				DataLCN.push_back(DataCN);
				ContentPadding = ContentPadding+High+Low+1;
				FirstBYTE = Buffer[ContentPadding];

			}

		}
		   
	}

};


class MFT {
public:
	WORD HeadBytes;
	WORD Attribute;
	DWORD Bytes_Used;
	NTFSFileName MFTFileName;
	NTFSStandardInfo  MFTStandardInfo;
	MFTData Data;
	

	MFT(){}
	MFT(BYTE *Buffer){
		this->HeadBytes = ((WORD)Buffer[0x15]<<8)+(WORD)Buffer[0x14];
		this->Attribute = ((WORD)Buffer[0x17]<<8)+(WORD)Buffer[0x16];
		this->Bytes_Used = ((DWORD)Buffer[0x1b]<<24)+((DWORD)Buffer[0x1a]<<16)+((DWORD)Buffer[0x19]<<8)+(DWORD)Buffer[0x18];
		int pos = HeadBytes;
		while((int)Bytes_Used-pos  > 23){
			DWORD AttributeType = ((DWORD)Buffer[pos+0x03]<<24)+((DWORD)Buffer[pos+0x03]<<16)+((DWORD)Buffer[pos+0x01]<<8)+(DWORD)Buffer[pos];
			DWORD AttributeBytes = ((DWORD)Buffer[pos+0x07]<<24)+((DWORD)Buffer[pos+0x06]<<16)+((DWORD)Buffer[pos+0x05]<<8)+(DWORD)Buffer[pos+0x04];
			
			switch(AttributeType){
			case 0x10:
				{
				WORD AttributeHeadBytes = ((WORD)Buffer[pos+0x15]<<8)+(WORD)Buffer[pos+0x14];
				LARGE_INTEGER BuildTime;
				BuildTime.QuadPart = ((long long)Buffer[pos+AttributeHeadBytes+0x07]<<56)+((long long)Buffer[pos+AttributeHeadBytes+0x06]<<48)+
					((long long)Buffer[pos+AttributeHeadBytes+0x05]<<40)+((long long)Buffer[pos+AttributeHeadBytes+0x04]<<32)
				+((long long)Buffer[pos+AttributeHeadBytes+0x03]<<24)+((long long)Buffer[pos+AttributeHeadBytes+0x02]<<16)+((long long)Buffer[pos+AttributeHeadBytes+0x01]<<8)+((long long)Buffer[pos+AttributeHeadBytes]);
				LARGE_INTEGER LastEditTime;
				LastEditTime.QuadPart = ((long long)Buffer[pos+AttributeHeadBytes+0x0f]<<56)+((long long)Buffer[pos+AttributeHeadBytes+0x0e]<<48)+
					((long long)Buffer[pos+AttributeHeadBytes+0x0d]<<40)+((long long)Buffer[pos+AttributeHeadBytes+0x0c]<<32)
				+((long long)Buffer[pos+AttributeHeadBytes+0x0b]<<24)+((long long)Buffer[pos+AttributeHeadBytes+0x0a]<<16)+((long long)Buffer[pos+AttributeHeadBytes+0x09]<<8)+((long long)Buffer[pos+AttributeHeadBytes+0x08]);
		     	LARGE_INTEGER MFTTime;
				MFTTime.QuadPart = ((long long)Buffer[pos+AttributeHeadBytes+0x17]<<56)+((long long)Buffer[pos+AttributeHeadBytes+0x16]<<48)+
					((long long)Buffer[pos+AttributeHeadBytes+0x15]<<40)+((long long)Buffer[pos+AttributeHeadBytes+0x14]<<32)
				+((long long)Buffer[pos+AttributeHeadBytes+0x13]<<24)+((long long)Buffer[pos+AttributeHeadBytes+0x12]<<16)+((long long)Buffer[pos+AttributeHeadBytes+0x11]<<8)+((long long)Buffer[pos+AttributeHeadBytes+0x10]);
				LARGE_INTEGER LastAccessTime;
				LastAccessTime.QuadPart = ((long long)Buffer[pos+AttributeHeadBytes+0x1f]<<56)+((long long)Buffer[pos+AttributeHeadBytes+0x1e]<<48)+
					((long long)Buffer[pos+AttributeHeadBytes+0x1d]<<40)+((long long)Buffer[pos+AttributeHeadBytes+0x1c]<<32)
				+((long long)Buffer[pos+AttributeHeadBytes+0x1b]<<24)+((long long)Buffer[pos+AttributeHeadBytes+0x1a]<<16)+((long long)Buffer[pos+AttributeHeadBytes+0x19]<<8)+((long long)Buffer[pos+AttributeHeadBytes+0x18]);
		
				NTFSStandardInfo TmpStandardInfo(BuildTime,LastEditTime,MFTTime,LastAccessTime);
				this->MFTStandardInfo = TmpStandardInfo;
				break;
				}
			case 0x30:
				{
				WORD AttributeHeadBytes = ((WORD)Buffer[pos+0x15]<<8)+(WORD)Buffer[pos+0x14];
				if(Buffer[pos+AttributeHeadBytes+0x41] == 2)break;
				WORD FileNameLength = Buffer[pos+AttributeHeadBytes+0x40];
				LARGE_INTEGER ParentMFTID;
				ParentMFTID.QuadPart = ((long long)Buffer[pos+AttributeHeadBytes+0x07]<<56)+((long long)Buffer[pos+AttributeHeadBytes+0x06]<<48)+
					((long long)Buffer[pos+AttributeHeadBytes+0x05]<<40)+((long long)Buffer[pos+AttributeHeadBytes+0x04]<<32)
				+((long long)Buffer[pos+AttributeHeadBytes+0x03]<<24)+((long long)Buffer[pos+AttributeHeadBytes+0x02]<<16)+((long long)Buffer[pos+AttributeHeadBytes+0x01]<<8)+((long long)Buffer[pos+AttributeHeadBytes]);
				wchar_t FileName[1024];
				if(FileNameLength*2 >1024){
					cout<<"";
				}
				memcpy_s(FileName,1024,Buffer+pos+AttributeHeadBytes+0x42,FileNameLength*2);
				FileName[FileNameLength]=0;
				wstring FileNameTmp(FileName);
				NTFSFileName TmpNTFSFileName(ParentMFTID,FileNameTmp);
				this->MFTFileName = TmpNTFSFileName;
				break;
				}
			case 0x80:
				{
					MFTData TmpData(Buffer+pos);
					if(this->Data.isNull())
						this->Data = TmpData;
				}
			default:
				break;
			}
			pos=pos+AttributeBytes;
		}

	}

	void SetMFTFileName(BYTE *Buffer){
	
	
	}
	void SetMFTStandardInfo(BYTE *Buffer){
	
	}

	

};

class FAT32Directory{
public:
	BYTE First_Byte;
	wstring FileName;
    
	wstring path;
	BYTE FileAttribute;
	FileTime CreateFileTime;
	FileDate CreateFileDate;
	FileDate LastAccessDate;
	DWORD Begin_Cluster;
	DWORD Total_Bytes;
	FileTime LastEditTime;
	FileDate LastEditDate;
	vector<DWORD> ClusterVector;

	list<FAT32Directory> SubDir;

	FAT32Directory(){}
	
	 FAT32Directory( BYTE First_Byte,wstring FileName,BYTE FileAttribute,FileTime CreateFileTime,FileDate CreateFileDate,
		FileDate LastAccessDate,DWORD Begin_Cluster,DWORD Total_Bytes,FileTime LastEditTime,FileDate LastEditDate,wstring ParentPath){
		this->First_Byte = First_Byte;
		
		this->FileName.append(FileName);
		this->FileAttribute = FileAttribute;
		this->CreateFileDate = CreateFileDate;
		this->CreateFileTime = CreateFileTime;
		this->LastAccessDate = LastAccessDate;
		this->Begin_Cluster = Begin_Cluster;
		this->Total_Bytes = Total_Bytes;
		this->LastEditTime = LastEditTime;
		this->LastEditDate = LastEditDate;
		this->path.append(ParentPath+L"/"+FileName);
	}

	 
	 bool GetSubDir(FAT32Directory &Dir,wstring SubDirPath){
		for( list<FAT32Directory>::iterator iter = this->SubDir.begin(); iter!= this->SubDir.end();iter++)
		{
			FAT32Directory tmp = (FAT32Directory)*iter;
			
			if(tmp.FileName.find(SubDirPath) != wstring::npos){
				Dir = tmp;
				return true;

			}

		}
		return false;


	 }

	 void ShowDirectory(){
		 if(this->FileAttribute ==0x10)
		 
			 for( list<FAT32Directory>::iterator iter = this->SubDir.begin(); iter!= this->SubDir.end();iter++)
		 {
			 FAT32Directory tmp = (FAT32Directory) *iter;
			
             cout<<"文件名";
			
             wcout.imbue(locale("")); 
			 wcout<<tmp.FileName;
			 wcout.clear();
			 cout<<"  是否被删除:"<<(int)tmp.First_Byte<<"  是否目录"<<(int)tmp.FileAttribute<<"  文件创建日期"<<tmp.CreateFileDate.Year<<"-"
				 <<tmp.CreateFileDate.Mon<<"-"<<tmp.CreateFileDate.Day<<endl;

		 }


	 }

	  

};



class DBR {
public:
   WORD Bytes_Per_Sector;
   BYTE Sectors_Per_Cluster;
   LARGE_INTEGER Relative_LBA;
   
   DBR(){}
   DBR(WORD Bytes_Per_Sector , BYTE Sectors_Per_Cluster,LARGE_INTEGER Relative_LBA){
	   this->Bytes_Per_Sector = Bytes_Per_Sector;
	   this->Sectors_Per_Cluster = Sectors_Per_Cluster;
	   this->Relative_LBA = Relative_LBA;
   }
 



};

class FAT32DBR:public DBR{
public:
	WORD Reserved_Sectors;
	BYTE FATSNumber;
	DWORD Sectors_FAT;
	DWORD RootDir_1st_Cluster;
	DWORD TotalSector;
	DWORD TotalCluster;
	
	FAT32DBR(){}
	FAT32DBR(WORD Bytes_Per_Sector , BYTE Sectors_Per_Cluster,LARGE_INTEGER Relative_LBA,WORD Reserved_Sectors,BYTE FATSNumber,
		DWORD Sectors_FAT,DWORD RootDir_1st_Cluster,DWORD TotalSector):DBR(Bytes_Per_Sector,Sectors_Per_Cluster,Relative_LBA){
			this->FATSNumber = FATSNumber;
			this->Reserved_Sectors = Reserved_Sectors;
			this->RootDir_1st_Cluster = RootDir_1st_Cluster;
			this->Sectors_FAT = Sectors_FAT;
			this->TotalSector = TotalSector;
			this->TotalCluster = (TotalSector- (DWORD)Reserved_Sectors-Sectors_FAT*(DWORD)FATSNumber)/Sectors_Per_Cluster;
	}
	  LARGE_INTEGER GetTheLBAFromCluster(DWORD ClusterNum){
		  
		  if(ClusterNum!=0x0fffffff&&ClusterNum != 0xf8ffff0f&&ClusterNum != 0x00000000){
		  long long  Sectors = (long long)(this->Reserved_Sectors+this->Sectors_FAT*this->FATSNumber+(ClusterNum-2)*this->Sectors_Per_Cluster);
		  LARGE_INTEGER Tmp ;
		  Tmp.QuadPart = this->Relative_LBA.QuadPart+(long long)(this->Bytes_Per_Sector)*Sectors;
		  return Tmp;
		  }

   }
	  DWORD GetTheNextCluster(DWORD ClusterNow,HANDLE dFile){
		  DWORD Fat = GetTheFAT(ClusterNow,dFile);
		  if(Fat==0x0fffffff||Fat == 0xf8ffff0f||Fat == 0x00000000){
			  return NULL;
		  }
		  else{
			  return Fat;
		  }

	  }
	  DWORD GetTheNextNullCluster(DWORD ClusterNow,HANDLE dFile){
		  if(GetTheFAT(ClusterNow,dFile)==0x00000000)
			  return ClusterNow;
		  else if(ClusterNow+1 > TotalCluster){
			  return GetTheNextNullCluster(2,dFile);
		  }
		  else{
			  return GetTheNextNullCluster(ClusterNow+1,dFile);
		  }
	  }
	  DWORD GetTheFAT(DWORD ClusterNow,HANDLE dFile){
		  if(ClusterNow > TotalCluster||ClusterNow < 2){
			  cout<<"Cluster error!"<<endl;
			  return NULL;

		  }
		  LARGE_INTEGER Tmp ;
		  Tmp.QuadPart = (long long ) (this->Reserved_Sectors*this->Bytes_Per_Sector)+this->Relative_LBA.QuadPart;
		  Tmp.QuadPart = Tmp.QuadPart +ClusterNow*4/512*512;
		  int Begin = (ClusterNow*4)%512;
		  
		  if(SetFilePointerEx(dFile,Tmp,0,FILE_BEGIN) == INVALID_SET_FILE_POINTER && GetLastError()

       != NO_ERROR){

		   printf("SeekFilePointer error %d",GetLastError());
		   return NULL;
			}
		  else{
			  DWORD dwRet;
			  BYTE Buffer[512];
			  if( ReadFile(dFile,Buffer,512,&dwRet,0)!=0){
					DWORD Tmp = ((DWORD)(Buffer[Begin+3])<<24)+((DWORD)(Buffer[Begin+2])<<16)+((DWORD)(Buffer[Begin+1])<<8)+(DWORD)Buffer[Begin];
				
					return Tmp;
			}
			  else{
				  printf("ReadFile error!  %d\n",GetLastError());
				  return NULL;
			  }
			  
		  }

	  }


};


class NTFSDBR:public DBR{
public:
	LARGE_INTEGER TotalSector;
	LARGE_INTEGER MFTBeginCluster;
	BYTE TotalMFTBytes;
	NTFSDBR(){}
	NTFSDBR(WORD Bytes_Per_Sector , BYTE Sectors_Per_Cluster,LARGE_INTEGER Relative_LBA,
		LARGE_INTEGER TotalSector,LARGE_INTEGER MFTBeginCluster,BYTE TotalMFTBytes):DBR(Bytes_Per_Sector,Sectors_Per_Cluster,Relative_LBA){
	 this->TotalSector = TotalSector;
	 this->MFTBeginCluster = MFTBeginCluster;
	 this->TotalMFTBytes = TotalMFTBytes;

	}

	LARGE_INTEGER GetTheLBAAdr(LARGE_INTEGER Cluster){
		LARGE_INTEGER LBA;
	     LBA.QuadPart = Cluster.QuadPart*(long long)this->Sectors_Per_Cluster*
			(long long)this->Bytes_Per_Sector+this->Relative_LBA.QuadPart;
		 return LBA;
	}

};

class Partition{
public:
	BYTE Boot_Indicator;
	BYTE System_ID;
	LARGE_INTEGER Relative_Sector_Begin;
	LARGE_INTEGER Total_Sector;
	FAT32DBR PartitionFAT32DBR;
	NTFSDBR PartitionNTFSDBR;
	FAT32Directory Root;
	wstring PartitionLabel;
	vector<MFT> NTFSMFTVector;


	Partition(){}
	Partition(BYTE Boot_Indicator,BYTE System_ID,LARGE_INTEGER Relative_Sector_Begin,LARGE_INTEGER Total_Sector,wstring PartitionLabel)
	{
		this->Boot_Indicator = Boot_Indicator;
		this->System_ID = System_ID;
		this->Relative_Sector_Begin = Relative_Sector_Begin;
        
        this->Total_Sector = Total_Sector;
		this->PartitionLabel = PartitionLabel;
		Root.path.append(PartitionLabel);
		Root.FileAttribute = 0x10;
	}

	
	bool isNull(){
		if(System_ID == 0x00)
			return true;
		return false;

	}

	bool GetTheDataFromPartiton(BYTE *Buffer,HANDLE dFile){
		DWORD  dwRet;
		if(!this->isNull() ){
		if(SetFilePointerEx(dFile,this->Relative_Sector_Begin,0,FILE_BEGIN) == INVALID_SET_FILE_POINTER && GetLastError()

       != NO_ERROR){

		   printf("SeekFilePointer error %d",GetLastError());
		   return false;
			}
		else{
			if( ReadFile(dFile,Buffer,512,&dwRet,0)!=0){
					
				return true;
			}
			else{
					return false;

			}
		}
		return false;
		}
	}

	
	void GetTheDBR(HANDLE dFile){
		if(this->System_ID == 0x0b||this->System_ID == 0x0c)
				GetFAT32DBR(dFile);
			else if(this->System_ID == 0x07)
				GetNTFSDBR(dFile);
			else{}
	}
	void GetTheDirectory(HANDLE dFile){
		if(this->System_ID == 0x0b||this->System_ID == 0x0c)
			GetFAT32Directory(Root,PartitionFAT32DBR.RootDir_1st_Cluster,dFile);
			else if(this->System_ID == 0x07)
				GetNTFSDirectory(dFile);
			else{}


	}
	void ReBuildFile(FAT32Directory &FAT32Dir,HANDLE hDev){
			
		if(FAT32Dir.First_Byte!=0xe5)
		{
			return ;
		}
		Recovery(FAT32Dir,hDev);
		
     
		
	  HANDLE File = CreateFile(FAT32Dir.FileName.c_str(),GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);
	  if(INVALID_HANDLE_VALUE != File){
		  for(vector<DWORD>::iterator iter = FAT32Dir.ClusterVector.begin();
			  iter != FAT32Dir.ClusterVector.end();iter++)
		  {
			  DWORD ClusterNow = (DWORD)*iter;
			 
			  BYTE Buffer[1024];
			   DWORD NumToRead;
			  DWORD BytesToRead = (DWORD)this->PartitionFAT32DBR.Bytes_Per_Sector*(DWORD)this->PartitionFAT32DBR.Sectors_Per_Cluster;
			   if(ClusterNow == 0) {
				   string BufferStr("------Message Lost in this Cluster--------");
				   memset(Buffer,0,1024);
				   memcpy_s(Buffer,1024,BufferStr.c_str(),BufferStr.size());
				   WriteFile(File,Buffer,BytesToRead,&NumToRead,0);
				   continue;
			  }
			 
              LARGE_INTEGER LBA  = this->PartitionFAT32DBR.GetTheLBAFromCluster(ClusterNow);
			  SetFilePointerEx(hDev,LBA,0,FILE_BEGIN);
			  ReadFile(hDev,Buffer,BytesToRead,&NumToRead,0);
			  WriteFile(File,Buffer,BytesToRead,&NumToRead,0);

		  }

	  }
	  CloseHandle(File);

	}
	void showDirectory(){
			if(this->System_ID == 0x0b||this->System_ID == 0x0c)
				this->Root.ShowDirectory();
			else if(this->System_ID == 0x07){
				for(vector<MFT>::iterator iter = this->NTFSMFTVector.begin();
					iter!= this->NTFSMFTVector.end();iter++)
				{
					MFT Tmp = (MFT)*iter;
					cout <<"文件名 :";
					wcout.imbue(locale(""));
					wcout<<Tmp.MFTFileName.FileName;
					wcout.clear();
					cout<<"  文件属性："<<Tmp.Attribute<<"  文件创建日期:"<<Tmp.MFTStandardInfo.BulidTime.wYear<<"-"<<
						Tmp.MFTStandardInfo.BulidTime.wMonth<<"-"<<Tmp.MFTStandardInfo.BulidTime.wDay<<endl;
				}


			}


	} 
	

private :
	void GetFAT32DBR(HANDLE dFile){
		BYTE Buffer[512];
		if(GetTheDataFromPartiton(Buffer,dFile)){
			WORD Bytes_Per_Sector = ((WORD) Buffer[0x0c]<<8) + (WORD)Buffer[0x0b];
			WORD Reserved_Sectors = ((WORD) Buffer[0x0f]<<8)+(WORD)Buffer[0x0e];
	        DWORD Sectors_FAT = ((DWORD) Buffer[0x27]<<24)+((DWORD) Buffer[0x26]<<16)+((DWORD) Buffer[0x25]<<8) + (DWORD) Buffer[0x24];
	        DWORD RootDir_1st_Cluster = ((DWORD) Buffer[0x2f]<<24) + ((DWORD) Buffer[0x2e] <<16) +((DWORD) Buffer[0x2d]<<8)+(DWORD) Buffer[0x2c];
			DWORD TotalSector = ((DWORD) Buffer[0x23]<<24) + ((DWORD) Buffer[0x22] <<16) +((DWORD) Buffer[0x21]<<8)+(DWORD) Buffer[0x20];
	
			FAT32DBR TmpDBR(Bytes_Per_Sector , Buffer[0x0d],this->Relative_Sector_Begin,Reserved_Sectors,Buffer[0x10],Sectors_FAT,RootDir_1st_Cluster,TotalSector);
			this->PartitionFAT32DBR = TmpDBR;
		}


	}
	void GetNTFSDBR(HANDLE dFile){
		BYTE Buffer[512];
		if(GetTheDataFromPartiton(Buffer,dFile)){
			WORD Bytes_Per_Sector = ((WORD) Buffer[0x0c]<<8) + (WORD)Buffer[0x0b];
			LARGE_INTEGER TotalSector,MFTBeginCluster;
			TotalSector.QuadPart = ((long long)Buffer[0x2f]<<56)+((long long)Buffer[0x2e]<<48)+((long long)Buffer[0x2d]<<40)+((long long)Buffer[0x2c]<<32)
				+((long long)Buffer[0x2b]<<24)+((long long)Buffer[0x2a]<<16)+((long long)Buffer[0x29]<<8)+((long long)Buffer[0x28]);
			MFTBeginCluster.QuadPart = ((long long)Buffer[0x37]<<56)+((long long)Buffer[0x36]<<48)+((long long)Buffer[0x35]<<40)+((long long)Buffer[0x34]<<32)
				+((long long)Buffer[0x33]<<24)+((long long)Buffer[0x32]<<16)+((long long)Buffer[0x31]<<8)+((long long)Buffer[0x30]);
			NTFSDBR TmpNTFS(Bytes_Per_Sector,Buffer[0x0d],this->Relative_Sector_Begin,TotalSector,MFTBeginCluster,Buffer[0x40]);
			this->PartitionNTFSDBR = TmpNTFS;

		}
		

	}
	void GetFAT32Directory(FAT32Directory &Parent,DWORD BeginCluster,HANDLE dFile){
		DWORD BytesToRead = (DWORD)this->PartitionFAT32DBR.Bytes_Per_Sector*(DWORD)this->PartitionFAT32DBR.Sectors_Per_Cluster;
		while(BeginCluster){
		DWORD readBytes;
		BYTE Buffer[1024];
		LARGE_INTEGER Tmpp = this->PartitionFAT32DBR.GetTheLBAFromCluster(BeginCluster);
		SetFilePointerEx(dFile,this->PartitionFAT32DBR.GetTheLBAFromCluster(BeginCluster),0,FILE_BEGIN); 
		ReadFile(dFile,Buffer,BytesToRead,&readBytes,NULL) ;
		if(Parent.First_Byte == 0xe5&&(Buffer[0]!=0x2e||Buffer[32]!=0x2e))
			return ;
		wstring FileName;
		for(DWORD i = 0 ;i <BytesToRead ;i = i+32 ) {
			
			if((Buffer[i] == 0x2e)||(Buffer[i] == 0x00)||(Buffer[i+0x0b]  == 0x08 ) || (Buffer[i+0x0b]  == 0x16)) { 
				FileName.clear();
				continue; 
			}
			if(Buffer[i+0x0b]  == 0x0F ){
				
				
				wchar_t FileNameTmp[14];
			
				 memcpy_s(FileNameTmp,28,Buffer+i+1,10);
				 memcpy_s(FileNameTmp+5,18,Buffer+i+14,12);
				  memcpy_s(FileNameTmp+11,6,Buffer+i+28,4);
				  FileNameTmp[13] = 0;
				  wstring FileName1(FileNameTmp);
				  
				 FileName=FileName1+FileName;
				
				continue;
			}
			if(FileName.size()==0){

				char FileNamechar[12];
			   memcpy(FileNamechar,Buffer+i,11);
			   FileNamechar[11] = 0;
			   size_t len = strlen(FileNamechar) + 1;
               size_t converted = 0;
               wchar_t *WStr;
                  WStr=(wchar_t*)malloc(len*sizeof(wchar_t));
               mbstowcs_s(&converted, WStr, len, FileNamechar, _TRUNCATE);
			   FileName.append(WStr);

			}


			DWORD Begin_Cluster = ((DWORD)(Buffer[i+0x15])<<24)+((DWORD)(Buffer[i+0x14])<<16)+((DWORD)(Buffer[i+0x1b])<<8)+(DWORD)Buffer[i+0x1a];
		    DWORD Total_Bytes =	((DWORD)(Buffer[i+0x1f])<<24)+((DWORD)(Buffer[i+0x1e])<<16)+((DWORD)(Buffer[i+0x1d])<<8)+(DWORD)Buffer[i+0x1c];				
			FileTime CreateFileTime(Buffer[0x0e],Buffer[0x0f]);
			FileDate CreateFileDate(Buffer[0x10],Buffer[0x11]);
			FileDate LastAccessFileDate(Buffer[0x12],Buffer[0x13]);
			FileTime LastEditFileTime(Buffer[0x16],Buffer[0x17]);
			FileDate LastEditFileDate(Buffer[0x18],Buffer[0x19]);
	
			FAT32Directory tmp(Buffer[i],FileName,Buffer[i+0x0b],CreateFileTime,CreateFileDate,LastAccessFileDate
				,Begin_Cluster,Total_Bytes,LastEditFileTime,LastEditFileDate,Parent.path);
			FileName.clear();
			
			if(tmp.FileAttribute ==0x10)
				GetFAT32Directory(tmp,tmp.Begin_Cluster,dFile);
		
			
			Parent.SubDir.push_back(tmp);

		}
		BeginCluster = PartitionFAT32DBR.GetTheNextCluster(BeginCluster,dFile);


		}

	}

	void GetNTFSDirectory(HANDLE dFile){
		GetMFTVector(dFile);



	}
	void GetMFTVector(HANDLE dFile){
		LARGE_INTEGER BeginLBA;
		BeginLBA = this->PartitionNTFSDBR.GetTheLBAAdr(this->PartitionNTFSDBR.MFTBeginCluster);
		if(SetFilePointerEx(dFile,BeginLBA,0,FILE_BEGIN)){
			 BYTE Buffer[1024];
			 DWORD NumToRead;
			 ReadFile(dFile,Buffer,1024,&NumToRead,0);
			 MFT MFTTmp(Buffer);
			 for(vector<CN>::iterator iter = MFTTmp.Data.DataLCN.begin();
				 iter != MFTTmp.Data.DataLCN.end(); iter++){
					 CN TmpLCN = (CN)*iter;
					 LARGE_INTEGER NowLBA = this->PartitionNTFSDBR.GetTheLBAAdr(TmpLCN.BeginCluster);
					 LARGE_INTEGER EndCluster;
					EndCluster.QuadPart = TmpLCN.BeginCluster.QuadPart+TmpLCN.Length.QuadPart;
					 LARGE_INTEGER EndLBA = this->PartitionNTFSDBR.GetTheLBAAdr(EndCluster);
					 while(NowLBA.QuadPart < EndLBA.QuadPart){	 
						  if(SetFilePointerEx(dFile,NowLBA,0,FILE_BEGIN)){
							   ReadFile(dFile,Buffer,1024,&NumToRead,0);
							  MFT  MFTTemp(Buffer);
		
							  this->NTFSMFTVector.push_back(MFTTemp);
							  NowLBA.QuadPart = NowLBA.QuadPart+1024;
						  }
						  else{}



					 }
                  
			 }
			 
			
		}
		else{}
	
	}
	
	void Recovery(FAT32Directory &FAT32Dir,HANDLE hDev){
		if(FAT32Dir.Total_Bytes == 0){
			return ;
		}
		DWORD BeginCluster = FAT32Dir.Begin_Cluster;
		DWORD TotalCluster;
		if(FAT32Dir.Total_Bytes%512) TotalCluster = FAT32Dir.Total_Bytes/512+1;
		else{
			TotalCluster = FAT32Dir.Total_Bytes/512;
		}
		if(this->PartitionFAT32DBR.GetTheFAT(BeginCluster,hDev) != 0x00000000){

			FAT32Dir.ClusterVector.push_back(0);
			
		}
		else{
			FAT32Dir.ClusterVector.push_back(BeginCluster);
		}

		for(int i = 0; i < (int)TotalCluster-1 ; i++){
			BeginCluster=this->PartitionFAT32DBR.GetTheNextNullCluster(BeginCluster+1,hDev);
			FAT32Dir.ClusterVector.push_back(BeginCluster);

		}


	}
	
};
class ExtendedPartition:public Partition{
public:
		ExtendedPartition(BYTE Boot_Indicator,BYTE System_ID,LARGE_INTEGER Relative_Sector_Begin,LARGE_INTEGER Total_Sector,wstring PartitionLabel):
Partition(  Boot_Indicator, System_ID,Relative_Sector_Begin,Total_Sector,PartitionLabel)
		{
	    }
		ExtendedPartition(){}

};

class MainPartition:public Partition{

public :
	list<ExtendedPartition> ExtendedVector;
	MainPartition(){

	}

	MainPartition(BYTE Boot_Indicator,BYTE System_ID,LARGE_INTEGER Relative_Sector_Begin,LARGE_INTEGER Total_Sector,wstring PartitionLabel):
		Partition( Boot_Indicator, System_ID, Relative_Sector_Begin, Total_Sector,PartitionLabel)
   {
			
	}
	
	void GetExtendedTable(HANDLE dFile){
	BYTE Buffer[512];
	MainPartition TmpTable = *this;

	int Count = 0;
	while(TmpTable.System_ID == 0x05||TmpTable.System_ID ==0x0f){
		if(TmpTable.GetTheDataFromPartiton(Buffer,dFile)){
			LARGE_INTEGER Relative_Sector_Begin,Total_Sector;
			Relative_Sector_Begin.QuadPart = 512*((long long)Buffer[454]+256*(long long)Buffer[455]+256*256*(long long)Buffer[456]+256*256*256*(long long)Buffer[457])+TmpTable.Relative_Sector_Begin.QuadPart;
			Total_Sector.QuadPart = 512*((long long)Buffer[458]+256*(long long)Buffer[459]+256*256*(long long)Buffer[456]+256*256*256*(long long)Buffer[460]);
			   
			   char newPartition[6];
			   _itoa_s(Count,newPartition,6);
			   size_t len = strlen(newPartition) + 1;
               size_t converted = 0;
               wchar_t *WStr;
                  WStr=(wchar_t*)malloc(len*sizeof(wchar_t));
               mbstowcs_s(&converted, WStr, len, newPartition, _TRUNCATE);
			   wstring tmp(WStr);
               ExtendedPartition  ExtendedTable(Buffer[446],Buffer[450],Relative_Sector_Begin,Total_Sector,this->PartitionLabel+L"/"+tmp);
					
				this->ExtendedVector.push_back(ExtendedTable);
				
				TmpTable.Boot_Indicator = Buffer[462];
				TmpTable.System_ID = Buffer[466];
			    
				TmpTable.Relative_Sector_Begin.QuadPart = 512*((long long)Buffer[470]+256*(long long)Buffer[471]+256*256*(long long)Buffer[472]+256*256*256*(long long)Buffer[473])+this->Relative_Sector_Begin.QuadPart;

				Count++;
				

			}
		
		}


	}


};

class Disk{
public:
	MainPartition MainTable[4];
	Disk(){}

    void getTable(HANDLE dFile){

	    BYTE Buffer[512];
        int Begin = 446;
		    
     DWORD dwRet;
    if( ReadFile(dFile,Buffer,512,&dwRet,0)!=0)
	   
		for(int i = 0; i < 4 ;i++ )
		{

			LARGE_INTEGER LBAAdr,LBATotal;
			LBAAdr.QuadPart = 512*((long long)Buffer[Begin+16*i+8]+256*(long long)Buffer[Begin+16*i+9]+256*256*(long long)Buffer[Begin+16*i+10]+256*256*256*(long long)Buffer[Begin+16*i+11]);
			LBATotal.QuadPart = 512*((long long)Buffer[Begin+16*i+12]+256*(long long)Buffer[Begin+16*i+13]+256*256*(long long)Buffer[Begin+16*i+14]+256*256*256*(long long)Buffer[Begin+16*i+15]);
			char newPartition[5];
			_itoa_s(i,newPartition,5);
			 size_t len = strlen(newPartition) + 1;
              size_t converted = 0;
              wchar_t *WStr;
              WStr=(wchar_t*)malloc(len*sizeof(wchar_t));
              mbstowcs_s(&converted, WStr, len, newPartition, _TRUNCATE);
			  wstring tmp(WStr);
			MainPartition MainPartitionTable(Buffer[Begin+16*i],Buffer[Begin+16*i+4],LBAAdr,LBATotal,tmp);
			


			if(!MainPartitionTable.isNull()){
				MainPartitionTable.GetExtendedTable(dFile);

			}
			
			MainTable[i] = MainPartitionTable;
		
		}


}
	void initDBR(int i,HANDLE hDev){
		if(MainTable[i].System_ID == 0x0f||MainTable[i].System_ID == 0x05)
			for(list<ExtendedPartition>::iterator iter = MainTable[i].ExtendedVector.begin(); 
				iter != MainTable[i].ExtendedVector.end();++iter){
					ExtendedPartition TmpPartition = (ExtendedPartition)*iter;
					TmpPartition.GetTheDBR(hDev);
					*iter = TmpPartition;
			}
		 
		 else{MainTable[i].GetTheDBR(hDev);
		 }

	}

	void GetTheDirectory(int i,HANDLE hDev){
		if(MainTable[i].System_ID == 0x0f||MainTable[i].System_ID == 0x05){
			for(list<ExtendedPartition>::iterator iter = MainTable[i].ExtendedVector.begin(); 
				iter != MainTable[i].ExtendedVector.end();++iter){
					ExtendedPartition TmpPartition = (ExtendedPartition)*iter;
					TmpPartition.GetTheDirectory(hDev);
					*iter = TmpPartition;
			}
		 }
		 else{ MainTable[i].GetTheDirectory(hDev);
		 }


	}


};

int _tmain(int argc, _TCHAR* argv[])
{

	Disk PhysicalDisk;
	 
	cout<<"----------------------------------------------------"<<endl;
	cout<<"---------Welcome to DiskReCovery System ------------"<<endl;
	cout<<"----------------------------------------------------"<<endl;
	cout<<""<<endl;
	cout<<""<<endl;
	cout<<"----------------System initialising-----------------"<<endl;

	LPCWSTR path = L"\\\\.\\PHYSICALDRIVE0";
     HANDLE hDev=CreateFile(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_FLAG_NO_BUFFERING,0);
	 if(INVALID_HANDLE_VALUE==hDev){
       cout<<"---------------Open Disk error-------------------- "<<endl;
	   cout<<"---------------Err Code "<<GetLastError()<<"--------------"<<endl;
		 system("pause");
		 return 0;
    }
	PhysicalDisk.getTable(hDev);
	 cout<<"----------------initialise succeed!-----------------"<<endl;
	 cout<<"-----------------------------------------------------"<<endl;
	 cout<<"----------------四个主分区情况--------------------"<<endl;
	   for(int i = 0 ; i < 4 ; i ++){
		  // wcout.imbue(locale(locale(),"",LC_CTYPE));
		   wcout<<L"Partition"<<PhysicalDisk.MainTable[i].PartitionLabel;
		   cout<<"  引导:"<<(int)PhysicalDisk.MainTable[i].Boot_Indicator
			   <<"  分区格式："<<(int)PhysicalDisk.MainTable[i].System_ID<<" 分区起始扇区:"
			   <<PhysicalDisk.MainTable[i].Relative_Sector_Begin.QuadPart<<" 分区扇区数:"
			   <<PhysicalDisk.MainTable[i].Total_Sector.QuadPart<<endl;
		  
		   PhysicalDisk.initDBR(i,hDev);		  
		   PhysicalDisk.GetTheDirectory(i,hDev);
		   
		  
	   }
	    cout<<"----------------拓展分区情况--------------------"<<endl;
		for(int i = 0 ; i < 4 ; i ++){
			if(PhysicalDisk.MainTable[i].System_ID == 0x0f||PhysicalDisk.MainTable[i].System_ID== 0x05){
				for(list<ExtendedPartition>::iterator iter = PhysicalDisk.MainTable[i].ExtendedVector.begin();
					iter !=PhysicalDisk.MainTable[i].ExtendedVector.end();iter++){
						ExtendedPartition tmp = (ExtendedPartition)*iter;
						//wcout.imbue(locale(locale(),"",LC_CTYPE));
						wcout<<L"Partition"<<tmp.PartitionLabel;
						cout<<"  引导:"<<(int)tmp.Boot_Indicator
			   <<"  分区格式："<<(int)tmp.System_ID<<" 分区起始扇区:"
			   <<tmp.Relative_Sector_Begin.QuadPart<<" 分区扇区数:"
			   <<tmp.Total_Sector.QuadPart<<endl;

				}


			}


		}
	   wstring code;
	   wstring nowLocation;
	   cout<<"--------选择进入上面的某个分区--------"<<endl;
	   wcin.imbue(std::locale("chs"));
	  getline(wcin,code);
	   
	    FAT32Directory  NowDir;
		
		FAT32Directory NowParentDir;
		Partition NowPartition;
		

		for(int i = 0 ; i < 4 ; i ++){
			if(PhysicalDisk.MainTable[i].System_ID == 0x0f||PhysicalDisk.MainTable[i].System_ID== 0x05){
				for(list<ExtendedPartition>::iterator iter = PhysicalDisk.MainTable[i].ExtendedVector.begin();
					iter !=PhysicalDisk.MainTable[i].ExtendedVector.end();iter++){
						ExtendedPartition tmp = (ExtendedPartition)*iter;
						if(tmp.PartitionLabel == code){
							tmp.showDirectory();
							NowDir = tmp.Root;
							NowPartition = tmp;
						}

				}


			}
			else if(PhysicalDisk.MainTable[i].PartitionLabel == code){
				PhysicalDisk.MainTable[i].Root.ShowDirectory();
				NowPartition = PhysicalDisk.MainTable[i];
				NowDir = PhysicalDisk.MainTable[i].Root;
				}
			
			else{}


		}
		getline(wcin,code);
	   while(true){
         if(code == L"quit")
			 break;
		 if(code.find(L"cd ",0) == 0)
		 {
			 wstring tmp = code.substr(3,code.size()-3);
			 if(tmp == L"..")
			 {
				 NowDir = NowParentDir;
				 NowDir.ShowDirectory();

			 }
			 else{
				 FAT32Directory tmpDir;
				 if(NowDir.GetSubDir(tmpDir,tmp)){
					 NowParentDir = NowDir;
					 NowDir = tmpDir;
					 tmpDir.ShowDirectory();
				 }
			 }

		 }
		 if(code.find(L"recovery ",0)==0){
			 wstring tmp = code.substr(9,code.size()-9);
			 FAT32Directory tmpFAT32Dir;
			 if(NowDir.GetSubDir(tmpFAT32Dir,tmp)){
				 NowPartition.ReBuildFile(tmpFAT32Dir,hDev);

			 }


		 }


		 getline(wcin,code);

	   }


	 CloseHandle(hDev);
	return 0;
}

