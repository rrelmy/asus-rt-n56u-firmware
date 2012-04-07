#include <stdio.h>
#include <stdarg.h>

int _xstart(const char *cmd, ...)
{
	va_list ap;
	char *argv[16];
	int argc;
	int pid;

	argv[0] = (char *)cmd;
	argc = 1;
	va_start(ap, cmd);
	while ((argv[argc++] = va_arg(ap, char *)) != NULL) {
		//
	}
	va_end(ap);

	return _eval(argv, NULL, 0, &pid);
}
