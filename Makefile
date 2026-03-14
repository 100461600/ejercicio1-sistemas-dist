#Makefile

CC = gcc
CFLAGS = -Wall -g
LDFLAGS_RT = -lrt
LDFLAGS_PTHREAD = -lpthread

#Archivos objeto
CLAVES_OBJ = claves.o
PROXY_OBJ = proxy-mq.o

#Bibliotecas
LIB_CLAVES = libclaves.so
LIB_PROXY = libproxyclaves.so

# Ejecutables
SERVER = servidor_mq
CLIENT_LOCAL = app-cliente
CLIENT_DIST = app-cliente-dist

# Compilar todo
all: $(LIB_CLAVES) $(LIB_PROXY) $(SERVER) $(CLIENT_LOCAL) $(CLIENT_DIST)

# Compilar claves.c
$(CLAVES_OBJ): claves.c claves.h
	$(CC) $(CFLAGS) -fPIC -c claves.c -o $(CLAVES_OBJ)

# Crear biblioteca libclaves.so
$(LIB_CLAVES): $(CLAVES_OBJ)
	$(CC) -shared -o $(LIB_CLAVES) $(CLAVES_OBJ)

# Compilar proxy-mq.c
$(PROXY_OBJ): proxy-mq.c claves.h common.h
	$(CC) $(CFLAGS) -fPIC -c proxy-mq.c -o $(PROXY_OBJ)

# Crear biblioteca libproxyclaves.so
$(LIB_PROXY): $(PROXY_OBJ)
	$(CC) -shared -o $(LIB_PROXY) $(PROXY_OBJ) $(LDFLAGS_RT)

# Compilar servidor
$(SERVER): servidor-mq.c $(LIB_CLAVES) claves.h common.h
	$(CC) $(CFLAGS) servidor-mq.c -o $(SERVER) -L. -lclaves $(LDFLAGS_PTHREAD) $(LDFLAGS_RT)

# Compilar cliente no distribuido
$(CLIENT_LOCAL): app-cliente.c $(LIB_CLAVES) claves.h
	$(CC) $(CFLAGS) app-cliente.c -o $(CLIENT_LOCAL) -L. -lclaves

# Compilar cliente distribuido
$(CLIENT_DIST): app-cliente.c $(LIB_PROXY) claves.h
	$(CC) $(CFLAGS) app-cliente.c -o $(CLIENT_DIST) -L. -lproxyclaves $(LDFLAGS_RT)

# Limpiar archivos generados
clean:
	rm -f $(CLAVES_OBJ) $(PROXY_OBJ)
	rm -f $(LIB_CLAVES) $(LIB_PROXY)
	rm -f $(SERVER) $(CLIENT_LOCAL) $(CLIENT_DIST)

# Limpiar colas de mensajes
clean-queues:
	rm -f /dev/mqueue/request_queue /dev/mqueue/response_queue_* 2>/dev/null || true

.PHONY: all clean clean-queues