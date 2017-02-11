#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

#include "util.h"

// TODO(hacksoi): Consider a better method of determining the size of the
// output file (possibly a pass over the input file to see how many entries
// have been processed)
// NOTE(hacksoi): I actually calculated around 6
#define TABLES_TO_ENTRY_RATIO 8
#define MAX_INPUT_FILE_SIZE (0xFFFFFFFF / TABLES_TO_ENTRY_RATIO)
#define SIZE_OF_TABLES 1940

#define MAX_CATEGORY_NAME_LENGTH 32
#define ENTRY_FORMAT_LENGTH (8 + MAX_CATEGORY_NAME_LENGTH)

//
// NOTE(hacksoi): Variables for processing tables 
//

#define SIZEOF_NEWLINE 2
#define BLANK_LINES_PER_DAY 2
#define SIZEOF_NULL_TERMINATOR 1

#define LAST_LINE_FLAG '$'
#define ENTRY_PROCESSED_FLAG '#'
#define ENTRY_UNPROCESSED_FLAG '@'

#define TABLE_ROWS 8
#define TABLE_COLUMNS 39
#define TABLE_SIZE (TABLE_ROWS * TABLE_COLUMNS)

#define TABLES_PER_ROW 3

#define HOUR_COLUMN 13
#define ANALYSIS_COLUMN 31

#define HOUR_LENGTH 11
#define HOUR_END_COLUMN (HOUR_COLUMN + HOUR_LENGTH)

#define DISTANCE_BETWEEN_TABLES_HORIZONTAL 1
#define TOTAL_TABLE_COLUMNS (TABLE_COLUMNS + DISTANCE_BETWEEN_TABLES_HORIZONTAL)

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
	A = A + B;

	return A;
}

struct number2_char
{
	char FirstDigit;
	char SecondDigit;
};

inline number2_char
ConvertTo2Char(uint32_t A)
{
	Assert(A < 99);

	uint32_t SecondDigit = (A % 10);
	uint32_t FirstDigit = (A / 10);

	number2_char Result;
	Result.FirstDigit = (char)('0' + FirstDigit);
	Result.SecondDigit = (char)('0' + SecondDigit);

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

	char Elements[ENTRY_FORMAT_LENGTH];
};

struct entry
{
	time StartTime;
	time_category Category;
};

enum bad_condition
{
	BAD_CONDITION_UNDER,
	BAD_CONDITION_OVER,
	BAD_CONDITION_NOT_EQUAL
};

struct time_category
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

	bad_condition BadCondition;

	char *Name;
};

struct process_day_result
{
	uint32_t SrcCharsProcessed;
	uint32_t DestCharsAdded;
};

struct process_entry_result
{
	uint32_t CharsPushed;
	entry Entry;
};

