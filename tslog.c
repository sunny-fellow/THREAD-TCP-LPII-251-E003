#include "tslog.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static FILE *log_fp = NULL;
static pthread_mutex_t log_mutex;

// Inicialização do arquivo
int tslog_init(const char *filename)
{
    log_fp = fopen(filename, "a");
    if (!log_fp)
        return -1;
    if (pthread_mutex_init(&log_mutex, NULL) != 0)
    {
        fclose(log_fp);
        log_fp = NULL;
        return -1;
    }
    return 0;
}

// Fecha o arquivo
void tslog_close(void)
{
    if (log_fp)
    {
        fclose(log_fp);
        log_fp = NULL;
    }
    pthread_mutex_destroy(&log_mutex);
}

// Escrita das outputs
static void tslog_vlog(const char *level, const char *fmt, va_list args)
{
    if (!log_fp)
        return;

    pthread_mutex_lock(&log_mutex);

    // Timestamp
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_now);

    // Thread ID
    pid_t tid = (pid_t)syscall(186); // Linux: gettid

    // Escrita no terminal
    printf("[%s] [TID %d] [%s] ", buf, tid, level);
    vprintf(fmt, args);
    printf("\n");

    // Escrita no arquivp
    fprintf(log_fp, "[%s] [TID %d] [%s] ", buf, tid, level);
    vfprintf(log_fp, fmt, args);
    fprintf(log_fp, "\n");
    fflush(log_fp);

    pthread_mutex_unlock(&log_mutex);
}

// Log generalizado
void tslog_log(const char *level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    tslog_vlog(level, fmt, args);
    va_end(args);
}

// Logs para contextos especificos
void tslog_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    tslog_vlog("INFO", fmt, args);
    va_end(args);
}

void tslog_warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    tslog_vlog("WARN", fmt, args);
    va_end(args);
}

void tslog_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    tslog_vlog("ERROR", fmt, args);
    va_end(args);
}
