#include <winsock2.h>
#include <mswsock.h>

#include "io.h"
#include "log.h"

COMPLETION_PORT *iocp_new(void)
{
	COMPLETION_PORT *result;

	result = malloc(sizeof(COMPLETION_PORT));
	result->handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (u_long) 0, 0);

	return result;
}

void iocp_start(COMPLETION_PORT *port)
{
	int result;
	DWORD bytes;

	unsigned long key;
	ASYNC_OVERLAPPED *received;

	while (1)
	{
		bytes = 0;
		received = NULL;

		result = GetQueuedCompletionStatus(port->handle, &bytes, &key, (OVERLAPPED**)&received, 3000);
		logger_debug("IOCP returned; status=%d; bytes=%d; data=%d; wsa=%d\n", result, bytes, received, WSAGetLastError());

		if (received)
		{
			logger_debug("Handling IOCP callback.\n");
			received->callback((OVERLAPPED*)received);

			logger_debug("Releasing IOCP resource; address=%d.\n", received);
			free(received);

			logger_debug("IOCP callback completed.\n");
		}
	}
}

void socket_initialize(void)
{
	int result;
	WSADATA data;

	result = WSAStartup(MAKEWORD(2, 2), &data);
	logger_debug("WinSock initialized; status=%d\n", result);
}

ASYNC_SOCKET *socket_new(COMPLETION_PORT *port)
{
	ASYNC_SOCKET *result;

	result = malloc(sizeof(ASYNC_SOCKET));
	result->port = port;

	result->handle = (HANDLE)WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	logger_debug("Created new socket %d\n", result->handle);

	CreateIoCompletionPort(result->handle, port->handle, (u_long) 0, 0);
	logger_debug("Attached socket %d to completion port %d\n", result->handle, port->handle);

	return result;
}

void socket_bind(ASYNC_SOCKET *socket, SOCKET_BIND_CALLBACK callback)
{
	int result;
	struct sockaddr_in service;
	int size = sizeof(service);

	memset(&service, 0, size);

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;

	SOCKET handle = (SOCKET)socket->handle;
	SOCKADDR *name = (SOCKADDR*) &service;

	result = bind(handle, name, size);
	logger_debug("Socket bound; status=%d\n", result);

	result = getsockname(handle, name, &size);
	logger_debug("Socket checked; status=%d\n", result);

	callback(socket, ntohs(service.sin_port));
}

void socket_listen(ASYNC_SOCKET *socket, int backlog)
{
	int result;
	SOCKET handle = (SOCKET)socket->handle;

	result = listen(handle, backlog);
	logger_debug("Socket listening; status=%d\n", result);
}

void socket_accept_complete(OVERLAPPED *overlapped)
{
	int error;
	int result;

	DWORD flags = 0;
	DWORD bytes = 0;		

    ASYNC_SOCKET_ACCEPT_OVERLAPPED *received = (ASYNC_SOCKET_ACCEPT_OVERLAPPED*)overlapped;
	SOCKET handle = (SOCKET)received->socket->handle;

	result = WSAGetOverlappedResult(handle, overlapped, &bytes, 1, &flags);
	error = WSAGetLastError();

	logger_debug("IOCP decoded; handle=%d; accept=%d; status=%d; bytes=%d; flags=%d; error=%d\n", handle, received->accept->handle, result, bytes, flags, error);

	if (result == FALSE)
	{
		closesocket((SOCKET)received->accept->handle);
		free(received->accept);
		received->accept = NULL;
	}

	free(received->buffer);
	received->callback(received->socket, error, received->accept);
}


void socket_accept(ASYNC_SOCKET *socket, SOCKET_ACCEPT_CALLBACK callback)
{
	int result;
	DWORD bytes;

    HANDLE port = socket->port;	
	SOCKET handle = (SOCKET)socket->handle;

	ASYNC_SOCKET *accept = socket_new(socket->port);

	LPFN_ACCEPTEX acceptex;
	GUID acceptid = WSAID_ACCEPTEX;

	int name_size = sizeof(struct sockaddr_in) + 16;
	int overlapped_size = sizeof(ASYNC_SOCKET_ACCEPT_OVERLAPPED);

	char *buffer = malloc(name_size * 2 + 4);
	ASYNC_SOCKET_ACCEPT_OVERLAPPED *overlapped = malloc(overlapped_size);

	memset(buffer, 0, name_size * 2 + 4);
	memset(overlapped, 0, overlapped_size);

	overlapped->overlapped.callback = socket_accept_complete;
	overlapped->socket = socket;
	overlapped->buffer = buffer;
	overlapped->callback = callback;
	overlapped->accept = accept;

	result = WSAIoctl(handle, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&acceptid, sizeof (acceptid), 
        &acceptex, sizeof (acceptex), 
        &bytes, NULL, NULL);

	result = acceptex(handle, (SOCKET)accept->handle, buffer, 
		0, name_size, name_size,
		(DWORD*)buffer + name_size * 2, (OVERLAPPED*)overlapped);

	logger_debug("Socket accepting; status=%d; overlapped=%d\n", result, overlapped);
	logger_debug("Socket accepting; status=%d\n", WSAGetLastError());
}

