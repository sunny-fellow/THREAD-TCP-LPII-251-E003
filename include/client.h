#ifndef CLIENT_H
#define CLIENT_H

#define SERVER_PORT 5000
#define SERVER_IP "127.0.0.1"

// Conecta ao servidor TCP.
// Retorna 0 em sucesso, -1 em erro.
// sock_out será preenchido com o socket criado
int client_connect(const char *ip, int port, int *sock_out);

// Loop principal do cliente para enviar/receber mensagens
void client_run(int sock);

#endif // CLIENT_H
