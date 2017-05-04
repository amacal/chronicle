#include <stdio.h>
#include <stdarg.h>

void logger_debug(const char *format, ...)
{
	va_list args;
  	va_start (args, format);
  	vprintf (format, args);
  	va_end (args);
}
