#include <windows.h>

typedef void (*COMPLETION_CALLBACK)(OVERLAPPED *overlapped);

typedef struct _COMPLETION_PORT
{
	HANDLE handle;
} COMPLETION_PORT;

typedef struct _ASYNC_OVERLAPPED
{
	OVERLAPPED overlapped;
	COMPLETION_CALLBACK callback;
} ASYNC_OVERLAPPED;

COMPLETION_PORT *iocp_new(void);
void iocp_start(COMPLETION_PORT *port);
