#!/bin/bash
# v2_test.sh - Script de teste para servidor e cliente TCP

# ---------------- Configurações ----------------
SERVER_PORT=5000
SERVER_BIN=bin/main_server
CLIENT_BIN=bin/main_client
LOG_FILE=logs/test.log

# ---------------- Limpeza ----------------
make clean
mkdir -p bin logs
rm -f $LOG_FILE

# ---------------- Compilação ----------------
echo "Compilando servidor e cliente..."
make
# gcc -pthread server.c tslog.c -o $SERVER_BIN
# gcc -pthread client.c tslog.c -o $CLIENT_BIN

if [ $? -ne 0 ]; then
    echo "Erro na compilação!"
    exit 1
fi

echo "Compilação concluída."
echo ""

# ---------------- Inicia servidor ----------------
echo "Iniciando servidor na porta $SERVER_PORT..."
$SERVER_BIN $SERVER_PORT &
SERVER_PID=$!
echo "Servidor PID: $SERVER_PID"

# Espera 1 segundo para o servidor iniciar
sleep 1

# ---------------- Teste clientes ----------------
echo ""
echo "Iniciando clientes de teste..."

# Cliente 1 em background
(
echo "Alice"
sleep 1
echo "Oi do cliente 1"
sleep 1
echo "/quit"
) | $CLIENT_BIN 127.0.0.1 $SERVER_PORT &

# Cliente 2 em background
(
echo "Bob"
sleep 1
echo "Olá do cliente 2"
sleep 1
echo "/quit"
) | $CLIENT_BIN 127.0.0.1 $SERVER_PORT &

# Espera os clientes terminarem
wait

# ---------------- Finaliza servidor ----------------
echo "Finalizando servidor..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

echo "Teste concluído. Logs em $LOG_FILE"