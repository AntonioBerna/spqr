CC=clang
CFLAGS=-Wall -Wextra -Werror -pedantic

SERVER_SCRS=src/server.c src/spqr_server.c src/common.c src/protocol.c -I./include
CLIENT_SCRS=src/client.c src/spqr_client.c src/common.c src/protocol.c -DIS_CLIENT -I./include

BINARY_DIR=bin/
SERVER_TARGET=spqr-server
CLIENT_TARGET=spqr-client

all:
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SERVER_SCRS) -o $(BINARY_DIR)$(SERVER_TARGET)
	$(CC) $(CFLAGS) $(CLIENT_SCRS) -o $(BINARY_DIR)$(CLIENT_TARGET)
	@echo "Build in release mode completed. Run with ./$(BINARY_DIR)$(SERVER_TARGET) and ./$(BINARY_DIR)$(CLIENT_TARGET) <IPv4>"

debug:
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SERVER_SCRS) -DDEBUG -o $(BINARY_DIR)$(SERVER_TARGET)
	$(CC) $(CFLAGS) $(CLIENT_SCRS) -DDEBUG -o $(BINARY_DIR)$(CLIENT_TARGET)
	@echo "Build in debug mode completed. Run with ./$(BINARY_DIR)$(SERVER_TARGET) and ./$(BINARY_DIR)$(CLIENT_TARGET) <IPv4>"

rick:
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SERVER_SCRS) -DRICK_ROLL -o $(BINARY_DIR)$(SERVER_TARGET)
	$(CC) $(CFLAGS) $(CLIENT_SCRS) -DRICK_ROLL -o $(BINARY_DIR)$(CLIENT_TARGET)
	@echo "Build in rick roll mode completed. Run with ./$(BINARY_DIR)$(SERVER_TARGET) and ./$(BINARY_DIR)$(CLIENT_TARGET) <IPv4>"

clean:
	$(RM) -r bin
	@echo "Cleaned up."

.PHONY: all debug rick clean
