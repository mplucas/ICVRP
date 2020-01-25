#ifndef __AG_H
#define __AG_H

#include "toolbox.hpp"
// #include <stdio.h>
// #include <vector>
// #include <cmath>
// #include <stdlib.h>
// #include <ctime>
//#include <windows.h>

using namespace std;

typedef struct{
	
	int id;
	int x, y;
	vector<int> assigneds;

}k_cluster;

class Genetic{
	private:
		//General variables:
		int pop_size;		//Overval population size.
		int elements;		//Elements per cromossome.
		int** pop;			//Overall Population.
		int generation;		//Amount of cicles GA will have.
		int curr_cicle;		//Current generation being developed
		char* source;
		Vrp* problem;
		bool readBackup;
		double bestPrior;
		double final_fitness;
		int lostCicle;
		bool titles;
		bool details;

		//Validation variables:
		double* generalFitness;
		double* newFitness;
		
		//Selection Variables:
		int** new_gen;		//New population from cicle
		int new_gen_pop;	//Amount of new members generated.
		int sel_rate;
		
		//Crossover variables:
		int max_interval;
		int cross_rate;
		int swap_factor;
		
		//Mutation variables:
		double mut_Rate;
		double mut_base;
		
		//Aux internal functions
		int random_percent_gen(int);
		int random_element_gen(int);
		double breakRoute(bool, int*);
		void cross_correction(int, int, int*, int*);
		void chooseInterval(int*, int*);
		int* copyGene(int*);
		double mutation_prob();
		void clearPopulation();
		void clearCrossover();
		void clearFitnessess();
		int* random_gene();
		int* k_means(int k, int max_x, int max_y);
		void printPop(int**, int);
		void update();
		double breakRoute2(int*);
		void swapGenes();
	
	public:
		Genetic(int, int, int, int, int, char[], bool, int);//Completed
		void populate();									//Completed
		void setPopulation(int**, int);						//Completed
		void validate();									//Completed
		void selection();									//completed
		void crossover();									//Completed
		void mutation();									//Completed
		void recovery();									//Completed
		void solve();										//Completed
		void displayBest();									//Completed
		void destroy();										//Completed
		double getImprovRate();
		double getFitness();
};

Genetic::Genetic(int population_size, int selection_rate, int cross_percent, int max_cross, int gen, char* file_source, bool backup, int description){
	switch(description){
		case 0:
			titles  = false;
			details = false;
		break;
		case 1:
			titles  = true;
			details = false;
		break;
		default:
			titles  = true;
			details = true;
		break;
	}

	if(titles) printf("\nPRE-RUN [1/2]: CONSTRUCTING GA.");
	if(details) printf("\n\t-Checking variables.");
	
	if(population_size<=0) population_size=2;
	if(population_size%2!=0) population_size+=1;
	
	if(details) printf("\n\t-Setting variables.");
	pop       = NULL;
	new_gen   = NULL;
	newFitness   = NULL;
	generalFitness   = NULL;
	readBackup = false;//backup;
	max_interval = max_cross;
	pop_size=population_size;
	sel_rate = selection_rate;
	new_gen_pop=selection_rate*pop_size/100;
	if(new_gen_pop==0) new_gen_pop=2;
	generation = gen;
	cross_rate=cross_percent;
	mut_Rate=10;
	swap_factor=0;
	lostCicle=0;
	curr_cicle=0;

	if(details) printf("\n\t-Reading problem.");
	problem = Vrp_init(file_source);
	elements = problem->client-1;
	if(details) printf(" [%d Clients instance]", elements);
	mut_base=0.1;
	//printf("\nF\n\n");
}

double Genetic::getImprovRate(){

	return 100.00-(double)100*final_fitness/bestPrior;
}

double Genetic::getFitness(){

	return final_fitness;
}

void Genetic::clearPopulation(){
	if(pop!=NULL)	for(int i=0; i<pop_size; i++)	freeMemory(pop[i]);
	freeMemory(pop);
}

void Genetic::clearCrossover(){
	int i;
	if(new_gen!=NULL)	for(i=0; i<new_gen_pop; i++)	freeMemory(new_gen[i]);
	freeMemory(new_gen);
}

void Genetic::clearFitnessess(){
	freeMemory(newFitness);
	freeMemory(generalFitness);
}

void Genetic::destroy(){
	clearPopulation();
	clearCrossover();
	clearFitnessess();
	eraseInstance(problem);
}

int Genetic::random_element_gen(int previous){
	int new_random;
	do{
		new_random = rand()%elements;
	}while(previous == new_random);
	return new_random;
}

