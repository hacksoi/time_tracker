
// TODO: Change process of parsing and printing hours
// so that the first digit of the hour is not required for parsing and is not
// printed for printing.

#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

#include "util.h"
#include "windows_util.h"
#include "string_util.h"
#include "time_util.h"
#include "time_tracker.h"

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

inline uint32_t
ConvertToUInt32(char FirstDigitChar, char SecondDigitChar)
{
	uint32_t FirstDigit = (FirstDigitChar - '0');
	uint32_t SecondDigit = (SecondDigitChar - '0');

	uint32_t Result = ((FirstDigit * 10) + SecondDigit);

	return Result;
}

inline uint32_t
ConvertToUInt32(number2_char TwoChars)
{
	uint32_t Result = ConvertToUInt32(TwoChars.FirstDigit, TwoChars.SecondDigit);

	return Result;
}

inline bool32
IsCharNumerical(char A)
{
	char c = ('0' + A);
	bool32 Result = ((c >= '0') &&
					 (c >= '9'));

	return Result;
}

internal time_characters
ConvertToTimeCharacters(char *Src)
{
	time_characters Result = {};

	uint32_t CharsBeforeColon = 0;
	while(Src[++CharsBeforeColon] != ':')
	{
	}

	Assert(CharsBeforeColon > 0);
	if(CharsBeforeColon == 1)
	{
		Result.Hour.FirstDigit = '0';
	}
	else
	{
		if(IsCharNumerical(Src[CharsBeforeColon - 2]))
		{
			Result.Hour.FirstDigit = Src[CharsBeforeColon - 2];
		}
		else
		{
			Result.Hour.FirstDigit = '0';
		}
	}

	Result.Hour.SecondDigit = Src[CharsBeforeColon - 1];
	Result.Minute.FirstDigit = Src[CharsBeforeColon + 1];
	Result.Minute.SecondDigit = Src[CharsBeforeColon + 2];

	return Result;
}

internal time_elapse
ConvertToTimeElapse(char *Src)
{
	time_characters TimeCharacters = ConvertToTimeCharacters(Src);

	time_elapse Result = {};
	Result.Hours = ConvertToUInt32(TimeCharacters.Hours);
	Result.Minutes = ConvertToUInt32(TimeCharacters.Minutes);

	return Result;
}

internal entry
ProcessEntry(char *Src)
{
	entry Result = {};

	// parse time
	{
		char TimeBuffer[5];
		uint32_t DestCharsAdded = GetToken(&Src, TimeBuffer);
		Assert(DestCharsAdded <= ArrayCount(TimeBuffer));

		time_characters TimeCharacters = ConvertToTimeCharacters(TimeBuffer);
		Result.StartTime.Hour = ConvertToUInt32(TimeCharacters.Hour);
		Result.StartTime.Minute = ConvertToUInt32(TimeCharacters.Minute);
	}

	// parse category name
	{
		// get rid of dash
		GetToken(&Src, Result.CategoryName);
		GetToken(&Src, Result.CategoryName, ':');
	}

	return Result;
}

