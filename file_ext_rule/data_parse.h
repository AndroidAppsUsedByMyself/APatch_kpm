#ifndef SYS_SECURITY_DATA_PARSE_H
#define SYS_SECURITY_DATA_PARSE_H

#include <linux/string.h>
#include "file.h"

void parsePaths(const char *input, char source_path[MAX_LINES][PATH_MAX], char redirect_path[MAX_LINES][PATH_MAX], int *line_count);

#endif
