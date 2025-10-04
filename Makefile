CC = gcc
CFLAGS = -Wall -g -pthread -Iinclude
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
LOGS_DIR = logs

# Criar pastas se não existirem
$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR))

# Arquivos fonte e objetos
OBJS = $(OBJ_DIR)/tslog.o
SERVER_OBJS = $(OBJ_DIR)/server.o
CLIENT_OBJS = $(OBJ_DIR)/client.o

# Alvos principais
all: $(BIN_DIR)/main_server $(BIN_DIR)/main_client

# Logger
$(OBJ_DIR)/tslog.o: $(SRC_DIR)/tslog.c include/tslog.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/tslog.c -o $@

# Servidor
$(OBJ_DIR)/server.o: $(SRC_DIR)/server.c include/server.h include/tslog.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/server.c -o $@

$(BIN_DIR)/main_server: $(SRC_DIR)/main_server.c $(SERVER_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SRC_DIR)/main_server.c $(SERVER_OBJS) $(OBJS)

# Cliente
$(OBJ_DIR)/client.o: $(SRC_DIR)/client.c include/client.h include/tslog.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/client.c -o $@

$(BIN_DIR)/main_client: $(SRC_DIR)/main_client.c $(CLIENT_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SRC_DIR)/main_client.c $(CLIENT_OBJS) $(OBJS)

# Teste do logger
$(BIN_DIR)/log_test: $(SRC_DIR)/log_test.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SRC_DIR)/log_test.c $(OBJS)

# Limpeza
clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/* $(LOGS_DIR)/*.log
