#include <stdio.h>      // Para entrada/salida estándar (printf, scanf)
#include <stdlib.h>     // Para funciones como rand(), srand()
#include <math.h>       // Para funciones matemáticas como sqrt(), fabs()
#include <time.h>       // Para inicializar la semilla de números aleatorios

#define MAX_POINTS 100      // Máximo número de puntos que se pueden ingresar
#define MAX_CLUSTERS 10     // Máximo número de clusters (zonas)
#define MAX_ITER 100        // Máximo número de iteraciones para evitar ciclos infinitos en K-means

// Prototipos de funciones (declaración previa)
void leerDatos(int n, float x[], float y[], float riesgo[]);
float distancia(float x1, float y1, float x2, float y2);
void inicializarCentroides(int k, int n, float x[], float y[], float centroideX[], float centroideY[]);
void asignarClusters(int n, int k, float x[], float y[], float centroideX[], float centroideY[], int cluster[]);
void actualizarCentroides(int n, int k, float x[], float y[], int cluster[], float centroideX[], float centroideY[]);
int centroidesIguales(int k, float centroideX[], float centroideY[], float centroideXPrev[], float centroideYPrev[]);
void imprimirResultados(int n, int k, float x[], float y[], float riesgo[], int cluster[]);
void imprimirMapaASCII(int n, float x[], float y[], int cluster[]);
float calcularSSE(int n, float x[], float y[], int cluster[], float centroideX[], float centroideY[]);

// Función para ingresar los datos de los puntos (coordenadas y riesgo)
void leerDatos(int n, float x[], float y[], float riesgo[]) {
    printf("==========================================\n");
    printf("    Ingreso de puntos georreferenciados    \n");
    printf("==========================================\n");
    for (int i = 0; i < n; i++) {
        printf("\nPunto %d/%d:\n", i + 1, n);

        // Ingreso de la coordenada X
        while (1) {
            printf("Ingrese coordenada X: ");
            if (scanf("%f", &x[i]) == 1)
                break;
            else {
                printf("Entrada invalida. Por favor ingrese un numero valido.\n");
                while(getchar() != '\n'); // limpiar buffer
            }
        }

        // Ingreso de la coordenada Y
        while (1) {
            printf("Ingrese coordenada Y: ");
            if (scanf("%f", &y[i]) == 1)
                break;
            else {
                printf("Entrada invalida. Por favor ingrese un número valido.\n");
                while(getchar() != '\n');
            }
        }

        // Ingreso validado del nivel de riesgo (1 a 10)
        while (1) {
            printf("Ingrese el nivel de riesgo (1 a 10): ");
            if (scanf("%f", &riesgo[i]) == 1) {
                if (riesgo[i] >= 1 && riesgo[i] <= 10)
                    break;
                else
                    printf("Valor no valido. Debe estar entre 1 y 10.\n");
            } else {
                printf("Entrada invalida. Por favor ingrese un número valido.\n");
                while(getchar() != '\n');
            }
        }
    }
    printf("\nDatos ingresados correctamente\n\n");
}


// Calcula la distancia euclidiana entre dos puntos (x1,y1) y (x2,y2)
float distancia(float x1, float y1, float x2, float y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));    // Devuelve la raíz cuadrada de la suma de cuadrados de las diferencias
}

// Inicializa los centroides escogiendo puntos aleatorios del conjunto original
void inicializarCentroides(int k, int n, float x[], float y[], float centroideX[], float centroideY[]) {
    srand(time(NULL));                                     // Inicializa la semilla del generador aleatorio con la hora actual
    for (int i = 0; i < k; i++) {                          // Para cada cluster
        int indiceAleatorio = rand() % n;                  // Elige un índice aleatorio entre 0 y n-1
        centroideX[i] = x[indiceAleatorio];                // Asigna la coordenada X del centroide
        centroideY[i] = y[indiceAleatorio];                // Asigna la coordenada Y del centroide
    }
}

