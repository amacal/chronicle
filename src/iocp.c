#include <windows.h>

#include "iocp.h"
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

		result = GetQueuedCompletionStatus(port->handle, &bytes, &key, (OVERLAPPED**)&received, 30000);
		logger_debug("IOCP returned; status=%d; bytes=%d; overlapped=%d\n", result, bytes, received);

		if (received)
		{
			logger_debug("Handling IOCP callback; overlapped=%d\n", received);
			received->callback((OVERLAPPED*)received);
			logger_debug("IOCP callback completed.\n");
		}
	}
}
