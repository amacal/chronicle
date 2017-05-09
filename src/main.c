#include <stdio.h>

#include "io.h"

#pragma comment(lib, "ws2_32.lib")

void on_socket_receive(SOCKET_RECEIVE_DATA *data);
void on_socket_send(SOCKET_SEND_DATA *data);

void on_socket_bound(ASYNC_SOCKET *socket, int port)
{
	printf("socket bound on port %d\n", port);
}

void on_socket_accept(ASYNC_SOCKET *socket, int status, ASYNC_SOCKET *accepted)
{
	printf("in accept callback; status=%d; accepted=%d\n", status, accepted->handle);
	socket_receive(accepted, buffer_new(4 * 1048 * 1024), on_socket_receive);

	printf("continue listing with %d\n", socket->handle);
	socket_accept(socket, on_socket_accept);
}

void on_socket_receive(SOCKET_RECEIVE_DATA *data)
{
	printf("in receive callback; status=%d; processed=%d\n", data->status, data->processed);

	if (data->processed > 0)
	{
		printf("responding to %d\n", data->socket->handle);
		socket_send(data->socket, data->buffer, 0, data->processed, on_socket_send);
	}
	else
	{
		printf("disposing %d\n", data->socket->handle);
		socket_close(data->socket);
		buffer_free(data->buffer);
	}
}

void on_socket_send(SOCKET_SEND_DATA *data)
{
	printf("in send callback; status=%d; processed=%d\n", data->status, data->processed);

	if (data->processed > 0)
	{
		printf("continue receiving with %d\n", data->socket->handle);
		socket_receive(data->socket, data->buffer, on_socket_receive);
	}
	else if (data->processed < data->count)
	{
		int offset = data->offset + data->processed;
		int count = data->count - data->processed;

		printf("continue sending with %d; offset=%d; count=%d\n", data->socket->handle, offset, count);
		socket_send(data->socket, data->buffer, offset, count, on_socket_send);
	}
	else
	{
		printf("disposing %d\n", data->socket->handle);
		socket_close(data->socket);
		buffer_free(data->buffer);
	}
}

int main(int argc, char *argv[])
{
	COMPLETION_PORT *port;
	ASYNC_SOCKET *listener;

	socket_initialize();

	port = iocp_new();
	listener = socket_new(port);

	socket_bind(listener, on_socket_bound);
	socket_listen(listener, 10);
	socket_accept(listener, on_socket_accept);

	iocp_start(port);

    return 0;
}
