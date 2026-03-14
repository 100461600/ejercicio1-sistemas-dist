#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "claves.h"
#include "common.h"

// Variables globales para este cliente
static mqd_t cola_peticiones = (mqd_t)-1;
static mqd_t cola_respuestas = (mqd_t)-1;
static int id_cliente = 0;
static char nombre_cola_respuestas[256];

/**
 * Inicializar cliente - abrir colas
 * Devuelve 0 si tiene éxito, -2 en caso de error de comunicación
 */
static int inicializar_cliente(void) {
    struct mq_attr attr;
    
    // Obtener ID del cliente (ID del proceso)
    if (id_cliente == 0) {
        id_cliente = getpid();
        snprintf(nombre_cola_respuestas, sizeof(nombre_cola_respuestas), 
                 "%s%d", RESPONSE_QUEUE_PREFIX, id_cliente);
    }
    
    // Abrir cola de peticiones (para enviar al servidor)
    if (cola_peticiones == (mqd_t)-1) {
        cola_peticiones = mq_open(REQUEST_QUEUE, O_WRONLY);
        if (cola_peticiones == (mqd_t)-1) {
            perror("[PROXY] Error al abrir cola de peticiones");
            return -2;
        }
    }
    
    // Crear cola de respuestas (para recibir del servidor)
    if (cola_respuestas == (mqd_t)-1) {
        // Eliminar cola antigua si existe
        mq_unlink(nombre_cola_respuestas);
        
        // Configurar atributos
        attr.mq_flags = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = sizeof(response_msg);
        attr.mq_curmsgs = 0;
        
        cola_respuestas = mq_open(nombre_cola_respuestas, O_CREAT | O_RDONLY, 0666, &attr);
        if (cola_respuestas == (mqd_t)-1) {
            perror("[PROXY] Error al crear cola de respuestas");
            return -2;
        }
    }
    
    return 0;
}

/**
 * Enviar petición y esperar respuesta
 * Devuelve 0 si tiene éxito, -2 en caso de error de comunicación
 */
static int enviar_peticion_y_esperar(request_msg* req, response_msg* res) {
    // Inicializar colas del cliente si es necesario
    if (inicializar_cliente() == -2) {
        return -2;
    }
    
    // Establecer ID de cliente en la petición
    req->client_id = id_cliente;
    
    // Enviar petición al servidor
    if (mq_send(cola_peticiones, (char*)req, sizeof(request_msg), 0) == -1) {
        perror("[PROXY] Error al enviar petición");
        return -2;
    }
    
    // Esperar respuesta
    ssize_t bytes_leidos = mq_receive(cola_respuestas, (char*)res, sizeof(response_msg), NULL);
    if (bytes_leidos == -1) {
        perror("[PROXY] Error al recibir respuesta");
        return -2;
    }
    
    return 0;
}

/**
 * destroy - Inicializar/limpiar el servicio
 */
int destroy(void) {
    request_msg req;
    response_msg res;
    int resultado_comunicacion;
    
    // Preparar petición
    memset(&req, 0, sizeof(request_msg));
    req.operation = OP_DESTROY;
    
    // Enviar y esperar
    resultado_comunicacion = enviar_peticion_y_esperar(&req, &res);
    if (resultado_comunicacion == -2) {
        return -2;
    }
    
    return res.result;
}

/**
 * set_value - Insertar una nueva tupla
 */
int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    request_msg req;
    response_msg res;
    int resultado_comunicacion;
    
    // Validar parámetros antes de enviar
    if (strlen(key) > 255) {
        return -1;
    }
    if (strlen(value1) > 255) {
        return -1;
    }
    if (N_value2 < 1 || N_value2 > 32) {
        return -1;
    }
    
    // Preparar petición
    memset(&req, 0, sizeof(request_msg));
    req.operation = OP_SET_VALUE;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    strncpy(req.value1, value1, 255);
    req.value1[255] = '\0';
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, N_value2 * sizeof(float));
    req.value3 = value3;
    
    // Enviar y esperar
    resultado_comunicacion = enviar_peticion_y_esperar(&req, &res);
    if (resultado_comunicacion == -2) {
        return -2;
    }
    
    return res.result;
}

/**
 * get_value - Recuperar valores asociados a una clave
 */
int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    request_msg req;
    response_msg res;
    int resultado_comunicacion;
    
    // Validar parámetros
    if (strlen(key) > 255) {
        return -1;
    }
    
    // Preparar petición
    memset(&req, 0, sizeof(request_msg));
    req.operation = OP_GET_VALUE;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    
    // Enviar y esperar
    resultado_comunicacion = enviar_peticion_y_esperar(&req, &res);
    if (resultado_comunicacion == -2) {
        return -2;
    }
    
    // Si tiene éxito, copiar datos de respuesta a parámetros de salida
    if (res.result == 0) {
        strncpy(value1, res.value1, 256);
        *N_value2 = res.N_value2;
        memcpy(V_value2, res.V_value2, res.N_value2 * sizeof(float));
        *value3 = res.value3;
    }
    
    return res.result;
}

/**
 * modify_value - Modificar valores asociados a una clave
 */
int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    request_msg req;
    response_msg res;
    int resultado_comunicacion;
    
    // Validar parámetros antes de enviar
    if (strlen(key) > 255) {
        return -1;
    }
    if (strlen(value1) > 255) {
        return -1;
    }
    if (N_value2 < 1 || N_value2 > 32) {
        return -1;
    }
    
    // Preparar petición
    memset(&req, 0, sizeof(request_msg));
    req.operation = OP_MODIFY_VALUE;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    strncpy(req.value1, value1, 255);
    req.value1[255] = '\0';
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, N_value2 * sizeof(float));
    req.value3 = value3;
    
    // Enviar y esperar
    resultado_comunicacion = enviar_peticion_y_esperar(&req, &res);
    if (resultado_comunicacion == -2) {
        return -2;
    }
    
    return res.result;
}

/**
 * delete_key - Eliminar una tupla por clave
 */
int delete_key(char *key) {
    request_msg req;
    response_msg res;
    int resultado_comunicacion;
    
    // Validar parámetros
    if (strlen(key) > 255) {
        return -1;
    }
    
    // Preparar petición
    memset(&req, 0, sizeof(request_msg));
    req.operation = OP_DELETE_KEY;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    
    // Enviar y esperar
    resultado_comunicacion = enviar_peticion_y_esperar(&req, &res);
    if (resultado_comunicacion == -2) {
        return -2;
    }
    
    return res.result;
}

/**
 * exist - Verificar si existe una clave
 */
int exist(char *key) {
    request_msg req;
    response_msg res;
    int resultado_comunicacion;
    
    // Validar parámetros
    if (strlen(key) > 255) {
        return -1;
    }
    
    // Preparar petición
    memset(&req, 0, sizeof(request_msg));
    req.operation = OP_EXIST;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    
    // Enviar y esperar
    resultado_comunicacion = enviar_peticion_y_esperar(&req, &res);
    if (resultado_comunicacion == -2) {
        return -2;
    }
    
    return res.result;
}
