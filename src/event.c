#include <windows.h>
#include "event.h"

EVENT *event_new(BUFFER *buffer, int offset, int length)
{
	size_t size = sizeof(EVENT);
	EVENT *event = malloc(size);

	event->buffer = buffer;

	event->data = buffer->data + offset;
	event->size = event->data - 2;
	event->hash = event->size - 8;
	event->identifier = event->hash - 8; 

	event->offset = offset - 18;
	event->length = length + 18;

	return event;
}

void event_hash(EVENT *event)
{
	char *data = event->data;
	int length = event->length - 18;
	unsigned long long hash = 0xda0105c5;

	for (int i = 0; i < length; i++)
    {
        hash = ((hash << 1) + hash + *data);
		data++;
    }

	data = event->hash + 7;
	for (int i = 0; i < 8; i++)
    {
		*data = (char)(hash & 0xff);
		data--;
		hash >>= 8;
    }
}

void event_identify(EVENT *event, long long identifier)
{
	char *data = event->identifier + 7;
	long long value = identifier;

	for (int i = 0; i < 8; i++)
    {
		*data = (char)(value & 0xff);
		data--;
		value >>= 8;
    }

	data = event->size + 1;
	value = event->length - 18;

	for (int i = 0; i < 2; i++)
    {
		*data = (char)(value & 0xff);
		data--;
		value >>= 8;
    }
}