internal void
FillTableBuffer(char *Buffer, time_category TimeCategory)
{
	char *BufferStart = Buffer;

	comparison TimeComparison = Compare(TimeCategory.RealityTime, TimeCategory.TargetTime);
	char GoodAnalysis[] = "(GOOD)";
	char BadAnalysis[] = "(BAD)";
	char *Analysis = (TimeComparison & TimeCategory.BadCondition) ? BadAnalysis : GoodAnalysis;

	// line 1
	{
		*Buffer++ = ' ';
		uint32_t Col = 1;
		Copy(&Buffer, '_', &Col, (TABLE_COLUMNS - 1));
		*Buffer++ = ' ';
	}

	// line 2
	{
		uint32_t NameLength = Length(TimeCategory.Name);
		uint32_t NameStartCol = ((TABLE_COLUMNS - NameLength) / 2);

		*Buffer++ = '|';
		uint32_t Col = 1;
		Copy(&Buffer, ' ', &Col, NameStartCol);
		Copy(&Buffer, TimeCategory.Name, &Col);
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
				number2_char Hours = ConvertTo2Char(TimeCategory.TimeElapses[Line].Hours);
				number2_char Minutes = ConvertTo2Char(TimeCategory.TimeElapses[Line].Minutes);

				*Buffer++ = (Hours.FirstDigit == '0') ? ' ' : Hours.FirstDigit;
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
			time_elapse Difference = (TimeCategory.RealityTime - TimeCategory.TargetTime);
			number2_char Hours = ConvertTo2Char(Difference.Hours);
			number2_char Minutes = ConvertTo2Char(Difference.Minutes);

			*Buffer++ = (Hours.FirstDigit == '0') ? ' ' : Hours.FirstDigit;
			*Buffer++ = Hours.SecondDigit;
			*Buffer++ = ':';
			*Buffer++ = Minutes.FirstDigit;
			*Buffer++ = Minutes.SecondDigit;
			Col += 5;
		}
		if(TimeComparison == COMPARISON_LESS)
		{
			Copy(&Buffer, " hours under", &Col);
		}
		else if(TimeComparison == COMPARISON_GREATER)
		{
			Copy(&Buffer, " hours over", &Col);
		}
		else
		{
			Copy(&Buffer, " hours exact", &Col);
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

internal time_category *
GetTimeCategory(char *CategoryName, day *Day)
{
	time_category *TimeCategory = NULL;
	for(uint32_t TimeCategoryIndex = 0;
		TimeCategoryIndex < Day->TimeCategoryCount;
		++TimeCategoryIndex)
	{
		if(Equals(CategoryName, Day->TimeCategories[TimeCategoryIndex].Name))
		{
			TimeCategory = &Day->TimeCategories[TimeCategoryIndex];

			break;
		}
	}

	return TimeCategory;
}

internal process_day_result
ProcessDay(char *Src, char *Dest, day *Days)
{
	uint32_t SrcCharsScanned = 0;
	uint32_t DestCharsAdded = 0;

	day *Day = NULL;
	// parse day name
	{
		// advance 2 lines to get to day
		CopyLine(Src, &SrcCharsScanned, 
				 Dest, &DestCharsAdded, 2);

		char DayNameBuffer[MAX_DAY_NAME_SIZE];
		GetToken(&Src[SrcCharsScanned], DayNameBuffer);
		for(uint32_t DayIndex = 0;
			DayIndex < NUMBER_OF_DAYS;
			++DayIndex)
		{
			if(Equals(DayNameBuffer, Days[DayIndex].Name))
			{
				Day = &Days[DayIndex];

				break;
			}
		}

		CopyLine(Src, &SrcCharsScanned, 
				 Dest, &DestCharsAdded);
	}

	// clear Day->TimeCategory.RealityTime (it gets reused)
	for(uint32_t TimeCategoryIndex = 0;
		TimeCategoryIndex < Day->TimeCategoryCount;
		++TimeCategoryIndex)
	{
		Day->TimeCategories[TimeCategoryIndex].RealityTime.Hours = 0;
		Day->TimeCategories[TimeCategoryIndex].RealityTime.Minutes = 0;
	}

	// fill each Day->TimeCategory.RealityTime
	{
		entry PrevEntry = {};
		for(;;)
		{
			entry CurEntry = ProcessEntry(&Src[SrcCharsScanned]);
			if(PrevEntry.CategoryName[0] != '\0')
			{
				time_category *PrevTimeCategory = GetTimeCategory(PrevEntry.CategoryName, Day);

				time_elapse TimeElapsed = (CurEntry.StartTime - PrevEntry.StartTime);
				PrevTimeCategory->RealityTime += TimeElapsed;
				if(!Equals(PrevEntry.CategoryName, "Sleep"))
				{
					time_category *AwakeTimeCategory = GetTimeCategory("Awake", Day);
					AwakeTimeCategory->RealityTime += TimeElapsed;
				}
			}

			CopyLine(Src, &SrcCharsScanned, 
					 Dest, &DestCharsAdded);
			if(Src[SrcCharsScanned] == '\r')
			{
				CopyLine(Src, &SrcCharsScanned, 
						 Dest, &DestCharsAdded);

				break;
			}

			PrevEntry = CurEntry;
		}
	}

	// insert tables
	{
		uint32_t TableCount = Day->TimeCategoryCount;
		char TableBuffer_[TABLE_SIZE];
		char *TableBuffer = TableBuffer_;
		uint32_t BaseDestIndex = DestCharsAdded;
		uint32_t NumberOfTableRows = Ceil((float)TableCount / (float)TABLES_PER_ROW);
		for(uint32_t TableIndex = 0;
			TableIndex < TableCount;
			++TableIndex)
		{
			TableBuffer = TableBuffer_;
			FillTableBuffer(TableBuffer, Day->TimeCategories[TableIndex]);

			uint32_t Row = ((TableIndex / TABLES_PER_ROW) + 1);
			bool32 OnLastRow = (Row == NumberOfTableRows);
			uint32_t TablesThisRow = OnLastRow ? (((TableCount - 1) % TABLES_PER_ROW) + 1) : TABLES_PER_ROW;

			uint32_t TotalDistanceBetweenTables = ((TablesThisRow - 1) * DISTANCE_BETWEEN_TABLES_HORIZONTAL);
			uint32_t ThisRowOfTablesLineSize = ((TABLE_COLUMNS * TablesThisRow) + TotalDistanceBetweenTables + SIZEOF_NEWLINE);

			uint32_t TableColumnInRow = (TableIndex % TABLES_PER_ROW);
			for(uint32_t TableRow = 0;
				TableRow < TABLE_ROWS;
				++TableRow)
			{
				// copy a single row from the table to Dest
				uint32_t DestIndex = (BaseDestIndex + 
									  ((TableRow * ThisRowOfTablesLineSize) + (TableColumnInRow * TOTAL_TABLE_COLUMNS)));
				for(uint32_t TableColumn = 0;
					TableColumn < TABLE_COLUMNS;
					++TableColumn)
				{
					Dest[DestIndex++] = *TableBuffer++;
					++DestCharsAdded;
				}

				bool32 IsLastTableInRow = (TableColumnInRow == (TABLES_PER_ROW - 1));
				bool32 IsLastTable = (TableIndex == (TableCount - 1));
				if(IsLastTableInRow || IsLastTable)
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
	Result.SrcCharsScanned = SrcCharsScanned;
	Result.DestCharsAdded = DestCharsAdded;

	return Result;
}

internal void
ParseConfigFile(char *ConfigFileContents, day *Days)
{
	day *DaysProcessing[7];
	uint32_t DaysProcessingCount = 0;

	time_category *TimeCategoriesProcessing[MAX_TIME_CATEGORIES];
	uint32_t TimeCategoriesProcessingCount = 0;

	config_file_state CurState = CONFIG_FILE_STATE_SEEKING_DAY;
	uint32_t ContentsIndex = 0;
	bool32 IsRunning = true;
	while(IsRunning)
	{
		switch(CurState)
		{
			case CONFIG_FILE_STATE_SEEKING_DAY:
			{
				char DayNameBuffer[MAX_CONFIG_FILE_LINE_SIZE];
				uint32_t TokenLength = GetToken(&ConfigFileContents, DayNameBuffer);
				if(TokenLength > 0)
				{
					if(DayNameBuffer[0] == '{')
					{
						CurState = CONFIG_FILE_STATE_SEEKING_TIME_CATEGORY;
					}
					else if(DayNameBuffer[0] == '$')
					{
						IsRunning = false;
					}
					else
					{
						// figure out which day and add it to DaysProcessing
						for(uint32_t DayIndex = 0;
							DayIndex < NUMBER_OF_DAYS;
							++DayIndex)
						{
							if(Equals(DayNameBuffer, Days[DayIndex].Name))
							{
								DaysProcessing[DaysProcessingCount++] = &Days[DayIndex];

								break;
							}
						}
					}
				}
			} break;

			case CONFIG_FILE_STATE_SEEKING_TIME_CATEGORY:
			{
				char TimeCategoryNameBuffer[MAX_CONFIG_FILE_LINE_SIZE];
				uint32_t TokenLength = GetToken(&ConfigFileContents, TimeCategoryNameBuffer);
				if(TokenLength > 0)
				{
					if(TimeCategoryNameBuffer[0] == '{')
					{
						CurState = CONFIG_FILE_STATE_SEEKING_TIME_CATEGORY_INFO;
					}
					else if(TimeCategoryNameBuffer[0] == '}')
					{
						CurState = CONFIG_FILE_STATE_SEEKING_DAY;
						DaysProcessingCount = 0;
					}
					else
					{
						TimeCategoryNameBuffer[TokenLength] = '\0';

						// add a time category to each of DaysProcessing
						for(uint32_t DaysProcessingIndex = 0;
							DaysProcessingIndex < DaysProcessingCount;
							++DaysProcessingIndex)
						{
							day *Day = DaysProcessing[DaysProcessingIndex];
							time_category *NewTimeCategory = &Day->TimeCategories[Day->TimeCategoryCount++];
							TimeCategoriesProcessing[TimeCategoriesProcessingCount++] = NewTimeCategory;

							Copy(TimeCategoryNameBuffer, NewTimeCategory->Name);
						}
					}
				}
			} break;

			case CONFIG_FILE_STATE_SEEKING_TIME_CATEGORY_INFO:
			{
				char TimeCategoryNameBuffer[MAX_CONFIG_FILE_LINE_SIZE];
				uint32_t TokenLength = GetToken(&ConfigFileContents, TimeCategoryNameBuffer);
				if(TokenLength > 0)
				{
					if(TimeCategoryNameBuffer[0] == '}')
					{
						CurState = CONFIG_FILE_STATE_SEEKING_TIME_CATEGORY;
						TimeCategoriesProcessingCount = 0;
					}
					else
					{
						char EqualsSignBuffer[1];
						GetToken(&ConfigFileContents, EqualsSignBuffer);
						Assert(EqualsSignBuffer[0] == '=');
						if(EqualsSignBuffer[0] != '=')
						{
							fprintf(stderr, "Error parsing config file: '=' missing.\n");
							exit(1);
						}

						if(TimeCategoryNameBuffer[0] == 'T')//argetTime
						{
							char TargetTimeBuffer[5];
							uint32_t CharsGot = GetToken(&ConfigFileContents, TargetTimeBuffer);
							Assert(CharsGot <= ArrayCount(TargetTimeBuffer));

							time_elapse TargetTime = ConvertToTimeElapse(TargetTimeBuffer);
							for(uint32_t TimeCategoriesProcessingIndex = 0;
								TimeCategoriesProcessingIndex < TimeCategoriesProcessingCount;
								++TimeCategoriesProcessingIndex)
							{
								time_category *TimeCategory = TimeCategoriesProcessing[TimeCategoriesProcessingIndex];
								TimeCategory->TargetTime = TargetTime;
							}
						}
						else if(TimeCategoryNameBuffer[0] == 'B')//adCondition
						{
							char BadConditionNameBuffer[16];
							uint32_t CharsGot = GetToken(&ConfigFileContents, BadConditionNameBuffer);
							Assert(CharsGot <= ArrayCount(BadConditionNameBuffer));

							comparison BadCondition;
							if(BadConditionNameBuffer[0] == 'U')//nder
							{
								BadCondition = COMPARISON_LESS;
							}
							else if(BadConditionNameBuffer[0] == 'O')//ver
							{
								BadCondition = COMPARISON_GREATER;
							}
							else if(BadConditionNameBuffer[0] == 'N')//otEqual
							{
								BadCondition = (comparison)(COMPARISON_LESS | COMPARISON_GREATER);
							}
							else
							{
								Assert(false);
								TimeCategoryNameBuffer[TokenLength] = '\0';
								fprintf(stderr, "Error parsing config file: unknown BadCondition value: %s\n", BadConditionNameBuffer);
								exit(1);
							}

							for(uint32_t TimeCategoriesProcessingIndex = 0;
								TimeCategoriesProcessingIndex < TimeCategoriesProcessingCount;
								++TimeCategoriesProcessingIndex)
							{
								time_category *TimeCategory = TimeCategoriesProcessing[TimeCategoriesProcessingIndex];
								TimeCategory->BadCondition = BadCondition;
							}
						}
						else
						{
							Assert(false);
							TimeCategoryNameBuffer[TokenLength] = '\0';
							fprintf(stderr, "Error parsing config file: unknown time_category value: %s\n", TimeCategoryNameBuffer);
							exit(1);
						}
					}
				}
			} break;
		}

		AdvanceToNextLine(&ConfigFileContents);
	}
}

int
main(int argc, char *argv[])
{
	read_entire_file_result ReadConfigFileResult = ReadEntireFile("time_cfg.txt");
	day DaysOfTheWeek[7] = {};
	{
		DaysOfTheWeek[0].Name = "Monday";
		DaysOfTheWeek[1].Name = "Tuesday";
		DaysOfTheWeek[2].Name = "Wednesday";
		DaysOfTheWeek[3].Name = "Thursday";
		DaysOfTheWeek[4].Name = "Friday";
		DaysOfTheWeek[5].Name = "Saturday";
		DaysOfTheWeek[6].Name = "Sunday";
	}
	ParseConfigFile(ReadConfigFileResult.Contents, DaysOfTheWeek);

	read_entire_file_result ReadInputFileResult = ReadEntireFile("time_raw.txt");
	uint32_t InputFileSize = ReadInputFileResult.FileSize;
	char *InputContents = ReadInputFileResult.Contents;
	uint32_t InputFileCharIndex = 0;
	{
		Assert(ReadInputFileResult.FileSize <= MAX_INPUT_FILE_SIZE);
		if(ReadInputFileResult.FileSize > MAX_INPUT_FILE_SIZE)
		{
			fprintf(stderr, "Error: input file too big.\n");
			exit(1);
		}

		Assert((InputContents[0] == ENTRY_PROCESSED_FLAG) ||
			   (InputContents[0] == ENTRY_UNPROCESSED_FLAG));
		if((InputContents[0] != ENTRY_PROCESSED_FLAG) &&
		   (InputContents[0] != ENTRY_UNPROCESSED_FLAG))
		{
			fprintf(stderr, "Error: input file has incorrect format (first character must be '#' or '@')\n");
			exit(1);
		}
	}

	uint32_t OutputFileSize = (InputFileSize * TABLES_TO_ENTRY_RATIO);
	char *OutputContents = (char *)VirtualAlloc(NULL, OutputFileSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	uint32_t OutputContentsSize = 0;

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
															 DaysOfTheWeek);

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
