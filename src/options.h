typedef struct _OPTIONS
{
	int port;
	char *log_level;
} OPTIONS;

OPTIONS options_parse(int argc, char *argv[]);
