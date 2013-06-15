/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** IMG.cpp
** Class to create or edit GTA IMG archives with efficient algorithms.
** Works wth archive versions:
** • version 1 (GTA III, GTA VC, GTA III Mobile)
** • version 2 (GTA SA)
** does not support GTA IV yet.
**
** Version: 1.6
** Author: fastman92
** Site: fastman92-site.tk
** -------------------------------------------------------------------------*/

#include "stdafx.h"
#include "img.h"

using namespace std;

int fileChangeSize(FILE *fp, __int64 size)
{ 
	__int64 position = _ftelli64(fp);
	fseek(fp, 0, SEEK_SET);

    int filedes = _fileno(fp); 
	int result =_chsize_s(filedes, size);

	if(position < size)
		_fseeki64(fp, position, SEEK_SET);

	return result;
}

// Default constructor
IMG::IMG()
{
#ifdef IMG_DEBUG
	this -> debugFile = fopen("IMG_debug.log", "w");
	
	if(debugFile)
	{
		const char text[] = "IMG object initialized\n";
		fwrite(text, sizeof(text)-1, 1, debugFile);
	}
#endif

	this -> IMG_Handle = NULL;
	this -> DIR_Handle = NULL;
	this -> archiveVersion = IMG_VERSION_UNDEFINED;

	this -> GTAIV_encryption_key = "\x1a\xb5\x6f\xed\x7e\xc3\xff\x1\x22\x7b\x69\x15\x33\x97\x5d\xce"
		"\x47\xd7\x69\x65\x3f\xf7\x75\x42\x6a\x96\xcd\x6d\x53\x7\x56\x5d";

	this -> FilenamesSize = 0;
}

// Destructor
IMG::~IMG()
{
#ifdef IMG_DEBUG
	fclose(this -> debugFile);
#endif

	this -> CloseArchive();
}

// Opens IMG archive, assumes IMG archive to exist.
// Detects archive type automatically
bool IMG::OpenArchive(const char* path)
{
	this -> CloseArchive();

	if(this -> IMG_Handle = fopen(path, "rb+"))
	{		
		GetFullPathNameA(path, _countof(this -> IMG_fullPath), this -> IMG_fullPath, &this -> IMG_filename);

		unsigned long int version;

		size_t NumberOfReadBytes = fread(&version, sizeof(char), 4, this -> IMG_Handle);

		if(NumberOfReadBytes == 4 && version == '2REV')
		{
			this -> archiveVersion = IMG_VERSION_2;

			unsigned long int n_files;

			if(fread(&n_files, 4, 1, this -> IMG_Handle))
			{
				while(n_files > 0)
				{
					if(feof(this -> IMG_Handle))
					{
						this -> CloseArchive();
						return false;
					}
					this -> ListOfEntries.push_back(IMG_Entry(this));
					this -> ReadEntryOfVersion1And2Header(ListOfEntries.back(), this -> IMG_Handle);
					n_files--;
				}

				return true;
			}
		}
		else
		{
			const char* extension = strrchr(path, '.');
			
			bool hasExtension = false;

			char dirPath[_MAX_PATH];

			if(extension)
			{
				extension++;
				hasExtension = true;
				
				memcpy(dirPath, path, extension - path);
				strcpy(&dirPath[extension - path], "dir");
			}
					
			if(hasExtension && (this -> DIR_Handle = fopen(dirPath, "rb+")))
			{
				fseek(this -> DIR_Handle, 0, SEEK_END);					

				size_t DIR_size = ftell(this -> DIR_Handle);

				if(DIR_size % 32 == 0)
				{
					fseek(this -> DIR_Handle, 0, SEEK_SET);
					 
					this -> archiveVersion = IMG_VERSION_1;
															
					for(unsigned int i = 0; i < DIR_size / 32; i++)
					{
						this -> ListOfEntries.push_back(IMG_Entry(this));
						this -> ReadEntryOfVersion1And2Header(ListOfEntries.back(), this -> DIR_Handle);
					}
							
					return true;
				}
			}
			else if(NumberOfReadBytes == 4)
			{
				if(version == GTAIV_MAGIC_ID)
					this -> archiveVersion = IMG_VERSION_3_UNENCRYPTED;
				else
					this -> archiveVersion = IMG_VERSION_3_ENCRYPTED;

				IMG_version3_header Header;

				if(fread(&Header.Version, 1, sizeof(Header) - 4, this -> IMG_Handle) == sizeof(Header) - 4 && Header.Version == 3
					&& Header.SingleTableItemSize == 16)
				{					
					for(unsigned int FileNum = 0; FileNum < Header.NumberOfItems; FileNum++)
					{
						IMG_version3_tableItem item;
						if(fread(&item, sizeof(IMG_version3_tableItem), 1, this -> IMG_Handle) != 1)
							goto closeArchiveAndReturnFalse;

						this -> ListOfEntries.push_back(IMG_Entry(this));
						auto& CurrentEntry = this -> ListOfEntries.back();
						CurrentEntry.Position = item.Position;
						CurrentEntry.SizeInBytes = item.SizeInBytes;

						DWORD SizeInBlocks = GET_ALIGNED_SIZE_IN_BLOCKS(item.SizeInBytes, IMG_BLOCK_SIZE);

						if(SizeInBlocks > item.SizeInBlocks)
						{
							cout << hex << item.SizeInBytes;
							goto closeArchiveAndReturnFalse;
						}

						CurrentEntry.ResourceType = (ResourceTypes)item.ResourceType;
					}					

					this -> FilenamesSize = Header.TableOfItemsSize - (Header.NumberOfItems * Header.SingleTableItemSize);
					char* Filenames = new char[FilenamesSize];

					if(fread(Filenames, FilenamesSize, 1, this -> IMG_Handle) != 1)
					{
						delete Filenames;
						return false;
					}

					char* CurrentFilename = Filenames;
					for(auto& entry = this -> ListOfEntries.begin(); entry < this -> ListOfEntries.end(); entry++)
					{	
						unsigned int i;

						for(i = 0; CurrentFilename[i]; i++)
							entry -> Name[i] = CurrentFilename[i];

						entry -> Name[i] = NULL;

						entry -> UpdateFileNameHash();

						CurrentFilename += i + 1;
					}

					delete Filenames;

					return true;
				}				
			}
		}

		closeArchiveAndReturnFalse:
		this -> CloseArchive();
		return false;
	}
	
	return false;
}

