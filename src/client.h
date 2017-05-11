#include "socket.h"
#include "buffer.h"
#include "file.h"

typedef struct _CLIENT
{
	ASYNC_SOCKET *socket;
	ASYNC_FILE *file;
} CLIENT;

typedef struct _CLIENT_RECEIVE_DATA
{
	CLIENT *client;
	BUFFER *buffer;

	int status;
	int processed;

	int offset;
	int count;
} CLIENT_RECEIVE_DATA;

typedef void (*CLIENT_RECEIVE_CALLBACK)(CLIENT_RECEIVE_DATA *data);

void client_receive(CLIENT *client, BUFFER *buffer, CLIENT_RECEIVE_CALLBACK callback);
void client_send(CLIENT *client, BUFFER *buffer, int offset, int count, SOCKET_SEND_CALLBACK callback);

void client_write(CLIENT *client, BUFFER *buffer, int offset, int count, FILE_WRITE_CALLBACK callback);