internal bool32
Equals(char *A, char *B)
{
	bool32 Result = true;

	while((*A != '\0') &&
		  (*B != '\0'))
	{
		if(*A++ != *B++)
		{
			Result = false;
			break;
		}
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
		Result = (uint32_t)(A + 1.0f);
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

inline uint32_t
ConvertToUInt32(char FirstDigitChar, char SecondDigitChar)
{
	uint32_t FirstDigit = (FirstDigitChar - '0');
	uint32_t SecondDigit = (SecondDigitChar - '0');

	uint32_t Result = ((FirstDigit * 10) + SecondDigit);

	return Result;
}

internal uint32_t
CopyLine(char *Src, char *Dest)
{
	uint32_t Result = 0;

	for(;;)
	{
		*Dest = *Src++;
		++Result;

		if(*Dest++ == '\n')
		{
			break;
		}
	}

	return Result;
}

internal uint32_t
Copy(char *Src, char *Dest)
{
	uint32_t Result = 0;
	while(*Src != '\0')
	{
		*Dest++ = *Src++;
		++Result;
	}

	return Result;
}

internal void
Copy(char **Buffer, char *Src, uint32_t *Column)
{
	while(*Src != '\0')
	{
		**Buffer = *Src++;
		++(*Buffer);
		++(*Column);
	}
}

internal void
Copy(char **Buffer, char C, uint32_t *StartColumn, uint32_t EndColumn)
{
	while(*StartColumn < EndColumn)
	{
		**Buffer = C;
		++(*Buffer);
		++(*StartColumn);
	}
}

internal void
FillWithDefaultValues(time_category *Tables)
{
	// Sleep
	{
		Tables[TIME_CATEGORY_SLEEP].TargetTime.Hours = 8;
		Tables[TIME_CATEGORY_SLEEP].TargetTime.Minutes = 0;

		Tables[TIME_CATEGORY_SLEEP].RealityTime.Hours = 0;
		Tables[TIME_CATEGORY_SLEEP].RealityTime.Minutes = 0;

		Tables[TIME_CATEGORY_SLEEP].IsUnderGood = true;
		Tables[TIME_CATEGORY_SLEEP].Name = "Sleep";
	}

	// Play
	{
		Tables[TIME_CATEGORY_PLAY].TargetTime.Hours = 3;
		Tables[TIME_CATEGORY_PLAY].TargetTime.Minutes = 0;

		Tables[TIME_CATEGORY_PLAY].RealityTime.Hours = 0;
		Tables[TIME_CATEGORY_PLAY].RealityTime.Minutes = 0;

		Tables[TIME_CATEGORY_PLAY].IsUnderGood = true;
		Tables[TIME_CATEGORY_PLAY].Name = "Play";
	}

	// School
	{
		Tables[TIME_CATEGORY_SCHOOL].TargetTime.Hours = 6;
		Tables[TIME_CATEGORY_SCHOOL].TargetTime.Minutes = 0;

		Tables[TIME_CATEGORY_SCHOOL].RealityTime.Hours = 0;
		Tables[TIME_CATEGORY_SCHOOL].RealityTime.Minutes = 0;

		Tables[TIME_CATEGORY_SCHOOL].IsUnderGood = true;
		Tables[TIME_CATEGORY_SCHOOL].Name = "School";
	}

	// Education
	{
		Tables[TIME_CATEGORY_EDUCATION].TargetTime.Hours = 3;
		Tables[TIME_CATEGORY_EDUCATION].TargetTime.Minutes = 0;

		Tables[TIME_CATEGORY_EDUCATION].RealityTime.Hours = 0;
		Tables[TIME_CATEGORY_EDUCATION].RealityTime.Minutes = 0;

		Tables[TIME_CATEGORY_EDUCATION].IsUnderGood = false;
		Tables[TIME_CATEGORY_EDUCATION].Name = "Education";
	}

	// Programming
	{
		Tables[TIME_CATEGORY_PROGRAMMING].TargetTime.Hours = 4;
		Tables[TIME_CATEGORY_PROGRAMMING].TargetTime.Minutes = 0;

		Tables[TIME_CATEGORY_PROGRAMMING].RealityTime.Hours = 0;
		Tables[TIME_CATEGORY_PROGRAMMING].RealityTime.Minutes = 0;

		Tables[TIME_CATEGORY_PROGRAMMING].IsUnderGood = false;
		Tables[TIME_CATEGORY_PROGRAMMING].Name = "Programming";
	}

	// Awake
	{
		Tables[TIME_CATEGORY_AWAKE].TargetTime.Hours = 16;
		Tables[TIME_CATEGORY_AWAKE].TargetTime.Minutes = 0;

		Tables[TIME_CATEGORY_AWAKE].RealityTime.Hours = 0;
		Tables[TIME_CATEGORY_AWAKE].RealityTime.Minutes = 0;

		Tables[TIME_CATEGORY_AWAKE].IsUnderGood = false;
		Tables[TIME_CATEGORY_AWAKE].Name = "Awake";
	}
}

internal process_entry_result
ProcessEntry(char *Src, char *Dest)
{
	process_entry_result Result = {};

	raw_entry_data RawEntryData = {};
	// parse entry data
	{
		RawEntryData.HourFirstDigit = *Src++;
		RawEntryData.HourSecondDigit = *Src++;

		RawEntryData.Colon = *Src++;

		RawEntryData.MinuteFirstDigit = *Src++;
		RawEntryData.MinuteSecondDigit = *Src++;

		RawEntryData.FirstSpace = *Src++;
		RawEntryData.Dash = *Src++;
		RawEntryData.SecondSpace = *Src++;

		uint32_t CategoryLength = 0;
		while(*Src != ':')
		{
			RawEntryData.Category[CategoryLength++] = *Src++;
		}
	}

	// copy RawEntryData and rest of line to Dest
	{
		for(uint32_t RawEntryDataElementsIndex = 0;
			RawEntryData.Elements[RawEntryDataElementsIndex] != '\0';
			++RawEntryDataElementsIndex)
		{
			*Dest++ = RawEntryData.Elements[RawEntryDataElementsIndex];
			++Result.CharsPushed;
		}

		for(;;)
		{
			*Dest = *Src++;
			++Result.CharsPushed;

			if(*Dest == '\n')
			{
				break;
			}

			++Dest;
		}
	}

	// fill out Entry
	{
		Result.Entry.StartTime.Hour = ConvertToUInt32(RawEntryData.HourFirstDigit, 
													  RawEntryData.HourSecondDigit);
		Result.Entry.StartTime.Minute = ConvertToUInt32(RawEntryData.MinuteFirstDigit, 
														RawEntryData.MinuteSecondDigit);

		if(Equals(RawEntryData.Category, "Sleep"))
		{
			Result.Entry.Category = TIME_CATEGORY_SLEEP;
		}
		else if(Equals(RawEntryData.Category, "Play"))
		{
			Result.Entry.Category = TIME_CATEGORY_PLAY;
		}
		else if(Equals(RawEntryData.Category, "School"))
		{
			Result.Entry.Category = TIME_CATEGORY_SCHOOL;
		}
		else if(Equals(RawEntryData.Category, "Education"))
		{
			Result.Entry.Category = TIME_CATEGORY_EDUCATION;
		}
		else if(Equals(RawEntryData.Category, "Programming"))
		{
			Result.Entry.Category = TIME_CATEGORY_PROGRAMMING;
		}
		else if(Equals(RawEntryData.Category, "Awake"))
		{
			Result.Entry.Category = TIME_CATEGORY_AWAKE;
		}
		else if(Equals(RawEntryData.Category, "**Executing"))
		{
			Result.Entry.Category = TIME_CATEGORY_DUMMY;
		}
		else
		{
			Assert(false);
			// TODO: Make this more descriptive (like the line number or day)
			fprintf(stderr, "Error: unknown category: %s\n", RawEntryData.Category);
			exit(1);
		}
	}

	return Result;
}

internal void
FillTableBuffer(char *Buffer, table Table)
{
	char *BufferStart = Buffer;

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
		uint32_t Col = 1;
		Copy(&Buffer, '_', &Col, (TABLE_COLUMNS - 1));
		*Buffer++ = ' ';
	}

	// line 2
	{
		uint32_t NameLength = Length(Table.Name);
		uint32_t NameStartCol = ((TABLE_COLUMNS - NameLength) / 2);

		*Buffer++ = '|';
		uint32_t Col = 1;
		Copy(&Buffer, ' ', &Col, NameStartCol);
		Copy(&Buffer, Table.Name, &Col);
		Copy(&Buffer, ' ', &Col, (TABLE_COLUMNS - 1));
		*Buffer++ = '|';
	}

	// line 3
	{
		*Buffer++ = '|';
		uint32_t Col = 1;
		Copy(&Buffer, '-', &Col, (TABLE_COLUMNS - 1));
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
			uint32_t Col = 0;
			Copy(&Buffer, Intros[Line], &Col);
			Copy(&Buffer, ' ', &Col, HOUR_COLUMN);
			{
				number2_char Hours = ConvertTo2Char(Table.TimeElapses[Line].Hours);
				number2_char Minutes = ConvertTo2Char(Table.TimeElapses[Line].Minutes);

				*Buffer++ = Hours.FirstDigit;
				*Buffer++ = Hours.SecondDigit;
				*Buffer++ = ':';
				*Buffer++ = Minutes.FirstDigit;
				*Buffer++ = Minutes.SecondDigit;
				Col += 5;
			}
			Copy(&Buffer, " hours", &Col);
			Copy(&Buffer, ' ', &Col, ANALYSIS_COLUMN);
			Copy(&Buffer, Analysis, &Col);
			Copy(&Buffer, ' ', &Col, (TABLE_COLUMNS - 1));
			*Buffer++ = '|';
		}
	}

	// line 6
	{
		*Buffer++ = '|';
		uint32_t Col = 1;
		Copy(&Buffer, ' ', &Col, HOUR_COLUMN);
		Copy(&Buffer, '-', &Col, HOUR_END_COLUMN);
		Copy(&Buffer, ' ', &Col, ANALYSIS_COLUMN);
		Copy(&Buffer, Analysis, &Col);
		Copy(&Buffer, ' ', &Col, (TABLE_COLUMNS - 1));
		*Buffer++ = '|';
	}

	// line 7
	{
		uint32_t Col = 0;
		Copy(&Buffer, "| Result:", &Col);
		Copy(&Buffer, ' ', &Col, HOUR_COLUMN);
		{
			time_elapse Difference = (Table.RealityTime - Table.TargetTime);
			number2_char Hours = ConvertTo2Char(Difference.Hours);
			number2_char Minutes = ConvertTo2Char(Difference.Minutes);

			*Buffer++ = Hours.FirstDigit;
			*Buffer++ = Hours.SecondDigit;
			*Buffer++ = ':';
			*Buffer++ = Minutes.FirstDigit;
			*Buffer++ = Minutes.SecondDigit;
			Col += 5;
		}
		if(IsUnder)
		{
			Copy(&Buffer, " hours under", &Col);
		}
		else
		{
			Copy(&Buffer, " hours over", &Col);
		}
		Copy(&Buffer, ' ', &Col, ANALYSIS_COLUMN);
		Copy(&Buffer, Analysis, &Col);
		Copy(&Buffer, ' ', &Col, (TABLE_COLUMNS - 1));
		*Buffer++ = '|';
	}

	// line 8
	{
		*Buffer++ = '|';
		uint32_t Col = 1;
		Copy(&Buffer, '_', &Col, (TABLE_COLUMNS - 1));
		*Buffer++ = '|';
	}

	uint64_t CharsWrote = (Buffer - BufferStart);
	Assert(CharsWrote == TABLE_SIZE);
}