// Asigna cada punto al cluster cuyo centroide está más cerca
void asignarClusters(int n, int k, float x[], float y[], float centroideX[], float centroideY[], int clusterAsignado[]) {
    for (int i = 0; i < n; i++) {                          // Para cada punto
        float menorDistancia = distancia(x[i], y[i], centroideX[0], centroideY[0]);   // Calcula distancia al primer centroide
        int indiceCentroideMasCercano = 0;                 // Por defecto, el centroide más cercano es el primero
        for (int j = 1; j < k; j++) {                      // Para cada centroide
            float distanciaActual = distancia(x[i], y[i], centroideX[j], centroideY[j]);   // Calcula distancia al centroide j
            if (distanciaActual < menorDistancia) {        // Si es menor a la distancia mínima actual
                menorDistancia = distanciaActual;          // Actualiza la menor distancia
                indiceCentroideMasCercano = j;             // Guarda el índice de este centroide
            }
        }
        clusterAsignado[i] = indiceCentroideMasCercano;    // Asigna el índice del centroide más cercano al punto i
    }
}

// Calcula nuevos centroides como el promedio de las coordenadas de los puntos asignados a cada cluster
void actualizarCentroides(int n, int k, float x[], float y[], int clusterAsignado[], float centroideX[], float centroideY[]) {
    int cantidadPorCluster[MAX_CLUSTERS];                  // Arreglo para contar cuántos puntos tiene cada cluster
    float sumaXPorCluster[MAX_CLUSTERS] = {}, sumaYPorCluster[MAX_CLUSTERS] = {};  // Suma acumulada de coordenadas por cluster
    for (int j = 0; j < k; j++) {                          // Inicializa los sumadores y contadores
        cantidadPorCluster[j] = 0;
        sumaXPorCluster[j] = 0;
        sumaYPorCluster[j] = 0;
    }
    for (int i = 0; i < n; i++) {                          // Para cada punto
        int cluster = clusterAsignado[i];                  // Cluster al que pertenece el punto i
        sumaXPorCluster[cluster] += x[i];                  // Suma la coordenada X al total del cluster
        sumaYPorCluster[cluster] += y[i];                  // Suma la coordenada Y al total del cluster
        cantidadPorCluster[cluster]++;                     // Aumenta el contador de puntos en ese cluster
    }
    for (int j = 0; j < k; j++) {                          // Para cada cluster
        if (cantidadPorCluster[j] > 0) {                   // Si el cluster tiene puntos asignados
            centroideX[j] = sumaXPorCluster[j] / cantidadPorCluster[j];    // Promedio X
            centroideY[j] = sumaYPorCluster[j] / cantidadPorCluster[j];    // Promedio Y
        }
    }
}

// Verifica si los centroides han cambiado (compara con los valores de la iteración anterior)
int centroidesIguales(int k, float centroideX[], float centroideY[], float centroideXPrev[], float centroideYPrev[]) {
    for (int i = 0; i < k; i++) {                                 // Para cada centroide
        if (fabs(centroideX[i] - centroideXPrev[i]) > 0.0001 ||   // Si X cambió significativamente
            fabs(centroideY[i] - centroideYPrev[i]) > 0.0001)     // O si Y cambió significativamente
                return 0;                                             // No son iguales, continuar iterando
    }
    return 1;                                                     // Todos los centroides son iguales, ya convergió
}

