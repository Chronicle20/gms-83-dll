#include "logger.h"

void Log(const char* format, ...)
{
	char buf[1024] = { 0 };

	va_list args;
	va_start(args, format);
	vsprintf_s(buf, format, args);

	OutputDebugString(buf);

	va_end(args);
}