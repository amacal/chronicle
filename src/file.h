#ifndef FILE_HEADER_INCLUDED
#define FILE_HEADER_INCLUDED

#include <windows.h>

#include "iocp.h"
#include "buffer.h"

typedef struct _ASYNC_FILE
{
	HANDLE handle;
	COMPLETION_PORT *port;
} ASYNC_FILE;

typedef struct _FILE_WRITTEN_DATA
{
	ASYNC_FILE *file;
	BUFFER *buffer;
	void *tag;

	int status;
	int processed;

	int offset;
	int count;
} FILE_WRITTEN_DATA;

typedef void (*FILE_WRITE_CALLBACK)(FILE_WRITTEN_DATA *data);

ASYNC_FILE *file_new(COMPLETION_PORT *port, char *path);
void file_close(ASYNC_FILE *file);

void file_write(ASYNC_FILE *file, long position, BUFFER *buffer, int offset, int count, FILE_WRITE_CALLBACK callback, void *tag);

#endif
