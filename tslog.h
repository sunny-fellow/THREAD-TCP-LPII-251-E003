#ifndef TSLOG_H
#define TSLOG_H

#include <stdio.h>

// Inicializacao do ts logging
int tslog_init(const char *filename);

// Fechar o log
void tslog_close(void);

// Log de mensagem
void tslog_log(const char *level, const char *fmt, ...);

void tslog_info(const char *fmt, ...);
void tslog_warn(const char *fmt, ...);
void tslog_error(const char *fmt, ...);

#endif // TSLOG_H