#include <string.h>
#include <stdlib.h>

#include "options.h"

OPTIONS options_parse(int argc, char *argv[])
{
	OPTIONS options;
	int position = 0;

	options.port = 0;
	options.log_level = "INFO";
	options.file_path = "z:\\\\file.dat";

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "--port") == 0)
		{
			position = 1;
			continue;
		}

		if (strcmp(argv[i], "--log-level") == 0)
		{
			position = 2;
			continue;
		}

		if (strcmp(argv[i], "--file-path") == 0)
		{
			position = 3;
			continue;
		}

		if (strncmp(argv[i], "--", 2) == 0)
		{
			position = 0;
			continue;
		}

		if (position == 1)
		{
			options.port = atoi(argv[i]);
			continue;
		}

		if (position == 2)
		{
			options.log_level = argv[i];
			continue;
		}

		if (position == 3)
		{
			options.file_path = argv[i];
		}
	}

	return options;
}
