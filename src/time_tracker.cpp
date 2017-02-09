#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

#include "util.h"

// TODO(hacksoi): Consider a better method of determining the size of the
// output file (possibly a pass over the input file to see how many entries
// have been processed)
#define TableToEntryRatio 4
#define MaxInputFileSize (0xFFFFFFFF / TableToEntryRatio)

#define MAX_CATEGORY_NAME_LENGTH 32
#define TIME_ENTRY_FORMAT_LENGTH (8 + 32)

enum time_category
{
	TIME_CATEGORY_SLEEP,
	TIME_CATEGORY_PLAY,
	TIME_CATEGORY_SCHOOL,
	TIME_CATEGORY_EDUCATION,
	TIME_CATEGORY_PROGRAMMING,
	TIME_CATEGORY_AWAKE,

	TIME_CATEGORY_NULL
};

union time
{
	struct
	{
		uint32_t Hour;
		uint32_t Minute;
	};

	struct
	{
		uint32_t Hours;
		uint32_t Minutes;
	};
};

inline uint32_t
ConvertToMinutes(time A)
{
	uint32_t Result = ((A.Hours * 60) + A.Minutes);

	return Result;
}

inline bool32
operator>(time A, time B)
{
	bool32 Result = true;

	if(A.Hour < B.Hour)
	{
		Result = false;
	}
	else if((A.Hour == B.Hour) && 
			(A.Minute < B.Minute))
	{
		Result = false;
	}

	return Result;
}

inline bool32
operator<(time A, time B)
{
	bool32 Result = (B > A);

	return Result;
}

// NOTE(hacksoi): _always_ assumes A is meant to be after B (in terms of time)
// and will add 24 hours to A if needed
inline time
operator-(time A, time B)
{
	if(A < B)
	{
		A.Hour += 24;
	}

	uint32_t ATotalMinutes = ConvertToMinutes(A);
	uint32_t BTotalMinutes = ConvertToMinutes(B);

	uint32_t ResultTotalMinutes = (ATotalMinutes - BTotalMinutes);

	time Result;
	Result.Hours = (ResultTotalMinutes / 60);
	Result.Minutes = (ResultTotalMinutes % 60);

	return Result;
}

// NOTE(hacksoi): this is really just for adding _elapsed_ times
inline time
operator+(time A, time B)
{
	uint32_t ATotalMinutes = ConvertToMinutes(A);
	uint32_t BTotalMinutes = ConvertToMinutes(B);

	uint32_t ResultTotalMinutes = (ATotalMinutes + BTotalMinutes);

	time Result;
	Result.Hours = (ResultTotalMinutes / 60);
	Result.Minutes = (ResultTotalMinutes % 60);

	return Result;
}

inline time &
operator+=(time &A, time B)
{
	time Result = A + B;

	return Result;
}

struct time_block
{
	time StartTime;
	time_category Category;
};

struct table_entry
{
	time TargetTime;
	time ActualTime;

	bool32 IsUnderGood;

	char *Name;
};

union table_info
{
	struct
	{
		table_entry Sleep;
		table_entry Play;
		table_entry School;
		table_entry Education;
		table_entry Programming;
		table_entry Awake;
	};

	table_entry Entries[6];
};

struct process_day_result
{
	uint32_t InputCharsProcessed;
	uint32_t OutputCharsPushed;
};

struct process_time_block_result
{
	uint32_t CharsPushed;
	time_block TimeBlock;
};

union time_block_info
{
	struct
	{
		char HourFirstDigit;
		char HourSecondDigit;

		char Colon;

		char MinuteFirstDigit;
		char MinuteSecondDigit;

		char FirstSpace;
		char Dash;
		char SecondSpace;

		char Category[MAX_CATEGORY_NAME_LENGTH];
	};

	char Elements[TIME_BLOCK_INFO_SIZE];
}