struct copy_day_to_last_entry_result
{
	uint32_t CharsCopied;
	bool32 IsLastDay;
};

internal copy_day_to_last_entry_result
CopyDayToLastEntry(char *Src, char *Dest)
{
	copy_day_to_last_entry_result Result = {};

	uint32_t BlankLinesCopied = 0;
	for(;;)
	{
		uint32_t CharsCopied = CopyLine(Src, Dest);
		Src += CharsCopied;
		Dest += CharsCopied;
		Result.CharsCopied += CharsCopied;

		if((*Src == ENTRY_PROCESSED_FLAG) ||
		   (*Src == ENTRY_UNPROCESSED_FLAG))
		{
			Result.IsLastDay = false;

			break;
		}
		else if(*Src == LAST_LINE_FLAG)
		{
			Result.IsLastDay = true;

			break;
		}
	}

	Result.CharsCopied -= SIZEOF_NEWLINE;

	return Result;
}

internal uint32_t
InsertDummyEntry(char *Dest)
{
	char DummyEntry[] = "00:00 - **Executing:\r\n";

	SYSTEMTIME CurrentTime;
	GetLocalTime(&CurrentTime);

	number2_char CurrentHour = ConvertTo2Char(CurrentTime.wHour);
	number2_char CurrentMinute = ConvertTo2Char(CurrentTime.wMinute);

	DummyEntry[0] = CurrentHour.FirstDigit;
	DummyEntry[1] = CurrentHour.SecondDigit;
	DummyEntry[3] = CurrentMinute.FirstDigit;
	DummyEntry[4] = CurrentMinute.SecondDigit;

	uint32_t Result = Copy(DummyEntry, Dest);

	return Result;
}

