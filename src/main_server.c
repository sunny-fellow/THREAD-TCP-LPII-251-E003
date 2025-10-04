#include "server.h"
#include "tslog.h"
#include <stdio.h>

int main(void)
{
    if (tslog_init("logs/server.log") != 0)
    {
        fprintf(stderr, "Erro ao inicializar logger\n");
        return 1;
    }

    if (server_start(SERVER_PORT) != 0)
    {
        tslog_error("Falha ao iniciar o servidor");
    }

    server_stop();
    tslog_close();
    return 0;
}