internal table_info
DefaultTableInfo()
{
	table_info Result = {};

	// Sleep
	{
		Result.Sleep.TargetTime.Hours = ;
		Result.Sleep.TargetTime.Minutes = ;

		Result.Sleep.IsUnderGood = false;
		Result.Sleep.Name = "Sleep";
	}

	// Play
	{
		Result.Play.TargetTime.Hours = ;
		Result.Play.TargetTime.Minutes = ;

		Result.Play.IsUnderGood = false;
		Result.Play.Name = "Play";
	}

	// School
	{
		Result.School.TargetTime.Hours = ;
		Result.School.TargetTime.Minutes = ;

		Result.School.IsUnderGood = false;
		Result.School.Name = "School";
	}

	// Education
	{
		Result.Education.TargetTime.Hours = ;
		Result.Education.TargetTime.Minutes = ;

		Result.Education.IsUnderGood = false;
		Result.Education.Name = "Education";
	}

	// Programming
	{
		Result.Programming.TargetTime.Hours = ;
		Result.Programming.TargetTime.Minutes = ;

		Result.Programming.IsUnderGood = false;
		Result.Programming.Name = "Programming";
	}

	// Awake
	{
		Result.Awake.TargetTime.Hours = ;
		Result.Awake.TargetTime.Minutes = ;

		Result.Awake.IsUnderGood = false;
		Result.Awake.Name = "Awake";
	}

	return Result;
}

internal bool32
Equals(char *A, char *B)
{
	bool32 Result = true;

	while((*A != '\0') &&
		  (*B != '\0'))
	{
		if(*A != *B)
		{
			Result = false;
			break;
		}
	}

	return Result;
}

internal uint32_t
Copy(char *A, char *B)
{
	uint32_t Result = 0;

	while()
	{
	}

	return Result;
}

inline uint32_t
ConvertToUInt32(char FirstDigitChar, char SecondDigitChar)
{
	uint32_t FirstDigit = (FirstDigitChar - '0');
	uint32_t SecondDigit = (SecondDigitChar - '0');

	uint32_t Result = ((FirstDigit * 10) + SecondDigit);

	return Result;
}

internal uint32_t
PushLine(char *Input, char *Output)
{
	uint32_t Result = 0;

	while((*Output) != '\n')
	{
		*Output = *Input++;
		++Result;
	}

	return Result;
}

internal process_time_block_result
ProcessTimeBlock(char *Input, char *Output)
{
	process_time_block_result Result = {};

	time_block_info TimeBlockInfo = {};
	{
		TimeBlockInfo.HourFirstDigit = *Input++;
		TimeBlockInfo.HourSecondDigit = *Input++;

		TimeBlockInfo.Colon = *Input++;

		TimeBlockInfo.MinuteFirstDigit = *Input++;
		TimeBlockInfo.MinuteSecondDigit = *Input++;

		TimeBlockInfo.FirstSpace = *Input++;
		TimeBlockInfo.Dash = *Input++;
		TimeBlockInfo.SecondSpace = *Input++;

		uint32_t CategoryLength = 0;
		while(*Input != ':')
		{
			TimeBlockInfo.Category[CategoryLength++] = *Input++;
		}
	}

	// copy TimeBlockInfo and rest of line to Output
	{
		for(uint32_t TimeBlockInfoElementsIndex = 0;
			TimeBlockInfo.Elements[TimeBlockInfoElementsIndex] != 0;
		   )
		{
			*Output++ = TimeBlockInfo.Elements[TimeBlockInfoElementsIndex];
			++Result.CharsPushed;
		}

		while(*Output != '\n')
		{
			*Output++ = *Input++;
			++Result.CharsPushed;
		}
	}

	{
		Result.TimeBlock.StartTime.Hour = ConvertToUInt32(TimeBlockInfo.HourFirstDigit, 
														  TimeBlockInfo.HourSecondDigit);
		Result.TimeBlock.StartTime.Minute = ConvertToUInt32(TimeBlockInfo.MinuteFirstDigit, 
															TimeBlockInfo.MinuteSecondDigit);

		if(Equals("Sleep", TimeBlockInfo.Category))
		{
			Result.TimeBlock.Category = TIME_CATEGORY_SLEEP;
		}
		else if(Equals("Play", TimeBlockInfo.Category))
		{
			Result.TimeBlock.Category = TIME_CATEGORY_PLAY;
		}
		else if(Equals("School", TimeBlockInfo.Category))
		{
			Result.TimeBlock.Category = TIME_CATEGORY_SCHOOL;
		}
		else if(Equals("Education", TimeBlockInfo.Category))
		{
			Result.TimeBlock.Category = TIME_CATEGORY_EDUCATION;
		}
		else if(Equals("Programming", TimeBlockInfo.Category))
		{
			Result.TimeBlock.Category = TIME_CATEGORY_PROGRAMMING;
		}
		else if(Equals("Awake", TimeBlockInfo.Category))
		{
			Result.TimeBlock.Category = TIME_CATEGORY_AWAKE;
		}
		else
		{
			fprintf("Error: unknown category: %s\n", TimeBlockInfo.Category);
			exit(1);
		}
	}

	return Result;
}

