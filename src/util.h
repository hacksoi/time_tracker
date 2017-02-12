#ifndef MISC_HEADER
#define MISC_HEADER

#define bool32 int
#define true 1
#define false 0

#define Kilobytes(NumberOfKbs) (NumberOfKbs * 1024)
#define Megabytes(NumberOfMbs) (NumberOfMbs * 1024 * 1024)
#define Gigabytes(NumberOfGbs) (NumberOfGbs * 1024 * 1024 * 1024)

#define internal static
#define local_persist static
#define global_variable static

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

#define Assert(Expression) if(!(Expression)) { *((int *)0) = 0; }

enum comparison
{
	COMPARISON_LESS = (1 << 0),
	COMPARISON_GREATER = (1 << 1),
	COMPARISON_EQUAL = (1 << 2)
};

#endif
