#include <stdio.h>

#include "io.h"

#pragma comment(lib, "ws2_32.lib")

void on_socket_receive(ASYNC_SOCKET *socket, int status, int processed, char *buffer);
void on_socket_send(ASYNC_SOCKET *socket, int status, int processed, char *buffer);

void on_socket_bound(ASYNC_SOCKET *socket, int port)
{
	printf("socket bound on port %d\n", port);
}

void on_socket_accept(ASYNC_SOCKET *socket, int status, ASYNC_SOCKET *accepted)
{
	printf("in accept callback; status=%d; accepted=%d\n", status, accepted->handle);
	socket_receive(accepted, malloc(1024), on_socket_receive);

	printf("continue listing with %d\n", socket->handle);
	socket_accept(socket, on_socket_accept);
}

void on_socket_receive(ASYNC_SOCKET *socket, int status, int processed, char *buffer)
{
	printf("in receive callback; status=%d; processed=%d\n", status, processed);

	if (processed > 0)
	{
		printf("in receive callback: message=%s\n", buffer);
		printf("responding to %d\n", socket->handle);
		socket_send(socket, buffer, processed, on_socket_send);
	}
	else
	{
		printf("disposing %d\n", socket->handle);
		socket_close(socket);
		free(buffer);
	}
}

void on_socket_send(ASYNC_SOCKET *socket, int status, int processed, char *buffer)
{
	printf("in send callback; status=%d; processed=%d\n", status, processed);

	if(processed > 0)
	{
		printf("continue receiving with %d\n", socket->handle);
		socket_receive(socket, buffer, on_socket_receive);
	}
	else
	{
		printf("disposing %d\n", socket->handle);
		socket_close(socket);
		free(buffer);
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
