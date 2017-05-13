#include <stdio.h>
#include <stdint.h>

#include "options.h"
#include "log.h"

#include "socket.h"
#include "file.h"
#include "partition.h"
#include "client.h"

#pragma comment(lib, "ws2_32.lib")

void on_client_receive(CLIENT_RECEIVE_DATA *data);
void on_client_send(CLIENT_SEND_DATA *data);
void on_client_written(CLIENT_WRITTEN_DATA *data);

void on_socket_bound(ASYNC_SOCKET *socket, int port)
{
	logger_info("socket bound on port %d\n", port);
}

void on_socket_accept(ASYNC_SOCKET *socket, int status, ASYNC_SOCKET *accepted)
{
	BUFFER *buffer = buffer_new(257 * 1024);
	CLIENT *client = client_new(accepted, socket->tag);

	logger_info("in accept callback; status=%d; accepted=%d\n", status, accepted->handle);
	client_receive(client, buffer, on_client_receive, NULL);

	logger_info("continue listening with %d\n", socket->handle);
	socket_accept(socket, on_socket_accept);
}

void dispose(CLIENT *client, BUFFER *buffer)
{
	logger_info("disposing socket %d\n", client->socket->handle);
	socket_close(client->socket);

	logger_info("disposing client %d\n", client);
	client_close(client);

	logger_info("disposing buffer %d\n", buffer);
	buffer_free(buffer);
}

void on_client_receive(CLIENT_RECEIVE_DATA *data)
{
	logger_debug("in receive callback; status=%d; processed=%d; tag=%d\n", data->status, data->processed, data->tag);

	if (data->status == 0 && data->processed > 0)
	{
		logger_debug("writing to %d\n", data->client->partition->file->handle);
		client_write(data->client, data->buffer, data->processed, on_client_written, NULL);
	}
	else
	{
		dispose(data->client, data->buffer);
	}
}

void on_client_written(CLIENT_WRITTEN_DATA *data)
{
	logger_debug("in written callback; status=%d; processed=%d; tag=%d\n", data->status, data->processed, data->tag);

	if (data->status == 0)
	{
		long long identifier = data->identifier;
		char *buffer = data->buffer->data + 7;

		for (int i = 0; i < 8; i++)
		{
			*buffer = (char)(identifier & 0xff);
			buffer--;
			identifier >>= 8;
		}

		logger_debug("responding to %d\n", data->client->partition->file->handle);
		client_send(data->client, data->buffer, 0, 8, on_client_send, NULL);
	}
	else 
	{
		dispose(data->client, data->buffer);
	}
}

void on_client_send(CLIENT_SEND_DATA *data)
{
	logger_debug("in send callback; status=%d; processed=%d; tag=%d\n", data->status, data->processed, data->tag);

	if (data->status == 0 && data->processed > 0)
	{
		logger_debug("receiving from %d\n", data->client->socket->handle);
		client_receive(data->client, data->buffer, on_client_receive, NULL);
	}
	else
	{
		dispose(data->client, data->buffer);
	}
}

int main(int argc, char *argv[])
{
	OPTIONS options = options_parse(argc, argv);

	COMPLETION_PORT *port;
	ASYNC_SOCKET *listener;

	ASYNC_FILE *file;
	PARTITION *partition;

	logger_initialize(options.log_level);
	socket_initialize();

	port = iocp_new();
	file = file_new(port, options.file_path);
	partition = partition_new(file);
	listener = socket_new(port, partition);

	socket_bind(listener, options.port, on_socket_bound);
	socket_listen(listener, 10);
	socket_accept(listener, on_socket_accept);

	iocp_start(port);

    return 0;
}