// Imprime los resultados de la agrupación, mostrando cada punto y el resumen de cada zona
void imprimirResultados(int n, int k, float x[], float y[], float riesgo[], int clusterAsignado[], float riesgoPromedio[]) {
    int cantidadPorCluster[MAX_CLUSTERS] = {0};                   // Contador de puntos por cluster
    float sumaRiesgoPorCluster[MAX_CLUSTERS] = {0};               // Acumulador de riesgo por cluster

    printf("\n--- Resultados del agrupamiento ---\n");            // Título
    for (int i = 0; i < n; i++) {                                 // Para cada punto
        printf("Punto (%.1f, %.1f) - Riesgo: %.1f -> Zona %d\n",  // Imprime el punto y su zona asignada
            x[i], y[i], riesgo[i], clusterAsignado[i]);
        sumaRiesgoPorCluster[clusterAsignado[i]] += riesgo[i];    // Suma el riesgo a su cluster
        cantidadPorCluster[clusterAsignado[i]]++;                 // Cuenta el punto en el cluster
    }

    printf("\n--- Resumen por zona ---\n");                       // Título del resumen
    for (int j = 0; j < k; j++) {                                 // Para cada cluster
        printf("Zona %d: Riesgo promedio: %.2f (Puntos: %d) - ",  // Muestra el promedio de riesgo y cantidad de puntos
            j, riesgoPromedio[j], cantidadPorCluster[j]);
        if (riesgoPromedio[j] == 0)
            printf("SIN RIESGO\n");
        else if (riesgoPromedio[j] <= 3)
            printf("RIESGO BAJO\n");
        else if (riesgoPromedio[j] <= 6)
            printf("RIESGO MODERADO\n");
        else
            printf("ALTO RIESGO\n");
    }
}

// Imprime un mapa ASCII con las zonas (clusters), usando caracteres para permitir mostrar '.'
void imprimirMapaASCII(int n, float x[], float y[], int clusterAsignado[]) {
    char mapa[11][11] = {};                                       // Matriz 11x11 para el mapa, inicializada en cero

    // Rellena el mapa con '.'
    for (int i = 0; i < 11; i++)
        for (int j = 0; j < 11; j++)
            mapa[i][j] = '.';                                     // Cada posición es un punto vacío

    // Para cada punto, coloca el número de zona (como carácter) en su posición correspondiente
    for (int i = 0; i < n; i++) {
        int posX = (int) (x[i] + 0.5);                            // Redondea la coordenada X
        int posY = (int) (y[i] + 0.5);                            // Redondea la coordenada Y
        if (posX >= 1 && posX <= 10 && posY >= 1 && posY <= 10)   // Valida que esté dentro del rango visible
            mapa[posY][posX] = clusterAsignado[i] + '0';          // Convierte el índice de zona a carácter
    }

    printf("\n--- Mapa por zonas ---\n");                   // Título del mapa
    for (int i = 10; i >= 1; i--) {                               // Desde la fila superior hasta la inferior
        for (int j = 1; j <= 10; j++)
            printf("%c ", mapa[i][j]);                            // Imprime el carácter en cada celda
        printf("\n");
    }
    printf("\nLeyenda: 0 = Zona 0, 1 = Zona 1, ... ; . = sin punto\n\n");   // Explica la leyenda
}

// Calcula la suma de errores al cuadrado (SSE) de todos los puntos respecto a su centroide
float calcularSSE(int n, float x[], float y[], int cluster[], float centroideX[], float centroideY[]) {
    float sse = 0;                                                // Inicializa el SSE
    for (int i = 0; i < n; i++) {                                 // Para cada punto
        float dx = x[i] - centroideX[cluster[i]];                 // Diferencia en X con el centroide de su cluster
        float dy = y[i] - centroideY[cluster[i]];                 // Diferencia en Y con el centroide de su cluster
        sse += dx*dx + dy*dy;                                     // Suma el cuadrado de la distancia al SSE
    }
    return sse;                                                   // Devuelve el SSE total
}

