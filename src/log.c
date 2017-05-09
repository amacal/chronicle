#include <stdio.h>
#include <stdarg.h>
#include <string.h>

int logger_level = 0;

void logger_initialize(const char *level)
{
	if (strcmp(level, "INFO") == 0)
		logger_level = 3;

	if (strcmp(level, "DEBUG") == 0)
		logger_level = 4;
}

void logger_info(const char *format, ...)
{
	if (logger_level >= 3)
	{
		va_list args;
  		va_start (args, format);
  		vprintf (format, args);
  		va_end (args);
	}
}

void logger_debug(const char *format, ...)
{
	if (logger_level >= 4)
	{
		va_list args;
  		va_start (args, format);
  		vprintf (format, args);
  		va_end (args);
	}
}
