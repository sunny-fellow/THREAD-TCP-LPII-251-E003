#!/bin/bash
# v2_test_terminals.sh - Script de teste para servidor e clientes TCP em múltiplos terminais

# ---------------- Configurações ----------------
SERVER_PORT=5000
SERVER_BIN=bin/main_server
CLIENT_BIN=bin/main_client
LOG_FILE=logs/test.log
TERMINAL_CMD="gnome-terminal --" # ou use xterm -e

# ---------------- Limpeza ----------------
make clean
mkdir -p bin logs

sleep 1

# ---------------- Compilação ----------------
echo "Compilando servidor e cliente..."
make

if [ $? -ne 0 ]; then
    echo "Erro na compilação!"
    exit 1
fi

echo "Compilação concluída."

# ---------------- Inicia servidor ----------------
echo "Iniciando servidor na porta $SERVER_PORT..."
$SERVER_BIN $SERVER_PORT &
SERVER_PID=$!
echo "Servidor PID: $SERVER_PID"

# Espera 1 segundo para o servidor iniciar
sleep 1

# ---------------- Teste clientes em terminais separados ----------------
echo "Abrindo terminais para clientes de teste..."

# Cliente 1
$TERMINAL_CMD bash -c "echo 'Cliente 1'; $CLIENT_BIN 127.0.0.1 $SERVER_PORT; exec bash" &

# Cliente 2
$TERMINAL_CMD bash -c "echo 'Cliente 2'; $CLIENT_BIN 127.0.0.1 $SERVER_PORT; exec bash" &

# Cliente 3 (opcional)
$TERMINAL_CMD bash -c "echo 'Cliente 3'; $CLIENT_BIN 127.0.0.1 $SERVER_PORT; exec bash" &

# ---------------- Espera até que o usuário finalize manualmente ----------------
echo "Clientes iniciados em terminais separados."
echo "Pressione Ctrl+C para encerrar o servidor."

# Mantém o script rodando até o usuário encerrar
trap "echo 'Finalizando servidor...'; kill $SERVER_PID; wait $SERVER_PID 2>/dev/null; exit" SIGINT

while true; do sleep 1; done