// Creates .img archive
// Example: object.CreateArchive("new.img", IMG::IMG_version::VERSION_1);
bool IMG::CreateArchive(const char* path, IMG_version version)
{
	this -> CloseArchive();
	
	GetFullPathNameA(path, _countof(this -> IMG_fullPath), this -> IMG_fullPath, &this -> IMG_filename);

	this -> archiveVersion = version;

	switch(version)
	{
	case IMG_VERSION_1:	
		{
			const char* extension;
			
				if((extension = strrchr(path, '.'))++)
				{
					char dirPath[_MAX_PATH];
					memcpy(dirPath, path, extension - path);
					strcpy(&dirPath[extension - path], "dir");

					if((this -> DIR_Handle = fopen(dirPath, "wb+")) && (this -> IMG_Handle = fopen(path, "wb+")))
					{
						this -> WriteListOfEntries();
						return true;
					}
					else
						this -> CloseArchive();
				}				
			return false;
		}
	case IMG_VERSION_2:		
		{
			if(this -> IMG_Handle = fopen(path, "wb+"))
			{
				fwrite("VER2\x0\x0\x0\x0", 1, 8, this -> IMG_Handle);

				this -> WriteListOfEntries();
				return true;
			}
			return false;
		}
	case IMG_VERSION_3_UNENCRYPTED:
	case IMG_VERSION_3_ENCRYPTED:
		{
			if(this -> IMG_Handle = fopen(path, "wb+"))
			{
				this -> WriteListOfEntries();
				return true;
			}

			return false;
		}

	default:
		this -> archiveVersion = IMG_VERSION_UNDEFINED;
		return false;
	}
}

// Opens .img archive if exists or creates if not exists
// Example: object.OpenOrCreateArchive("new.img", IMG::IMG_version::VERSION_1);
bool IMG::OpenOrCreateArchive(const char* path, IMG_version version)
{
	if(PathFileExistsA(path))
		return this -> OpenArchive(path);
	else
		return this -> CreateArchive(path, version);
}

// Checks if archive is currently opened.
bool IMG::IsArchiveOpened()
{
	return this -> IMG_Handle != false;
}

// Closes archive
void IMG::CloseArchive()
{
	this -> ListOfEntries.clear();

	if(this -> IMG_Handle)
	{
		fclose(this -> IMG_Handle);
		this -> IMG_Handle = NULL;
	}
	if(this -> DIR_Handle)
	{
		fclose(this -> DIR_Handle);
		this -> DIR_Handle = NULL;
	}
}

// Rebuilds archive
bool IMG::RebuildArchive()
{
	char temporaryImgFilepath[_countof(this -> IMG_fullPath) + 16];
	strcpy(temporaryImgFilepath, this -> IMG_fullPath);
	strcat(temporaryImgFilepath, ".IMGclassTmpFile");

	FILE* newIMGhandle;
	
	if(newIMGhandle = fopen(temporaryImgFilepath, "wb+"))
	{
		if(this -> archiveVersion == IMG_VERSION_2)
			fwrite("VER2\x0\x0\x0\x0", 1, 8, newIMGhandle);

		fseek(newIMGhandle, GET_ALIGNED_SIZE(GetEndOfHeaderDataPastTheListOfFiles(), IMG_BLOCK_SIZE), SEEK_SET);

		for(auto entry = ListOfEntries.begin(); entry < ListOfEntries.end(); entry++)
		{
			size_t size = entry -> GetFilesizeAllignedToBlocks();
			char* loadedFile = new char[size];
			
			_fseeki64(this -> IMG_Handle, (unsigned __int64) entry ->Position * IMG_BLOCK_SIZE, SEEK_SET);
			fread(loadedFile, 1, size, this -> IMG_Handle);			

			entry -> Position =(DWORD) (_ftelli64(newIMGhandle) / IMG_BLOCK_SIZE);
			fwrite(loadedFile, 1, size, newIMGhandle);
	
			delete loadedFile;			
		}

		this -> WriteListOfEntries(newIMGhandle);

		fclose(newIMGhandle);
		fclose(this -> IMG_Handle);		

		remove(this -> IMG_fullPath);
		rename(temporaryImgFilepath, this -> IMG_fullPath);

		return (this -> IMG_Handle = this -> IMG_Handle = fopen(this -> IMG_fullPath, "rb+")) != 0;		
	}

		return false;
}

// Gets the size of .img archive in bytes
unsigned __int64 IMG::GetImgArchiveSize()
{
	if(this -> IMG_Handle)
	{
		auto currentPos = _ftelli64(this -> IMG_Handle);
		_fseeki64(this -> IMG_Handle, 0, SEEK_END);

		unsigned __int64 sizeOfArchive = _ftelli64(this -> IMG_Handle);

		_fseeki64(this -> IMG_Handle, currentPos, SEEK_SET);
		return sizeOfArchive;
	}
	else
		return UNDEFINED;
}

// Gets size of unused space in .img file
unsigned __int64 IMG::GetSizeOfUnusedSpace()
{
	auto sortedListByPosition = this -> ListOfEntries;

	this -> SortListOfEntriesByPositionAndSize(sortedListByPosition);

	unsigned __int64 UnusedSpace = 0;

	if(sortedListByPosition.size() >= 1)
	{	
		size_t headerSize = GET_ALIGNED_SIZE(GetEndOfHeaderDataPastTheListOfFiles(), IMG_BLOCK_SIZE);
		UnusedSpace = sortedListByPosition.front().Position * IMG_BLOCK_SIZE - headerSize;

		auto sortedEntry = sortedListByPosition.begin();

		while(sortedEntry < sortedListByPosition.end() - 1)
		{
			signed __int64 UnusedSpaceItem = ((sortedEntry + 1) -> Position - (sortedEntry -> Position + sortedEntry -> GetFilesizeAllignedToBlocks() / IMG_BLOCK_SIZE))*IMG_BLOCK_SIZE;

			if((signed int)UnusedSpaceItem > 0)
				UnusedSpace += UnusedSpaceItem;

			sortedEntry++;
		}	
		UnusedSpace += this -> GetImgArchiveSize() - (sortedListByPosition.back().Position * IMG_BLOCK_SIZE + sortedListByPosition.back().GetFilesizeAllignedToBlocks());		

		return UnusedSpace;
	}
	else
	{
		size_t headerSize = GET_ALIGNED_SIZE(GetEndOfHeaderDataPastTheListOfFiles(), IMG_BLOCK_SIZE);
		unsigned __int64 IMG_size = this -> GetImgArchiveSize();
		return IMG_size > headerSize ? IMG_size - headerSize : 0;
	}
}