int Genetic::random_percent_gen(int previous){
	int new_random;
	int sort[100] = {24, 61, 28, 17, 36, 43, 97, 71, 60, 11, 64, 99, 74, 67, 59, 68, 16, 13, 34, 51, 76, 6, 72, 50, 37, 1, 70, 20, 2, 14, 25, 12, 18,
					10, 30, 5, 65, 83, 21, 45, 27, 31, 93, 90, 69, 88, 38, 47, 87, 40, 94, 19, 4, 80, 62, 82, 54, 86, 53, 42, 55, 96, 89, 26, 35, 49, 41, 44, 29, 56,
					8, 23, 77, 85, 39, 57, 92, 0, 95, 7, 58, 78, 63, 98, 3, 66, 48, 22, 81, 33, 15, 46, 91, 73, 75, 84, 32, 79, 52, 9 };
	do{
		new_random = rand() %100;
	}while(previous == sort[new_random]);
	return sort[new_random];
}

void Genetic::cross_correction(int pt1, int pt2, int* a, int* b){
	int i, j;
	//int collision[pt2-pt1];
	int* collision = (int*)malloc(sizeof(int)*(pt2-pt1));
	int* frag1 = (int*)malloc(sizeof(int)*(pt2-pt1));
	int* frag2 = (int*)malloc(sizeof(int)*(pt2-pt1));

	int c1=0, c2=0, ind1=0, ind2=0;
	bool m1, m2;
	
	//Find missing elements.
	for(i=pt1; i<pt2; i++){
		m1=true;
		m2=true;
		for(j=pt1; j<pt2; j++){
			if(a[i]==b[j])	m1=false;
			if(b[i]==a[j])	m2=false;
		}
		if(m1){
			frag1[ind1++]=a[i];
		}
		if(m2){
			frag2[ind2++]=b[i];
		}
	}
	
	//Removing conflicting members
	for(i=0; i<pt1; i++){
		for(j=0; j<ind1; j++){
			if(a[i]==frag1[j]){
				a[i]=frag2[j];
				c1++;
			}
			if(b[i]==frag2[j]){
				b[i]=frag1[j];
				c2++;
			}
			if(c1==ind1 && c2==ind2){
				i=pt1;
				j=ind1;
			}
		}
	}
	for(i=pt2; i<elements; i++){
		for(j=0; j<ind1; j++){
			if(a[i]==frag1[j]){
				a[i]=frag2[j];
				c1++;
			}
			if(b[i]==frag2[j]){
				b[i]=frag1[j];
				c2++;
			}
			if(c1==ind1 && c2==ind2){
				i=elements;
				j=ind1;
			}
		}
	}

	freeMemory(collision);
	freeMemory(frag1);
	freeMemory(frag2);
}

void Genetic::chooseInterval(int* pointer1, int* pointer2){
	int pt1=-1, pt2=-1;
	bool valid=false;
	
	do{
		switch(elements){
			case 25:
				pt1 = random_gen_025(pt2);
				pt2 = random_gen_025(pt1);
			break;
			case 50:
				pt1 = random_gen_050(pt2);
				pt2 = random_gen_050(pt1);
			break;
			case 100:
				pt1 = random_gen_100(pt2);
				pt2 = random_gen_100(pt1);
			break;
			default:
				pt1=random_element_gen(pt2);
				pt2=random_element_gen(pt1);
			break;
		}
		if(pt1>=elements-2){
			pt1=elements-2;
		}
		if(pt2>=elements-2){
			pt2=elements-2;
		}
		if(pt1>pt2){
			int tmp = pt1;
			pt1=pt2;
			pt2=tmp;
		}
		
		int inter = pt2-pt1;
		
		if((double)((100*inter)/elements) < max_interval && pt1!=pt2){
			valid=true;
		}
	}while(valid==false);
	*pointer1=pt1;
	*pointer2=pt2;
}

int* Genetic::copyGene(int* source){
	int* gene = (int*)malloc(sizeof(int)*elements);
	int i;
	for(i=0; i<elements; i++) gene[i]=source[i];
	
	return gene;
}

double Genetic::mutation_prob(){
	double sum=0;
	int i;
	for(i=0; i<pop_size; i++){
		sum+=newFitness[i];
	}
	double media = sum/pop_size;
	double somatorio = 0;
	for(i=0; i<pop_size; i++)	somatorio = (newFitness[i]-media)*(newFitness[i]-media) + somatorio;
	
	double partial = somatorio/(pop_size-1);
	double S = sqrt(partial);
	if(S>= 5) return mut_base;
	return mut_base + 0.18*(5-S);
}

void Genetic::setPopulation(int** new_pop, int new_pop_size){
	clearPopulation();
	clearCrossover();
	
	pop      = new_pop;
	pop_size = new_pop_size;
}

