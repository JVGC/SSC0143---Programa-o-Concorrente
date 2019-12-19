/*
Ana Carolina Fainelo de Oliveira 10284542
Beatriz Campos de Almeida de Castro Monteiro 9778619
João Victor Garcia Coelho 10349540
Mateus Virginio Silva 10284156
Matheus Sanchez 9081453
*/

#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <limits.h>

// structs auxiliares que conterão os valores que forem serem calculados
typedef struct cidade{
	int menor;
	int maior;
	double mediana;
	double media;
	double dp;
}Cidade;

typedef struct regiao{
	int menor;
	int maior;
	double mediana;
	double media;
	double dp;
}Regiao;

typedef struct pais{
	int menor;
	int maior;
	double mediana;
	double media;
	double dp;
}Pais;

// variaveis globais e compartilhadas durante o trabalho
int media_mcid, melhor_cidade, mcid_reg, melhor_regiao, media_mr;


// MergeSort retirado do https://www.geeksforgeeks.org/merge-sort/
void merge(int* arr, int l, int m, int r) 
{ 
    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 =  r - m; 
  
    
    int L[n1], R[n2]; 
  
   
    for (i = 0; i < n1; i++) 
        L[i] = arr[l + i]; 
    for (j = 0; j < n2; j++) 
        R[j] = arr[m + 1+ j]; 
  
   
    i = 0; 
    j = 0; 
    k = l; 
    while (i < n1 && j < n2) 
    { 
        if (L[i] <= R[j]) 
        { 
            arr[k] = L[i]; 
            i++; 
        } 
        else
        { 
            arr[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 
  
    while (i < n1) 
    { 
        arr[k] = L[i]; 
        i++; 
        k++; 
    } 
 
    while (j < n2) 
    { 
        arr[k] = R[j]; 
        j++; 
        k++; 
    } 
} 
void mergeSort(int* arr, int l, int r) 
{ 
    if (l < r) 
    { 
        int m = l+(r-l)/2; 
  
        mergeSort(arr, l, m); 
        mergeSort(arr, m+1, r); 
  
        merge(arr, l, m, r); 
    } 
}

void ordena_cidade(int* brasil, int r, int c, int a, Cidade* cidades){
	// variavel de lock
	omp_lock_t mylock;
	omp_init_lock(&mylock);
	
	//// cria uma regiao paralela para percorrer e calcular as operacoes para cada regiao ao mesmo tempo
	#pragma omp parallel num_threads(r*c) shared(media_mcid, melhor_cidade, mcid_reg)// separa por cidades (sem se importar com regiao)
	{
		// cada thread pega o seu id
		int id = omp_get_thread_num();
		int first_i, last_i;
		double som = 0, dp= 0;


		first_i = id*a; // primeira posiçao do primeiro aluno da cidade
		last_i = first_i + a; // ultima posicao

		// ordena cada cidade
		mergeSort(brasil+first_i, 0, a-1);
		// fazendo uma reducao para calcular a soma de todas as notas dos alunos daquela cidade
		#pragma omp parallel for reduction (+:som)
		for(int j=first_i; j<last_i; j++) som+=brasil[j];

		// calculando media e as maiores e menores notas da cidade
		cidades[id].menor = brasil[first_i];
		cidades[id].maior = brasil[last_i-1];
		cidades[id].media = som/a;

		// calculando a mediana da cidade
		if(a%2==0) cidades[id].mediana = (brasil[(first_i+last_i)/2]+brasil[((first_i+last_i)/2)-1])/2.0;
		else cidades[id].mediana = brasil[(first_i+last_i)/2];
		
		// fazendo uma reducao para calcular o desvio padrao da regiao
		#pragma omp parallel for reduction (+:dp)
		for(int k=first_i;k<last_i;k++) dp+= pow(brasil[k]-cidades[id].media, 2);
		
		dp = dp/(a-1);
		cidades[id].dp = sqrt(dp);

		// entao uso lock para acessar ela sem perigo
		omp_set_lock(&mylock);
		if(cidades[id].media>media_mcid){
			
			media_mcid = cidades[id].media;
			melhor_cidade = (id%(r+1));
			mcid_reg = first_i/(a*c);
			
		}
		omp_unset_lock(&mylock);
	}
	return;
}

void ordena_regiao(int* brasil, int r, int c, int a, Regiao* regioes){

	// variaveis de lock
	omp_lock_t mylock;
	omp_init_lock(&mylock);

	// cria uma regiao paralela para percorrer e calcular as operacoes para cada regiao ao mesmo tempo
	#pragma omp parallel num_threads(r) shared(media_mr, melhor_regiao) 
	{
		double dp=0, som=0;

		// cada thread pega o seu id
		int id = omp_get_thread_num();
		int first_i, last_i;

		first_i = id*a*c; // primeira posiçao do primeiro aluno da regiao
		last_i = first_i + a*c; // ultima posicao
		
		//ordena a regiao
		mergeSort(brasil+first_i,0 , (c*a)-1);
		// fazendo uma reducao para calcular a soma de todas as notas dos alunos daquela regiao
		#pragma omp parallel for reduction (+:som)
		for(int j=first_i; j<last_i; j++) som+=brasil[j];
		// calculando media e as maiores e menores notas da região
		regioes[id].menor = brasil[first_i];
		regioes[id].maior = brasil[last_i-1];
		regioes[id].media = som/(a*c);
		
		// calculando a mediana da regiao
		if((a*c)%2==0) regioes[id].mediana = (brasil[(first_i+last_i)/2]+brasil[(first_i+last_i)/2-1])/2.0;
		else regioes[id].mediana = brasil[(first_i+last_i)/2];
		
		// fazendo uma reducao para calcular o desvio padrao da regiao
		#pragma omp parallel for reduction (+:dp)
		for(int k=first_i;k<last_i;k++) dp+= pow(brasil[k]-regioes[id].media, 2);
		dp = dp/((a*c)-1);
		regioes[id].dp = sqrt(dp);
		
		// a variavel media_mr e melhor_regiao são variaveis compartilhadas entre as threads
		// por isso é uma regiao critica, entao uso lock para acessar ela sem perigo
		omp_set_lock(&mylock);
		if(regioes[id].media>media_mr){
			melhor_regiao = id;
			media_mr = regioes[id].media;
		}
		omp_unset_lock(&mylock);
	}
	return;
}

void ordena_brasil(int*brasil, int r, int c, int a, Pais* p, Regiao* regioes){

	double  dp=0, som=0;

	// ordenando toda a matriz
	mergeSort(brasil, 0, r*c*a -1);
	
	// fazendo uma reducao para somar todas as medias das regioes e achar a media do pais
	#pragma omp parallel for reduction (+:som)
	for(int i=0; i<r; i++)som+=regioes[i].media;
	p->media = som/r;
	
	// o menor valor do brasil esta na posicao 0, e o maior na ultima posicao
	p->menor = brasil[0];
	p->maior = brasil[(r*c*a)-1];	
	
	// calculando a mediana
	if((a*c*r)%2==0)p->mediana = (brasil[(r*a*c)/2]+brasil[((r*a*c)/2)-1])/2.0;
	else p->mediana = brasil[((r*a*c)/2)];

	// fazendo uma reducao para calcular o desvio padrao
	#pragma omp parallel for reduction (+:dp)
	for(int i=0;i<(r*a*c);i++) dp+= pow(brasil[i]-p->media, 2);
		
	dp = dp/((a*c*r)-1);
	p->dp = sqrt(dp);
	
}

int main(){
	// Inicializando as variaveis globais
	melhor_cidade = -1;
	melhor_regiao = -1;
	mcid_reg = -1;
	media_mcid = -1;
	media_mr = -1;

	int r, c, a, seed;
	// lendo os parametros do arquivo de entrada
	fscanf(stdin, "%d%d%d%d", &r, &c, &a, &seed);

	
	// alocando a matriz de notas e as structs auxiliares
	int* brasil = (int*)malloc(r*c*a* sizeof(int));
	Cidade* cidades = (Cidade*) malloc(sizeof(Cidade)*r*c);
	Regiao* regioes = (Regiao*) malloc(sizeof(Regiao)*r);
	Pais* p = (Pais*)malloc(sizeof(Pais));

	srand(seed);
	//for(int i=0; i<r*c*a; i++) fscanf(stdin, "%d", &brasil[i]);
	for(int i=0; i<r*c*a; i++) brasil[i] = rand()%101;

	// resolvendo o trabalho em si
	double start = omp_get_wtime();

	ordena_cidade(brasil, r, c, a, cidades);
	
	ordena_regiao(brasil, r, c, a, regioes);  
	ordena_brasil(brasil, r, c, a, p, regioes);
	start = omp_get_wtime()-start;
	
	// trabalho resolvido, agora só resta printar na tela tudo em ordem
	for(int i = 0; i < c*r; i++)
		printf("Reg %d - Cid %d: menor: %d, maior: %d, mediana: %.2lf, media: %.2lf e DP: %.2lf\n", i/(r+1), i%(r+1), cidades[i].menor, cidades[i].maior, cidades[i].mediana, cidades[i].media, cidades[i].dp);
	printf("\n\n");
	
	for(int i = 0; i < r; i++)
		printf("Reg %d - menor: %d, maior: %d, mediana: %.2lf, media: %.2lf e DP: %.2lf\n", i, regioes[i].menor, regioes[i].maior, regioes[i].mediana, regioes[i].media, regioes[i].dp);
	
	printf("\n\n");
	printf("Brasil: menor: %d, maior: %d, mediana: %.2lf, media: %.2lf e DP: %.2lf\n\n", p->menor, p->maior, p->mediana, p->media, p->dp);
	printf("Melhor regiao: Regiao %d\n", melhor_regiao);
	printf("Melhor cidade: Regiao %d, Cidade %d\n\n", mcid_reg, melhor_cidade);
	printf("Time: \t %lf \n", start);

	free(brasil);
	free(cidades);
	free(regioes);
	free(p);
	

	return 0;
}