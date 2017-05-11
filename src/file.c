#include <windows.h>

#include "file.h"
#include "log.h"

typedef struct _ASYNC_FILE_WRITE_OVERLAPPED
{
	ASYNC_OVERLAPPED overlapped;
	FILE_WRITE_CALLBACK callback;
	FILE_WRITTEN_DATA *data;
} ASYNC_FILE_WRITE_OVERLAPPED;

ASYNC_FILE *file_new(COMPLETION_PORT *port, char *path)
{
	ASYNC_FILE *result;

	result = malloc(sizeof(ASYNC_FILE));
	result->port = port;

	result->handle = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	logger_debug("Created new file %d at %s\n", result->handle, path);

	if (result->handle == INVALID_HANDLE_VALUE)
	{
		result->handle = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_FLAG_OVERLAPPED, NULL);
		logger_debug("Created new file %d at %s\n", result->handle, path);
	}

	CreateIoCompletionPort(result->handle, port->handle, (u_long) 0, 0);
	logger_debug("Attached file %d to completion port %d\n", result->handle, port->handle);

	return result;
}

void file_close(ASYNC_FILE *file)
{
	CloseHandle(file->handle);
	free(file);
}

void file_write_complete(OVERLAPPED *overlapped)
{
	int result;
	DWORD bytes = 0;		

    ASYNC_FILE_WRITE_OVERLAPPED *written = (ASYNC_FILE_WRITE_OVERLAPPED*)overlapped;
	HANDLE handle = written->data->file->handle;

	result = GetOverlappedResult(handle, overlapped, &bytes, FALSE);
	written->data->status = GetLastError();
	written->data->processed = bytes;

	if (bytes > 0)
	{
		written->data->status = 0;
	}

	logger_debug("IOCP decoded; handle=%d; status=%d; bytes=%d; error=%d\n", handle, result, bytes, written->data->status);
	written->callback(written->data);
}

void file_write(ASYNC_FILE *file, long position, BUFFER *buffer, int offset, int count, FILE_WRITE_CALLBACK callback, void *tag)
{
	int error;
	int result;

	int size_data = sizeof(FILE_WRITTEN_DATA);
	int size_overlapped = sizeof(ASYNC_FILE_WRITE_OVERLAPPED);
	int size_outbound = size_overlapped + size_data;

	char *offset_overlapped = buffer->data + buffer->size - size_outbound;
	ASYNC_FILE_WRITE_OVERLAPPED *overlapped = (ASYNC_FILE_WRITE_OVERLAPPED*)offset_overlapped;

	char *offset_data = offset_overlapped + size_overlapped;
	FILE_WRITTEN_DATA *data = (FILE_WRITTEN_DATA*)offset_data;

	memset(offset_overlapped, 0, size_outbound);

	overlapped->overlapped.callback = file_write_complete;
	overlapped->overlapped.overlapped.Offset = position;

	overlapped->callback = callback;
	overlapped->data = data;

	data->file = file;
	data->buffer = buffer;
	data->tag = tag;

	data->offset = offset;
	data->count = count - offset;

	result = WriteFile(file->handle, buffer->data + offset, count, NULL, (OVERLAPPED*)overlapped);
	error = GetLastError();

	logger_debug("File writing; outbound=%d; status=%d; error=%d; overlapped=%d\n", size_outbound, result, error, overlapped);

	if (result == FALSE && error != ERROR_IO_PENDING)
	{
		logger_debug("Writing failed; status=%d; error=%d\n", result, error);

		data->status = error;
		callback(data);
	}
}