// Retrieves the number of files inside of IMG archive
DWORD IMG::GetFileCount()
{
	return this -> ListOfEntries.size();
}

// Adds or replaces file if exists
void IMG::AddOrReplaceFile(const char* name, const void* ptr, size_t size)
{
	DWORD index = this -> GetFileIndexByName(name);

	if(index != UNDEFINED)
		this -> ReplaceSingleFile(index, ptr, size);
	else
		this -> AddFile(name, ptr, size);
}

// Adds file
void IMG::AddFile(const char* name, const void* ptr, size_t size)
{		
	vector<DWORD> RelatedFiles;

	this -> GetIDsOfAssociatedFiles(name, RelatedFiles);

	size_t allignedSize = GET_ALIGNED_SIZE(size, IMG_BLOCK_SIZE);

	size_t SizeOfAllReplacedFiles = allignedSize;

	for(auto i = RelatedFiles.begin(); i < RelatedFiles.end(); i++)
		SizeOfAllReplacedFiles += (this -> ListOfEntries.begin() + *i) -> GetFilesizeAllignedToBlocks();

	char* pAddedFiles = new char[SizeOfAllReplacedFiles];
	char* iAddedFiles = pAddedFiles;	

	unsigned __int64 PositionOfNewFiles = this -> FindFirstEmptySpaceForFile(SizeOfAllReplacedFiles, &RelatedFiles);
	unsigned __int64 iPositionOfNewFiles = PositionOfNewFiles;

	DWORD insertIndex;

	if(RelatedFiles.size() > 0)
	{		
		for(auto i = RelatedFiles.end() - 1; i >= RelatedFiles.begin(); i--)
		{
			if(strcmp(name, (this -> ListOfEntries.begin() + *i) -> Name) > 0)
			{			
				insertIndex = *i + 1;

				for(auto j = i+1; j < RelatedFiles.end(); j++)			
				 	(*j)++;				

				RelatedFiles.insert(i+1, insertIndex);

				goto InsertNewFile;
			}
		}		

		insertIndex = RelatedFiles.front();		

		for(auto i = RelatedFiles.begin(); i < RelatedFiles.end(); i++)
			(*i)++;

		RelatedFiles.insert(RelatedFiles.begin(), insertIndex);
	}
	else
	{
		insertIndex = this -> ListOfEntries.size();

		RelatedFiles.push_back(insertIndex);
	}

InsertNewFile:		
	this -> ListOfEntries.insert(this -> ListOfEntries.begin() + insertIndex, IMG_Entry(this));
	auto& currentEntry = this -> ListOfEntries.begin() + insertIndex;
	
	strncpy_s(currentEntry -> Name, name,
		(archiveVersion == IMG_VERSION_1 || archiveVersion == IMG_VERSION_2) ?
			_countof(((IMG_version2_tableItem*)0) -> Name) - 1 : _countof(currentEntry -> Name)-1);
			
	currentEntry ->UpdateFileNameHash();

	DWORD index = insertIndex;

	for(auto i = RelatedFiles.begin(); i < RelatedFiles.end(); i++)
	{
		auto entry = this -> ListOfEntries.begin() + *i;	

		if(*i == index)
		{
			memcpy(iAddedFiles, ptr, size);
			memset (iAddedFiles + size, NULL, allignedSize - size);
		
			iAddedFiles += allignedSize;

			if(archiveVersion == IMG_VERSION_3_UNENCRYPTED || archiveVersion == IMG_VERSION_3_UNENCRYPTED)
				entry -> SizeInBytes = size;
			else
			{
				if(entry -> SizeFirstPriority)
					entry -> SizeFirstPriority = allignedSize / IMG_BLOCK_SIZE;
				else
					entry -> SizeSecondPriority = allignedSize / IMG_BLOCK_SIZE;
			}

			entry -> Position = (DWORD)iPositionOfNewFiles / IMG_BLOCK_SIZE;
			iPositionOfNewFiles += allignedSize;
		}
		else
		{
			size_t processedFileSize = entry -> GetFilesize();
			size_t processedAlignedFileSize = GET_ALIGNED_SIZE(processedFileSize, IMG_BLOCK_SIZE);

			entry -> ReadEntireFile(iAddedFiles);

			memset (iAddedFiles + processedFileSize, NULL, processedAlignedFileSize - processedFileSize);

			entry -> Position = (DWORD)iPositionOfNewFiles / IMG_BLOCK_SIZE;
			iAddedFiles += processedAlignedFileSize;

			iPositionOfNewFiles += processedAlignedFileSize;
		}
	}

	_fseeki64(this -> IMG_Handle, PositionOfNewFiles, SEEK_SET);

	fwrite(pAddedFiles, 1, SizeOfAllReplacedFiles, this -> IMG_Handle);

	delete[] pAddedFiles;

	this -> WriteListOfEntries();
}