void printGene(int* gene, int gene_size){

	for(int i = 0; i < gene_size; i++){
		printf(" %i", gene[i]);
	}

}

void plotKMeans( vector<k_cluster> clusters, int max_x, int max_y, int** info ){

	int matrix[max_y][max_x];

	for(int i = 0; i < max_y; i++){
		for(int j = 0; j < max_x; j++){
			matrix[i][j] = -1;
		}
	}

	for(unsigned int i = 0; i < clusters.size(); i++){	
		printf("cluster %i:\n", clusters[i].id);
		for(unsigned int j = 0; j < clusters[i].assigneds.size(); j++){
			printf(" %i", clusters[i].assigneds[j]);
			matrix[info[clusters[i].assigneds[j]][1]][info[clusters[i].assigneds[j]][0]] = clusters[i].id;
		}
		matrix[clusters[i].y][clusters[i].x] = -2;
		printf("\n");	
	}
	
	for(int i = 0; i < max_y; i++){
		printf("|");
		for(int j = 0; j < max_x; j++){
			if(matrix[i][j] == -1){
				printf(" ");
			}else if(matrix[i][j] == -2){
				printf("*");
			}else{
				printf("%i", matrix[i][j]);
			}
		}
		printf("|\n");
	}

}

// returns a randomic population
int* Genetic::random_gene(){
	int* gene = (int*)malloc(sizeof(int)*elements);
	bool* setted = (bool*)malloc(sizeof(bool)*elements);
	int i, j, choosen=-1;
	bool got_it=false;
	for(i=0; i<elements; i++){
		setted[i]=false;
		gene[i]=-1;
	}
	for(i=0; i<elements; i++){
		switch(elements){
			case 25: 	choosen = random_gen_025(choosen);	break;
			case 50: 	choosen = random_gen_050(choosen);	break;
			case 100:	choosen = random_gen_100(choosen);	break;
			default:	choosen = (rand() % elements) + 1;	break; //aqui
		}
		choosen-=1;
		//printf("\nchoosen %i\n", choosen);
		//printf("\nsetted[choosen] %i\n", setted[choosen]);
		if(setted[choosen]==false){
			gene[i] = choosen+1;
			setted[choosen]=true;
		}else{
			got_it=false;
			for(j=choosen; j<elements; j++){
				if(setted[j]==false){
					gene[i] = j+1;
					setted[j]=true;
					got_it=true;
					j=elements;
				}
			}
			if(got_it==false){
				for(j=choosen; j>=0; j--){
					if(setted[j]==false){
						gene[i] = j+1;
						setted[j]=true;
						got_it=true;
						j=-1;
					}
				}
				if(got_it==false)	printf("\nERROR!\n");
			}
		}
		//printf("\n0a\n\n");
	}
	//printf("\n001\n\n");
	// for(int i = 0; i < elements; i++){
	// 	printf("\n%i %i\n", i, setted[i]);
	// }
	freeMemory(setted);
	//printf("\n002\n\n");
	return gene;
}

// returns a population generated by kmeans
// atributes:
	// k     : number of clusters
	// max_x : maximum size of the bidimensional map in x coordinate
	// max_y : maximum size of the bidimensional map in y coordinate
int* Genetic::k_means(int k, int max_x, int max_y){

	vector< k_cluster > clusters(k);
	
	// starting variables
	for( int i = 0; i < k; i++ ){
		clusters[i].id = i;
		
		// position from 0 to 200
		clusters[i].x  = rand() % (max_x + 1);
		clusters[i].y  = rand() % (max_y + 1);
	}

	// assigning nodes to clusters
	for( int i = 1; i < problem->client; i++ ){

		int current_x = problem->info[i][0];
		int current_y = problem->info[i][1];
		int choosen_cluster_id = 0;
		int min_distance = INT_MAX; // infinity

		// comparing node distance to all clusters centers
		for( int j = 0; j < k; j++ ){
			
			double current_distance = distanceAB( clusters[j].x, clusters[j].y, current_x, current_y );
			if( current_distance < min_distance ){
				min_distance = current_distance;
				choosen_cluster_id = j;
			}

		}

		// assigning
		clusters[choosen_cluster_id].assigneds.push_back(i);
		
		// recalculating cluster center
		clusters[choosen_cluster_id].x += current_x;
		clusters[choosen_cluster_id].x /= 2;

		clusters[choosen_cluster_id].y += current_y;
		clusters[choosen_cluster_id].y /= 2;
	
	}

	// building return to the default format
	int* gene = (int*)malloc(sizeof(int)*elements);
	int  gene_size = 0;

	for( int i = 0; i < k; i++ ){
		for( unsigned int j = 0; j < clusters[i].assigneds.size(); j++ ){
			gene[gene_size] = clusters[i].assigneds[j];
			gene_size++;
		}
	}

	// printf("\n\n");
	// plotKMeans( clusters, max_x, max_y, problem->info );
	// printf("\n\n");
	return gene;

}

