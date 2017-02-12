struct read_entire_file_result
{
	char *Contents;
	uint32_t FileSize;
};

internal read_entire_file_result
ReadEntireFile(char *Filename)
{
	read_entire_file_result Result = {};

	HANDLE File = CreateFile(Filename,
							 GENERIC_READ,
							 FILE_SHARE_READ,
							 NULL,
							 OPEN_EXISTING,
							 FILE_ATTRIBUTE_NORMAL,
							 NULL);

	Assert(File != INVALID_HANDLE_VALUE);
	if(File == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: failed to create handle to source file (time_raw.txt)\n");
		exit(1);
	}

	LARGE_INTEGER FileSize_;
	if(!GetFileSizeEx(File, &FileSize_))
	{
		Assert(false);
		fprintf(stderr, "Error: failed to read file size\n");
		exit(1);
	}

	Assert(FileSize_.QuadPart <= 0xFFFFFFFF);
	if(FileSize_.QuadPart > 0xFFFFFFFF)
	{
		fprintf(stderr, "Error: file too big.\n");
		exit(1);
	}

	Result.FileSize = (uint32_t)FileSize_.QuadPart;
	Result.Contents = (char *)VirtualAlloc(NULL,
										   Result.FileSize,
										   MEM_COMMIT | MEM_RESERVE,
										   PAGE_EXECUTE_READWRITE);

	DWORD BytesRead;
	if(!ReadFile(File,
				 Result.Contents,
				 Result.FileSize,
				 &BytesRead,
				 NULL))
	{
		Assert(false);
		fprintf(stderr, "Error: failed to read source file (time_raw.txt).\n");
		exit(1);
	}

	Assert(BytesRead == Result.FileSize);
	if(BytesRead != Result.FileSize)
	{
		fprintf(stderr, "Error: read incorrect number of bytes: file size is %d but read %d.\n", 
				Result.FileSize, BytesRead);
		exit(1);
	}

	CloseHandle(File);

	return Result;
}
