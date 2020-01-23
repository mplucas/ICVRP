#ifndef __TOOLBOX_H
#define __TOOLBOX_H

#include "vrp.hpp"

typedef struct Tupla{
	double fitness;
	int index;
	struct Tupla* next;
	int* part;
}Tupla;

typedef struct{
	Tupla* head;
	Tupla* tail;
	int size;
}Lista;

void freeMemory(void* var){
	free(var);
	//printf("\n003\n\n");
	var=NULL;
	//printf("\n004\n\n");
}

void printfGene(int* gene, int size){
	int i;
	for(i=0; i<size; i++){
		printf("%d ", gene[i]);
	}
}

void merge(Tupla arr[], int l, int m, int r){
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
 
    /* create temp arrays */
    Tupla L[n1], R[n2];
 
    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];
 
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2)
    {
        if (L[i].fitness <= R[j].fitness)
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
void mergeSort(Tupla arr[], int l, int r){
    if (l < r)
    {
        // Same as (l+r)/2, but avoids overflow for
        // large l and h
        int m = l+(r-l)/2;
 
        // Sort first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
 
        merge(arr, l, m, r);
    }
}

int** readSol(int size, int elements, char *file){
	FILE* instance_file = fopen(file,"rt");
	char buffer[3];
	int** cromo = (int**)malloc(sizeof(int*)*size);
	int i, j;
	for(j=0; j<size; j++){
		cromo[j]=(int*)malloc(sizeof(int)*elements);
		for(i=0; i<elements; i++){
			fread(&buffer, sizeof(char), 3, instance_file);
			if(buffer[0]<'0' || buffer[0]>'9'){
				buffer[0]=0;
			}else{
				buffer[0]-=48;
			}
			
			if(buffer[1]<'0' || buffer[1]>'9'){
				buffer[1]=0;
			}else{
				buffer[1]-=48;
			}
			
			if(buffer[2]<'0' || buffer[2]>'9'){
				buffer[2]=0;
			}else{
				buffer[2]-=48;
			}
			cromo[j][i]=buffer[0]*100+buffer[1]*10+buffer[2];
		}
	}
	fclose(instance_file);
	return cromo;
}

void writeSol(int* sol, int elements, char *file){
	FILE* instance_file = fopen(file,"a+");
	if(instance_file == NULL){
		instance_file = fopen(file,"wt");
	}
	int i;
	for(i=0; i<elements; i++)	fprintf(instance_file,"%3i", sol[i]);
	fclose(instance_file);
}

void writePar(int* partial, int elements, double fit){
	FILE* instance_file = fopen("partial.txt","a+");
	if(instance_file == NULL){
		instance_file = fopen("partial.txt","wt");
	}
	int i;
	fprintf(instance_file,"%f", fit);
	for(i=0; i<elements; i++)	fprintf(instance_file,"%3i", partial[i]);
	fprintf(instance_file,"...\n");
	fclose(instance_file);
}

int random_gen_025(int previous){
	int new_random;
	int sort[] = {19,4,12,17,16,20,10,7,9,11,18,3,23,15,13,2,14,1,21,6,22,8,25,5,24};
	do{
		new_random = rand() %25;
	}while(previous == sort[new_random]);
	return sort[new_random];
}

int random_gen_050(int previous){
	int new_random;
	int sort[] = {17,31,41,30,3,19,43,26,47,20,25,13,35,12,29,15,44,33,14,37,6,2,42,9,34,38,16,49,27,28,22,7,23,5,18,32,46,21,4,40,39,24,11,10,45,36,8,1,50,48};
	do{
		new_random = rand() %50;
	}while(previous == sort[new_random]);
	return sort[new_random];
}

int random_gen_100(int previous){
	int new_random;
	int sort[] = {18,51,39,87,93,13,4,36,32,19,22,67,75,63,27,96,97,47,82,29,98,61,16,55,23,99,5,62,70,90,77,76,15,35,65,37,92,33,26,52,59,81,20,86,6,79,42,17,31,2,56,38,34,85,84,40,
					54,94,73,48,89,50,21,53,95,41,69,91,28,60,49,3,57,58,24,30,72,1,12,46,10,9,83,71,80,66,25,68,88,43,8,14,74,11,78,7,64,45,100,44};
	do{
		new_random = rand() %100;
	}while(previous == sort[new_random]);
	return sort[new_random];
}

int* endTimeVector(Vrp* prob, int elements){
	Tupla* result = (Tupla*)malloc(sizeof(Tupla)*elements);
	bool* visited = (bool*)malloc(sizeof(bool)*elements);
	int* final_sol = (int*)malloc(sizeof(int)*elements);
	int i, index = 0, prev = 0, k = prob->vehicle, j, cont = 0, capacity = 0;
	double* timer_truck = (double*)malloc(sizeof(double)*k);
	bool lost=false;
	double minTime, cost=0.0;
	
	for(i=0; i<elements; i++){
		visited[i]=false;
		result[i].index   =i+1;
		result[i].fitness = prob->dueTime[i+1];
	}
	for(i=0; i<k; i++){
		timer_truck[i]=0.0;
	}
	mergeSort(result, 0, elements-1);
	
	int target;
	////printf("\n\n");
	for(i=0; i<elements; i++){
		target=result[i].index;
		if( (timer_truck[index] + prob->cost[prev][target] <= result[i].fitness)
			&& (capacity + prob->demand[target] <= prob->capacity)
			&& visited[i]==false){

			cost+=prob->cost[prev][target];
			//printf("%d ", target);
			visited[i]=true;
			lost=false;
			final_sol[cont++]=target;

			capacity += prob->demand[target];
			timer_truck[index] += prob->cost[prev][target];

			if(timer_truck[index]<prob->readyTime[target]) timer_truck[index] = prob->readyTime[target];
			

			timer_truck[index] += prob->serviceTime[target];

			prev=target;
		}

		if(i+1 == elements && cont != elements){
			//printf(" |");
			capacity=0;
			timer_truck[index] += prob->cost[prev][0];
			cost+=prob->cost[prev][0];
			prev=0;
			if(lost==true){
				//printf("\nERROR!");
			}
			i=-1;
			index++;
			if(index==k){
				minTime=timer_truck[0];
				index=0;
				for(j=0; j<k; j++){
					if(minTime>timer_truck[j]){
						minTime=timer_truck[j];
						index=j;
					}
				}
			}
			lost=true;
		}
	}
	////printf("\nCost: %.2f\n", cost);
	freeMemory(result);
	freeMemory(visited);
	freeMemory(timer_truck);
	return final_sol;
}

int* startTimeVector(Vrp* prob, int elements){
	Tupla* result = (Tupla*)malloc(sizeof(Tupla)*elements);
	bool* visited = (bool*)malloc(sizeof(bool)*elements);
	/*unsigned*/ int j, k= prob->vehicle, cont=0;
	int* final_sol = (int*)malloc(sizeof(int)*elements);
	int i, capacity=0, index=0, prev=0;
	double* timer_truck = (double*)malloc(sizeof(double)*k);
	bool lost=false;
	double minTime, cost=0.0;
	
	for(i=0; i<elements; i++){
		visited[i]=false;
		result[i].index   =i+1;
		result[i].fitness = prob->readyTime[i+1];
	}
	for(i=0; i<k; i++){
		timer_truck[i]=0.0;
	}
	mergeSort(result, 0, elements-1);
	
	int target;
	////printf("\n\n");
	for(i=0; i<elements; i++){
		target=result[i].index;
		if( (timer_truck[index] + prob->cost[prev][target] <= prob->dueTime[target] )
			&& (capacity + prob->demand[target] <= prob->capacity)
			&& visited[i]==false){

			cost+=prob->cost[prev][target];
			//printf("%d ", target);
			visited[i]=true;
			lost=false;
			final_sol[cont++]=target;

			capacity += prob->demand[target];
			timer_truck[index] += prob->cost[prev][target];

			if(timer_truck[index]<prob->readyTime[target]) timer_truck[index] = prob->readyTime[target];
			

			timer_truck[index] += prob->serviceTime[target];

			prev=target;
		}

		if(i+1 == elements && cont != elements){
			//printf(" |");
			capacity=0;
			timer_truck[index] += prob->cost[prev][0];
			cost+=prob->cost[prev][0];
			prev=0;
			if(lost==true){
				//printf("\nERROR!");
			}
			i=-1;
			index++;
			if(index==k){
				minTime=timer_truck[0];
				index=0;
				for(j=0; j<k; j++){
					if(minTime>timer_truck[j]){
						minTime=timer_truck[j];
						index=j;
					}
				}
			}
			lost=true;
		}
	}
	////printf("\nCost: %.2f\n", cost);
	freeMemory(result);
	freeMemory(visited);
	freeMemory(timer_truck);
	return final_sol;
}

int*  mix_half(int* gene, int elements){
	int i;
	int* result = (int*)malloc(sizeof(int)*elements);
	int half;

	// maybe if elements is odd than it will cause error
	if(elements%2==1)	half= (elements+1)/2;
	else	half= elements/2;

	for(i=0; i<half-1; i++)	result[i] = gene[half + i];

	for(i=half-1; i<elements; i++)	result[i] = gene[i - half];
	
	return result;
}
int*  mix_complement(int* gene, int elements){
	int i;
	int* result = (int*)malloc(sizeof(int)*elements);
	for(i=0; i<elements; i++){
		result[i]= elements+1 - gene[i];
	}

	return result;
}
int*  mix_invert(int* gene, int elements){
	int i;
	int* result = (int*)malloc(sizeof(int)*elements);
	for(i=0; i<elements; i++){
		result[i]= gene[elements-1-i];
	}

	return result;
}

bool checkConsistency(int* v, int elements){

	bool visited[elements];
	for (int i = 0; i < elements; i++)
	{
		visited[i]=false;
	}

	for (int i = 0; i < elements; i++)
	{
		if(visited[v[i]-1])	return false;
		else visited[v[i]-1] = true;
	}

	for (int i = 0; i < elements; i++){
		if(visited[i]==false) return false;
	}

	return true;
}

int* cost_shortcut(Vrp* prob){
	int* result= (int*)malloc(sizeof(int)*(prob->client-1));;
	int i, j;
	Tupla lower[prob->client][prob->client];
	for(i=0; i<prob->client; i++){
		for (j = 0; j < prob->client; j++){
			lower[i][j].fitness = prob->cost[i][j];
			lower[i][j].index = j;
		}
		mergeSort(lower[i], 0, prob->client-1);
	}
	int index=0, dest, prev=0, capacity=0, k=0;
	double timer=0.0, acum[prob->vehicle];
	bool found, flip=false, visited[prob->client-1];
	for(i=0; i<prob->vehicle;   i++) acum[i]=0;
	for(i=0; i<prob->client -1; i++) visited[i]=false;
	do{
		found=false;
		for(i=1; i<prob->client; i++){
			if(lower[prev][i].index == 0) i++;
			dest = lower[prev][i].index;
			if((timer + prob->cost[prev][dest] <= prob->dueTime[dest]) && (capacity + prob->demand[dest] <= prob->capacity) && !visited[dest]){
				visited[dest]=true;
				found=true;
				flip=false;
				result[index++]=dest;
				timer += prob->cost[prev][dest];
				if(timer < prob->readyTime[dest]) timer=prob->readyTime[dest];
				timer+=prob->serviceTime[dest];

				prev=dest;
				i=prob->client;
			}
		}
		if(!found && index < prob->client -1){
			acum[k++]+=timer + prob->cost[prev][0];
			timer=0;
			capacity=0;
			if(flip){
				free(result);
				return NULL;
			}else{
				if(k>=prob->vehicle){
					k=0;
					timer=acum[0];
					for(i=1; i<prob->vehicle; i++){
						if(timer>acum[i]){
							k=i;
							timer=acum[i];
						}
					}
				}
			}
		}
	}while(index < prob->client-1);
	return result;
}

int* copyPart(int* part, int size){
	int i;
	int* dual = (int*)malloc(sizeof(int)*size);
	for(i=0; i<size; i++){
		dual[i]=part[i];
	}
	return dual;
}

void printfPart(int* part){
	int i;
	for(i=1; i<part[0]+1; i++){
		printf("%d ", part[i]);
	}
}

Lista* newList(){
	Lista* list = (Lista*)malloc(sizeof(Lista));
	list->size=0;
	return list;
}

Tupla* newNode(Lista* list){
	Tupla* node = (Tupla*)malloc(sizeof(Tupla));
	if(list->head==NULL){
		list->head = node;
		list->tail = node;
	}else{
		list->tail->next = node;
		list->tail		 = node;
	}
	node->index = list->size;
	node->next = NULL;
	list->size+=1;
	return node;
}

void freeLista(Lista* list){
	Tupla *atual = list->head;
	Tupla *next;
	if(atual==NULL) return;
	do{
		free(atual->part);
		atual->part=NULL;
		next=atual->next;
		free(atual);
		atual=next;
	}while(next!=NULL);
	list->head=NULL;
	list->tail=NULL;
	list->size=0;
	free(list);
	list=NULL;
}

void setVisit(bool v[], int size, bool signal){
	int i;
	for(i=0; i<size; i++) v[i]=signal;
}

bool intersectionPart(int* v, bool m[]){
	int i;
	for(i=1; i< v[0]+1; i++){
		if(m[v[i]-1]) return true;
	}
	return false;
}

void setPart(int* v, bool m[], bool signal){
	int i;
	for(i=1; i< v[0]+1; i++){
		m[v[i]-1] = signal;
	}
}

bool allSet(bool m[], int size){
	int i;
	for(i=0; i<size; i++){
		if(m[i]==false) return false;
	}
	return true;
}

void newBest(int* result, int* best){
	int i;
	for(i=0; i<result[0]+1; i++) best[i]=result[i];
}

int* agrupamento(int toAdd, bool visited[], int elements, Tupla parts[], int tSize,
	int* result, int cont, Tupla original[], int* best, double* bestFit, double sum){
	
	if(toAdd>=tSize) return best;
	
	int i;
	result[cont++]=parts[toAdd].index;
	sum+=original[result[cont-1]].fitness;

	if(cont>25 && best[0]!=0){
		visited[elements]=true;
		return best;
	}

	if(best[0]!=0){
		if(sum - *bestFit > 0.0){
			visited[elements]=true;
			return best;
		}
	}

	if(!intersectionPart(parts[toAdd].part, visited)){
		setPart(parts[toAdd].part, visited, true);
		if(allSet(visited, elements)){
			result[0]=cont;
			
			*bestFit = sum;
			//printf("\nFitness: %.2f, parts: %d", sum, cont);
			newBest(result, best);
			visited[elements]=true;
		}else{
			for(i=toAdd+1; i<tSize; i++){
				best=agrupamento(i, visited, elements, parts, tSize, result, cont, original, best, bestFit, sum);
				if(visited[elements]==true){
					visited[elements]=false;
					i=tSize;
				}
			}
		}
		setPart(parts[toAdd].part, visited, false);
	}
	return best;
}

int* arranjo(int elements, Tupla parts[], int tSize, Tupla original[]){
	bool visited[elements+1];
	int i, cont=1;
	int* result = (int*)malloc(sizeof(int)*elements+1);
	int* best = (int*)malloc(sizeof(int)*elements+1);
	double bestFit, sum=0.0;
	best[0]=0;
	for(i=0; i<tSize; i++){
		//printf("\nCiclo %d/%d", i+1, tSize);
		setVisit(visited, elements, false);
		best = agrupamento(i, visited, elements, parts, tSize, result, cont, original, best, &bestFit, sum);
	}
	return best;
}
#endif