// Replaces file depending on index
void IMG::ReplaceSingleFile(DWORD index, const void* ptr, size_t size)
{
	vector<DWORD> RelatedFiles;

	this -> GetIDsOfAssociatedFiles(this -> ListOfEntries.begin() + index, RelatedFiles);

	size_t allignedSize = GET_ALIGNED_SIZE(size, IMG_BLOCK_SIZE);

	size_t SizeOfAllReplacedFiles = allignedSize;

	for(auto i = RelatedFiles.begin(); i < RelatedFiles.end(); i++)
	{
		if(*i != index)
			SizeOfAllReplacedFiles += (this -> ListOfEntries.begin() + *i) -> GetFilesizeAllignedToBlocks();
	}

	unsigned __int64 PositionOfNewFiles = this -> FindFirstEmptySpaceForFile(SizeOfAllReplacedFiles, &RelatedFiles);
	unsigned __int64 iPositionOfNewFiles = PositionOfNewFiles;

	char* pReplacedFiles = new char[SizeOfAllReplacedFiles];
	char* iReplacedFiles = pReplacedFiles;

	for(auto i = RelatedFiles.begin(); i < RelatedFiles.end(); i++)
	{
		auto entry = this -> ListOfEntries.begin() + *i;	

		if(*i == index)
		{
			memcpy(iReplacedFiles, ptr, size);
			memset (iReplacedFiles + size, NULL, allignedSize - size);
		
			iReplacedFiles += allignedSize;

			if(archiveVersion == IMG_VERSION_3_UNENCRYPTED || archiveVersion == IMG_VERSION_3_UNENCRYPTED)
				entry -> SizeInBytes = size;
			else
			{
				if(entry -> SizeFirstPriority)
					entry -> SizeFirstPriority = allignedSize / IMG_BLOCK_SIZE;
				else
					entry -> SizeSecondPriority = allignedSize / IMG_BLOCK_SIZE;
			}

			entry -> Position = (DWORD)iPositionOfNewFiles / IMG_BLOCK_SIZE;
			iPositionOfNewFiles += allignedSize;
		}
		else
		{
			size_t processedFileSize = entry -> GetFilesize();
			size_t processedAlignedFileSize = GET_ALIGNED_SIZE(processedFileSize, IMG_BLOCK_SIZE);

			entry -> ReadEntireFile(iReplacedFiles);

			memset (iReplacedFiles + processedFileSize, NULL, processedAlignedFileSize - processedFileSize);

			entry -> Position = (DWORD)iPositionOfNewFiles / IMG_BLOCK_SIZE;
			iReplacedFiles += processedAlignedFileSize;

			iPositionOfNewFiles += processedAlignedFileSize;
		}
	}

	_fseeki64(this -> IMG_Handle, PositionOfNewFiles, SEEK_SET);

	fwrite(pReplacedFiles, 1, SizeOfAllReplacedFiles, this -> IMG_Handle);

	delete[] pReplacedFiles;

	this -> WriteListOfEntries();
}

// Renames a file
void IMG::RenameFile(DWORD index, const char* NewName)
{
	auto& FileInfo = this -> ListOfEntries.begin() + index;

	if(archiveVersion == IMG_VERSION_3_UNENCRYPTED || archiveVersion == IMG_VERSION_3_UNENCRYPTED)
		FilenamesSize -= strlen(FileInfo -> Name) + 1;

	strncpy_s(FileInfo -> Name, NewName,
		(archiveVersion == IMG_VERSION_1 || archiveVersion == IMG_VERSION_2) ?
			_countof(((IMG_version2_tableItem*)0) -> Name) - 1 : _countof(FileInfo -> Name)-1);
	FileInfo -> UpdateFileNameHash();

	if(archiveVersion == IMG_VERSION_3_UNENCRYPTED || archiveVersion == IMG_VERSION_3_UNENCRYPTED)
		FilenamesSize += strlen(FileInfo -> Name) + 1;
	
	this -> WriteListOfEntries();
}

// Removes a file
void IMG::RemoveFile(DWORD index)
{
	if(index != UNDEFINED)
		this -> RemoveFile(this -> ListOfEntries.begin() + index);
}


// Removes files
void IMG::RemoveFiles(DWORD start, DWORD end)
{
	if(start != UNDEFINED && end != UNDEFINED)
		this -> RemoveFiles(this -> ListOfEntries.begin() + start, this -> ListOfEntries.begin() + end);
}

// Remove a file
std::vector<IMG::IMG_Entry>::iterator IMG::RemoveFile(std::vector<IMG::IMG_Entry>::const_iterator _Where)
{
	auto result = this -> ListOfEntries.erase(_Where);

	this -> WriteListOfEntries();

	return result;
}

// Removes files
std::vector<IMG::IMG_Entry>::iterator IMG::RemoveFiles(
	std::vector<IMG::IMG_Entry>::const_iterator _First_arg,
	std::vector<IMG::IMG_Entry>::const_iterator _Last_arg
	)
{
	auto result = this -> ListOfEntries.erase(_First_arg, _Last_arg);

	this -> WriteListOfEntries();

	return result;
}

// Gets index of file pointing to ListOfFiles
unsigned int IMG::GetFileIndexByName(const char* name)
{

	if(ListOfEntries.size())
	{
		unsigned long hash = GTASA_CRC32_fromUpCaseString(name);

		auto& entry = ListOfEntries.begin();

		do
		{
			if(entry -> NameHash == hash && !_stricmp(entry -> Name, name))
				return entry - ListOfEntries.begin();

			entry++;
		}
		while(entry < ListOfEntries.end());
	}

	return UNDEFINED;
}

// Checks if file with specified name exists and returns TRUE/FALSE
bool IMG::FileExists(const char* name)
{
	return GetFileIndexByName(name) != UNDEFINED;
}

// Gets filename for imported file, filename may be truncated if archive version is 1 or 2.
// Returns true if name is tr
errno_t IMG::GetFilenameForImportedFile(const char* lpFileName, char* lpFilePart, DWORD nBufferLength)
{
	DWORD NormalMaxSize = (archiveVersion == IMG_VERSION_1 || archiveVersion == IMG_VERSION_2) ?
				_countof(((IMG_version2_tableItem*)0) -> Name) : _countof(((IMG_Entry*)0) -> Name);

	char FullPath[_MAX_PATH];
	char* FileName;

	GetFullPathNameA(lpFileName, _countof(FullPath), FullPath, &FileName);

	size_t ThisSize = nBufferLength > NormalMaxSize ? NormalMaxSize : nBufferLength;	
	return strncpy_s(lpFilePart, ThisSize, FileName, _TRUNCATE);
}


// Access element
// Returns a reference to the file at position n in the vector.
std::vector<IMG::IMG_Entry>::reference IMG::at(std::vector<IMG::IMG_Entry>::size_type n)
{
	return this -> ListOfEntries.at(n);
}

// Access element
// Returns a reference to the file at position n in the vector.
std::vector<IMG::IMG_Entry>::const_reference IMG::at(std::vector<IMG::IMG_Entry>::size_type n) const
{
	return this -> ListOfEntries.at(n);
}

