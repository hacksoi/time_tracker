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
	bool32 Result;

	if(A.Hour > B.Hour)
	{
		Result = true;
	}
	else if(A.Hour < B.Hour)
	{
		Result = false;
	}
	else
	{
		if(A.Minute > B.Minute)
		{
			Result = true;
		}
		else if(A.Minute < B.Minute)
		{
			Result = false;
		}
		else
		{
			Result = false;
		}
	}

	return Result;
}

inline bool32
operator>(time_elapse A, time_elapse B)
{
	bool32 Result = true;

	if(A.Hours > B.Hours)
	{
		Result = true;
	}
	else if(A.Hours < B.Hours)
	{
		Result = false;
	}
	else
	{
		if(A.Minutes > B.Minutes)
		{
			Result = true;
		}
		else if(A.Minutes < B.Minutes)
		{
			Result = false;
		}
		else
		{
			Result = false;
		}
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

inline comparison
Compare(time_elapse A, time_elapse B)
{
	comparison Result;
	if(A < B)
	{
		Result = COMPARISON_LESS;
	}
	else if(A > B)
	{
		Result = COMPARISON_GREATER;
	}
	else
	{
		Result = COMPARISON_EQUAL;
	}

	return Result;
}
