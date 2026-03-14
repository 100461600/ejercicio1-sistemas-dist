# 🗄️ Sistema de Almacenamiento Clave-Valor Distribuido

Sistema de almacenamiento de tuplas `<clave, valor1, valor2, valor3>` con soporte para versiones local y distribuida mediante colas de mensajes POSIX.

## 📋 Características

- **Versión local**: Biblioteca compartida (`libclaves.so`)
- **Versión distribuida**: Arquitectura cliente-servidor con colas de mensajes
- **Servidor concurrente**: Manejo de múltiples clientes simultáneos con hilos
- **Sin límites**: Almacenamiento dinámico basado en listas enlazadas

## 🛠️ Compilación

```bash
make          # Compilar todo
make clean    # Limpiar archivos generados
```

## 🚀 Ejecución

### Versión Local (No Distribuida)

```bash
LD_LIBRARY_PATH=. ./app-cliente
```

### Versión Distribuida (Colas de Mensajes)

**Terminal 1** - Servidor:
```bash
LD_LIBRARY_PATH=. ./servidor_mq
```

**Terminal 2** - Cliente:
```bash
LD_LIBRARY_PATH=. ./app-cliente-dist
```

## 📦 Estructura del Proyecto

```
├── claves.h           # API del servicio
├── claves.c           # Implementación (lista enlazada)
├── common.h           # Protocolo de mensajes
├── servidor-mq.c      # Servidor concurrente
├── proxy-mq.c         # Proxy cliente para colas
├── app-cliente.c      # Programa de pruebas
└── Makefile           # Sistema de compilación
```

## 🔧 API

```c
int destroy(void);
int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3);
int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3);
int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3);
int delete_key(char *key);
int exist(char *key);
```

## 📊 Códigos de Retorno

- `0` - Operación exitosa
- `-1` - Error del servicio (clave no existe, parámetros inválidos)
- `-2` - Error de comunicación (solo versión distribuida)

## 🧪 Pruebas

El cliente ejecuta automáticamente 14 pruebas que validan:
- Inserción y recuperación de tuplas
- Modificación y eliminación
- Validación de parámetros
- Manejo de errores

## 📝 Requisitos

- Linux con soporte para colas de mensajes POSIX
- GCC
- pthread
- Acceso a `/dev/mqueue`

---

**Sistemas Distribuidos** • Universidad Carlos III de Madrid • 2025-2026
