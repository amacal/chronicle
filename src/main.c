#include <stdio.h>

#include "options.h"
#include "log.h"

#include "socket.h"
#include "file.h"
#include "partition.h"

#pragma comment(lib, "ws2_32.lib")

void on_socket_receive(SOCKET_RECEIVE_DATA *data);
void on_socket_send(SOCKET_SEND_DATA *data);
void on_partition_written(PARTITION_WRITTEN_DATA *data);

void on_socket_bound(ASYNC_SOCKET *socket, int port)
{
	logger_info("socket bound on port %d\n", port);
}

void on_socket_accept(ASYNC_SOCKET *socket, int status, ASYNC_SOCKET *accepted)
{
	logger_info("in accept callback; status=%d; accepted=%d\n", status, accepted->handle);
	socket_receive(accepted, buffer_new(1048576), on_socket_receive, socket->tag);

	logger_info("continue listing with %d\n", socket->handle);
	socket_accept(socket, on_socket_accept);
}

void dispose(ASYNC_SOCKET *socket, PARTITION *partition, BUFFER *buffer)
{
	logger_info("disposing socket %d\n", socket->handle);
	socket_close(socket);

	logger_info("disposing buffer %d\n", buffer);
	buffer_free(buffer);
}

void on_socket_receive(SOCKET_RECEIVE_DATA *data)
{
	PARTITION *partition = data->tag;
	ASYNC_SOCKET *socket = data->socket;

	logger_debug("in receive callback; status=%d; processed=%d; tag=%d\n", data->status, data->processed, data->tag);

	if (data->status == 0 && data->processed > 0)
	{
		logger_debug("writing to %d\n", partition->file->handle);
		partition_write(partition, data->buffer, data->processed, on_partition_written, socket);
	}
	else
	{
		dispose(socket, partition, data->buffer);
	}
}

void on_partition_written(PARTITION_WRITTEN_DATA *data)
{
	PARTITION *partition = data->partition;
	ASYNC_SOCKET *socket = data->tag;

	logger_debug("in written callback; status=%d; processed=%d; tag=%d\n", data->status, data->processed, data->tag);

	if (data->status == 0 && data->processed > 0)
	{
		logger_debug("continue sending with %d\n", partition->file->handle);
		socket_send(socket, data->buffer, 0, data->count, on_socket_send, partition);
	}
	else 
	{
		dispose(socket, partition, data->buffer);
	}
}

void on_socket_send(SOCKET_SEND_DATA *data)
{
	PARTITION *partition = data->tag;
	ASYNC_SOCKET *socket = data->socket;

	logger_debug("in send callback; status=%d; processed=%d; tag=%d\n", data->status, data->processed, data->tag);

	if (data->status == 0 && data->processed < data->count)
	{
		int offset = data->offset + data->processed;
		int count = data->count - data->processed;

		logger_debug("continue sending with %d; offset=%d; count=%d\n", socket->handle, offset, count);
		socket_send(socket, data->buffer, offset, count, on_socket_send, partition);
	}
	else if (data->status == 0 && data->processed > 0)
	{
		logger_debug("continue receiving with %d\n", socket->handle);
		socket_receive(socket, data->buffer, on_socket_receive, partition);
	}
	else
	{
		dispose(socket, partition, data->buffer);
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
