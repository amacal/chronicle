#include "client.h"
#include "log.h"

typedef struct _CLIENT_RECEIVE_COMPLETE
{
	CLIENT_RECEIVE_CALLBACK callback;
	CLIENT_RECEIVE_DATA *data;
} CLIENT_RECEIVE_COMPLETE;

typedef struct _CLIENT_SEND_COMPLETE
{
	CLIENT_SEND_CALLBACK callback;
	CLIENT_SEND_DATA *data;
} CLIENT_SEND_COMPLETE;

typedef struct _CLIENT_WRITTEN_COMPLETE
{
	CLIENT_WRITTEN_CALLBACK callback;
	CLIENT_WRITTEN_DATA *data;
} CLIENT_WRITTEN_COMPLETE;

CLIENT *client_new(ASYNC_SOCKET *socket, PARTITION *partition)
{
	size_t size = sizeof(CLIENT);
	CLIENT *result = malloc(size);

	result->socket = socket;
	result->partition = partition;

	return result;
}

void client_close(CLIENT *client)
{
	free(client);
}

void client_receive_callback(SOCKET_RECEIVE_DATA *data)
{
	CLIENT_RECEIVE_COMPLETE *complete = data->tag;
	CLIENT_RECEIVE_DATA *argument = complete->data;

	if (data->status == 0)
	{
		logger_debug("Client receive completed; calling callback.\n");

		argument->processed = data->processed;
		complete->callback(argument);
	}
	else
	{
		logger_debug("Client receive failed; calling callback; status=%d.\n", data->status);

		argument->status = data->status;
		complete->callback(argument);
	}

	free(argument);
	free(complete);
}

void client_receive(CLIENT *client, BUFFER *buffer, CLIENT_RECEIVE_CALLBACK callback, void *tag)
{
	size_t complete_size = sizeof(CLIENT_RECEIVE_COMPLETE);
	size_t data_size = sizeof(CLIENT_RECEIVE_DATA);

	CLIENT_RECEIVE_COMPLETE *complete = malloc(complete_size);
	CLIENT_RECEIVE_DATA *data = malloc(data_size);

	data->client = client;
	data->buffer = buffer;
	data->tag = tag;

	data->offset = 0;
	data->processed = 0;
	data->status = 0;

	complete->data = data;
	complete->callback = callback;

	socket_receive(client->socket, buffer, client_receive_callback, complete);
}

void client_send_callback(SOCKET_SEND_DATA *data)
{
	CLIENT_SEND_COMPLETE *complete = data->tag;
	CLIENT_SEND_DATA *argument = complete->data;

	if (data->status == 0 && data->processed < data->count)
	{
		int offset = data->offset + data->processed;
		int count = data->count - data->processed;

		argument->processed += data->processed;

		logger_debug("Client send splited; continuing; left=%d\n", count);
		socket_send(data->socket, data->buffer, offset, count, client_send_callback, complete);

		return;
	}
	else if (data->status == 0)
	{
		logger_debug("Client send completed; calling callback.\n");

		argument->processed += data->processed;
		complete->callback(argument);
	}
	else
	{
		logger_debug("Client send failed; calling callback; status=%d.\n", data->status);

		argument->status = data->status;
		complete->callback(argument);
	}

	free(argument);
	free(complete);
}

void client_send(CLIENT *client, BUFFER *buffer, int offset, int count, CLIENT_SEND_CALLBACK callback, void *tag)
{
	size_t complete_size = sizeof(CLIENT_SEND_COMPLETE);
	size_t data_size = sizeof(CLIENT_SEND_DATA);

	CLIENT_SEND_COMPLETE *complete = malloc(complete_size);
	CLIENT_SEND_DATA *data = malloc(data_size);

	data->client = client;
	data->buffer = buffer;
	data->tag = tag;

	data->offset = offset;
	data->count = count;

	data->processed = 0;
	data->status = 0;

	complete->data = data;
	complete->callback = callback;

	socket_send(client->socket, buffer, offset, count, client_send_callback, complete);
}

void client_write_callback(PARTITION_WRITTEN_DATA *data)
{
	CLIENT_WRITTEN_COMPLETE *complete = data->tag;
	CLIENT_WRITTEN_DATA *argument = complete->data;

	if (data->status == 0)
	{
		logger_debug("Client write completed; calling callback.\n");

		argument->processed = data->processed;
		argument->identifier = data->identifier;

		complete->callback(argument);
	}
	else
	{
		logger_debug("Client write failed; calling callback; status=%d.\n", data->status);

		argument->status = data->status;
		complete->callback(argument);
	}

	free(argument);
	free(complete);
}

void client_write(CLIENT *client, BUFFER *buffer, int count, CLIENT_WRITTEN_CALLBACK callback, void *tag)
{
	size_t complete_size = sizeof(CLIENT_WRITTEN_COMPLETE);
	size_t data_size = sizeof(CLIENT_WRITTEN_DATA);

	CLIENT_WRITTEN_COMPLETE *complete = malloc(complete_size);
	CLIENT_WRITTEN_DATA *data = malloc(data_size);

	data->client = client;
	data->buffer = buffer;
	data->tag = tag;

	data->offset = 0;
	data->processed = 0;
	data->status = 0;
	data->count = count;
	data->identifier = 0;

	complete->data = data;
	complete->callback = callback;

	partition_write(client->partition, buffer, count, client_write_callback, complete);
}