double Genetic::breakRoute(bool printRoutes, int* major){
	if(major==NULL) return -1.0;

	int i, k, index=0, prev=0;
	/*unsigned*/ int capacity=0;
	double finalTime[problem->vehicle], timer=0.0;
	vector<vector<int>> routes(problem->vehicle);
	bool first_track=true, flip=false;
	double fitness=0, min;
	int dest;
	
	for(i=0; i<problem->vehicle; i++){
		finalTime[i]=0.0;
		routes[i].push_back(0);
	} 

	for(i=0; i<elements; i++){	
		dest = major[i];
		if( (timer + problem->cost[prev][dest] > problem->dueTime[dest]) ||
			(capacity +problem->demand[dest] > problem->capacity)){
			
			//printf(" | ");
			if(flip==true){
				fitness=-1.0;
				return fitness;
			}
			flip=true;

			// send vehicle back to depot
			timer += problem->cost[prev][0];
			fitness += problem->cost[prev][0];
			
			routes[index].push_back(0);
			finalTime[index++] += timer;
			
			prev	 = 0;
			timer    = 0;
			capacity = 0;

			if(first_track==false || index>=problem->vehicle){
				first_track=false;
				min=finalTime[0];
				index=0;
				for(k=1; k<problem->vehicle; k++){
					if(finalTime[k]<min){
						index=k;
						min=finalTime[k];
					}
				}
				timer=min;
			}
			i--;
		}else{
			timer += problem->cost[prev][dest];
			fitness += problem->cost[prev][dest];
			capacity +=  problem->demand[dest];
			routes[index].push_back(dest);
			
			//printf("%d ", major[i]);
			flip=false;
			
			if(timer < problem->readyTime[dest])	timer = problem->readyTime[dest];
			
			timer += problem->serviceTime[dest];
			prev=dest;
		}
	}
	if(printRoutes){

		for(unsigned int i = 0; i < routes.size(); i++){
			printf("Route %i: ", i);
			for(unsigned int j = 0; j < routes[i].size(); j++){
				printf("%i ", routes[i][j]);
			}
			printf("\n");
		}

	}
	return fitness;
}

double Genetic::breakRoute2(int* major){
	if(major==NULL) return -1.0;
	int* parcial = (int*)malloc(sizeof(int)*(elements+1));
	int i, /*k, index=0,*/ prev=0;
	/*unsigned*/ int capacity=0;
	double timer=0.0;
	double fitness=0/*, min*/;
	int dest, passo=0, cont=0;
	Lista* list = newList();
	//int contador=0;
	for(i=0; i<elements; i++){	
		dest = major[i];
		if( (timer + problem->cost[prev][dest] > problem->dueTime[dest]) ||
			(capacity +problem->demand[dest] > problem->capacity)){
			cont++;

			prev	 = 0;
			timer    = 0.0;
			capacity = 0.0;
			fitness  = 0.0;
			i=cont;
			passo=0;
		}else{
			passo+=1;
			parcial[passo]=dest;
			parcial[0]=passo;

			timer += problem->cost[prev][dest];
			fitness += problem->cost[prev][dest];
			capacity +=  problem->demand[dest];
			
			Tupla* node = newNode(list);
			node->part = copyPart(parcial, passo+1);
			node->fitness = fitness + problem->cost[dest][0];
			////printf("\n%d", list->size);
			if(timer < problem->readyTime[dest])	timer = problem->readyTime[dest];
			
			timer += problem->serviceTime[dest];
			prev=dest;
		}
	}
	int tSize = list->size;
	Tupla order[tSize];
	Tupla original[tSize];
	Tupla* node = list->head;
	for(i=0 ; i<tSize; i++){
		order[i].index = node->index;
		order[i].fitness = node->fitness/(double)node->part[0];
		order[i].part = copyPart(node->part, node->part[0]+1);
		
		original[i].index = node->index;
		original[i].fitness = node->fitness;
		original[i].part = copyPart(node->part, node->part[0]+1);

		node = node->next;
	}
	mergeSort(order, 0, tSize-1);
	
	printf("\nLista: %d", tSize);
	for(i=0; i<list->size; i++){
		printf("\n%.2f", original[i].fitness);
	}
	freeLista(list);
	printf("\nStandard: %.2f", breakRoute(false, major));
	int* final = arranjo(elements, order, tSize, original);
	double sum=0.0;
	if(final!=NULL){
		printf("\nGrupos: ");
		for(i=1; i<final[0]+1; i++){
			//printf("\n%d: ", final[i]);
			//printfPart(original[final[i]].part);
			sum += original[final[i]].fitness;
		}
	}else{
		printf("\nNone found.");
	}
	printf("\nFitness: %.2f", sum);
	printf("\nStandard: %.2f", breakRoute(false, major));

	return 0.0;
}

