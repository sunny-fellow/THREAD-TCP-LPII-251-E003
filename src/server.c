#include "server.h"
#include "tslog.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define BUF_SIZE 1024
#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

typedef struct
{
    char **messages;
    int count;
    int capacity;
    pthread_mutex_t mutex;
} MessageHistory;

static int server_sock;
static int clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static MessageHistory history;

// ------------------- Histórico de mensagens -------------------

static void history_init(MessageHistory *h)
{
    h->count = 0;
    h->capacity = 100;
    h->messages = malloc(sizeof(char *) * h->capacity);
    pthread_mutex_init(&h->mutex, NULL);
}

static void history_add(MessageHistory *h, const char *msg)
{
    pthread_mutex_lock(&h->mutex);
    if (h->count >= h->capacity)
    {
        h->capacity *= 2;
        h->messages = realloc(h->messages, sizeof(char *) * h->capacity);
    }
    h->messages[h->count++] = strdup(msg); // msg já vem com "nome: mensagem"
    pthread_mutex_unlock(&h->mutex);
}

static void history_send_all(MessageHistory *h, int sock)
{
    pthread_mutex_lock(&h->mutex);
    for (int i = 0; i < h->count; i++)
    {
        send(sock, h->messages[i], strlen(h->messages[i]), MSG_NOSIGNAL);
        send(sock, "\n", 1, MSG_NOSIGNAL);
    }
    pthread_mutex_unlock(&h->mutex);
}

static void history_free(MessageHistory *h)
{
    pthread_mutex_lock(&h->mutex);
    for (int i = 0; i < h->count; i++)
        free(h->messages[i]);
    free(h->messages);
    pthread_mutex_unlock(&h->mutex);
    pthread_mutex_destroy(&h->mutex);
}

// ------------------- Funções utilitárias -------------------

static void add_client(int sock)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == 0)
        {
            clients[i] = sock;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

static void remove_client(int sock)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == sock)
        {
            clients[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// envia toda a mensagem de forma segura
static int send_full(int sock, const char *msg)
{
    size_t len = strlen(msg);
    size_t sent = 0;
    while (sent < len)
    {
        ssize_t n = send(sock, msg + sent, len - sent, MSG_NOSIGNAL);
        if (n <= 0)
            return -1;
        sent += n;
    }
    return 0;
}

// retransmite mensagem para todos os outros clientes
static void broadcast(int sender, const char *msg)
{
    history_add(&history, msg);

    int clients_copy[MAX_CLIENTS];
    int count = 0;

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != 0)
            clients_copy[count++] = clients[i];
    }
    pthread_mutex_unlock(&clients_mutex);

    for (int i = 0; i < count; i++)
    {
        int csock = clients_copy[i];
        if (csock != sender)
        {
            if (send_full(csock, msg) < 0)
                tslog_warn("Falha ao enviar mensagem para cliente %d", csock);
        }
    }
}

// ------------------- Thread do cliente -------------------

static void *handle_client(void *arg)
{
    char aux_buf[100];

    int sock = *(int *)arg;
    free(arg);

    tslog_info("Cliente conectado (socket %d)", sock);
    sprintf(aux_buf, "\x1B[34m=== CLIENTE %i CONECTADO ===\x1B[0m", sock);
    broadcast(sock, aux_buf);

    // envia histórico para o novo cliente
    history_send_all(&history, sock);

    char buffer[BUF_SIZE];
    ssize_t n;

    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[n] = '\0';
        tslog_message("%s", buffer);
        broadcast(sock, buffer);
    }

    if (n == 0)
    {
        tslog_info("Cliente desconectado (socket %d)", sock);

        sprintf(aux_buf, "\x1B[34m=== CLIENTE %i DESCONECTOU ===\x1B[0m", sock);
        broadcast(sock, aux_buf);
    }
    else
        tslog_warn("Erro na conexão com cliente %d: %s", sock, strerror(errno));

    close(sock);
    remove_client(sock);
    return NULL;
}

// ------------------- API do servidor -------------------

int server_start(int port)
{
    signal(SIGPIPE, SIG_IGN); // evita crash ao escrever em sockets fechados

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        tslog_error("Falha ao criar socket: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        tslog_error("Erro no bind: %s", strerror(errno));
        close(server_sock);
        return -1;
    }

    if (listen(server_sock, 10) < 0)
    {
        tslog_error("Erro no listen: %s", strerror(errno));
        close(server_sock);
        return -1;
    }

    tslog_info("Servidor iniciado na porta %d", port);

    history_init(&history);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addrlen);
        if (client_sock < 0)
        {
            tslog_warn("Falha no accept: %s", strerror(errno));
            continue;
        }

        add_client(client_sock);

        int *pclient = malloc(sizeof(int));
        if (!pclient)
        {
            tslog_error("Falha ao alocar memória para cliente");
            close(client_sock);
            continue;
        }
        *pclient = client_sock;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, pclient) != 0)
        {
            tslog_error("Falha ao criar thread do cliente");
            free(pclient);
            close(client_sock);
            continue;
        }
        pthread_detach(tid);
    }

    return 0;
}

void server_stop(void)
{
    tslog_info("Encerrando servidor...");

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != 0)
        {
            close(clients[i]);
            clients[i] = 0;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(server_sock);
    history_free(&history);

    tslog_info("Servidor encerrado");
}