internal process_day_result
ProcessDay(char *Input, char *Output)
{
	uint32_t InputCharsProcessed = 0;
	uint32_t OutputCharsPushed = 0;

	for(int Line = 0;
		Line < 3;
		++Line)
	{
		uint32_t CharsPushed = PushLine(&Input[InputCharsProcessed], 
										&Output[OutputCharsPushed]);

		InputCharsProcessed += CharsPushed;
		OutputCharsPushed += CharsPushed;
	}

	table_info TableInfo = DefaultTableInfo();
	{
		time_block PrevTimeBlock = {};
		PrevTimeBlock.Category = TIME_CATEGORY_NULL;
		for(;;)
		{
			process_time_block_result ProcessTimeBlockResult = ProcessTimeBlock(&Input[InputCharsProcessed], 
																				&Output[OutputCharsPushed]);
			InputCharsProcessed += ProcessTimeBlockResult.CharsPushed;
			OutputCharsPushed += ProcessTimeBlockResult.CharsPushed;

			time_block CurTimeBlock = ProcessTimeBlockResult.TimeBlock;

			if(Input[InputCharsProcessed] == '\0')
			{
				// end of day
				break;
			}
			else if(PrevTimeBlock.Category != TIME_CATEGORY_NULL)
			{
				time_block TimeElapsed = (CurTimeBlock.StartTime - PrevTimeBlock.StartTime);
				TableInfo.Entries[PrevTimeBlock.Category].ActualTime += TimeElapsed;
				TableInfo.Entries[TIME_CATEGORY_AWAKE].ActualTime += TimeElapsed;
			}

			PrevTimeBlock = CurTimeBlock;
		}
	}

	// insert table
	{
#define TABLE_WIDTH 36
#define TABLE_HEIGHT 37
#define NUMBER_OF_CATEGORIES 6

		for(uint32_t Col = 0;
			Col < TABLE_WIDTH;
			++Col)
		{
			Output[OutputCharsPushed++] = '_';
		}
		Output[OutputCharsPushed++] = '\r';
		Output[OutputCharsPushed++] = '\n';

		Output[OutputCharsPushed++] = '|';
		for(uint32_t Col = 0;
			Col < (TABLE_WIDTH - 2);
			++Col)
		{
			Output[OutputCharsPushed++] = ' ';
		}
		Output[OutputCharsPushed++] = '|';
		Output[OutputCharsPushed++] = '\r';
		Output[OutputCharsPushed++] = '\n';

		Output[OutputCharsPushed++] = '|';
		Output[OutputCharsPushed++] = ' ';
		uint32_t CharsCopied = Copy(TimeBlockInfo.Sleep.Name, &Output[OutputCharsPushed]);
		OutputCharsPushed += CharsCopied;
	}

	process_day_result Result;
	Result.InputCharsProcessed = InputCharsProcessed;
	Result.OutputCharsPushed = OutputCharsPushed;

	return Result;
}