int main() {
    int n, k; // Número de puntos (n) y número de clusters/zones (k)

    float x[MAX_POINTS], y[MAX_POINTS], riesgo[MAX_POINTS];
    // x: coordenadas X de cada punto
    // y: coordenadas Y de cada punto
    // riesgo: nivel de riesgo de cada punto

    float centroideX[MAX_CLUSTERS], centroideY[MAX_CLUSTERS], centroideXPrev[MAX_CLUSTERS], centroideYPrev[MAX_CLUSTERS];
    // centroideX, centroideY: posiciones actuales de los centroides de cada cluster
    // centroideXPrev, centroideYPrev: posiciones anteriores de los centroides para comparar si han cambiado

    int clusterAsignado[MAX_POINTS];
    // clusterAsignado[i]: índice del cluster asignado al punto i

    int cantidadPorCluster[MAX_CLUSTERS];
    // cantidadPorCluster[j]: cantidad de puntos en el cluster j

    float sumaRiesgoPorCluster[MAX_CLUSTERS];
    // sumaRiesgoPorCluster[j]: suma total de los riesgos en el cluster j

    float riesgoPromedio[MAX_CLUSTERS];
    // riesgoPromedio[j]: riesgo promedio del cluster j

    // Solicita y valida la cantidad de puntos a procesar
    while (1) {
        printf("Ingrese la cantidad de puntos (max %d): ", MAX_POINTS);
        if (scanf("%d", &n) == 1 && n >= 1 && n <= MAX_POINTS)
            break;
        else {
            printf("Valor no valido. Debe ser entre 1 y %d.\n", MAX_POINTS);
            while(getchar() != '\n'); // Limpiar buffer
        }
    }

    leerDatos(n, x, y, riesgo); // Solicita ingresar las coordenadas y riesgo de cada punto

    // Solicita y valida la cantidad de clusters/zones (K)
    while (1) {
        printf("Ingrese el valor de K (zonas): ");
        if (scanf("%d", &k) == 1 && k >= 1 && k <= n)
            break;
        else {
            printf("K debe ser al menos 1 y no mayor que la cantidad de puntos (%d).\n", n);
            while(getchar() != '\n');
        }
    }

    inicializarCentroides(k, n, x, y, centroideX, centroideY); // Inicializa los centroides aleatoriamente con datos existentes

    // Bucle principal de K-means: repite hasta que los centroides no cambien o llegue a MAX_ITER
    for (int iter = 0; iter < MAX_ITER; iter++) {
        for (int i = 0; i < k; i++) {
            centroideXPrev[i] = centroideX[i]; // Guarda la posición X actual del centroide i
            centroideYPrev[i] = centroideY[i]; // Guarda la posición Y actual del centroide i
        }
        asignarClusters(n, k, x, y, centroideX, centroideY, clusterAsignado); // Asigna cada punto al cluster más cercano
        actualizarCentroides(n, k, x, y, clusterAsignado, centroideX, centroideY); // Recalcula las posiciones de los centroides
        if (centroidesIguales(k, centroideX, centroideY, centroideXPrev, centroideYPrev))
            break; // Sale del bucle si los centroides ya no han cambiado (convergió)
    }

    // Calcula la suma y cantidad de riesgos por cada zona/cluster
    for (int i = 0; i < k; i++) {
        sumaRiesgoPorCluster[i] = 0;      // Inicializa suma de riesgos para el cluster i en 0
        cantidadPorCluster[i] = 0;        // Inicializa el contador de puntos para el cluster i en 0
    }
    for (int i = 0; i < n; i++) {
        sumaRiesgoPorCluster[clusterAsignado[i]] += riesgo[i]; // Suma el riesgo del punto i a su cluster
        cantidadPorCluster[clusterAsignado[i]]++;              // Aumenta el contador de puntos de ese cluster
    }
    for (int i = 0; i < k; i++) {
        if (cantidadPorCluster[i] > 0)
            riesgoPromedio[i] = sumaRiesgoPorCluster[i] / cantidadPorCluster[i]; // Calcula el promedio de riesgo si hay puntos
        else
            riesgoPromedio[i] = 0; // Si no hay puntos, el promedio de riesgo es 0
    }

    imprimirResultados(n, k, x, y, riesgo, clusterAsignado, riesgoPromedio); // Imprime el resultado detallado y resumen por zona
    imprimirMapaASCII(n, x, y, clusterAsignado); // Imprime el mapa visual ASCII

    // Calcula y muestra la métrica de error SSE y MSE
    float sse = calcularSSE(n, x, y, clusterAsignado, centroideX, centroideY); // Calcula la suma de errores al cuadrado
    float mse = sse / n; // Calcula el error cuadrático medio
    printf("Suma de errores al cuadrado (SSE): %.2f\n", sse); // Imprime SSE
    printf("Error cuadratico medio (MSE): %.2f\n", mse); // Imprime MSE

    return 0; // Fin del programa correctamente
}
