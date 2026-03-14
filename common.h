#ifndef COMMON_H
#define COMMON_H

#include "claves.h"

// Códigos de operación para las diferentes funciones de la API
#define OP_DESTROY       0
#define OP_SET_VALUE     1
#define OP_GET_VALUE     2
#define OP_MODIFY_VALUE  3
#define OP_DELETE_KEY    4
#define OP_EXIST         5

// Nombres de colas de mensajes
#define REQUEST_QUEUE "/request_queue"
#define RESPONSE_QUEUE_PREFIX "/response_queue_"

// Tamaño máximo de mensaje
#define MAX_QUEUE_MSG_SIZE 8192

// Estructura de mensaje de petición (Cliente -> Servidor)
typedef struct {
    int operation;      // Código de operación (OP_DESTROY, OP_SET_VALUE, etc.)
    int client_id;      // ID del proceso cliente (para cola de respuesta)
    
    // Campos de datos (usados según la operación)
    char key[256];
    char value1[256];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;
} request_msg;

// Estructura de mensaje de respuesta (Servidor -> Cliente)
typedef struct {
    int result;       // Valor de retorno: 0 (éxito), -1 (error de servicio), -2 (error de comunicación)
    
    // Campos de datos (usados para respuesta de get_value)
    char value1[256];
    int N_value2;
    float V_value2[32];
    struct Paquete value3;
} response_msg;

#endif
