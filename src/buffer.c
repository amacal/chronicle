#include <windows.h>

#include "buffer.h"

BUFFER *buffer_new(int size)
{
	int bytes = sizeof(BUFFER);

	char *data = malloc(bytes + size);
	BUFFER *result = (BUFFER*)data;

	result->data = data + bytes;
	result->size = size;

	return result;
}

void buffer_free(BUFFER *buffer)
{
	free(buffer);
}
