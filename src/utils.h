#ifndef UTILS_HEADER
#define UTILS_HEADER

void close_fd(int fd);
int equalStr(const char *s, const char *s2);
void format_result(char *message, struct Result *_result);
long tv_to_ms(const struct timeval *tv);
long tv_to_us(const struct timeval *tv);

#endif