// Access file by name
// Returns a reference to the last element in the vector container.
std::vector<IMG::IMG_Entry>::reference IMG::GetFileRefByName(const char* name)
{
	return this -> at(this -> GetFileIndexByName(name));
}

// Return iterator to beginning
std::vector<IMG::IMG_Entry>::iterator IMG::begin()
{
	return this -> ListOfEntries.begin();
}

// Return iterator to beginning
std::vector<IMG::IMG_Entry>::const_iterator IMG::begin() const
{
	return this -> ListOfEntries.begin();
}

// Return iterator to end
std::vector<IMG::IMG_Entry>::iterator IMG::end()
{
	return this -> ListOfEntries.end();
}

// Return iterator to end
std::vector<IMG::IMG_Entry>::const_iterator IMG::end() const
{
	return this -> ListOfEntries.end();
}

// Return reverse iterator to reverse beginning
std::vector<IMG::IMG_Entry>::reverse_iterator IMG::rbegin()
{
	return this -> ListOfEntries.rbegin();
}

// Return reverse iterator to reverse beginning
std::vector<IMG::IMG_Entry>::const_reverse_iterator IMG::rbegin() const
{
	return this -> ListOfEntries.rbegin();
}

// Return reverse iterator to reverse end
std::vector<IMG::IMG_Entry>::reverse_iterator IMG::rend()
{
	return this -> ListOfEntries.rend();
}

// Return reverse iterator to reverse end
std::vector<IMG::IMG_Entry>::const_reverse_iterator IMG::rend() const
{
	return this -> ListOfEntries.rend();
}

// Access last element
// Returns a reference to the last element in the vector container.
std::vector<IMG::IMG_Entry>::reference IMG::back()
{
	return this -> ListOfEntries.back();
}

// Access last element
// Returns a reference to the last element in the vector container.
std::vector<IMG::IMG_Entry>::const_reference IMG::back() const
{
	return this -> ListOfEntries.back();
}

// Access first element
// Returns a reference to the first element in the vector container.
std::vector<IMG::IMG_Entry>::reference IMG::front()
{
	return this -> ListOfEntries.front();
}

// Access first element
// Returns a reference to the first element in the vector container.
std::vector<IMG::IMG_Entry>::const_reference IMG::front() const
{
	return this -> ListOfEntries.front();
}

// Get end of list entries and header
DWORD IMG::GetEndOfHeaderDataPastTheListOfFiles()
{
	switch (this -> archiveVersion)
	{
	case IMG_VERSION_1:
		return 32;	// watermark
	case IMG_VERSION_2:
		return
			8	// version, number of files
			+ this -> ListOfEntries.size() * 32
			+ 32;	// watermark

	case IMG_VERSION_3_UNENCRYPTED:
	case IMG_VERSION_3_ENCRYPTED:		
		return
			20	// header
			+ this -> ListOfEntries.size() * 16
			+ FilenamesSize
			+ 32;	// watermark
	default:
		return UNDEFINED;
	}
}

// Gets IPL filename and num, returns true on success and false on failure.
bool IMG::GetIPLfilenameAndNum(const char* fullFilename, char* IPL_name, DWORD* IPL_num)
{
	size_t nameLen = strlen(fullFilename);

	const char* DotAndExtension = fullFilename + nameLen - 4;

	if(nameLen >= 5 && !stricmp(DotAndExtension, ".ipl"))
	{		
		const char* StartOfNum = DotAndExtension - 1;

		while(isdigit(*StartOfNum))
			StartOfNum--;

		StartOfNum++;

		if(StartOfNum < DotAndExtension)
		{
			if(IPL_num)
				*IPL_num = atoi(StartOfNum);

			if(IPL_name)
			{
				memcpy(IPL_name, fullFilename, StartOfNum - fullFilename);
				IPL_name[StartOfNum - fullFilename] = NULL;
			}

			return true;
		}
	}

	if(IPL_num)
		*IPL_num = UNDEFINED;

	return false;
}

// Gets ID of this file OR IDs of all .ipl files with this name
void IMG::GetIDsOfAssociatedFiles(std::vector<IMG::IMG_Entry>::iterator currentEntry, std::vector<DWORD>& list)
{
	list.clear();

	char Cur_IPL_Name[256];
	DWORD Cur_IPL_Num;
	GetIPLfilenameAndNum(currentEntry -> Name, Cur_IPL_Name, &Cur_IPL_Num);

	if(GetIPLfilenameAndNum(currentEntry -> Name, Cur_IPL_Name, &Cur_IPL_Num))
	{
		auto listEnd = this -> ListOfEntries.end();

		for(auto entry = this -> ListOfEntries.begin(); entry < listEnd; entry++)
		{
			char tested_IPL_Name[256];
			DWORD tested_IPL_Num;

			if(GetIPLfilenameAndNum(entry -> Name, tested_IPL_Name, &tested_IPL_Num))
			{
				if(!stricmp(Cur_IPL_Name, tested_IPL_Name))
					list.push_back(entry - this -> ListOfEntries.begin());
			}
		}
	}
	else
		list.push_back(currentEntry - this -> ListOfEntries.begin());
}

// Gets ID of this file OR IDs of all .ipl files with this name
void IMG::GetIDsOfAssociatedFiles(const char* SearchedName, std::vector<DWORD>& list)
{
	list.clear();

	char Cur_IPL_Name[256];
	DWORD Cur_IPL_Num;
	GetIPLfilenameAndNum(SearchedName, Cur_IPL_Name, &Cur_IPL_Num);

	if(GetIPLfilenameAndNum(SearchedName, Cur_IPL_Name, &Cur_IPL_Num))
	{
		auto listEnd = this -> ListOfEntries.end();

		for(auto entry = this -> ListOfEntries.begin(); entry < listEnd; entry++)
		{
			char tested_IPL_Name[256];
			DWORD tested_IPL_Num;

			if(GetIPLfilenameAndNum(entry -> Name, tested_IPL_Name, &tested_IPL_Num))
			{
				if(!stricmp(Cur_IPL_Name, tested_IPL_Name))
					list.push_back(entry - this -> ListOfEntries.begin());
			}
		}
	}
}