internal process_day_result
ProcessDay(char *Src, char *Dest)
{
	uint32_t SrcCharsProcessed = 0;
	uint32_t DestCharsAdded = 0;

	// advance 3 lines to get to first entry
	for(uint32_t Line = 0;
		Line < 3;
		++Line)
	{
		uint32_t CharsCopied = CopyLine(&Src[SrcCharsProcessed], 
										&Dest[DestCharsAdded]);
		SrcCharsProcessed += CharsCopied;
		DestCharsAdded += CharsCopied;
	}

	table Tables[6];
	// fill Tables (just have to set RealityTime)
	{
		FillWithDefaultValues(Tables);

		entry PrevEntry = {};
		PrevEntry.Category = TIME_CATEGORY_NULL;
		for(;;)
		{
			process_entry_result ProcessEntryResult = ProcessEntry(&Src[SrcCharsProcessed], 
																   &Dest[DestCharsAdded]);
			SrcCharsProcessed += ProcessEntryResult.CharsPushed;
			DestCharsAdded += ProcessEntryResult.CharsPushed;

			entry CurEntry = ProcessEntryResult.Entry;
			if(PrevEntry.Category != TIME_CATEGORY_NULL)
			{
				time_elapse TimeElapsed = (CurEntry.StartTime - PrevEntry.StartTime);
				Tables[PrevEntry.Category].RealityTime += TimeElapsed;
				if(PrevEntry.Category != TIME_CATEGORY_SLEEP)
				{
					Tables[TIME_CATEGORY_AWAKE].RealityTime += TimeElapsed;
				}
			}

			if(Src[SrcCharsProcessed] == '\r')
			{
				Dest[DestCharsAdded++] = Src[SrcCharsProcessed++]; // '\r'
				Dest[DestCharsAdded++] = Src[SrcCharsProcessed++]; // '\n'

				break;
			}

			PrevEntry = CurEntry;
		}
	}

	// insert tables
	{
		char TableBuffer_[TABLE_SIZE];
		char *TableBuffer = TableBuffer_;
		uint32_t BaseDestIndex = DestCharsAdded;
		uint32_t NumberOfTableRows = Ceil((float)ArrayCount(Tables) / (float)TABLES_PER_ROW);
		for(uint32_t TableIndex = 0;
			TableIndex < ArrayCount(Tables);
			++TableIndex)
		{
			TableBuffer = TableBuffer_;
			FillTableBuffer(TableBuffer, Tables[TableIndex]);

			uint32_t Row = ((TableIndex / TABLES_PER_ROW) + 1);
			bool32 OnLastRow = (Row == NumberOfTableRows);
			uint32_t TablesThisRow = OnLastRow ? (((ArrayCount(Tables) - 1) % TABLES_PER_ROW) + 1) : TABLES_PER_ROW;

			uint32_t TotalDistanceBetweenTables = ((TablesThisRow - 1) * DISTANCE_BETWEEN_TABLES_HORIZONTAL);
			uint32_t ThisRowOfTablesLineSize = ((TABLE_COLUMNS * TablesThisRow) + TotalDistanceBetweenTables + SIZEOF_NEWLINE);

			uint32_t ColumnInRow = (TableIndex % TABLES_PER_ROW);
			for(uint32_t TableRow = 0;
				TableRow < TABLE_ROWS;
				++TableRow)
			{
				// copy a single row from the table to Dest
				uint32_t DestIndex = (BaseDestIndex + 
									  ((TableRow * ThisRowOfTablesLineSize) + (ColumnInRow * TOTAL_TABLE_COLUMNS)));
				for(uint32_t TableColumn = 0;
					TableColumn < TABLE_COLUMNS;
					++TableColumn)
				{
					Dest[DestIndex++] = *TableBuffer++;
					++DestCharsAdded;
				}

				if((ColumnInRow == (TABLES_PER_ROW - 1)) ||
				   (TableIndex == (ArrayCount(Tables) - 1)))
				{
					// last table in row
					Dest[DestIndex++] = '\r';
					Dest[DestIndex++] = '\n';
					DestCharsAdded += 2;

					if(TableRow == (TABLE_ROWS - 1))
					{
						// last row of last table in row
						Dest[DestIndex++] = '\r';
						Dest[DestIndex++] = '\n';
						DestCharsAdded += 2;

						BaseDestIndex = DestCharsAdded;
					}
				}
				else
				{
					for(uint32_t Space = 0;
						Space < DISTANCE_BETWEEN_TABLES_HORIZONTAL;
						++Space)
					{
						Dest[DestIndex++] = ' ';
						++DestCharsAdded;
					}
				}
			}
		}
	}

	Dest[0] = ENTRY_PROCESSED_FLAG;

	process_day_result Result;
	Result.SrcCharsProcessed = SrcCharsProcessed;
	Result.DestCharsAdded = DestCharsAdded;

	return Result;
}