void Genetic::printPop(int** popu, int le_size){
	int i, j;
	for(j=0; j<le_size; j++){
		printf("\nGene %2d: ", j);
		for(i=0; i<elements; i++){
			printf("%d ", popu[j][i]);
		}
	}
}

void Genetic::swapGenes(){
	int i;
	int* tmp;
	for(i=0; i<new_gen_pop/2; i++){
		if(i%2==swap_factor){
			tmp=new_gen[i];
			new_gen[i]=new_gen[new_gen_pop-1-i];
			new_gen[new_gen_pop-1-i]=tmp;
		}
	}
	if(swap_factor==0) swap_factor=1;
	else swap_factor=0;
}


void Genetic::solve(){
	/*unsigned*/ int i;
	//breakRoute2(cost_shortcut(problem));
	//printf("\nCost: %.2f", breakRoute(false, cost_shortcut(problem)));
	//printf("\n0\n\n");
	populate();
	//printf("\n1\n\n");
	if(titles) printf("\n------------------------------------------------");
	for(i=0; i<generation; i++){
		if(titles)
			printf("\n-------------------CICLE %2d/%2d------------------", i+1, generation);
		selection();
		//printf("\n2\n\n");
		crossover();
		//printf("\n3\n\n");
		mutation();
		//printf("\n4\n\n");
		validate();
		recovery();
		update();
	}
	displayBest();
	
}