// Loads files identified by DWORD indexes into continous aligned memory
// Returned is size of all files loaded from list
// Remember to free memory when it's no longer neccessary! Use delete!
// None of addresses may be NULL
void IMG::LoadFilesInCountinousAlignedMemory(DWORD* FileIndexes, DWORD cFileIndexes, void** retAddress, size_t* retSize, FILE* imgHandle)
{
	if(!imgHandle)
		imgHandle = this -> IMG_Handle;

	size_t SizeOfFiles = 0;

	for(DWORD i = 0; i < cFileIndexes; i++)
		SizeOfFiles += (this -> ListOfEntries.begin() + FileIndexes[i]) -> GetFilesizeAllignedToBlocks();

	char* pFiles = new char[SizeOfFiles];
	char* iFiles = pFiles;

	for(DWORD i = 0; i < cFileIndexes; i++)
	{
		auto& entry = this -> ListOfEntries.begin() + FileIndexes[i];

		_fseeki64(imgHandle, (unsigned __int64) entry -> Position * IMG_BLOCK_SIZE, SEEK_SET);
		size_t Filesize = entry -> GetFilesizeAllignedToBlocks();
		fread(iFiles, 1, Filesize, imgHandle);

		iFiles += Filesize;
	}

	*retAddress = pFiles;
	*retSize = SizeOfFiles;
}

// Moves files located in position before list of files to suitable position
void IMG::MoveFilesLocatedBeforeListOfFilesToSuitablePosition(FILE* imgHandle)
{
	DWORD treshold = this -> GetEndOfHeaderDataPastTheListOfFiles();	
		
	auto listEnd = ListOfEntries.end();
	
	for (auto entry = ListOfEntries.begin(); entry < listEnd; entry++)
	{
		if(entry -> Position * IMG_BLOCK_SIZE < treshold)
		{
			vector<DWORD> RelatedFiles;

			this -> GetIDsOfAssociatedFiles(entry, RelatedFiles);

			char* MovedFiles;
			size_t MovedFilesSize;

			LoadFilesInCountinousAlignedMemory(
				&*RelatedFiles.begin(),
				RelatedFiles.size(),
				(void**)&MovedFiles,
				&MovedFilesSize,
				imgHandle);

			DWORD PositionOfFiles = (DWORD) (this -> FindFirstEmptySpaceForFile(MovedFilesSize, &RelatedFiles)) / IMG_BLOCK_SIZE;

			DWORD CurrentPosition = PositionOfFiles;
			for(auto m = RelatedFiles.begin(); m < RelatedFiles.end(); m++)
			{
				(this -> ListOfEntries.begin() + *m) -> Position = CurrentPosition;
				CurrentPosition += (this -> ListOfEntries.begin() + *m) -> GetFilesizeAllignedToBlocks() / IMG_BLOCK_SIZE;
			}

			_fseeki64(imgHandle, PositionOfFiles * IMG_BLOCK_SIZE, SEEK_SET);
			fwrite(MovedFiles, 1, MovedFilesSize, imgHandle);
			delete[] MovedFiles;
		}
	}
}

// Returns pattern of IMG validity
void IMG::GetPatternToCheckIfPositionIsValid(char* str)
{
	const char a[] = "\x5f\x5b\x6e\x70\x6a\x5f\x6d\x39\x33\x22\x4c\x51\x4c\x26\x4a\x33\x34\x2a\x6e\x78\x6e\x81\x82\x30\x42\x40\x49\x14\x15\x16\x17\x18";

	char c;

	DWORD d = 0;

	do
	{
		c = a[d] - (char)d + 7;
		str[d] = c;

		d++;
	}
	while(d < 32);
}

// Tests if file position is valid
void IMG::TestIfPositionIsValid(FILE* imgHandle)
{
	char b[32];
	GetPatternToCheckIfPositionIsValid(b);

	fwrite(b, 1, 32, imgHandle);
}

// reads version 1 or 2 header
void IMG::ReadEntryOfVersion1And2Header(IMG_Entry& entry, FILE* file)
{
	fread(&entry.Position, 4, 1, file);
	fread(&entry.SizeSecondPriority, 2, 1, file);
	fread(&entry.SizeFirstPriority, 2, 1, file);
	fread(&entry.Name, 24, 1, file);
	entry.UpdateFileNameHash();
}

// writes version 1 or 2 header
void IMG::WriteListOfEntriesVersion1And2(FILE* file)
{
	IMG_version2_tableItem* pBuffer = new IMG_version2_tableItem [this -> ListOfEntries.size()];

	IMG_version2_tableItem* iBuffer = pBuffer;

	for_each(ListOfEntries.begin(), ListOfEntries.end(),
		[&] (IMG::IMG_Entry entry)
	{
		iBuffer -> Position = entry.Position;
		iBuffer -> SizeSecondPriority = entry.SizeSecondPriority;
		iBuffer -> SizeFirstPriority = entry.SizeFirstPriority;

		strncpy_s(iBuffer -> Name, entry.Name, _countof(((IMG_version2_tableItem*)0) -> Name) - 1);
		iBuffer++;
	}
	);

	fwrite(pBuffer, sizeof(IMG_version2_tableItem), this -> ListOfEntries.size(), file);
	delete pBuffer;
}