int
main(int argc, char *argv[])
{
	table_info TableInfo;

	HANDLE InputFile = CreateFile("time_raw.txt",
								  GENERIC_READ,
								  FILE_SHARE_READ,
								  NULL,
								  OPEN_EXISTING,
								  FILE_ATTRIBUTE_NORMAL,
								  NULL);

	if(InputFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: failed to create handle to source file (time_raw.txt)\n");
		exit(1);
	}

	LARGE_INTEGER InputFileSize_;
	if(!GetFileSizeEx(InputFile, &InputFileSize_))
	{
		fprintf(stderr, "Error: failed to read file size\n");
		exit(1);
	}

	if(InputFileSize_.QuadPart > MaxInputFileSize)
	{
		fprintf(stderr, "Error: file too big.\n");
		exit(1);
	}

	uint32_t InputFileSize = (uint32_t)InputFileSize_.QuadPart;
	char *InputContents = (char *)VirtualAlloc(NULL,
											   InputFileSize,
											   MEM_COMMIT | MEM_RESERVE,
											   PAGE_EXECUTE_READWRITE);

	DWORD BytesRead;
	if(!ReadFile(InputFile,
				 InputFileContents,
				 InputFileSize,
				 &BytesRead,
				 NULL))
	{
		fprintf(stderr, "Error: failed to read source file (time_raw.txt).\n");
		exit(1);
	}

	if(BytesRead != InputFileSize)
	{
		fprintf(stderr, "Error: read incorrect number of bytes: file size is %d but read %d.\n", 
				InputFileSize, BytesRead);
		exit(1);
	}

	HANDLE OutputFile = CreateFile("time_processed.txt",
								   GENERIC_WRITE,
								   0,
								   NULL,
								   CREATE_ALWAYS,
								   FILE_ATTRIBUTE_NORMAL,
								   NULL);

	if(OutputFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: failed to create handle to processed file (time_processed.txt)\n");
		exit(1);
	}

	uint32_t OutputFileSize = (InputFileSize * TableToEntryRatio);
	char *OutputContents = (char *)VirtualAlloc(NULL,
												OutputFileSize,
												MEM_COMMIT | MEM_RESERVE,
												PAGE_EXECUTE_READWRITE);
	uint32_t OutputContentsSize = 0;

#define ENTRY_PROCESSED_FLAG '#'
#define ENTRY_UNPROCESSED_FLAG '@'

	Assert((InputFileContents[0] == ENTRY_PROCESSED_FLAG) ||
		   (InputFileContents[0] == ENTRY_UNPROCESSED_FLAG));
	if((InputFileContents[0] != ENTRY_PROCESSED_FLAG) &&
	   (InputFileContents[0] != ENTRY_UNPROCESSED_FLAG))
	{
		fprintf(stderr, "Error: input file has incorrect format (first character must be '#' or '@')\n");
		exit(1);
	}

	uint32_t InputFileCharIndex = 0;
	while(InputFileCharIndex < InputFileSize)
	{
		char c = InputFileContents[InputFileCharIndex];
		Assert((c == ENTRY_PROCESSED_FLAG) ||
			   (c == ENTRY_UNPROCESSED_FLAG));

		process_day_result EntryResult = ProcessDay(&InputFileContents[InputFileCharIndex], 
													&OutputContents[OutputContentsSize]);
		InputFileCharIndex += EntryResult.InputCharsProcessed;
		OutputContentsSize += EntryResult.OutputCharsPushed;
	}

	DWORD BytesWritten;
	if(!WriteFile(OutputFile,
				  OutputFileContents,
				  OutputFileSize,
				  &BytesWritten,
				  NULL))
	{
		fprintf(stderr, "Error: failed to write processed file (time_processed.txt)\n");
		exit(1);
	}

	if(BytesWritten != OutputFileSize)
	{
		fprintf(stderr, "Error: wrote incorrect number of bytes: expected %d but wrote %d.\n", 
				OutputFileSize, BytesWritten);
		exit(1);
	}

	return 0;
}
