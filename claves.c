#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "claves.h"

// Estructura de nodo para la lista enlazada
typedef struct Node {
    char key[256];              // Clave (cadena de caracteres)
    char value1[256];           // Valor1 (cadena de caracteres)
    int N_value2;               // Número de elementos en V_value2
    float V_value2[32];         // Vector de floats
    struct Paquete value3;      // Estructura con x, y, z
    struct Node *next;          // Puntero al siguiente nodo
} Node;

// Cabeza de la lista enlazada (global)
static Node *head = NULL;

// Mutex para operaciones thread-safe (importante para Parte B)
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * Función auxiliar: Buscar un nodo por clave
 * Devuelve puntero al nodo si se encuentra, NULL en caso contrario
 */
static Node* find_node(const char *key) {
    Node *current = head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}


/**
 * destroy - Inicializa/limpia el servicio
 * Elimina todas las tuplas almacenadas
 */
int destroy(void) {
    pthread_mutex_lock(&mutex);
    
    Node *current = head;
    Node *temp;
    
    // Liberar todos los nodos de la lista
    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }
    
    head = NULL;
    
    pthread_mutex_unlock(&mutex);
    return 0;
}


/**
 * set_value - Insertar una nueva tupla
 * Devuelve 0 si tiene éxito, -1 si la clave existe o N_value2 fuera de rango
 */
int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // Validar rango de N_value2
    if (N_value2 < 1 || N_value2 > 32) {
        return -1;
    }
    
    pthread_mutex_lock(&mutex);
    
    // Verificar si la clave ya existe
    if (find_node(key) != NULL) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    
    // Crear nuevo nodo
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    
    // Copiar datos al nuevo nodo
    strncpy(new_node->key, key, 255);
    new_node->key[255] = '\0';
    
    strncpy(new_node->value1, value1, 255);
    new_node->value1[255] = '\0';
    
    new_node->N_value2 = N_value2;
    memcpy(new_node->V_value2, V_value2, N_value2 * sizeof(float));
    
    new_node->value3 = value3;
    
    // Insertar al principio de la lista
    new_node->next = head;
    head = new_node;
    
    pthread_mutex_unlock(&mutex);
    return 0;
}


/**
 * get_value - Recuperar valores asociados a una clave
 * Devuelve 0 si tiene éxito, -1 si la clave no existe
 */
int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    pthread_mutex_lock(&mutex);
    
    Node *node = find_node(key);
    
    if (node == NULL) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    
    // Copiar datos a los parámetros de salida
    strncpy(value1, node->value1, 256);
    *N_value2 = node->N_value2;
    memcpy(V_value2, node->V_value2, node->N_value2 * sizeof(float));
    *value3 = node->value3;
    
    pthread_mutex_unlock(&mutex);
    return 0;
}


/**
 * modify_value - Modificar valores asociados a una clave
 * Devuelve 0 si tiene éxito, -1 si la clave no existe o N_value2 fuera de rango
 */
int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // Validar rango de N_value2
    if (N_value2 < 1 || N_value2 > 32) {
        return -1;
    }
    
    pthread_mutex_lock(&mutex);
    
    Node *node = find_node(key);
    
    if (node == NULL) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    
    // Actualizar los valores del nodo
    strncpy(node->value1, value1, 255);
    node->value1[255] = '\0';
    
    node->N_value2 = N_value2;
    memcpy(node->V_value2, V_value2, N_value2 * sizeof(float));
    
    node->value3 = value3;
    
    pthread_mutex_unlock(&mutex);
    return 0;
}


/**
 * delete_key - Eliminar una tupla por clave
 * Devuelve 0 si tiene éxito, -1 si la clave no existe
 */
int delete_key(char *key) {
    pthread_mutex_lock(&mutex);
    
    Node *current = head;
    Node *previous = NULL;
    
    // Buscar el nodo
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Encontrado - eliminar de la lista
            if (previous == NULL) {
                // Eliminar nodo cabeza
                head = current->next;
            } else {
                // Eliminar nodo intermedio o final
                previous->next = current->next;
            }
            free(current);
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        previous = current;
        current = current->next;
    }
    
    // Clave no encontrada
    pthread_mutex_unlock(&mutex);
    return -1;
}


/**
 * exist - Verificar si existe una clave
 * Devuelve 1 si existe, 0 si no existe, -1 en caso de error
 */
int exist(char *key) {
    pthread_mutex_lock(&mutex);
    
    Node *node = find_node(key);
    
    pthread_mutex_unlock(&mutex);
    
    return (node != NULL) ? 1 : 0;
}