void Genetic::populate(){
	if(titles) printf("\n\nPRE-RUN [2/2]: CREATING POPULATION");
	double fit=0.0;
	int i;
	bestPrior = -1.0;

	generalFitness = (double*)malloc(sizeof(double)*pop_size);
	pop = (int**)malloc(sizeof(int*)*pop_size);

	if(details) printf("\n\t-Creating %d genes.", pop_size);
	
	char instance[]={"pop_000.txt"};
	switch(elements){
			case 25:
				instance[5] = '2';
				instance[6] = '5';
			break;
			case 50:
				instance[5] = '5';
				break;
			case 100:
				instance[4] = '1';
			break;
	}
	// if(elements!=100 && elements!=50 && elements!=25 ) readBackup=false;
	if(readBackup){ // readBackup allways false
		//printf("\n02\n\n");
		pop=readSol(pop_size, elements, instance);
		//printf("\n03\n\n");
		for(i=0; i<pop_size; i++){
			generalFitness[i]=breakRoute(false, pop[i]);
			if(generalFitness[i]<bestPrior || bestPrior<0) bestPrior = generalFitness[i];
		}
		//printf("\n04\n\n");
	}else{

		// size of populations generated by different methods
		float randomic_pop_size = ceil(float(pop_size)*0.4); // 40%
		float kmeans3_pop_size  = ceil(float(pop_size)*0.6); // 20%
		float kmeans4_pop_size  = ceil(float(pop_size)*0.8); // 20% (the 20% missing is for kmeans5)

		bool tryFirst = false;
		i = 0;

		// randomic generation
		do{
			// printf("a");
			// printf("\ni: %i\n",i);
			if(tryFirst){
				int* tmp;

				tmp=endTimeVector(problem, elements);
				// printf("\n01\n\n");
				fit = breakRoute(false, tmp);
				// printf("\n02\n\n");
				if(fit == -1 || !checkConsistency(tmp, elements) || i>=randomic_pop_size){
					freeMemory(tmp);
				}else{
					if(fit<bestPrior || bestPrior<0) bestPrior = fit;
					generalFitness[i]=fit;
					pop[i++]=tmp;
				}
				// printf("\n03\n\n");

				tmp=startTimeVector(problem, elements);
				// printf("\n04\n\n");
				fit = breakRoute(false, tmp);
				// printf("\n05\n\n");
				if(fit == -1 || !checkConsistency(tmp, elements) || i>=randomic_pop_size){
					freeMemory(tmp);
				}else{
					if(fit<bestPrior || bestPrior<0) bestPrior = fit;
					generalFitness[i]=fit;
					pop[i++]=tmp;
				}
				// printf("\n06\n\n");

				tmp=cost_shortcut(problem);
				fit = breakRoute(false, tmp);
				if(fit == -1 || !checkConsistency(tmp, elements) || i>=randomic_pop_size){
					freeMemory(tmp);
				}else{
					if(fit<bestPrior || bestPrior<0) bestPrior = fit;
					generalFitness[i]=fit;
					pop[i++]=tmp;
				}
				// printf("\n07\n\n");

				tryFirst=false;
			}
			// printf("b");

			int* a = random_gene();
			//printf("\n08\n\n");
			// for(int j = 0; j < elements; j++){
			// 	printf("%i ",a[j]);
			// }
			//printf("\n");
			int* b = mix_half(a, elements);
			// printf("\n09\n\n");
			int* c = mix_invert(a, elements);
			// printf("\n010\n\n");
			int* d = mix_complement(a, elements);
			// printf("\n011\n\n");


			fit = breakRoute(false, a);
			// printf("\n012\n\n");
			if(fit!=-1 && i<randomic_pop_size && checkConsistency(a, elements)){
				if(fit<bestPrior || bestPrior<0) bestPrior = fit;
				generalFitness[i]=fit;
				pop[i++]=a;
			}else	freeMemory(a);

			fit = breakRoute(false, b);
			// printf("\n013\n\n");
			if(fit!=-1 && i<randomic_pop_size && checkConsistency(b, elements)){
				if(fit<bestPrior || bestPrior<0) bestPrior = fit;
				generalFitness[i]=fit;
				pop[i++]=b;
			}else	freeMemory(b);
			
			fit = breakRoute(false, c);
			// printf("\n014\n\n");
			if(fit!=-1 && i<randomic_pop_size && checkConsistency(c, elements)){
				if(fit<bestPrior || bestPrior<0) bestPrior = fit;
				generalFitness[i]=fit;
				pop[i++]=c;
			}else	freeMemory(c);

			fit = breakRoute(false, d);
			// printf("\n015\n\n");
			if(fit!=-1 && i<randomic_pop_size && checkConsistency(d, elements)){
				if(fit<bestPrior || bestPrior<0) bestPrior = fit;
				generalFitness[i]=fit;
				pop[i++]=d;
			}else	freeMemory(d);

		}while(i<randomic_pop_size);

		// k-means generations
		// k = 3
		while(i < kmeans3_pop_size){

			int* a = k_means(3, elements, elements);
			fit = breakRoute(false, a);

			if(fit!=-1 && i<pop_size && checkConsistency(a, elements)){
				if(fit<bestPrior || bestPrior<0) bestPrior = fit;
				generalFitness[i]=fit;
				pop[i++]=a;
			}else	freeMemory(a);

		}
		// k = 4
		while(i < kmeans4_pop_size){

			int* a = k_means(4, elements, elements);
			fit = breakRoute(false, a);

			if(fit!=-1 && i<pop_size && checkConsistency(a, elements)){
				if(fit<bestPrior || bestPrior<0) bestPrior = fit;
				generalFitness[i]=fit;
				pop[i++]=a;
			}else	freeMemory(a);

		}
		// k = 5
		while(i < pop_size){

			int* a = k_means(5, elements, elements);
			fit = breakRoute(false, a);

			if(fit!=-1 && i<pop_size && checkConsistency(a, elements)){
				if(fit<bestPrior || bestPrior<0) bestPrior = fit;
				generalFitness[i]=fit;
				pop[i++]=a;
			}else	freeMemory(a);

		}

		//rem(instance);
		for(i=0; i<pop_size; i++)	writeSol(pop[i], elements, instance);
	}
}

void Genetic::validate(){
	if(titles) printf("\n\nStage 4: VALIDATING");
	int i;

	if(details) printf("\n\t-Checking feasability.");

	for(i=0; i<new_gen_pop; i++){
		newFitness[i]=breakRoute(false, new_gen[i]);
	}
}

