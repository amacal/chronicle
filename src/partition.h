#ifndef PARTITION_HEADER_INCLUDED
#define PARTITION_HEADER_INCLUDED

#include <windows.h>

#include "file.h"

typedef struct _PARTITION
{
	ASYNC_FILE *file;
	long long position;
} PARTITION;

typedef struct _PARTITION_WRITTEN_DATA
{
	PARTITION *partition;
	BUFFER *buffer;
	void *tag;

	int status;
	int processed;

	long long offset;
	int count;
} PARTITION_WRITTEN_DATA;

typedef void (*PARTITION_WRITTEN_CALLBACK)(PARTITION_WRITTEN_DATA *data);

PARTITION *partition_new(ASYNC_FILE *file);

void partition_write(PARTITION *partition, BUFFER *buffer, int count, PARTITION_WRITTEN_CALLBACK callback, void *tag);

#endif
