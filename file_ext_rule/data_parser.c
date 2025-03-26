#include "data_parse.h"

char *my_strtok(char *str, const char *delim) {
    static char *saved_str = NULL;
    char *start = NULL;
    char *end = NULL;

    if (str != NULL) {
        saved_str = str;
    }

    if (saved_str == NULL) {
        return NULL;
    }

    start = saved_str;
    while (*start && strchr(delim, *start) != NULL) {
        start++;
    }

    if (*start == '\0') {
        saved_str = NULL;
        return NULL;
    }

    end = start;
    while (*end && strchr(delim, *end) == NULL) {
        end++;
    }

    if (*end != '\0') {
        *end = '\0';
        saved_str = end + 1;
    } else {
        saved_str = NULL;
    }

    return start;
}

void parsePaths(const char *input, char source_path[MAX_LINES][PATH_MAX], char redirect_path[MAX_LINES][PATH_MAX], int *line_count) {
    char temp[MAX_INPUT_SIZE];
    strncpy(temp, input, MAX_INPUT_SIZE - 1);
    temp[MAX_INPUT_SIZE - 1] = '\0'; // 确保字符串以 '\0' 结尾

    char *line = my_strtok(temp, "\n");
    *line_count = 0;

    while (line != NULL) {
        char *arrow = strstr(line, " -> ");
        if (arrow != NULL) {
            // 分隔符前后的路径
            size_t path1_len = arrow - line;
            if (path1_len < PATH_MAX) {
                strncpy(source_path[*line_count], line, path1_len);
                source_path[*line_count][path1_len] = '\0'; // 确保字符串以 '\0' 结尾
            }

            // 确保不越界
            if (strlen(arrow + 4) < PATH_MAX) {
                strncpy(redirect_path[*line_count], arrow + 4, PATH_MAX - 1);
                redirect_path[*line_count][PATH_MAX - 1] = '\0'; // 确保字符串以 '\0' 结尾
            }

            (*line_count)++;
        }
        line = my_strtok(NULL, "\n");
    }
}