// Updates list of entries in IMG archive/DIR file
void IMG::WriteListOfEntries(FILE* imgHandle)
{
	if(this -> ListOfEntries.size())
	{
		vector<IMG_Entry>::iterator FileWithHighestPosition = max_element(ListOfEntries.begin(), ListOfEntries.end(),
		[](IMG_Entry a, IMG_Entry b) { return a.Position < b.Position; });

		fileChangeSize(this -> IMG_Handle, (unsigned __int64) FileWithHighestPosition -> Position * IMG_BLOCK_SIZE + FileWithHighestPosition -> GetFilesizeAllignedToBlocks());
	}
	else
		fileChangeSize(this -> IMG_Handle, GET_ALIGNED_SIZE(GetEndOfHeaderDataPastTheListOfFiles(), IMG_BLOCK_SIZE));

	if(!imgHandle)
		imgHandle = this -> IMG_Handle;

	this -> MoveFilesLocatedBeforeListOfFilesToSuitablePosition(imgHandle);

	if(archiveVersion == IMG_VERSION_1)
	{
		fseek(imgHandle, 0, SEEK_SET);
		this -> TestIfPositionIsValid(imgHandle);

		fileChangeSize(this -> DIR_Handle, 0);
		_fseeki64(this -> DIR_Handle, 0, SEEK_SET);		

		this -> WriteListOfEntriesVersion1And2(this -> DIR_Handle);		
	}
	else if(archiveVersion == IMG_VERSION_2)
	{
		fseek(imgHandle, 4, SEEK_SET);		// number of files
			
		DWORD numberOfFiles = this -> ListOfEntries.size();
		fwrite(&numberOfFiles, 4, 1, imgHandle);

		this -> WriteListOfEntriesVersion1And2(imgHandle);

		this -> TestIfPositionIsValid(imgHandle);
	}
	else if(archiveVersion == IMG_VERSION_3_UNENCRYPTED || archiveVersion == IMG_VERSION_3_UNENCRYPTED)
	{
		fseek(imgHandle, 0, SEEK_SET);
	
		size_t BeginningSizeAlligned = GET_ALIGNED_SIZE(GetEndOfHeaderDataPastTheListOfFiles(), IMG_BLOCK_SIZE);
		char* pBeginning = new char[BeginningSizeAlligned];

		char* iBeginning = pBeginning;
		this -> WriteVersion3HeaderToMemory((IMG_version3_header*)iBeginning);

		iBeginning += sizeof(IMG_version3_header);

		IMG_version3_tableItem* ItemDescription = (IMG_version3_tableItem*) iBeginning;

		for(auto& entry = this -> ListOfEntries.begin(); entry < this -> ListOfEntries.end(); entry++, ItemDescription++)
		{
			ItemDescription -> SizeInBytes = entry -> SizeInBytes;
			ItemDescription -> ResourceType = entry -> ResourceType;
			ItemDescription -> Position = entry -> Position;
			ItemDescription -> SizeInBlocks = (WORD)GET_ALIGNED_SIZE_IN_BLOCKS(entry -> SizeInBytes, IMG_BLOCK_SIZE);
			ItemDescription -> Unknown = NULL;
		}

		char* Filenames = (char*) ItemDescription;
		for(auto& entry = this -> ListOfEntries.begin(); entry < this -> ListOfEntries.end(); entry++)
		{
			unsigned int i;

			for(i = 0; entry -> Name[i]; i++)
				*(Filenames++) = entry -> Name[i];

			*(Filenames++) = NULL;
		}

		GetPatternToCheckIfPositionIsValid(Filenames);

		fwrite(pBeginning, BeginningSizeAlligned, 1, imgHandle);

		delete pBeginning;
	}
}

// Writes file content alligned to IMG_BLOCK_SIZE
// Returned value is number of blocks written ( realSize / IMG_BLOCK_SIZE )
size_t IMG::WriteAllignedFileToIMGblocks(const void* ptr, size_t size)
{
	fwrite(ptr, 1, size, this -> IMG_Handle);
	size_t allignedSize = GET_ALIGNED_SIZE(size, IMG_BLOCK_SIZE);

	size_t sizeToAllign = allignedSize - size;
	char contentToAllign[2048];
	memset (contentToAllign, NULL, sizeToAllign);

	fwrite(contentToAllign, 1, sizeToAllign, this -> IMG_Handle);

	return allignedSize / IMG_BLOCK_SIZE;
}

// Writes IMG archive version 3 header to memory.
void IMG::WriteVersion3HeaderToMemory(IMG_version3_header* header)
{
	header -> MagicID = GTAIV_MAGIC_ID;
	header -> Version = 3;
	header -> NumberOfItems = this -> ListOfEntries.size();
	header -> TableOfItemsSize = header -> NumberOfItems * sizeof(IMG_version3_tableItem) + this -> FilenamesSize;
	header -> SingleTableItemSize = sizeof(IMG_version3_tableItem);
	header -> Unknown = NULL;
}

// Sorts an array by
// 1. Position
// 2. Size
void IMG::SortListOfEntriesByPositionAndSize(std::vector<IMG_Entry>& list)
{
	sort(list.begin(), list.end(),
		[](IMG_Entry& a, IMG_Entry& b) -> bool
	{
		size_t a_filesize = a.GetFilesizeAllignedToBlocks();
		size_t b_filesize = b.GetFilesizeAllignedToBlocks();

			return
				a.Position < b.Position ? true :
				a.Position > b.Position ? false :

				a_filesize < b_filesize ? true :
				b_filesize > b_filesize;
	}
	);
}

// Finds a first free space for target file
unsigned __int64 IMG::FindFirstEmptySpaceForFile(size_t filesize, std::vector<DWORD>* overwrittenFileIndexes)
{
	filesize = GET_ALIGNED_SIZE_IN_BLOCKS(filesize, IMG_BLOCK_SIZE);

	auto sortedListByPosition = this -> ListOfEntries;

	if(overwrittenFileIndexes)
	{
		auto sortedOverwrittenFileIndexes = *overwrittenFileIndexes;

		sort(sortedOverwrittenFileIndexes.begin(), sortedOverwrittenFileIndexes.end(), 
			[](DWORD a, DWORD b) { return a > b; } );

		for(auto id = sortedOverwrittenFileIndexes.begin(); id < sortedOverwrittenFileIndexes.end(); id++)
			sortedListByPosition.erase(sortedListByPosition.begin() + *id);
	}

	this -> SortListOfEntriesByPositionAndSize(sortedListByPosition);

	unsigned __int64 BeginningOfFileSpace = GET_ALIGNED_SIZE(GetEndOfHeaderDataPastTheListOfFiles(), IMG_BLOCK_SIZE);

	if(sortedListByPosition.size() >= 1 && ((signed __int64)sortedListByPosition.front().Position
			- BeginningOfFileSpace / IMG_BLOCK_SIZE
			<= filesize || BeginningOfFileSpace / IMG_BLOCK_SIZE > sortedListByPosition.front().Position))
	{		
		auto sortedEntry = sortedListByPosition.begin();
		
		while(sortedEntry < sortedListByPosition.end() - 1)
		{
			__int64 FreeSpace = (sortedEntry + 1) -> Position - (sortedEntry -> Position + sortedEntry -> GetFilesizeAllignedToBlocks() / IMG_BLOCK_SIZE);

			if(FreeSpace >= filesize)				
				return (unsigned __int64)sortedEntry -> Position * IMG_BLOCK_SIZE + sortedEntry -> GetFilesizeAllignedToBlocks();

			sortedEntry++;
		}		

		return (unsigned __int64)sortedListByPosition.back().Position * IMG_BLOCK_SIZE + sortedListByPosition.back().GetFilesizeAllignedToBlocks();
	}
	else
		return BeginningOfFileSpace;
}

