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

#include <windows.h>
#include <vector>
#include <algorithm>
#include <io.h>
#include <iostream>
#include "Shlwapi.h"
#pragma comment( lib, "shlwapi.lib")

#include "GTASA_CRC32.h"
// #include "Rijndael.h"

#define GET_ALIGNED_SIZE(requested_size, block_size) (requested_size % block_size == 0 ? requested_size : ((requested_size / block_size)+1)*block_size)
#define GET_ALIGNED_SIZE_IN_BLOCKS(requested_size, block_size) (requested_size % block_size == 0 ? requested_size / block_size : (requested_size / block_size) + 1)

// #define IMG_DEBUG

class IMG
{
public:	
	const static unsigned int UNDEFINED = 0xFFFFFFFF;

	enum ResourceTypes : DWORD
	{
		RESOURCE_UNDEFINED = 0x00,
		RESOURCE_GENERIC = 0x01,
		RESOURCE_TEXTURE_ARCHIVE = 0x08,
		RESOURCE_BOUNDS = 0x20,
		RESOURCE_MODEL_FILE = 0x6E,
		RESOURCE_XPFL = 0x24
	};

	struct AssociatedFilesMatch
	{
		AssociatedFilesMatch(DWORD FileID, DWORD IPL_num)
		{
			this -> FileID = FileID;
			this -> IPL_num = IPL_num;
		}

		DWORD FileID;
		DWORD IPL_num;
	};

	class IMG_Entry
	{
		friend IMG;

	private:
		char Name[256];				// NULL terminated
		DWORD NameHash;

		// In IMG archive
		DWORD Position;				// In blocks
		WORD SizeSecondPriority;	// In blocks
		WORD SizeFirstPriority;		// In blocks

		// Version 3
		DWORD SizeInBytes;
		// DWORD SizeInBlocks;
		ResourceTypes ResourceType;

		// Updates hash of file name 
		void IMG::IMG_Entry::UpdateFileNameHash();

		// Get file size aligned blocks
		size_t IMG::IMG_Entry::GetFilesizeAllignedToBlocks();

	public:	
		IMG* IMG_Instance;

		// For reading/writing
		unsigned int curPos;

		/////// Methods

		// Constructor
		IMG::IMG_Entry::IMG_Entry(IMG * IMG_Instance);

		// Returns pointer to file name.
		const char* IMG::IMG_Entry::GetFilename();

		// Reads whole file, doesn't affect current file position (curPos)
		// Returns number of bytes read.
		size_t IMG::IMG_Entry::ReadEntireFile(void* ptr);

		// Get file size in bytes
		size_t IMG::IMG_Entry::GetFilesize();

		//////// File operations that may affect curPos
		// Reads to memory
		size_t IMG::IMG_Entry::Read(void* Ptr, size_t Size);

		// Reads one byte
		char IMG::IMG_Entry::ReadC();

		// Write data
		size_t IMG::IMG_Entry::Write(void* ptr, size_t size);

		// Check End-of-File indicator
		bool IMG::IMG_Entry::isEOF();

		// Seek to certain position
		bool IMG::IMG_Entry::Seek(unsigned int offset, int origin);

		// Returns current position
		unsigned int IMG::IMG_Entry::Tell();
	};

private:
#ifdef IMG_DEBUG
	FILE* debugFile;
#endif

	// File entries
	std::vector <IMG_Entry> ListOfEntries;

	FILE* IMG_Handle;
	FILE* DIR_Handle;

	char IMG_fullPath [MAX_PATH];
	char* IMG_filename;

	// Size of all filenames, only usable, when IMG version 3
	DWORD FilenamesSize;

	// Get end of list entries and header
	DWORD IMG::GetEndOfHeaderDataPastTheListOfFiles();

	// Gets IPL filename and num, returns true on success and false on failure.
	static bool IMG::GetIPLfilenameAndNum(const char* fullFilename, char* IPL_name, DWORD* IPL_num);

	// Gets ID of this file OR IDs of all .ipl files with this name
	void IMG::GetIDsOfAssociatedFiles(std::vector<IMG::IMG_Entry>::iterator currentEntry, std::vector<DWORD>& list);

	// Gets ID of this file OR IDs of all .ipl files with this name
	void IMG::GetIDsOfAssociatedFiles(const char* SearchedName, std::vector<DWORD>& list);

