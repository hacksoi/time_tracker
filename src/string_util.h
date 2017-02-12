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

internal uint32_t
GetToken(char **Src, char *Dest, char ExtraDelimeter = ' ')
{
	uint32_t DestCharsAdded = 0;

	while((**Src == ' ') ||
		  (**Src == '\t'))
	{
		++(*Src);
	}

	while((**Src != ExtraDelimeter) &&
		  (**Src != ' ') &&
		  (**Src != '\t') &&
		  (**Src != '\r') &&
		  (**Src != '\n') &&
		  (**Src != '\0'))
	{
		Dest[DestCharsAdded++] = **Src;
		++(*Src);
	}

	return DestCharsAdded;
}

struct get_token_result
{
	uint32_t SrcCharsScanned;
	uint32_t DestCharsAdded;
};
internal get_token_result
GetToken(char *Src, char *Dest, char ExtraDelimeter = ' ')
{
	get_token_result Result = {};

	char *BaseSrc = Src;
	Result.DestCharsAdded = GetToken(&Src, Dest, ExtraDelimeter);

	uint64_t SrcCharsScanned64 = (Src - BaseSrc);
	uint32_t SrcCharsScanned32 = (uint32_t)SrcCharsScanned64;
	Assert(SrcCharsScanned32 == SrcCharsScanned64);
	if(SrcCharsScanned32 == SrcCharsScanned64)
	{
		Result.SrcCharsScanned = SrcCharsScanned32;
	}

	return Result;
}

// NOTE(hacksoi): stops at null-terminators (so "Antidisestablishmentarianism"
// == "Anti")
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

internal uint32_t
AdvanceToNextLine(char **Src)
{
	uint32_t CharsScanned = 0;
	while(**Src != '\n')
	{
		++(*Src);
		++CharsScanned;
	}
	++(*Src);
	++CharsScanned;

	return CharsScanned;
}

internal uint32_t
CopyLine(char *Src, char *Dest)
{
	uint32_t CharsCopied = 0;
	while((Dest[CharsCopied++] = *Src++) != '\n')
	{
	}

	return CharsCopied;
}

internal void
CopyLine(char *Src, uint32_t *SrcIndex,
		 char *Dest, uint32_t *DestIndex,
		 uint32_t LineCount = 1)
{
	for(uint32_t Line = 0;
		Line < LineCount;
		++Line)
	{
		uint32_t CharsCopied = CopyLine(&Src[*SrcIndex], 
										&Dest[*DestIndex]);
		*SrcIndex += CharsCopied;
		*DestIndex += CharsCopied;
	}
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
