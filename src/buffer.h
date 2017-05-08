typedef struct _BUFFER
{
	char *data;
	int size;
} BUFFER;

BUFFER *buffer_new(int size);
void buffer_free(BUFFER *buffer);
