#include "client.h"

void client_receive_callback(SOCKET_RECEIVE_DATA *data)
{
}

void client_receive(CLIENT *client, BUFFER *buffer, CLIENT_RECEIVE_CALLBACK callback)
{
	socket_receive(client->socket, buffer, client_receive_callback, NULL);
}
