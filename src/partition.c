#include "partition.h"
#include "log.h"

typedef struct _PARTITION_WRITE_COMPLETE
{
	PARTITION_WRITTEN_CALLBACK callback;
	PARTITION_WRITTEN_DATA *data;
} PARTITION_WRITE_COMPLETE;

PARTITION *partition_new(ASYNC_FILE *file)
{
	size_t size = sizeof(PARTITION);
	PARTITION *partition = malloc(size);

	partition->file = file;
	partition->position = 0;

	return partition;
}

void partition_write_complete(FILE_WRITTEN_DATA *data)
{
	PARTITION_WRITE_COMPLETE *complete = data->tag;
	PARTITION_WRITTEN_DATA *argument = complete->data;

	if (data->status == 0 && data->processed < data->count)
	{
		int position = argument->offset + data->processed;
		int offset = data->offset + data->processed;
		int count = data->count - data->processed;

		logger_debug("Partition write splited; continuing; left=%d\n", count);
		file_write(data->file, position, data->buffer, offset, count, partition_write_complete, complete);

		return;
	}
	else if (data->status == 0)
	{
		logger_debug("Partition write completed; calling callback.\n");

		argument->processed += data->processed;
		complete->callback(argument);
	}
	else
	{
		logger_debug("Partition write failed; calling callback; status=%d.\n", data->status);

		argument->status = data->status;
		complete->callback(argument);
	}

	free(argument);
	free(complete);
}

void partition_write(PARTITION *partition, BUFFER *buffer, int count, PARTITION_WRITTEN_CALLBACK callback, void *tag)
{
	size_t complete_size = sizeof(PARTITION_WRITE_COMPLETE);
	size_t data_size = sizeof(PARTITION_WRITTEN_DATA);

	PARTITION_WRITE_COMPLETE *complete = malloc(complete_size);
	PARTITION_WRITTEN_DATA *data = malloc(data_size);

	long previous = partition->position;
	long next = previous + count;

	data->partition = partition;
	data->buffer = buffer;

	data->tag = tag;
	data->processed = 0;
	data->status = 0;

	data->offset = previous;
	data->count = count;
	
	complete->data = data;
	complete->callback = callback;

	logger_debug("Moving partition position to %d\n", next);
	partition->position = next;

	logger_debug("Writing into file.\n");
	file_write(partition->file, previous, buffer, 0, count, partition_write_complete, complete);
}
