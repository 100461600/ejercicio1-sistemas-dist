#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "claves.h"
#include "common.h"

// Variables globales
mqd_t cola_peticiones;
int servidor_ejecutando = 1;

// Manejador de señales para apagado limpio
void manejador_señal(int sig) {
    printf("\n[SERVIDOR] Apagando...\n");
    servidor_ejecutando = 0;
}

// Función de hilo para manejar una petición de cliente
void* manejar_peticion(void* arg) {
    request_msg* req = (request_msg*)arg;
    response_msg res;
    mqd_t cola_respuesta;
    char nombre_cola_respuesta[256];
    
    // Inicializar respuesta
    memset(&res, 0, sizeof(response_msg));
    
    printf("[SERVIDOR] Hilo %lu manejando operación %d del cliente %d\n", 
           pthread_self(), req->operation, req->client_id);
    
    // Procesar la petición según la operación
    switch (req->operation) {
        case OP_DESTROY:
            printf("[SERVIDOR] Ejecutando destroy()\n");
            res.result = destroy();
            break;
            
        case OP_SET_VALUE:
            printf("[SERVIDOR] Ejecutando set_value(clave=%s)\n", req->key);
            res.result = set_value(req->key, req->value1, req->N_value2, 
                                  req->V_value2, req->value3);
            break;
            
        case OP_GET_VALUE:
            printf("[SERVIDOR] Ejecutando get_value(clave=%s)\n", req->key);
            res.result = get_value(req->key, res.value1, &res.N_value2, 
                                  res.V_value2, &res.value3);
            break;
            
        case OP_MODIFY_VALUE:
            printf("[SERVIDOR] Ejecutando modify_value(clave=%s)\n", req->key);
            res.result = modify_value(req->key, req->value1, req->N_value2, 
                                     req->V_value2, req->value3);
            break;
            
        case OP_DELETE_KEY:
            printf("[SERVIDOR] Ejecutando delete_key(clave=%s)\n", req->key);
            res.result = delete_key(req->key);
            break;
            
        case OP_EXIST:
            printf("[SERVIDOR] Ejecutando exist(clave=%s)\n", req->key);
            res.result = exist(req->key);
            break;
            
        default:
            printf("[SERVIDOR] Operación desconocida: %d\n", req->operation);
            res.result = -1;
            break;
    }
    
    // Crear nombre de cola de respuesta basado en client_id
    snprintf(nombre_cola_respuesta, sizeof(nombre_cola_respuesta), 
             "%s%d", RESPONSE_QUEUE_PREFIX, req->client_id);
    
    // Abrir cola de respuesta del cliente
    cola_respuesta = mq_open(nombre_cola_respuesta, O_WRONLY);
    if (cola_respuesta == (mqd_t)-1) {
        perror("[SERVIDOR] Error al abrir cola de respuesta");
        free(req);
        return NULL;
    }
    
    // Enviar respuesta al cliente
    if (mq_send(cola_respuesta, (char*)&res, sizeof(response_msg), 0) == -1) {
        perror("[SERVIDOR] Error al enviar respuesta");
        res.result = -2;  // Error de comunicación
    } else {
        printf("[SERVIDOR] Respuesta enviada al cliente %d (resultado=%d)\n", 
               req->client_id, res.result);
    }
    
    // Cerrar cola de respuesta
    mq_close(cola_respuesta);
    
    // Liberar memoria de la petición
    free(req);
    
    return NULL;
}

int main(int argc, char** argv) {
    struct mq_attr attr;
    request_msg req;
    pthread_t hilo;
    pthread_attr_t attr_hilo;
    
    printf("=================================================\n");
    printf("  SERVIDOR DE ALMACENAMIENTO (Colas de Mensajes)\n");
    printf("=================================================\n\n");
    
    // Configurar manejador de señales para Ctrl+C
    signal(SIGINT, manejador_señal);
    
    // Inicializar atributos de hilos (hilos separados)
    pthread_attr_init(&attr_hilo);
    pthread_attr_setdetachstate(&attr_hilo, PTHREAD_CREATE_DETACHED);
    
    // Configurar atributos de cola de mensajes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;  // Máximo 10 mensajes en cola
    attr.mq_msgsize = sizeof(request_msg);
    attr.mq_curmsgs = 0;
    
    // Eliminar cola antigua si existe
    mq_unlink(REQUEST_QUEUE);
    
    // Crear cola de peticiones
    cola_peticiones = mq_open(REQUEST_QUEUE, O_CREAT | O_RDONLY, 0666, &attr);
    if (cola_peticiones == (mqd_t)-1) {
        perror("[SERVIDOR] Error al crear cola de peticiones");
        exit(EXIT_FAILURE);
    }
    
    printf("[SERVIDOR] Cola de peticiones creada: %s\n", REQUEST_QUEUE);
    printf("[SERVIDOR] Servidor iniciado. Esperando peticiones...\n\n");
    
    // Bucle principal del servidor
    while (servidor_ejecutando) {
        // Recibir petición de la cola
        ssize_t bytes_leidos = mq_receive(cola_peticiones, (char*)&req, 
                                       sizeof(request_msg), NULL);
        
        if (bytes_leidos == -1) {
            if (servidor_ejecutando) {
                perror("[SERVIDOR] Error al recibir petición");
            }
            continue;
        }
        
        printf("[SERVIDOR] Petición recibida (operación=%d, cliente=%d)\n", 
               req.operation, req.client_id);
        
        // Reservar memoria para la petición (será liberada por el hilo)
        request_msg* copia_req = (request_msg*)malloc(sizeof(request_msg));
        if (copia_req == NULL) {
            fprintf(stderr, "[SERVIDOR] Fallo en asignación de memoria\n");
            continue;
        }
        memcpy(copia_req, &req, sizeof(request_msg));
        
        // Crear hilo para manejar la petición
        if (pthread_create(&hilo, &attr_hilo, manejar_peticion, copia_req) != 0) {
            perror("[SERVIDOR] Error al crear hilo");
            free(copia_req);
            continue;
        }
        
        printf("[SERVIDOR] Hilo creado para manejar petición\n\n");
    }
    
    // Limpieza
    printf("\n[SERVIDOR] Limpiando...\n");
    mq_close(cola_peticiones);
    mq_unlink(REQUEST_QUEUE);
    pthread_attr_destroy(&attr_hilo);
    
    printf("[SERVIDOR] Servidor detenido.\n");
    
    return 0;
}