int
main(int argc, char *argv[])
{
	HANDLE InputFile = CreateFile("time_raw.txt",
								  GENERIC_READ,
								  FILE_SHARE_READ,
								  NULL,
								  OPEN_EXISTING,
								  FILE_ATTRIBUTE_NORMAL,
								  NULL);

	Assert(InputFile != INVALID_HANDLE_VALUE);
	if(InputFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: failed to create handle to source file (time_raw.txt)\n");
		exit(1);
	}

	LARGE_INTEGER InputFileSize_;
	if(!GetFileSizeEx(InputFile, &InputFileSize_))
	{
		Assert(false);
		fprintf(stderr, "Error: failed to read file size\n");
		exit(1);
	}

	Assert(InputFileSize_.QuadPart <= MAX_INPUT_FILE_SIZE);
	if(InputFileSize_.QuadPart > MAX_INPUT_FILE_SIZE)
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
				 InputContents,
				 InputFileSize,
				 &BytesRead,
				 NULL))
	{
		Assert(false);
		fprintf(stderr, "Error: failed to read source file (time_raw.txt).\n");
		exit(1);
	}

	Assert(BytesRead == InputFileSize);
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

	Assert(OutputFile != INVALID_HANDLE_VALUE);
	if(OutputFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error: failed to create handle to processed file (time_processed.txt)\n");
		exit(1);
	}

	uint32_t OutputFileSize = (InputFileSize * TABLES_TO_ENTRY_RATIO);
	char *OutputContents = (char *)VirtualAlloc(NULL,
												OutputFileSize,
												MEM_COMMIT | MEM_RESERVE,
												PAGE_EXECUTE_READWRITE);
	uint32_t OutputContentsSize = 0;

	Assert((InputContents[0] == ENTRY_PROCESSED_FLAG) ||
		   (InputContents[0] == ENTRY_UNPROCESSED_FLAG));
	if((InputContents[0] != ENTRY_PROCESSED_FLAG) &&
	   (InputContents[0] != ENTRY_UNPROCESSED_FLAG))
	{
		fprintf(stderr, "Error: input file has incorrect format (first character must be '#' or '@')\n");
		exit(1);
	}

	ConfigFile = LoadConfigFile();
	DaysOfTheWeek = ParseConfigFile(ConfigFile);

	uint32_t InputFileCharIndex = 0;
	for(;;)
	{
		char c = InputContents[InputFileCharIndex];

		Assert((c == ENTRY_PROCESSED_FLAG) || 
			   (c == ENTRY_UNPROCESSED_FLAG) ||
			   (c == LAST_LINE_FLAG));

		if(c == LAST_LINE_FLAG)
		{
			// copy last line
			uint32_t CharsCopied = CopyLine(&InputContents[InputFileCharIndex],
											&OutputContents[OutputContentsSize]);
			OutputContentsSize += CharsCopied;

			break;
		} 
		else if(c == ENTRY_UNPROCESSED_FLAG)
		{
			char DayBuffer[4096];
			copy_day_to_last_entry_result CopyDayToLastEntryResult = CopyDayToLastEntry(&InputContents[InputFileCharIndex], 
																						DayBuffer);
			uint32_t InputCharsProcessed = CopyDayToLastEntryResult.CharsCopied;
			uint32_t DayBufferIndex = InputCharsProcessed;

			if(CopyDayToLastEntryResult.IsLastDay)
			{
				DayBufferIndex += InsertDummyEntry(&DayBuffer[DayBufferIndex]);
			}

			DayBuffer[DayBufferIndex++] = '\r';
			DayBuffer[DayBufferIndex++] = '\n';
			InputCharsProcessed += SIZEOF_NEWLINE;

			process_day_result ProcessDayResult = ProcessDay(DayBuffer,
															 &OutputContents[OutputContentsSize],
															 &DaysOfTheWeek);

			InputFileCharIndex += InputCharsProcessed;
			OutputContentsSize += ProcessDayResult.DestCharsAdded;
		}
		else
		{
			while((InputContents[InputFileCharIndex] != ENTRY_UNPROCESSED_FLAG) &&
				  (InputContents[InputFileCharIndex] != LAST_LINE_FLAG))
			{
				uint32_t CharsCopied = CopyLine(&InputContents[InputFileCharIndex],
												&OutputContents[OutputContentsSize]);

				InputFileCharIndex += CharsCopied;
				OutputContentsSize += CharsCopied;
			}
		}
	}

	DWORD BytesWritten;
	if(!WriteFile(OutputFile,
				  OutputContents,
				  OutputContentsSize,
				  &BytesWritten,
				  NULL))
	{
		Assert(false);
		fprintf(stderr, "Error: failed to write processed file (time_processed.txt)\n");
		exit(1);
	}

	Assert(BytesWritten == OutputContentsSize);
	if(BytesWritten != OutputContentsSize)
	{
		fprintf(stderr, "Error: wrote incorrect number of bytes: expected %d but wrote %d.\n", 
				OutputContentsSize, BytesWritten);
		exit(1);
	}

	return 0;
}
