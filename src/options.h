typedef struct _OPTIONS
{
	int port;
	char *log_level;
	char *file_path;
} OPTIONS;

OPTIONS options_parse(int argc, char *argv[]);
