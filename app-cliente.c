#include <stdio.h>
#include <string.h>
#include "claves.h"

void imprimir_separador(const char *nombre_test) {
    printf("\n%s", nombre_test);
}

void imprimir_resultado(const char *operacion, int resultado, int esperado) {
    if (resultado == esperado) {
        printf("PASS %s: ÉXITO (devolvió %d)\n", operacion, resultado);
    } else {
        printf("FAIL %s: FALLO (devolvió %d, esperado %d)\n", operacion, resultado, esperado);
    }
}

int main(int argc, char **argv) {
    int resultado;
    char value1_salida[256];
    int N_value2_salida;
    float V_value2_salida[32];
    struct Paquete value3_salida;
    
    printf("SERVICIO ALMACENAMIENTO - CLIENTE DE PRUEBAS\n");
    
    //PRUEBA 1: Inicializar el servicio
    imprimir_separador("PRUEBA 1: Inicializar Servicio (destroy)");
    resultado = destroy();
    imprimir_resultado("destroy()", resultado, 0);
    
    
    //PRUEBA 2: Insertar una nueva tupla
    imprimir_separador("PRUEBA 2: Insertar Nueva Tupla (set_value)");
    
    char *clave1 = "usuario_123";
    char *v1 = "Juan Pérez";
    float v2[] = {1.5, 2.7, 3.9};
    struct Paquete v3;
    v3.x = 10;
    v3.y = 20;
    v3.z = 30;
    
    resultado = set_value(clave1, v1, 3, v2, v3);
    imprimir_resultado("set_value(usuario_123)", resultado, 0);
    
    
    //PRUEBA 3: Insertar otra tupla
    imprimir_separador("PRUEBA 3: Insertar Segunda Tupla");
    
    char *clave2 = "producto_456";
    char *v1_2 = "Portátil";
    float v2_2[] = {999.99, 1200.50};
    struct Paquete v3_2;
    v3_2.x = 5;
    v3_2.y = 10;
    v3_2.z = 15;
    
    resultado = set_value(clave2, v1_2, 2, v2_2, v3_2);
    imprimir_resultado("set_value(producto_456)", resultado, 0);
    
    
    //PRUEBA 4: Intentar insertar clave duplicada (debe fallar)
    imprimir_separador("PRUEBA 4: Insertar Clave Duplicada (debe fallar)");
    
    resultado = set_value(clave1, "Valor Diferente", 1, v2, v3);
    imprimir_resultado("set_value(usuario_123) duplicado", resultado, -1);
    
    
    //PRUEBA 5: Verificar si las claves existen
    imprimir_separador("PRUEBA 5: Verificar Existencia de Claves (exist)");
    
    resultado = exist(clave1);
    imprimir_resultado("exist(usuario_123)", resultado, 1);
    
    resultado = exist("clave_inexistente");
    imprimir_resultado("exist(clave_inexistente)", resultado, 0);
    
    
    //PRUEBA 6: Recuperar valores (get_value)
    imprimir_separador("PRUEBA 6: Recuperar Valores (get_value)");
    
    resultado = get_value(clave1, value1_salida, &N_value2_salida, V_value2_salida, &value3_salida);
    imprimir_resultado("get_value(usuario_123)", resultado, 0);
    
    if (resultado == 0) {
        printf("  Datos recuperados:\n");
        printf("    value1: %s\n", value1_salida);
        printf("    N_value2: %d\n", N_value2_salida);
        printf("    V_value2: [");
        for (int i = 0; i < N_value2_salida; i++) {
            printf("%.2f", V_value2_salida[i]);
            if (i < N_value2_salida - 1) printf(", ");
        }
        printf("]\n");
        printf("    value3: {x=%d, y=%d, z=%d}\n", value3_salida.x, value3_salida.y, value3_salida.z);
    }
    
    
    //PRUEBA 7: Intentar obtener clave inexistente (debe fallar)
    imprimir_separador("PRUEBA 7: Obtener Clave Inexistente (debe fallar)");
    
    resultado = get_value("clave_falsa", value1_salida, &N_value2_salida, V_value2_salida, &value3_salida);
    imprimir_resultado("get_value(clave_falsa)", resultado, -1);
    
    
    //PRUEBA 8: Modificar tupla existente
    imprimir_separador("PRUEBA 8: Modificar Tupla Existente (modify_value)");
    
    char *v1_modificado = "María García";
    float v2_modificado[] = {10.1, 20.2, 30.3, 40.4};
    struct Paquete v3_modificado;
    v3_modificado.x = 100;
    v3_modificado.y = 200;
    v3_modificado.z = 300;
    
    resultado = modify_value(clave1, v1_modificado, 4, v2_modificado, v3_modificado);
    imprimir_resultado("modify_value(usuario_123)", resultado, 0);
    
    // Verificar la modificación
    resultado = get_value(clave1, value1_salida, &N_value2_salida, V_value2_salida, &value3_salida);
    if (resultado == 0) {
        printf("  Datos modificados:\n");
        printf("    value1: %s\n", value1_salida);
        printf("    N_value2: %d\n", N_value2_salida);
        printf("    value3: {x=%d, y=%d, z=%d}\n", value3_salida.x, value3_salida.y, value3_salida.z);
    }
    
    
    //PRUEBA 9: Intentar modificar clave inexistente (debe fallar)
    imprimir_separador("PRUEBA 9: Modificar Clave Inexistente (debe fallar)");
    
    resultado = modify_value("inexistente", v1_modificado, 2, v2_modificado, v3_modificado);
    imprimir_resultado("modify_value(inexistente)", resultado, -1);
    
    
    // PRUEBA 10: Probar N_value2 inválido (fuera de rango)
    imprimir_separador("PRUEBA 10: Valores N_value2 Inválidos");
    
    resultado = set_value("clave_test", "test", 0, v2, v3);  // N_value2 = 0 (muy pequeño)
    imprimir_resultado("set_value con N_value2=0", resultado, -1);
    
    resultado = set_value("clave_test", "test", 33, v2, v3);  // N_value2 = 33 (muy grande)
    imprimir_resultado("set_value con N_value2=33", resultado, -1);
    
    resultado = modify_value(clave1, v1_modificado, 40, v2_modificado, v3_modificado);  // N_value2 = 40 (muy grande)
    imprimir_resultado("modify_value con N_value2=40", resultado, -1);
    
    
    //PRUEBA 11: Eliminar una clave
    imprimir_separador("PRUEBA 11: Eliminar Clave (delete_key)");
    
    resultado = delete_key(clave2);
    imprimir_resultado("delete_key(producto_456)", resultado, 0);
    
    // Verificar eliminación
    resultado = exist(clave2);
    imprimir_resultado("exist(producto_456) después de eliminar", resultado, 0);
    
    
    //PRUEBA 12: Intentar eliminar clave inexistente (debe fallar)
    imprimir_separador("PRUEBA 12: Eliminar Clave Inexistente (debe fallar)");
    
    resultado = delete_key("inexistente");
    imprimir_resultado("delete_key(inexistente)", resultado, -1);
    
    
    //PRUEBA 13: Insertar con tamaño máximo de array
    imprimir_separador("PRUEBA 13: Insertar con Tamaño Máximo de Array (N=32)");
    
    float array_grande[32];
    for (int i = 0; i < 32; i++) {
        array_grande[i] = i * 1.1;
    }
    
    resultado = set_value("array_max", "Prueba tamaño máximo", 32, array_grande, v3);
    imprimir_resultado("set_value con N_value2=32", resultado, 0);
    
    
    //PRUEBA 14: Limpiar - destruir todo
    imprimir_separador("PRUEBA 14: Limpiar Todo (destroy)");
    
    resultado = destroy();
    imprimir_resultado("destroy()", resultado, 0);
    
    // Verificar que todas las claves han sido eliminadas
    resultado = exist(clave1);
    imprimir_resultado("exist(usuario_123) después de destroy", resultado, 0);
    
    resultado = exist("array_max");
    imprimir_resultado("exist(array_max) después de destroy", resultado, 0);
    
    printf("  TODAS LAS PRUEBAS COMPLETADAS\n");   
    
    return 0;
}
