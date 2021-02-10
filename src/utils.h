#ifndef UTILS_HEADER
#define UTILS_HEADER

void split(char **arr, char *str, const char *del);
void process_cmd(char **arr, char *str);
void close_fd(int fd);
int equalStr(const char *s, const char *s2);
void format_result(char *message, struct Result *_result);

#endif
