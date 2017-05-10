#include <winsock2.h>
#include <mswsock.h>

#include "io.h"
#include "log.h"

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

void socket_close(ASYNC_SOCKET *socket)
{
	closesocket((SOCKET)socket->handle);
	free(socket);
}

void socket_bind(ASYNC_SOCKET *socket, int port, SOCKET_BIND_CALLBACK callback)
{
	int result;
	struct sockaddr_in service;
	int size = sizeof(service);

	memset(&service, 0, size);

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = ntohs(port);

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
	free(overlapped);
}

void socket_accept(ASYNC_SOCKET *socket, SOCKET_ACCEPT_CALLBACK callback)
{
	int result;
	DWORD bytes;

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
	int result;

	DWORD flags = 0;
	DWORD bytes = 0;		

    ASYNC_SOCKET_RECEIVE_OVERLAPPED *received = (ASYNC_SOCKET_RECEIVE_OVERLAPPED*)overlapped;
	SOCKET handle = (SOCKET)received->data->socket->handle;

	result = WSAGetOverlappedResult(handle, overlapped, &bytes, 1, &flags);
	received->data->status = WSAGetLastError();
	received->data->processed = bytes;

	logger_debug("IOCP decoded; handle=%d; status=%d; bytes=%d; flags=%d; error=%d\n", handle, result, bytes, flags, received->data->status);
	received->callback(received->data);
}

void socket_receive(ASYNC_SOCKET *socket, BUFFER *buffer, SOCKET_RECEIVE_CALLBACK callback)
{
	int error;
	int result;

	int size_bytes = sizeof(DWORD);
	int size_buffer = sizeof(WSABUF);
	int size_data = sizeof(SOCKET_SEND_DATA);

	int size_overlapped = sizeof(ASYNC_SOCKET_RECEIVE_OVERLAPPED);
	int size_outbound = size_overlapped + size_buffer + size_data + size_bytes * 2;

	char *offset_overlapped = buffer->data + buffer->size - size_outbound;
	ASYNC_SOCKET_RECEIVE_OVERLAPPED *overlapped = (ASYNC_SOCKET_RECEIVE_OVERLAPPED*)offset_overlapped;

	char *offset_data = offset_overlapped + size_overlapped;
	SOCKET_RECEIVE_DATA *data = (SOCKET_RECEIVE_DATA*)offset_data;

	char *offset_buffer = offset_data + size_data;
	WSABUF *buffers = (WSABUF*)offset_buffer;

	char *offset_received = offset_buffer + size_buffer;
	DWORD *received = (DWORD*)offset_received;

	char *offset_flags = offset_received + size_bytes;
	DWORD *flags = (DWORD*)offset_flags;

	SOCKET handle = (SOCKET)socket->handle;
	memset(offset_overlapped, 0, size_outbound);

	buffers[0].buf = buffer->data;
	buffers[0].len = buffer->size - 128;

	overlapped->overlapped.callback = socket_receive_complete;
	overlapped->callback = callback;
	overlapped->data = data;

	data->socket = socket;
	data->buffer = buffer;

	data->offset = 0;
	data->count = buffers[0].len;

	result = WSARecv(handle, buffers, 1, received, flags, (OVERLAPPED*)overlapped, NULL);
	error = WSAGetLastError();

	logger_debug("Socket receiving; status=%d; error=%d\n", result, error);

	if (result == SOCKET_ERROR && error != WSA_IO_PENDING)
	{
		logger_debug("Receiving failed; status=%d; error=%d\n", result, error);

		data->status = error;
		callback(data);
	}
}

void socket_send_complete(OVERLAPPED *overlapped)
{
	int result;

	DWORD flags = 0;
	DWORD bytes = 0;		

    ASYNC_SOCKET_SEND_OVERLAPPED *received = (ASYNC_SOCKET_SEND_OVERLAPPED*)overlapped;
	SOCKET handle = (SOCKET)received->data->socket->handle;

	result = WSAGetOverlappedResult(handle, overlapped, &bytes, 1, &flags);
	received->data->status = WSAGetLastError();
	received->data->processed = bytes;

	logger_debug("IOCP decoded; handle=%d; status=%d; bytes=%d; flags=%d; error=%d\n", handle, result, bytes, flags, received->data->status);
	received->callback(received->data);
}

void socket_send(ASYNC_SOCKET *socket, BUFFER *buffer, int offset, int count, SOCKET_SEND_CALLBACK callback)
{
	int error;
	int result;

	int size_buffer = sizeof(WSABUF);
	int size_data = sizeof(SOCKET_SEND_DATA);

	int size_overlapped = sizeof(ASYNC_SOCKET_SEND_OVERLAPPED);
	int size_outbound = size_overlapped + size_buffer + size_data;

	char *offset_overlapped = buffer->data + buffer->size - size_outbound;
	ASYNC_SOCKET_SEND_OVERLAPPED *overlapped = (ASYNC_SOCKET_SEND_OVERLAPPED*)offset_overlapped;

	char *offset_data = offset_overlapped + size_overlapped;
	SOCKET_SEND_DATA *data = (SOCKET_SEND_DATA*)offset_data;

	char *offset_buffer = offset_data + size_data;
	WSABUF *buffers = (WSABUF*)offset_buffer;

	SOCKET handle = (SOCKET)socket->handle;
	memset(offset_overlapped, 0, size_outbound);

	buffers[0].buf = buffer->data + offset;
	buffers[0].len = count - offset;

	overlapped->overlapped.callback = socket_send_complete;
	overlapped->callback = callback;
	overlapped->data = data;

	data->socket = socket;
	data->buffer = buffer;

	data->offset = offset;
	data->count = count - offset;
	
	result = WSASend(handle, buffers, 1, NULL, 0, (OVERLAPPED*)overlapped, NULL);
	error = WSAGetLastError();

	logger_debug("Socket sending; outbound=%d; status=%d; error=%d; overlapped=%d\n", size_outbound, result, error, overlapped);

	if (result == SOCKET_ERROR && error != WSA_IO_PENDING)
	{
		logger_debug("Sending failed; status=%d; error=%d\n", result, error);

		data->status = error;
		callback(data);
	}
}
