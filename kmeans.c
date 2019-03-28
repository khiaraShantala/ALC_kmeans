#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


struct GRUPO
{
    float** clusters;
    int* clusterElem;
    // int indice;
};

typedef struct GRUPO Grupo;

//aloca
float* malocaVetor(int);
float** malocaMatriz(int, int);
int* malocaVetorInt(int);
Grupo* alocaGrupo(int, int, int);


//calculo banco de dados
float** lerArquivo(float**, int, int);
float** lerArquivoTeste(float**, int, int);


//calculo kmeans
float** initClusters(float**, float**, int, int);
float distEuclidiana(float*, float*, int);
int clusterMaisProximo(int, int, float*, float**);
Grupo* kmeans(float**, int, int, int, float, int*, Grupo*);
int* preverCluster(float**, int,int, int, float**);


//printa
void print(float*, int);
void printM(float**, int, int);


//printa no arquivo
void printMatrizArquivo(float**, int, int, int);
void printVetorArquivo(float*, int, int, int*);
void printSomethingArquivo(int, int);


int main(int argc, char** argv)
{
    int numLinhas  = 49301;//Numero de linhas da matriz 90%
    int numLinhas2 = 5478;// Numero de linhas da matriz 10%
    int numColunas = 8;//Numero de colunas
    int numCluster = 2;//Numero de cluster inicial
    int numClusterFinal = 24; //Numero de cluster Finla
    float threshold = 0.001; //Criterio de parada do while
    int indice = 0; //Contagem de quantidade iteracoes

    int k = numClusterFinal - numCluster;

    float** matriz = malocaMatriz(numLinhas, numColunas);
    matriz = lerArquivo(matriz, numLinhas, numColunas);

    float** matrizTreino = malocaMatriz(numLinhas2, numColunas);
    matrizTreino = lerArquivoTeste(matrizTreino, numLinhas2, numColunas);

    // KMeans
    Grupo* g;
    int i;

    for (i = 0; i < numCluster ; i++)
    {

        g = alocaGrupo(numCluster, numLinhas, numColunas);

        g = kmeans(matriz, numLinhas, numColunas, numCluster, threshold, &indice, g);

        printMatrizArquivo(g->clusters, numClusterFinal, numColunas, 3);
        printVetorArquivo(NULL, numLinhas, 3, g->clusterElem);
        printSomethingArquivo(indice, numClusterFinal);

        indice = 0;

        numClusterFinal++;
    }

    int* id = (int*)malloc(numLinhas2*sizeof(int));
    id = preverCluster(matrizTreino, numCluster, numLinhas2, numColunas, g->clusters);
    printVetorArquivo(NULL, numLinhas2, 3, id);

    return 0;
}


//Aloca matriz
float** malocaMatriz(int numLinhas, int numColunas)
{
    float** matriz = (float**)malloc(numLinhas * (sizeof(float*)));
    for(int i = 0; i < numLinhas; i++)
    {
        matriz[i] = (float*)malloc(numColunas * (sizeof(float)));

    }

    return matriz;
}


//Aloca vetor de float
float* malocaVetor(int numLinhas)
{
    float* vetor = (float*)malloc(numLinhas * (sizeof(float)));

    for (int i = 0; i < numLinhas; i++)
    {
        vetor[i] = 0;
    }

    return vetor;
}

//Aloca vetor de int
int* malocaVetorInt(int numLinhas)
{
    int* vetor = (int*)malloc(numLinhas * (sizeof(int)));

    for (int i = 0; i < numLinhas; i++)
    {
        vetor[i] = 0;
    }

    return vetor;
}


//Aloca struct de Grupo
Grupo* alocaGrupo(int numCluster, int numLinhas, int numColunas)
{

    Grupo* grupo = (Grupo*)malloc(sizeof(Grupo));

    grupo->clusters = malocaMatriz(numCluster, numColunas);
    grupo->clusterElem = malocaVetorInt(numLinhas);

    return grupo;
}

//Le a base de treino
float** lerArquivo(float** matriz, int numLinhas, int numColunas)
{
    FILE* arquivo = fopen("baseTreino.txt", "r");

    if(arquivo == NULL)
        exit(0);

    for(int i = 0; i < numColunas; i++)
    {
        for(int j = 0; j < numLinhas; j++)
        {
            fscanf(arquivo, "%f", &matriz[j][i]);
        }

    }

    fclose(arquivo);

    return matriz;

}


float** lerArquivoTeste(float** matrizTreino, int numLinhas2, int numColunas)
{
    FILE* fin = fopen("base10.txt", "r");

    if(fin == NULL)
        exit(0);

    for(int i = 0; i < numColunas; i++)
    {
        for(int j = 0; j < numLinhas2; j++)
        {
            fscanf(fin, "%f", &matrizTreino[j][i]);
        }

    }

    fclose(fin);

    return matrizTreino;

}


// Calcula distancia euclidiana em uma matriz
float distEuclidiana(float* coord, float* centroide, int numColunas)
{
    float d = 0.0;

    for(int j = 0; j < numColunas; j++)
    {
        d += (coord[j] - centroide[j]) * (coord[j] - centroide[j]);
    }

    return sqrt(d);
}