void Genetic::selection(){
	if(titles) printf("\n\nStage 1: SELECTION");
	curr_cicle+=1;
	if(new_gen_pop==0) new_gen_pop=1;
	new_gen=(int**)malloc(sizeof(int*)*new_gen_pop);
	newFitness=(double*)malloc(sizeof(double)*new_gen_pop);
	
	int i;
	
	for(i=0; i<new_gen_pop; i++){
		new_gen[i]=NULL;
	}

	double* fitness_inv = (double*)malloc(sizeof(double)*pop_size);
	double sum =0, sum_inv=0;
	if(details) printf("\n\t-Calculating roullet.");
	for(i=0; i<pop_size; i++){
		sum+=generalFitness[i];
	}
	for(i=0; i<pop_size; i++){
		fitness_inv[i] = sum - generalFitness[i];
		sum_inv += fitness_inv[i];
	}
	double* roullet = (double*)malloc(sizeof(double)*pop_size);
	double percent;
	sum=0.0;
	for(i=0; i<pop_size; i++){
		percent = 100.0*fitness_inv[i]/sum_inv;
		roullet[i]= sum + percent;
		sum+=percent;
	}
	if(details) printf("\n\t-Setting up roullet.");
	double* choosen = (double*)malloc(sizeof(double)*pop_size);;
	double* choosen_order = (double*)malloc(sizeof(double)*pop_size);;
	double passo = (double)(100/new_gen_pop);
	int num = random_percent_gen(-1);
	int break_point=0;
	choosen[0] = (double)num;
	for(i=1; i<new_gen_pop; i++){
		choosen[i]= choosen[i-1] + (double)(passo);
		if(choosen[i]>=100.0){
			choosen[i]-=100.0;
			break_point=i;
		}
	}
	int index=0;
	for(i=break_point; i<new_gen_pop; i++)	choosen_order[index++]=choosen[i];
	for(i=0; i<break_point; i++)	choosen_order[index++]=choosen[i];
	for(i=0; i<pop_size; i++){
		
	}
	if(details) printf("\n\t-Picking champions. [%d]", new_gen_pop);
	
	index=0;
	for(i=0; i<pop_size; i++){
		if(choosen_order[index]>99.99) choosen_order[index]=99.99;

		if((int)roullet[i]>=(int)choosen_order[index] || (roullet[i]>=100.0 && choosen_order[index]>=100.0)){
			new_gen[index]=copyGene(pop[i]);
			newFitness[index]=generalFitness[i];
			i--;
			index++;
			if(index>=new_gen_pop){
				i=pop_size;
			}
		}
	}

	freeMemory(fitness_inv);
	freeMemory(roullet);
	freeMemory(choosen);
	freeMemory(choosen_order);
}

void Genetic::crossover(){
	int i=0, j=0, counter=0;
	bool keep_going = true;
	int num=-1;
	if(titles) printf("\n\nStage 2: CROSSOVER");
	if(details) printf("\n\t-Gambling for crossover.");
	int count=0;
	int* sort = (int*)malloc(sizeof(int)*new_gen_pop);
	if(cross_rate!=100){
		for(i=0; i<new_gen_pop; i++){
			num = random_percent_gen(num);
			if(num<=cross_rate)	sort[count++]=i;
		}
		if(count<new_gen_pop)	sort[count]=-1;
	}else{
		for(i=0; i<new_gen_pop; i++){
			sort[i]=i;
		}
		count=i;
	}
	if(details) printf(" [%i picked-up]", count);
	i=0;
	int point1, point2;
	if(count>=2){
		if(details) printf("\n\t-Defining interval.");
		chooseInterval(&point1, &point2);
		if(details){
			printf(" [%i, %i]", point1, point2);
			printf("\n\t-Crossing genes.");
		}
		while(keep_going){
			int *f1 = (int*)malloc(sizeof(int)*elements);
			int *f2 = (int*)malloc(sizeof(int)*elements);

			for(j=0; j<point1; j++){
				f1[j]=new_gen[sort[i]][j];
				f2[j]=new_gen[sort[i+1]][j];
			}
			for(j=point1; j<point2; j++){
				f1[j]=new_gen[sort[i+1]][j];
				f2[j]=new_gen[sort[i]][j];
			}
			for(j=point2; j<elements; j++){
				f1[j]=new_gen[sort[i]][j];
				f2[j]=new_gen[sort[i+1]][j];
			}
			
			cross_correction(point1, point2, f1, f2);
			
			freeMemory(new_gen[sort[i]]);
			freeMemory(new_gen[sort[i+1]]);

			new_gen[sort[i]]=f1;
			new_gen[sort[i+1]]=f2;
			i+=2;
			counter+=2;
			if(counter+2>=count || i>=count)	keep_going=false;
		}
	}else{
		if(details) printf("\n\t-No matches on crossover.");
	}
	freeMemory(sort);
	if(details) printf("\n\t-Swapping genes.");
	swapGenes();
}

void Genetic::mutation(){
	if(titles) printf("\n\nStage 3: MUTATION");

	if(details) printf("\n\t-Calculating probability.");
	mut_Rate = mutation_prob()*100;
	if(details) printf(" [%.2f%%]", mut_Rate);
	int i, index[2], num=-1, gene, a, cont=0;
	if(details) printf("\n\t-Performing Mutation.");
	for(gene=0; gene<new_gen_pop; gene++){
		a = random_percent_gen(a);
		if(a<=mut_Rate){
			cont++;
			for(i=0; i<2; i++){
				num = random_element_gen(num);
				index[i]=num;
			}
			int tmp=new_gen[gene][index[0]];
			new_gen[gene][index[0]]=new_gen[gene][index[1]];
			new_gen[gene][index[1]]=tmp;
		}
	}
	if(details){
		if(cont==0) printf("\n\t-Mutation count: ZERO members");
		else if (cont==1) printf("\n\t-Mutation count: %i member [%.2f%%].", cont, (double)((100*cont)/new_gen_pop));
		else printf("\n\t-Mutation count: %i members [%.2f%%].", cont, (double)((100*cont)/new_gen_pop));
	}
}

