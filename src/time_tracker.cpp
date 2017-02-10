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

	NumTimeCategories,
	TIME_CATEGORY_NULL
};

struct time
{
	uint32_t Hour;
	uint32_t Minute;
};

struct time_elapse
{
	uint32_t Hours;
	uint32_t Minutes;

	bool IsNegative;
};

inline uint32_t
ConvertToMinutes(time A)
{
	uint32_t Result = ((A.Hour * 60) + A.Minute);

	return Result;
}

inline uint32_t
ConvertToMinutes(time_elapse A)
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
operator>(time_elapse A, time_elapse B)
{
	bool32 Result = true;

	if(A.Hours < B.Hours)
	{
		Result = false;
	}
	else if((A.Hours == B.Hours) && 
			(A.Minutes < B.Minutes))
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

inline bool32
operator<(time_elapse A, time_elapse B)
{
	bool32 Result = (B > A);

	return Result;
}

// NOTE(hacksoi): _always_ assumes A is meant to be after B (in terms of time)
// and will add 24 hours to A if needed
inline time_elapse
operator-(time A, time B)
{
	if(A < B)
	{
		A.Hour += 24;
	}

	uint32_t ATotalMinutes = ConvertToMinutes(A);
	uint32_t BTotalMinutes = ConvertToMinutes(B);

	uint32_t ResultTotalMinutes = (ATotalMinutes - BTotalMinutes);

	time_elapse Result;
	Result.Hours = (ResultTotalMinutes / 60);
	Result.Minutes = (ResultTotalMinutes % 60);

	return Result;
}

inline time_elapse
operator-(time_elapse A, time_elapse B)
{
	time_elapse Result = {};
	if(A < B)
	{
		Result.IsNegative = true;

		time_elapse Temp = A;
		A = B;
		B = Temp;
	}

	uint32_t ATotalMinutes = ConvertToMinutes(A);
	uint32_t BTotalMinutes = ConvertToMinutes(B);

	uint32_t ResultTotalMinutes = (ATotalMinutes - BTotalMinutes);

	Result.Hours = (ResultTotalMinutes / 60);
	Result.Minutes = (ResultTotalMinutes % 60);

	return Result;
}

inline time_elapse
operator+(time_elapse A, time_elapse B)
{
	uint32_t ATotalMinutes = ConvertToMinutes(A);
	uint32_t BTotalMinutes = ConvertToMinutes(B);

	uint32_t ResultTotalMinutes = (ATotalMinutes + BTotalMinutes);

	time_elapse Result;
	Result.Hours = (ResultTotalMinutes / 60);
	Result.Minutes = (ResultTotalMinutes % 60);

	return Result;
}

inline time_elapse &
operator+=(time_elapse &A, time_elapse B)
{
	time_elapse Result = A + B;

	return Result;
}

struct number2_char
{
	char FirstDigit;
	char SecondDigit;
};

inline number2_char
ConvertToChar(uint32_t A)
{
	Assert(A < 99);

	uint32_t SecondDigit = (A % 10);
	uint32_t FirstDigit = (A / 10);

	number2Char Result;
	number2Char.FirstDigit = (char)('0' + FirstDigit);
	number2Char.SecondDigit = (char)('0' + SecondDigit);

	return Result;
}

union raw_entry_data
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

struct entry
{
	time StartTime;
	time_category Category;
};

struct table
{
	union
	{
		struct
		{
			time_elapse TargetTime;
			time_elapse RealityTime;
		};

		time_elapse TimeElapses[2];
	};

	bool32 IsUnderGood;

	char *Name;
};

struct process_day_result
{
	uint32_t InputCharsProcessed;
	uint32_t OutputCharsPushed;
};

struct process_entry_result
{
	uint32_t CharsPushed;
	entry Entry;
};

internal void
FillWithDefaultValues(table *Tables)
{
	// Sleep
	{
		Tables[TIME_CATEGORY_SLEEP].TargetTime.Hours = ;
		Tables[TIME_CATEGORY_SLEEP].TargetTime.Minutes = ;

		Tables[TIME_CATEGORY_SLEEP].ActualTime.Hours = 0;
		Tables[TIME_CATEGORY_SLEEP].ActualTime.Minutes = 0;

		Tables[TIME_CATEGORY_SLEEP].IsUnderGood = false;
		Tables[TIME_CATEGORY_SLEEP].Name = "Sleep";
	}

	// Play
	{
		Tables[TIME_CATEGORY_PLAY].TargetTime.Hours = ;
		Tables[TIME_CATEGORY_PLAY].TargetTime.Minutes = ;

		Tables[TIME_CATEGORY_PLAY].ActualTime.Hours = 0;
		Tables[TIME_CATEGORY_PLAY].ActualTime.Minutes = 0;

		Tables[TIME_CATEGORY_PLAY].IsUnderGood = false;
		Tables[TIME_CATEGORY_PLAY].Name = "Play";
	}

	// School
	{
		Tables[TIME_CATEGORY_SCHOOL].TargetTime.Hours = ;
		Tables[TIME_CATEGORY_SCHOOL].TargetTime.Minutes = ;

		Tables[TIME_CATEGORY_SCHOOL].ActualTime.Hours = 0;
		Tables[TIME_CATEGORY_SCHOOL].ActualTime.Minutes = 0;

		Tables[TIME_CATEGORY_SCHOOL].IsUnderGood = false;
		Tables[TIME_CATEGORY_SCHOOL].Name = "School";
	}

	// Education
	{
		Tables[TIME_CATEGORY_EDUCATION].TargetTime.Hours = ;
		Tables[TIME_CATEGORY_EDUCATION].TargetTime.Minutes = ;

		Tables[TIME_CATEGORY_EDUCATION].ActualTime.Hours = 0;
		Tables[TIME_CATEGORY_EDUCATION].ActualTime.Minutes = 0;

		Tables[TIME_CATEGORY_EDUCATION].IsUnderGood = false;
		Tables[TIME_CATEGORY_EDUCATION].Name = "Education";
	}

	// Programming
	{
		Tables[TIME_CATEGORY_PROGRAMMING].TargetTime.Hours = ;
		Tables[TIME_CATEGORY_PROGRAMMING].TargetTime.Minutes = ;

		Tables[TIME_CATEGORY_PROGRAMMING].ActualTime.Hours = 0;
		Tables[TIME_CATEGORY_PROGRAMMING].ActualTime.Minutes = 0;

		Tables[TIME_CATEGORY_PROGRAMMING].IsUnderGood = false;
		Tables[TIME_CATEGORY_PROGRAMMING].Name = "Programming";
	}

	// Awake
	{
		Tables[TIME_CATEGORY_AWAKE].TargetTime.Hours = ;
		Tables[TIME_CATEGORY_AWAKE].TargetTime.Minutes = ;

		Tables[TIME_CATEGORY_AWAKE].ActualTime.Hours = 0;
		Tables[TIME_CATEGORY_AWAKE].ActualTime.Minutes = 0;

		Tables[TIME_CATEGORY_AWAKE].IsUnderGood = false;
		Tables[TIME_CATEGORY_AWAKE].Name = "Awake";
	}
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

	while(*A != '\0')
	{
		*B++ = *A++;
		++Result;
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

inline uint32_t
Ceil(float A)
{
	uint32_t Result;

	bool32 HasFractional = (A != (uint32_t)A);
	if(HasFractional)
	{
		Result = (uint32_t)(A + 1.0f)
	}
	else
	{
		Result = (uint32_t)A;
	}

	return Result;
}

inline uint32_t
Length(char *A)
{
	uint32_t Result = 0;

	while(*A++ != '\0')
	{
		++Result;
	}

	return Result;
}

internal process_entry_result
ProcessEntry(char *Input, char *Output)
{
	process_entry_result Result = {};

	raw_entry_data RawEntryData = {};
	{
		RawEntryData.HourFirstDigit = *Input++;
		RawEntryData.HourSecondDigit = *Input++;

		RawEntryData.Colon = *Input++;

		RawEntryData.MinuteFirstDigit = *Input++;
		RawEntryData.MinuteSecondDigit = *Input++;

		RawEntryData.FirstSpace = *Input++;
		RawEntryData.Dash = *Input++;
		RawEntryData.SecondSpace = *Input++;

		uint32_t CategoryLength = 0;
		while(*Input != ':')
		{
			RawEntryData.Category[CategoryLength++] = *Input++;
		}
	}

	// copy RawEntryData and rest of line to Output
	{
		for(uint32_t RawEntryDataElementsIndex = 0;
			RawEntryData.Elements[RawEntryDataElementsIndex] != 0;
		   )
		{
			*Output++ = RawEntryData.Elements[RawEntryDataElementsIndex];
			++Result.CharsPushed;
		}

		while(*Output != '\n')
		{
			*Output++ = *Input++;
			++Result.CharsPushed;
		}
	}

	{
		Result.Entry.StartTime.Hour = ConvertToUInt32(RawEntryData.HourFirstDigit, 
														  RawEntryData.HourSecondDigit);
		Result.Entry.StartTime.Minute = ConvertToUInt32(RawEntryData.MinuteFirstDigit, 
															RawEntryData.MinuteSecondDigit);

		if(Equals("Sleep", RawEntryData.Category))
		{
			Result.Entry.Category = TIME_CATEGORY_SLEEP;
		}
		else if(Equals("Play", RawEntryData.Category))
		{
			Result.Entry.Category = TIME_CATEGORY_PLAY;
		}
		else if(Equals("School", RawEntryData.Category))
		{
			Result.Entry.Category = TIME_CATEGORY_SCHOOL;
		}
		else if(Equals("Education", RawEntryData.Category))
		{
			Result.Entry.Category = TIME_CATEGORY_EDUCATION;
		}
		else if(Equals("Programming", RawEntryData.Category))
		{
			Result.Entry.Category = TIME_CATEGORY_PROGRAMMING;
		}
		else if(Equals("Awake", RawEntryData.Category))
		{
			Result.Entry.Category = TIME_CATEGORY_AWAKE;
		}
		else
		{
			fprintf("Error: unknown category: %s\n", RawEntryData.Category);
			exit(1);
		}
	}

	return Result;
}

#define TABLE_ROWS 8
#define TABLE_COLUMNS 39
#define TABLE_SIZE (TABLE_ROWS * TABLE_COLUMNS)

#define TABLES_PER_ROW 3

#define NEWLINE_SIZE 2
#define ROW_OF_TABLES_SPACES_PER_LINE (TABLES_PER_ROW - 1)
#define ROW_OF_TABLES_LINE_SIZE ((TABLE_COLUMNS * TABLES_PER_ROW) + ROW_OF_TABLES_SPACES_PER_LINE + NEWLINE_SIZE)

#define HOUR_COLUMN 13
#define ANALYSIS_COLUMN 31

internal void
FillTableBuffer(char *Buffer, table Table)
{
	bool32 IsUnder = (Table.RealityTime < Table.TargetTime);
	char GoodAnalysis[] = "(GOOD)";
	char BadAnalysis[] = "(BAD)";
	char *Analysis;
	if(Table.IsUnderGood)
	{
		Analysis = IsUnder ? GoodAnalysis : BadAnalysis;
	}
	else
	{
		Analysis = IsUnder ? BadAnalysis : GoodAnalysis;
	}

	// line 1
	{
		*Buffer++ = ' ';
		for(uint32_t Col = 1;
			Col < (TABLE_COLUMNS - 1);
			++Col)
		{
			*Buffer++ = '_';
		}
		*Buffer++ = ' ';
	}

	// line 2
	{
		uint32_t NameLength = Length(Table.Name);
		uint32_t NameStartCol = ((TABLE_COLUMNS - NameLength) / 2);

		*Buffer++ = '|';
		uint32_t Col = 1;
		for(;
			Col < NameStartCol;
			++Col)
		{
			*Buffer++ = ' ';
		}
		for(;
			Col < NameLength;
			++Col)
		{
			*Buffer++ = *Table.Name++;
		}
		for(;
			Col < (TABLE_COLUMNS - 1);
			++Col)
		{
			*Buffer++ = ' ';
		}
		*Buffer++ = '|';
	}

	// line 3
	{
		*Buffer++ = '|';
		for(uint32_t Col = 1;
			Col < (TABLE_COLUMNS - 1);
			++Col)
		{
			*Buffer++ = '-';
		}
		*Buffer++ = '|';
	}

	// line 4/5
	{
		char *Intros[] = 
		{
			"| Target:",
			"| Reality:"
		};
		for(uint32_t Line = 0;
			Line < 2;
			++Line)
		{
			uint32_t CharsAdded = Copy(Intros[Line], Buffer);
			Buffer += CharsAdded;

			uint32_t Col = CharsAdded;
			for(;
				Col < HOUR_COLUMN;
				++Col)
			{
				*Buffer++ = ' ';
			}

			number2_char Hours = ConvertToChar(Table.TimeElapses[Line].Hours);
			number2_char Minutes = ConvertToChar(Table.TimeElapses[Line].Minutes);

			*Buffer++ = Hours.FirstDigit;
			*Buffer++ = Hours.SecondDigit;
			*Buffer++ = ':';
			*Buffer++ = Minutes.FirstDigit;
			*Buffer++ = Minutes.SecondDigit;
			Col += 5;

			CharsAdded = Copy(" hours", Buffer);
			Buffer += CharsAdded;
			Col += CharsAdded;

			for(;
				Col < HOUR_COLUMN;
				++Col)
			{
				*Buffer++ = ' ';
			}

			CharsAdded = Copy(Analysis, Buffer);
			Buffer += CharsAdded;
			Col += CharsAdded;

			for(;
				Col < (TABLE_COLUMNS - 1);
				++Col)
			{
				*Buffer++ = ' ';
			}

			*Buffer++ = '|';

		}
	}

	// line 6
	{
#define HOUR_LENGTH 11
#define HOUR_END_COLUMN (HOUR_COLUMN + HOUR_LENGTH)
		*Buffer++ = '|';
		for(;
			Col < HOUR_COLUMN;
			++Col)
		{
			*Buffer++ = ' ';
		}
		for(;
			Col < (HOUR_END_COLUMN + 1);
			++Col)
		{
			*Buffer++ = '-';
		}
	}

	// line 7
	{
	}

	// line 8
	{
	}
}

internal process_day_result
ProcessDay(char *Input, char *Output)
{
	uint32_t InputCharsProcessed = 0;
	uint32_t OutputCharsPushed = 0;

	// advance 3 lines to get to first entry
	for(int Line = 0;
		Line < 3;
		++Line)
	{
		uint32_t CharsPushed = PushLine(&Input[InputCharsProcessed], 
										&Output[OutputCharsPushed]);

		InputCharsProcessed += CharsPushed;
		OutputCharsPushed += CharsPushed;
	}

	table Tables[6];
	// fill Tables (really just the RealityTime member)
	{
		FillWithDefaultValues(Tables);

		entry PrevEntry = {};
		PrevEntry.Category = TIME_CATEGORY_NULL;
		for(;;)
		{
			process_entry_result ProcessEntryResult = ProcessEntry(&Input[InputCharsProcessed], 
																   &Output[OutputCharsPushed]);
			InputCharsProcessed += ProcessEntryResult.CharsPushed;
			OutputCharsPushed += ProcessEntryResult.CharsPushed;

			entry CurEntry = ProcessEntryResult.Entry;

			if((Input[InputCharsProcessed] == PROCESSED_ENTRY_FLAG) || 
			   (Input[InputCharsProcessed] == UNPROCESSED_ENTRY_FLAG))
			{
				// reached next day
				break;
			}
			else if(PrevEntry.Category != TIME_CATEGORY_NULL)
			{
				entry TimeElapsed = (CurEntry.StartTime - PrevEntry.StartTime);
				Tables[PrevEntry.Category].RealityTime += TimeElapsed;
				Tables[TIME_CATEGORY_AWAKE].RealityTime += TimeElapsed;
			}

			PrevEntry = CurEntry;
		}
	}

	// insert tables
	{
		char TableBuffer[TABLE_SIZE];
		for(uint32_t TableIndex;
			TableIndex < ArrayCount(Tables);
			++TableIndex)
		{
			FillTableBuffer(TableBuffer, Tables[TableIndex]);

			uint32_t ColumnInRow = (TableIndex % TABLES_PER_ROW);
			for(uint32_t TableRow = 0;
				TableRow < TABLE_ROW;
				++TableRow)
			{
				// copy a single row from the table to Output
				uint32_t OutputIndex = ((TableRow * ROW_OF_TABLES_LINE_SIZE) + (ColumnInRow * TABLE_COLUMNS));
				for(uint32_t TableColumn = 0;
					TableColumn < TABLE_COLUMNS;
					++TableColumn)
				{
					&Output[OutputIndex++] = *TableBuffer++;
				}

				if(ColumnInRow == (TABLES_PER_ROW - 1))
				{
					// last table in row
					Output[OutputIndex++] = '\r';
					Output[OutputIndex++] = '\n';

					if(TableRow == (TABLE_ROW - 1))
					{
						// last row of last table
						Output[OutputIndex++] = '\r';
						Output[OutputIndex++] = '\n';
					}
				}
				else
				{
					Output[OutputIndex++] = ' ';
				}
			}
		}
	}

	// calculate number of characterss added
	{
		uint32_t NumberOfTableRows = Ceil((float)NumTimeCategories / (float)TABLES_PER_ROW);
		uint32_t NumberOfLines = (NumberOfTableRows * TABLE_ROWS);
		uint32_t SizeOfTables = (NumberOfLines * ROW_OF_TABLES_LINE_SIZE);

		uint32_t SizeOfNewlines = (NumberOfTableRows * NEWLINE_SIZE);

		uint32_t TotalSize = (SizeOfTables + SizeOfNewlines);
		OutputCharsPushed += TotalSize;
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
