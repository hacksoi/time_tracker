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

enum config_file_state
{
	CONFIG_FILE_STATE_SEEKING_DAY,
	CONFIG_FILE_STATE_SEEKING_TIME_CATEGORY,
	CONFIG_FILE_STATE_SEEKING_TIME_CATEGORY_INFO
};

enum comparsion
{
	COMPARISON_LESS = (1 << 0),
	COMPARISON_GREATER = (1 << 1),
	COMPARISON_EQUAL = (1 << 2)
};

struct number2_char
{
	char FirstDigit;
	char SecondDigit;
};

union time_characters
{
	struct
	{
		number2_char Hour;
		number2_char Minute;
	};

	struct
	{
		number2_char Hours;
		number2_char Minutes;
	};
};

struct entry
{
	time StartTime;
	char CategoryName[MAX_CATEGORY_NAME];
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

	comparision GoodCondition;
	char Name[MAX_TABLE_NAME_SIZE];
};

struct day
{
	time_category TimeCategories[MAX_TIME_CATEGORIES];
	uint32_t TimeCategoryCount;

	char *Name;
};

struct process_day_result
{
	uint32_t SrcCharsProcessed;
	uint32_t DestCharsAdded;
};

struct copy_day_to_last_entry_result
{
	uint32_t CharsCopied;
	bool32 IsLastDay;
};
