#ifndef TSLOG_H
#define TSLOG_H

#include <stdio.h>

// Inicializa o logger (arquivo)
int tslog_init(const char *filename);

// Fecha o logger
void tslog_close(void);

// Logs genéricos
void tslog_log(const char *level, const char *fmt, ...);

// Logs específicos
void tslog_info(const char *fmt, ...);
void tslog_warn(const char *fmt, ...);
void tslog_error(const char *fmt, ...);
void tslog_message(const char *fmt, ...);

#endif // TSLOG_H