	// Loads files identified by DWORD indexes into continous aligned memory
	// Returned is size of all files loaded from list
	// Remember to free memory when it's no longer neccessary! Use delete!
	// None of addresses may be NULL
	void IMG::LoadFilesInCountinousAlignedMemory(DWORD* FileIndexes, DWORD cFileIndexes, void** retAddress, size_t* retSize, FILE* imgHandle = NULL);

	// Moves files located in position before list of files to suitable position
	void IMG::MoveFilesLocatedBeforeListOfFilesToSuitablePosition(FILE* imgHandle);

	// Returns pattern of IMG validity
	void IMG::GetPatternToCheckIfPositionIsValid(char* str);

	// Tests if file position is valid
	void IMG::TestIfPositionIsValid(FILE* imgHandle);

	// reads version 1 or 2 header
	void IMG::ReadEntryOfVersion1And2Header(IMG_Entry& entry, FILE* file);

	// writes version 1 or 2 header
	void IMG::WriteListOfEntriesVersion1And2(FILE* file);
	
	// Updates list of entries in IMG archive/DIR file
	void IMG::WriteListOfEntries(FILE* imgHandle = NULL);

	// Writes file content alligned to IMG_BLOCK_SIZE
	// Returned value is number of blocks written ( realSize / IMG_BLOCK_SIZE )
	size_t IMG::WriteAllignedFileToIMGblocks(const void* ptr, size_t size);

	// Sorts an array by
	// 1. Position
	// 2. Size
	void IMG::SortListOfEntriesByPositionAndSize(std::vector<IMG_Entry>& list);

#ifdef IMG_DEBUG
	public:
#endif
	// Finds a first free space for target file
	unsigned __int64 IMG::FindFirstEmptySpaceForFile(size_t filesize, std::vector<DWORD>* overwrittenFileIndexes = NULL);

public:
	const static unsigned int IMG_BLOCK_SIZE = 2048;		// 2 KB
	const static unsigned int MAX_FILESIZE = 0xFFFF * IMG_BLOCK_SIZE;		// 128 MB - IMG_BLOCK_SIZE bytes
	const static unsigned int GTAIV_MAGIC_ID = 0xA94E2A52;

	char* GTAIV_encryption_key;

	#pragma pack(push, 1)
	struct IMG_version2_tableItem		// size: 20 bytes
	{
		DWORD Position;				// In blocks
		WORD SizeSecondPriority;	// In blocks
		WORD SizeFirstPriority;		// In blocks
		char Name[24];				// NULL terminated
	};
	#pragma pack(push, 1)

	#pragma pack(push, 1)
	struct IMG_version3_header		// size: 20 bytes
	{
		DWORD MagicID;
		DWORD Version;				// always should be 3
		DWORD NumberOfItems;
		DWORD TableOfItemsSize;
		WORD SingleTableItemSize;	// should be 0x10 (16)
		WORD Unknown;
	};
	#pragma pack(push, 1)

	struct IMG_version3_tableItem		// size: 20 bytes
	{
		DWORD SizeInBytes;
		ResourceTypes ResourceType;
		DWORD Position;		// in blocks
		WORD SizeInBlocks;
		WORD Unknown;
	};

	enum IMG_version
	{
		IMG_VERSION_UNDEFINED,
		IMG_VERSION_1,	// GTA III, GTA VC, GTA III Mobile
		IMG_VERSION_2,	// GTA SA
		IMG_VERSION_3_UNENCRYPTED,	// GTA IV
		IMG_VERSION_3_ENCRYPTED		// GTA IV
	};

	IMG_version archiveVersion;

private:
	// Writes IMG archive version 3 header to memory.
	void IMG::WriteVersion3HeaderToMemory(IMG_version3_header* header);

public:

	// Default constructor
	IMG::IMG();

	// Destructor
	IMG::~IMG();

	// Copy constructor
	// IMG::IMG(const IMG &cSource);

	// Opens IMG archive, assumes IMG archive to exist.
	// Detects archive type automatically
	bool IMG::OpenArchive(const char* path);

	// Creates .img archive
	// Example: object.CreateArchive("new.img", IMG::IMG_version::VERSION_1);
	bool IMG::CreateArchive(const char* path, IMG_version version);

	// Opens .img archive if exists or creates if not exists
	// Example: object.OpenOrCreateArchive("new.img", IMG::IMG_version::VERSION_1);
	bool IMG::OpenOrCreateArchive(const char* path, IMG_version version);

