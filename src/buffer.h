#ifndef BUFFER_HEADER_INCLUDED
#define BUFFER_HEADER_INCLUDED

typedef struct _BUFFER
{
	char *data;
	int size;
} BUFFER;

BUFFER *buffer_new(int size);
void buffer_free(BUFFER *buffer);

#endif