void socket_receive_complete(OVERLAPPED *overlapped)
{
	int error;
	int result;

	DWORD flags = 0;
	DWORD bytes = 0;		

    ASYNC_SOCKET_RECEIVE_OVERLAPPED *received = (ASYNC_SOCKET_RECEIVE_OVERLAPPED*)overlapped;
	SOCKET handle = (SOCKET)received->socket->handle;

	result = WSAGetOverlappedResult(handle, overlapped, &bytes, 1, &flags);
	error = WSAGetLastError();

	logger_debug("IOCP decoded; handle=%d; status=%d; bytes=%d; flags=%d; error=%d\n", handle, result, bytes, flags, error);
	received->callback(received->socket, error, bytes, received->buffer);
}

void socket_receive(ASYNC_SOCKET *socket, char *buffer, SOCKET_RECEIVE_CALLBACK callback)
{
	int result;

	SOCKET handle = (SOCKET)socket->handle;
	LPWSABUF buffers = malloc(sizeof(struct WSABUFF));

	DWORD *received = malloc(sizeof(DWORD));
	DWORD *flags = malloc(sizeof(DWORD));

	int overlapped_size = sizeof(ASYNC_SOCKET_RECEIVE_OVERLAPPED);
	ASYNC_SOCKET_RECEIVE_OVERLAPPED *overlapped = malloc(overlapped_size);

	buffers[0].buf = buffer;
	buffers[0].len = 1024;

	*received = 0;
	*flags = 0;

	memset(overlapped, 0, overlapped_size);
	memset(buffer, 0, 1024);

	overlapped->overlapped.callback = socket_receive_complete;
	overlapped->socket = socket;
	overlapped->buffer = buffer;
	overlapped->callback = callback;

	result = WSARecv(handle, buffers, 1, received, flags, (OVERLAPPED*)overlapped, NULL);

	logger_debug("Socket receiving; status=%d; overlapped=%d\n", result, overlapped);
	logger_debug("Socket receiving; status=%d\n", WSAGetLastError());
}

void socket_send_complete(OVERLAPPED *overlapped)
{
	int error;
	int result;

	DWORD flags = 0;
	DWORD bytes = 0;		

    ASYNC_SOCKET_SEND_OVERLAPPED *received = (ASYNC_SOCKET_SEND_OVERLAPPED*)overlapped;
	SOCKET handle = (SOCKET)received->socket->handle;

	result = WSAGetOverlappedResult(handle, overlapped, &bytes, 1, &flags);
	error = WSAGetLastError();

	logger_debug("IOCP decoded; handle=%d; status=%d; bytes=%d; flags=%d; error=%d\n", handle, result, bytes, flags, error);
	received->callback(received->socket, error, bytes, received->buffer);
}

void socket_send(ASYNC_SOCKET *socket, char *buffer, int count, SOCKET_SEND_CALLBACK callback)
{
	int result;

	SOCKET handle = (SOCKET)socket->handle;
	LPWSABUF buffers = malloc(sizeof(struct WSABUFF));

	DWORD *received = malloc(sizeof(DWORD));
	DWORD flags = 0;

	int overlapped_size = sizeof(ASYNC_SOCKET_SEND_OVERLAPPED);
	ASYNC_SOCKET_SEND_OVERLAPPED *overlapped = malloc(overlapped_size);

	buffers[0].buf = buffer;
	buffers[0].len = count;

	*received = 0;

	memset(overlapped, 0, overlapped_size);

	overlapped->overlapped.callback = socket_send_complete;
	overlapped->socket = socket;
	overlapped->buffer = buffer;
	overlapped->callback = callback;

	result = WSASend(handle, buffers, 1, received, flags, (OVERLAPPED*)overlapped, NULL);

	logger_debug("Socket sending; status=%d; overlapped=%d\n", result, overlapped);
	logger_debug("Socket sending; status=%d\n", WSAGetLastError());
}