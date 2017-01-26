
#pragma once

// reserved keywords
#define new
#define delete



// Debugging Macros
#ifdef DEBUG
	#define Log(x) Print(x)
	#define DebugPrint(x) Print(x)
	#define Assert(x) if(!x){ ThrowException(x); }
#else
	#define Log(x)
	#define Assert(x)
	#define DebugPrint(x)
#endif

