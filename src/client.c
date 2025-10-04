#include "client.h"
#include "tslog.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdatomic.h>

#define BUF_SIZE 1024
#define NAME_MAX 32

// flag de controle para a thread de recebimento
static atomic_int running;

// thread para ouvir mensagens do servidor
static void *receive_handler(void *arg)
{
    int sock = *(int *)arg;
    free(arg);

    char buffer[BUF_SIZE];
    ssize_t n;

    while (atomic_load(&running))
    {
        n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n > 0)
        {
            buffer[n] = '\0';

            // reposiciona prompt de entrada
            printf("\33[2K\r%s\n", buffer);
            printf("> ");
            fflush(stdout);
        }
        else if (n == 0)
        {
            tslog_warn("Conexão encerrada pelo servidor");
            break;
        }
        else
        {
            if (errno == EINTR)
                continue;
            tslog_error("Erro no recv: %s", strerror(errno));
            break;
        }
    }
    return NULL;
}

// conecta-se ao servidor TCP
int client_connect(const char *ip, int port, int *sock_out)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        tslog_error("Falha ao criar socket: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        tslog_error("Endereço inválido: %s", ip);
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        tslog_error("Erro ao conectar ao servidor: %s", strerror(errno));
        close(sock);
        return -1;
    }

    tslog_info("Conectado ao servidor %s:%d", ip, port);
    *sock_out = sock;
    return 0;
}

// loop principal do cliente
void client_run(int sock)
{
    char name[NAME_MAX];

    // ---------------- Header ----------------
    printf("========================================\n");
    printf("Cliente TCP simples\n");
    printf("Digite seu nome: ");
    fflush(stdout);
    if (!fgets(name, sizeof(name), stdin))
        strcpy(name, "Anon");
    name[strcspn(name, "\n")] = 0; // remove \n
    if (strlen(name) == 0)
        strcpy(name, "Anon");
    printf("Olá %s!\n\n", name);

    printf("========================================\n");
    printf("/info\t- Lista de comandos\n");
    printf("/quit\t- Sair\n");
    printf("========================================\n\n");

    // ---------------- Thread de recebimento ----------------
    pthread_t recv_thread;
    int *psock = malloc(sizeof(int));
    if (!psock)
    {
        tslog_error("Falha ao alocar memória para thread de recebimento");
        close(sock);
        return;
    }
    *psock = sock;
    atomic_store(&running, 1);

    // thread que recebe mensagens e atualiza prompt
    if (pthread_create(&recv_thread, NULL, receive_handler, psock) != 0)
    {
        tslog_error("Falha ao criar thread de recebimento");
        free(psock);
        close(sock);
        return;
    }

    // ---------------- Envio de mensagens ----------------
    char buffer[BUF_SIZE];
    while (1)
    {
        printf("> "); // prompt de entrada
        fflush(stdout);

        if (!fgets(buffer, BUF_SIZE, stdin))
            break;

        buffer[strcspn(buffer, "\n")] = 0; // remove \n
        if (strlen(buffer) == 0)
            continue;

        // comandos especiais
        if (strcmp(buffer, "/quit") == 0)
            break;
        if (strcmp(buffer, "/info") == 0)
        {
            printf("\n========================================\n");
            printf("/info\t- Lista de comandos\n");
            printf("/quit\t- Sair\n");
            printf("========================================\n\n");
            continue;
        }

        // cria mensagem com nome do usuário
        char msg[BUF_SIZE + NAME_MAX + 4];
        snprintf(msg, sizeof(msg), "[%s]: %s", name, buffer);

        fflush(stdout);

        // envia a mensagem
        ssize_t sent = send(sock, msg, strlen(msg), 0);
        if (sent < 0)
        {
            tslog_error("Falha ao enviar mensagem: %s", strerror(errno));
            break;
        }

        tslog_message("%s", msg);
    }

    // ---------------- Encerramento ----------------
    tslog_warn("Encerrando cliente...");
    atomic_store(&running, 0);
    shutdown(sock, SHUT_RDWR); // força recv a retornar
    pthread_join(recv_thread, NULL);
    close(sock);
    tslog_warn("Cliente encerrado");
}
