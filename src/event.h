#ifndef EVENT_HEADER_INCLUDED
#define EVENT_HEADER_INCLUDED

#include "buffer.h"

typedef struct _EVENT
{
	BUFFER *buffer;
	long long identifier;
 
	char *data;
	char *hash;

	int offset;
	int length;
} EVENT;

EVENT *event_new(BUFFER *buffer, int offset, int length);

#endif
