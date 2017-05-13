#include "socket.h"
#include "buffer.h"
#include "partition.h"

typedef struct _CLIENT
{
	ASYNC_SOCKET *socket;
	PARTITION *partition;
} CLIENT;

typedef struct _CLIENT_RECEIVE_DATA
{
	CLIENT *client;
	BUFFER *buffer;
	void *tag;

	int status;
	int processed;

	int offset;
	int count;
} CLIENT_RECEIVE_DATA;

typedef struct _CLIENT_SEND_DATA
{
	CLIENT *client;
	BUFFER *buffer;
	void *tag;

	int status;
	int processed;

	int offset;
	int count;
} CLIENT_SEND_DATA;

typedef struct _CLIENT_WRITTEN_DATA
{
	CLIENT *client;
	BUFFER *buffer;
	void *tag;

	int status;
	int processed;

	long long offset;
	int count;
} CLIENT_WRITTEN_DATA;

typedef void (*CLIENT_RECEIVE_CALLBACK)(CLIENT_RECEIVE_DATA *data);
typedef void (*CLIENT_SEND_CALLBACK)(CLIENT_SEND_DATA *data);
typedef void (*CLIENT_WRITTEN_CALLBACK)(CLIENT_WRITTEN_DATA *data);

CLIENT *client_new(ASYNC_SOCKET *socket, PARTITION *partition);
void client_close(CLIENT *client);

void client_receive(CLIENT *client, BUFFER *buffer, CLIENT_RECEIVE_CALLBACK callback, void *tag);
void client_send(CLIENT *client, BUFFER *buffer, int offset, int count, CLIENT_SEND_CALLBACK callback, void *tag);

void client_write(CLIENT *client, BUFFER *buffer, int count, CLIENT_WRITTEN_CALLBACK callback, void *tag);
