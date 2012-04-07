#include <stdio.h>

#define xstart(args...)	_xstart(args, NULL)
extern int _xstart(const char *cmd, ...);