//Calcula o cluster mais proximo da pessoa[i] retornando o numero do cluster a qual ela pertence
int clusterMaisProximo(int numClusters, int numColunas, float* instancia, float** clusters)
{
    int index;
    float dist, minDist;

    index = 0;
    minDist = distEuclidiana(instancia, clusters[0], numColunas);

    for (int i = 1; i < numClusters; i++)
    {
        dist = distEuclidiana(instancia, clusters[i], numColunas);

        if (dist < minDist)
        {
            minDist = dist;
            index = i;
        }
    }
    return index;
}


//inicia os centroides fazendo os primeiros k numClusters serem os centroides
float** initClusters(float** matrizNormal, float** clusters, int numCluster, int numColunas)
{
    for (int i = 0; i < numCluster; i++)
    {
        clusters[i] = matrizNormal[i];
    }
    return clusters;
}


//retorna uma struct tipo Grupo onde possui o vetor centroide e quais grupos as pessoas estao
Grupo* kmeans (float** matrizNormal, int numLinhas, int numColunas, int numCluster, float threshold, int* indice, Grupo* g)
{

    float** auxClusters = malocaMatriz(numCluster, numColunas);
    int* numElemCluster = malocaVetorInt(numLinhas);

    int loop = 0, indiceCluster;
    float numMudancas = 0.0;


    g->clusters = initClusters(matrizNormal, g->clusters, numCluster, numColunas);

    //indica que todas as pessoas estao inseridas em nenhum cluster
    for (int i = 0; i < numLinhas; i++)
        g->clusterElem[i] = -1;

    //inicia a matriz de centroides auxiliar como 0
    for (int i = 0; i < numCluster; i++)
    {
        for (int j = 0; j < numColunas; j++)
        {
            auxClusters[i][j] = 0.0;
        }
    }

    //Loop K-Means
    do{

        //criterio de parada, indica quantas vezes uma pessoa mudou de cluster na iteracao atual
        numMudancas = 0.0;

        for (int i = 0; i < numLinhas; i++)
        {
            indiceCluster = clusterMaisProximo(numCluster, numColunas, matrizNormal[i], g->clusters);


            if(indiceCluster != g->clusterElem[i])
                numMudancas += 1.0;

            g->clusterElem[i] = indiceCluster;

            //indica a quantidade de pessoas estao no cluster atual
            numElemCluster[indiceCluster]++;

            //somatorio de todas as pessoas que fazem parte do cluster atual
            for (int j = 0; j < numColunas; j++)
                auxClusters[indiceCluster][j] += matrizNormal[i][j];
        }

        for (int i = 0; i < numCluster; i++)
        {
            for (int j = 0; j < numColunas; j++)
            {
                //calculo para encontrar o novo centroide com base na media
                if(numElemCluster[i] > 0)
                    g->clusters[i][j] = auxClusters[i][j] / numElemCluster[i];

                auxClusters[i][j] = 0.0;
            }
            numElemCluster[i] = 0;
        }

        numMudancas /= numLinhas;
        loop++;

    }while (numMudancas > threshold & loop < 500);

    //numero de iteracoes
    *indice = loop + 1;

    return g;
}


int* preverCluster(float** matrizTreino, int numCluster,int numLinhas2, int numColunas, float** clusters)
{
    int* indice = (int*)malloc(numLinhas2*sizeof(int));


    for (int i = 0; i < numLinhas2; i++)
    {
        indice[i] = clusterMaisProximo(numCluster, numColunas, matrizTreino[i], clusters);
    }

    return indice;

}

void printVetorArquivo(float* vetor, int numColunas, int escolha, int* cluster)
{
    FILE* arquivo = fopen("teste.txt", "a+");

    if(arquivo == NULL)
    {
        arquivo = fopen("teste.txt","w");
    }

    if(escolha == 3)
        fprintf(arquivo, "Elementos e seus respectivos Clusters:\n");

    for(int i = 0; i < numColunas; i++)
    {
        if(escolha == 3)
            fprintf(arquivo, "Pessoa #%d, se encontra no cluster #%2d\n", i, cluster[i]);

        else
            fprintf(arquivo, "%7.3f", vetor[i]);

    }
    fprintf(arquivo, "\n\n\n");

    fclose(arquivo);

}


void printMatrizArquivo(float** matriz, int numLinhas, int numColunas, int escolha)
{
    FILE* arquivo = fopen("teste.txt", "a+");

    if(arquivo == NULL)
    {
        arquivo = fopen("teste.txt","w");

    }
    if(escolha == 0)
        fprintf(arquivo, "A matriz inicial eh: \n");
    else if(escolha == 3)
        fprintf(arquivo, "Os centroides dos Clusters sao: \n");

    fprintf(arquivo, "Matrix %d x %d\n", numLinhas, numColunas);
    for(int i = 0; i < numLinhas; i++)
    {
        for(int j = 0; j < numColunas; j++)
        {
            fprintf(arquivo, "%7.3f,", matriz[i][j]);
        }
        fprintf(arquivo, "\n");
    }
    fprintf(arquivo, "\n");

    fclose(arquivo);

}


void printSomethingArquivo(int indice, int numCluster)
{
    FILE* arquivo = fopen("teste.txt", "a+");

    if(arquivo == NULL)
    {
        arquivo = fopen("teste.txt","w");
    }

    fprintf(arquivo, " Num Cluster #%d, iteracao #%3d\n", numCluster, indice);
}
