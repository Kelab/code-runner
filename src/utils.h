#ifndef UTILS_HEADER
#define UTILS_HEADER

void close_fd(int fd);
int equalStr(const char *, const char *s2);
void format_result(char *, struct Result *);
long tv_to_ms(const struct timeval *tv);
long tv_to_us(const struct timeval *tv);

#endif
