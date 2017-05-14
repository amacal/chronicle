#ifndef PARTITION_HEADER_INCLUDED
#define PARTITION_HEADER_INCLUDED

#include "event.h"
#include "file.h"

typedef struct _PARTITION
{
	ASYNC_FILE *file;
	long long position;
	long long sequence;
} PARTITION;

typedef struct _PARTITION_WRITTEN_DATA
{
	PARTITION *partition;
	EVENT *event;

	void *tag;
	int status;

	long long offset;
	int count;
} PARTITION_WRITTEN_DATA;

typedef void (*PARTITION_WRITTEN_CALLBACK)(PARTITION_WRITTEN_DATA *data);

PARTITION *partition_new(ASYNC_FILE *file);
long long partition_next(PARTITION *partition);

void partition_write(PARTITION *partition, EVENT *event, PARTITION_WRITTEN_CALLBACK callback, void *tag);

#endif
