#include <windows.h>

#include "iocp.h"
#include "buffer.h"

typedef struct _ASYNC_SOCKET
{
	HANDLE handle;
	COMPLETION_PORT *port;
} ASYNC_SOCKET;

typedef struct _SOCKET_RECEIVE_DATA
{
	ASYNC_SOCKET *socket;
	BUFFER *buffer;

	int status;
	int processed;

	int offset;
	int count;
} SOCKET_RECEIVE_DATA;

typedef struct _SOCKET_SEND_DATA
{
	ASYNC_SOCKET *socket;
	BUFFER *buffer;

	int status;
	int processed;

	int offset;
	int count;
} SOCKET_SEND_DATA;

typedef void (*SOCKET_BIND_CALLBACK)(ASYNC_SOCKET *socket, int port);
typedef void (*SOCKET_ACCEPT_CALLBACK)(ASYNC_SOCKET *socket, int status, ASYNC_SOCKET *accepted);
typedef void (*SOCKET_RECEIVE_CALLBACK)(SOCKET_RECEIVE_DATA *data);
typedef void (*SOCKET_SEND_CALLBACK)(SOCKET_SEND_DATA *data);

typedef struct _ASYNC_SOCKET_ACCEPT_OVERLAPPED
{
	ASYNC_OVERLAPPED overlapped;
	ASYNC_SOCKET *socket;
	SOCKET_ACCEPT_CALLBACK callback;
	char *buffer;
	ASYNC_SOCKET *accept;
} ASYNC_SOCKET_ACCEPT_OVERLAPPED;

typedef struct _ASYNC_SOCKET_RECEIVE_OVERLAPPED
{
	ASYNC_OVERLAPPED overlapped;
	SOCKET_RECEIVE_CALLBACK callback;
	SOCKET_RECEIVE_DATA *data;
} ASYNC_SOCKET_RECEIVE_OVERLAPPED;

typedef struct _ASYNC_SOCKET_SEND_OVERLAPPED
{
	ASYNC_OVERLAPPED overlapped;
	SOCKET_SEND_CALLBACK callback;
	SOCKET_SEND_DATA *data;
} ASYNC_SOCKET_SEND_OVERLAPPED;

typedef struct _ASYNC_FILE
{
	HANDLE handle;
} ASYNC_FILE;

void socket_initialize(void);
ASYNC_SOCKET *socket_new(COMPLETION_PORT *port);
void socket_close(ASYNC_SOCKET *socket);

void socket_bind(ASYNC_SOCKET *socket, int port, SOCKET_BIND_CALLBACK callback);
void socket_listen(ASYNC_SOCKET *socket, int backlog);
void socket_accept(ASYNC_SOCKET *socket, SOCKET_ACCEPT_CALLBACK callback);
void socket_receive(ASYNC_SOCKET *socket, BUFFER *buffer, SOCKET_RECEIVE_CALLBACK callback);
void socket_send(ASYNC_SOCKET *socket, BUFFER *buffer, int offset, int count, SOCKET_SEND_CALLBACK callback);
