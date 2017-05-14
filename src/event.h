#ifndef EVENT_HEADER_INCLUDED
#define EVENT_HEADER_INCLUDED

#include "buffer.h"

typedef struct _EVENT
{
	BUFFER *buffer;
	char *identifier;

	char *hash;
	char *size;
	char *data;

	int offset;
	int length;
} EVENT;

EVENT *event_new(BUFFER *buffer, int offset, int length);

void event_hash(EVENT *event);
void event_identify(EVENT *event, long long identifier);

#endif
