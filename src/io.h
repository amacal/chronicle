#include <windows.h>

#include "iocp.h"

typedef struct _ASYNC_SOCKET
{
	HANDLE handle;
	COMPLETION_PORT *port;
} ASYNC_SOCKET;

typedef void (*SOCKET_BIND_CALLBACK)(ASYNC_SOCKET *socket, int port);
typedef void (*SOCKET_ACCEPT_CALLBACK)(ASYNC_SOCKET *socket, int status, ASYNC_SOCKET *accepted);
typedef void (*SOCKET_RECEIVE_CALLBACK)(ASYNC_SOCKET *socket, int status, int processed, char *buffer);
typedef void (*SOCKET_SEND_CALLBACK)(ASYNC_SOCKET *socket, int status, int processed, char *buffer);

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
	ASYNC_SOCKET *socket;
	SOCKET_RECEIVE_CALLBACK callback;
	char *buffer;
} ASYNC_SOCKET_RECEIVE_OVERLAPPED;

typedef struct _ASYNC_SOCKET_SEND_OVERLAPPED
{
	ASYNC_OVERLAPPED overlapped;
	ASYNC_SOCKET *socket;
	SOCKET_SEND_CALLBACK callback;
	char *buffer;
} ASYNC_SOCKET_SEND_OVERLAPPED;

typedef struct _ASYNC_FILE
{
	HANDLE handle;
} ASYNC_FILE;

void socket_initialize(void);
ASYNC_SOCKET *socket_new(COMPLETION_PORT *port);
void socket_close(ASYNC_SOCKET *socket);

void socket_bind(ASYNC_SOCKET *socket, SOCKET_BIND_CALLBACK callback);
void socket_listen(ASYNC_SOCKET *socket, int backlog);
void socket_accept(ASYNC_SOCKET *socket, SOCKET_ACCEPT_CALLBACK callback);
void socket_receive(ASYNC_SOCKET *socket, char *buffer, SOCKET_RECEIVE_CALLBACK callback);
void socket_send(ASYNC_SOCKET *socket, char *buffer, int count, SOCKET_SEND_CALLBACK callback);