// Outputs list of entries specified as argument
void IMG::DebugListOfEntries(std::vector<IMG_Entry>& list)
{
	ios::fmtflags f;

	f = cout.flags(); // store flags

	for(auto entry = list.begin(); entry < list.end(); entry++)
	{
		cout << endl
			<< dec << (entry - list.begin())
			<< " Offset: " << hex << "0x" << (unsigned __int64)entry -> Position * IMG_BLOCK_SIZE;

		if(archiveVersion == IMG_VERSION_1 || archiveVersion == IMG_VERSION_2)
			cout << " 2nd size: " << entry -> SizeSecondPriority * IMG_BLOCK_SIZE << " 1st size: " << entry -> SizeFirstPriority * IMG_BLOCK_SIZE;
		else
			cout << " Size in bytes: " << entry -> SizeInBytes << " ResourceType: " << (DWORD)entry -> ResourceType;

		cout << " Name: " << entry -> Name;
	}

	cout.flags( f );
}

// Outputs ListOfEntries
void IMG::DebugListOfEntries()
{
	this -> DebugListOfEntries(this -> ListOfEntries);
}

// Constructor
IMG::IMG_Entry::IMG_Entry(IMG * IMG_Instance)
{
	this -> IMG_Instance = IMG_Instance;

	this -> Position = 0;
	this -> SizeSecondPriority = 0;
	this -> SizeFirstPriority = 0;
	this -> SizeInBytes = 0;
	this -> ResourceType = RESOURCE_UNDEFINED;
	this -> Name[0] = NULL;

	this -> curPos = 0;
}

void IMG::IMG_Entry::UpdateFileNameHash()
{
	this -> NameHash = GTASA_CRC32_fromUpCaseString(this -> Name);
}

// Returns pointer to file name.
const char* IMG::IMG_Entry::GetFilename()
{
	return this -> Name;
}

// Reads whole file, doesn't affect current file position (curPos)
// Returns number of bytes read.
size_t IMG::IMG_Entry::ReadEntireFile(void* ptr)
{
	_fseeki64(this -> IMG_Instance -> IMG_Handle, (unsigned __int64) this ->Position * IMG_BLOCK_SIZE, SEEK_SET);

	size_t Filesize = this -> GetFilesize();
	fread(ptr, 1, Filesize, this -> IMG_Instance -> IMG_Handle);
	return Filesize;
}

// Reads to memory
size_t IMG::IMG_Entry::Read(void* Ptr, size_t Size)
{
	if(this -> curPos < this -> GetFilesize())
	{
		size_t SizeToRead = this -> curPos + Size > this -> GetFilesize() ? this -> GetFilesize() - this -> curPos : Size;

		this -> Seek(0, SEEK_CUR);

		size_t n_readBytes = fread(Ptr, 1, SizeToRead, this -> IMG_Instance -> IMG_Handle);
		this -> curPos += n_readBytes;

		return n_readBytes;
	}
	else 
		return false;
}

// Reads one byte
char IMG::IMG_Entry::ReadC()
{
	if(this -> curPos < this -> GetFilesize())
	{
		this -> Seek(0, SEEK_CUR);
		char result;
		this -> curPos += fread(&result, 1, 1, this -> IMG_Instance -> IMG_Handle);
		
		return result;
	}
	else
		return -1;
}

// Get file size in bytes
size_t IMG::IMG_Entry::GetFilesize()
{
	if(this ->IMG_Instance -> archiveVersion == IMG_VERSION_3_ENCRYPTED
		|| this ->IMG_Instance ->  archiveVersion == IMG_VERSION_3_UNENCRYPTED)
		return this -> SizeInBytes;
	else
	{
		size_t realSize = this -> SizeFirstPriority ? this -> SizeFirstPriority : this -> SizeSecondPriority;
		realSize *= IMG_BLOCK_SIZE;
		return realSize;
	}
}

// Get file size in blocks
size_t IMG::IMG_Entry::GetFilesizeAllignedToBlocks()
{
	if(this ->IMG_Instance -> archiveVersion == IMG_VERSION_3_ENCRYPTED
		|| this ->IMG_Instance ->  archiveVersion == IMG_VERSION_3_UNENCRYPTED)
		return GET_ALIGNED_SIZE(this -> SizeInBytes, IMG_BLOCK_SIZE);
	else
	{
		size_t realSize = this -> SizeFirstPriority ? this -> SizeFirstPriority : this -> SizeSecondPriority;
		realSize *= IMG_BLOCK_SIZE;
		return realSize;
	}
}

// Check End-of-File indicator
bool IMG::IMG_Entry::isEOF()
{
	return this -> curPos >= this -> GetFilesize();
}

// Seek to certain position
bool IMG::IMG_Entry::Seek(unsigned int offset, int origin)
{
	bool result = false;

	if(origin == SEEK_SET)
	{
		if(offset <= this -> GetFilesize())
		{
			this -> curPos = offset;
			result = true;
		}
	}
	else if(origin == SEEK_CUR)
	{
		if(this -> curPos + offset <= this -> GetFilesize())
		{
			this -> curPos += offset;
			result = true;
		}
	}
	else if(origin == SEEK_END)
	{
		if(this -> curPos + this -> GetFilesize() + offset <= this -> GetFilesize())
		{
			this -> curPos += this -> GetFilesize() + offset;
			result = true;
		}
	}

	return _fseeki64(this -> IMG_Instance -> IMG_Handle, (unsigned __int64)this -> Position * IMG_BLOCK_SIZE + this -> curPos, SEEK_SET) != 0;
}

// Returns current position
unsigned int IMG::IMG_Entry::Tell()
{
	return this -> curPos;
}