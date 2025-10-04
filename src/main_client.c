#include "client.h"
#include "tslog.h"
#include <stdio.h>

int main(void)
{
    if (tslog_init("logs/client.log") != 0)
    {
        fprintf(stderr, "Erro ao inicializar logger\n");
        return 1;
    }

    int sock;
    if (client_connect(SERVER_IP, SERVER_PORT, &sock) == 0)
    {
        client_run(sock);
    }

    tslog_close();
    return 0;
}
