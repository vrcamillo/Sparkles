
#if _WIN32 
#include <windows.h>

void system_sleep_ms(int milliseconds) {
	timeBeginPeriod(1);
	Sleep(milliseconds);
}

#endif