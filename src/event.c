#include <windows.h>
#include "event.h"

EVENT *event_new(BUFFER *buffer, int offset, int length)
{
	size_t size = sizeof(EVENT);
	EVENT *event = malloc(size);

	event->buffer = buffer;

	event->data = buffer->data + offset;
	event->hash = buffer->data + offset;

	event->offset = offset;
	event->length = length;

	return event;
}