void Genetic::recovery(){
	
	if(titles) printf("\n\nStage 5: RECOVERY");

	int ratio, i;
	
	int max_rate = (int)(mut_Rate+0.5);

	ratio = rand() %max_rate;
	int amount = (int)(ratio*new_gen_pop/100);
	if (amount==0) amount=1;
	Tupla populacao[pop_size];
	Tupla nova_populacao[new_gen_pop];
	for(i=0; i<pop_size; i++){
		populacao[i].fitness=generalFitness[i];
		populacao[i].index=i;
	}

	for(i=0; i<new_gen_pop; i++){
		nova_populacao[i].fitness=newFitness[i];
		nova_populacao[i].index=i;
	}

	mergeSort(populacao, 0, pop_size-1);
	mergeSort(nova_populacao, 0, new_gen_pop-1);

	int index=nova_populacao[new_gen_pop-1].index;
	double min = nova_populacao[new_gen_pop-1].fitness;
	for(i=0; i<new_gen_pop; i++){
		if(nova_populacao[i].fitness>0 && nova_populacao[i].fitness<min){
			min = nova_populacao[i].fitness;
			index = i;
		}
	}
	
	if(details) printf("\n\tBest Fitness: old: %.2f   new: %.2f", populacao[0].fitness, nova_populacao[index].fitness);
	int trade=0;
	
	if(populacao[0].fitness >= nova_populacao[index].fitness) lostCicle++;
	
	
	//Trade-off invalid for best individual.
	if(details){
		if(amount==1)	printf("\n\t-Exchanging individuals [%d member]:", amount);
		else	printf("\n\t-Exchanging individuals [%d members]:", amount);
	}

	int invalid=0;
	if(details) printf("\n\t\t+Eliminating unfeasable solutions.");
	for(i=0; i<new_gen_pop; i++){
		if(nova_populacao[i].fitness==-1){
			trade++;
			freeMemory(new_gen[nova_populacao[i].index]);
			new_gen[nova_populacao[i].index]    = copyGene(pop[populacao[invalid].index]);
			newFitness[nova_populacao[i].index] = populacao[invalid].fitness;
			invalid++;
		}else{
			i=new_gen_pop;
		}
	}
	if(details) printf("\n\t\t+Overwritting  new worst -> former best.");
	if(trade<amount){
		for(i=new_gen_pop-1; i>=0; i--){
			trade++;
			freeMemory(new_gen[nova_populacao[i].index]);
			new_gen[nova_populacao[i].index]    = copyGene(pop[populacao[invalid].index]);
			newFitness[nova_populacao[i].index] = populacao[invalid].fitness;
			invalid++;
			if(trade>=amount){
				i=-1;
			}
		}
	}
}

void Genetic::update(){
	if(titles) printf("\n\nStage 6: UPDATE");
	
	if(details) printf("\n\t-Releasing former population.");
	clearPopulation();

	if(details) printf("\n\t-Overwriting new population as base.");
	pop=new_gen;
	pop_size = new_gen_pop;
	new_gen=NULL;
	
	if(details) printf("\n\t-Releasing former fitness.");
	freeMemory(generalFitness);

	if(details) printf("\n\t-Overwriting new fitness as base.");
	generalFitness = newFitness;
	newFitness=NULL;
	if(curr_cicle<generation){
		if(details) printf("\n\t-Setting up new selection rate.");
		if(new_gen_pop<5)	sel_rate=100;
		new_gen_pop = sel_rate*pop_size/100;
	}
	if(details) printf("\n------------------------------------------------");
}

void Genetic::displayBest(){
	printf("\nPOS-RUN BEST RESULT");
	int best_i = 0;
	double fit = generalFitness[0];
	for(int i = 1; i < pop_size; i++){
		if(fit>generalFitness[i]){
			fit = generalFitness[i];
			best_i = i;
		}
	}
	final_fitness = fit;
	
	printf("\nCost: %.2f", fit);
	
	if(details){
		printf("\nRoutes:\n");
		breakRoute(true,  pop[best_i] );

		printf("\nInitial Population Best: %.2f", bestPrior);
		printf("\nImprovement ratio: %.2f", 100-(double)100*fit/bestPrior);
		printf("\nTotal lost cicles: %d [%.2f]\n", lostCicle, (double)(lostCicle*100/generation));
	}
}
#endif