	// Checks if archive is currently opened.
	bool IMG::IsArchiveOpened();

	// Closes archive
	void IMG::CloseArchive();

	// Rebuilds archive
	bool IMG::RebuildArchive();

	// Gets the size of .img archive in bytes
	unsigned __int64 IMG::GetImgArchiveSize();

	// Gets size of unused space in .img file
	unsigned __int64 IMG::GetSizeOfUnusedSpace();
	
	// Retrieves the number of files inside of IMG archive
	DWORD IMG::GetFileCount();

	// Adds or replaces file if exists
	void IMG::AddOrReplaceFile(const char* name, const void* ptr, size_t size);

	// Adds file
	void IMG::AddFile(const char* name, const void* ptr, size_t size);

	// Replaces file depending on index
	void IMG::ReplaceSingleFile(DWORD index, const void* ptr, size_t size);

	// Renames a file
	void IMG::RenameFile(DWORD index, const char* NewName);

	// Removes a file
	void IMG::RemoveFile(DWORD index);

	// Removes files
	void IMG::RemoveFiles(DWORD start, DWORD end);

	// Remove a file
	std::vector<IMG::IMG_Entry>::iterator IMG::RemoveFile(std::vector<IMG::IMG_Entry>::const_iterator _Where);

	// Removes files
	std::vector<IMG::IMG_Entry>::iterator IMG::RemoveFiles(
		std::vector<IMG::IMG_Entry>::const_iterator _First_arg,
		std::vector<IMG::IMG_Entry>::const_iterator _Last_arg
		);

	// Gets index of file pointing to ListOfFiles
	unsigned int IMG::GetFileIndexByName(const char* name);

	// Checks if file with specified name exists and returns TRUE/FALSE
	bool IMG::FileExists(const char* name);

	// Gets filename for imported file, filename may be truncated if archive version is 1 or 2.
	errno_t IMG::GetFilenameForImportedFile(const char* lpFileName, char* lpFilePart, DWORD nBufferLength);

	// Access element
	// Returns a reference to the file at position n in the vector.
	std::vector<IMG::IMG_Entry>::reference IMG::at(std::vector<IMG::IMG_Entry>::size_type n);

	// Access element
	// Returns a reference to the file at position n in the vector.
	std::vector<IMG::IMG_Entry>::const_reference IMG::at(std::vector<IMG::IMG_Entry>::size_type n) const;

	// Access file by name
	// Returns a reference to the last element in the vector container.
	std::vector<IMG::IMG_Entry>::reference IMG::GetFileRefByName(const char* name);
	
	// Return iterator to beginning
	std::vector<IMG::IMG_Entry>::iterator IMG::begin();

	// Return iterator to beginning
	std::vector<IMG::IMG_Entry>::const_iterator IMG::begin() const;

	// Return iterator to end
	std::vector<IMG::IMG_Entry>::iterator IMG::end();

	// Return iterator to end
	std::vector<IMG::IMG_Entry>::const_iterator IMG::end() const;

	// Return reverse iterator to reverse beginning
	std::vector<IMG::IMG_Entry>::reverse_iterator IMG::rbegin();

	// Return reverse iterator to reverse beginning
	std::vector<IMG::IMG_Entry>::const_reverse_iterator IMG::rbegin() const;

	// Return reverse iterator to reverse end
	std::vector<IMG::IMG_Entry>::reverse_iterator IMG::rend();

	// Return reverse iterator to reverse end
	std::vector<IMG::IMG_Entry>::const_reverse_iterator IMG::rend() const;

	// Access last element
	// Returns a reference to the last element in the vector container.
	std::vector<IMG::IMG_Entry>::reference IMG::back();

	// Access last element
	// Returns a reference to the last element in the vector container.
	std::vector<IMG::IMG_Entry>::const_reference IMG::back() const;

	// Access first element
	// Returns a reference to the first element in the vector container.
	std::vector<IMG::IMG_Entry>::reference IMG::front();

	// Access first element
	// Returns a reference to the first element in the vector container.
	std::vector<IMG::IMG_Entry>::const_reference IMG::front() const;

	// Outputs list of entries specified as argument
	void IMG::DebugListOfEntries(std::vector<IMG_Entry>& list);

	// Outputs ListOfEntries
	void IMG::DebugListOfEntries();
};