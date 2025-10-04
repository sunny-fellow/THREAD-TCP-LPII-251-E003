#ifndef SERVER_H
#define SERVER_H

#define SERVER_PORT 5000 // porta padrão do servidor

// Inicia o servidor TCP na porta especificada
// Retorna 0 em sucesso, -1 em caso de erro
int server_start(int port);

// Para o servidor, fechando clientes e liberando recursos
void server_stop(void);

#endif // SERVER_H
