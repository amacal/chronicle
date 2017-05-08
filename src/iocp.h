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

/*
   Creates new IOCP port and returns a pointer to newly created
   instance. If function fails the null is returned.
*/
COMPLETION_PORT *iocp_new(void);

/*
   Starts processing all messages sent to the completion port.
   The function uses current thread. It blocks and never returns.
*/
void iocp_start(COMPLETION_PORT *port);
