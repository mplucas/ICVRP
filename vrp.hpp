#ifndef __VRP_H
#define __VRP_H

#include<bits/stdc++.h>

typedef struct{
	/*unsigned*/ int vehicle;	// number of vehicles
	/*unsigned*/ int client;	// number of clients
	/*unsigned*/ int capacity;	// capacity os the vehicles
	/*unsigned*/ int linear3;	// not used
	double** cost; 			// matrix with the costs between all the clients
	int ** info;			// array with information about each client:
								// [i][0]: x position
								// [i][1]: y position
								// [i][2]: demand
								// [i][3]: readyTime
								// [i][4]: dueTime
								// [i][5]: serviceTime
	char* source;			// sourcefile
	int* demand;			// demands of clients
	double* readyTime;		// ready times of clients
	double* dueTime;		// due times of clients
	double* serviceTime;	// service times of clients
	int** getInfo();
}Vrp;

typedef struct{
	char* src;
	/*unsigned*/ int nVehicle;
	/*unsigned*/ int nDestination;
	/*unsigned*/ int capacity;
	int** dest_info;
	double **cost_Matrix;
}Solomon;

void eraseInstance(Vrp* problem){
	/*unsigned*/ int i;
	if(problem->cost!=NULL){
		for(i=0; i<problem->client; i++){
			free(problem->cost[i]);
		}
	}
	if(problem->info!=NULL){
		for(i=0; i<problem->client; i++){
			free(problem->info[i]);
		}
	}
	
	free(problem->demand);
	free(problem->readyTime);
	free(problem->dueTime);
	free(problem->serviceTime);
	free(problem->source);
	free(problem->cost);
	free(problem->info);
	problem->vehicle=0;
	problem->client=0;
	problem->capacity=0;
	problem->cost=NULL;
	problem->info=NULL;
	problem->source=NULL;
	problem->demand=NULL;
	problem->readyTime=NULL;
	problem->dueTime=NULL;
	problem->serviceTime=NULL;
	free(problem);
}

void printInstance(Vrp* problem){
	/*unsigned*/ int i;
	printf("\nINSTANCE SRC: <%s>", problem->source);
	printf("\nVEHICLE NUMBER: %d     CAPACITY: %d", problem->vehicle, problem->capacity);
	printf("\nCUST NO.  XCOORD.   YCOORD.    DEMAND   READY TIME  DUE DATE   SERVICE TIME");
	for(i=0; i<problem->client; i++)
		printf("\n%5d%8d%11d%11d%11d%11d%11d", i, problem->info[i][0], problem->info[i][1], problem->info[i][2], problem->info[i][3], problem->info[i][4], problem->info[i][5]);
													//   x					y					demand					ready				due					service
	printf("\n");
}

void printfCost(double** cost, int elements){
	int i, j;
	for(i=0; i<elements; i++){
		printf("\n");
		for(j=0; j<elements; j++){
			printf("%.2f ", cost[i][j]);
		}
	}
}

int clientCheck(FILE* instance){
	int flag=1, lines=1;
	char* buffer=(char*)malloc(sizeof(char));
	while(flag){
		flag=fread(buffer, sizeof(char), 1, instance);
		if(flag)
			if(*buffer=='\n')
				lines+=1;
	}
	free(buffer);
	return lines-10;
}

void readHeader(FILE* instance, /*unsigned*/ int* vehicle, /*unsigned*/ int* load){
	int i, flag;
	char* buffer=(char*)malloc(sizeof(char));
	
	for(i=0; i<34; i++)
		flag=fread(buffer, sizeof(char), 1, instance);
	
	//Leitura Quantidade de veículos
	while((*buffer<'0' || *buffer>'9') && flag)
		flag=fread(buffer, sizeof(char), 1, instance);
	if(!flag){
		exit(-1);
	}

	*vehicle=*buffer-48;
	while((*buffer>='0' && *buffer<='9') && flag){
		flag=fread(buffer, sizeof(char), 1, instance);
		if(!flag){
			exit(-1);
		}else if((*buffer>='0' && *buffer<='9') && flag){
			*vehicle=*vehicle*10+(*buffer-48);
		}
	}
	
	//Leitura da carga
	while((*buffer<'0' || *buffer>'9') && flag)
		flag=fread(buffer, sizeof(char), 1, instance);
	if(!flag){
		exit(-1);
	}

	*load=*buffer-48;
	while((*buffer>='0' && *buffer<='9') && flag){
		flag=fread(buffer, sizeof(char), 1, instance);
		if(!flag){
			exit(-1);
		}else if((*buffer>='0' && *buffer<='9') && flag){
			*load=*load*10+(*buffer-48);
		}
	}

	free(buffer);
}

int** readRow(FILE *instance, int clients){
	char* buffer=(char*)malloc(sizeof(char));
	/*unsigned*/ int flag=1;
	int i, customer, x, y, demand, ready, due, service;
	int** matrix = (int**)malloc(sizeof(int*)*(clients+1));
	for(i=0; i<=clients; i++)
		matrix[i]=(int*)malloc(sizeof(int)*5);
	i=0;
	while(flag){
		
		//Leitura Customer Number
		while((*buffer<'0' || *buffer>'9') && flag)
			flag=fread(buffer, sizeof(char), 1, instance);
		/*
		if(!flag){
			exit(-1);
		}
		*/
	
		customer=*buffer-48;
		while((*buffer>='0' && *buffer<='9') && flag){
			flag=fread(buffer, sizeof(char), 1, instance);
			if(!flag){
				exit(-1);
			}else if((*buffer>='0' && *buffer<='9') && flag){
				customer=customer*10+(*buffer-48);
			}
		}
		
		//Leitura Pos X
		while((*buffer<'0' || *buffer>'9') && flag)
			flag=fread(buffer, sizeof(char), 1, instance);
		if(!flag){
			exit(-1);
		}
	
		x=*buffer-48;
		while((*buffer>='0' && *buffer<='9') && flag){
			flag=fread(buffer, sizeof(char), 1, instance);
			if(!flag){
				exit(-1);
			}else if((*buffer>='0' && *buffer<='9') && flag){
				x=x*10+(*buffer-48);
			}
		}
		
		//Leitura Pos Y
		while((*buffer<'0' || *buffer>'9') && flag)
			flag=fread(buffer, sizeof(char), 1, instance);
		if(!flag){
			exit(-1);
		}
	
		y=*buffer-48;
		while((*buffer>='0' && *buffer<='9') && flag){
			flag=fread(buffer, sizeof(char), 1, instance);
			if(!flag){
				exit(-1);
			}else if((*buffer>='0' && *buffer<='9') && flag){
				y=y*10+(*buffer-48);
			}
		}
		
		//Leitura Demand
		while((*buffer<'0' || *buffer>'9') && flag)
			flag=fread(buffer, sizeof(char), 1, instance);
		if(!flag){
			exit(-1);
		}
	
		demand=*buffer-48;
		while((*buffer>='0' && *buffer<='9') && flag){
			flag=fread(buffer, sizeof(char), 1, instance);
			if(!flag){
				exit(-1);
			}else if((*buffer>='0' && *buffer<='9') && flag){
				demand=demand*10+(*buffer-48);
			}
		}
		
		//Leitura Ready Time
		while((*buffer<'0' || *buffer>'9') && flag)
			flag=fread(buffer, sizeof(char), 1, instance);
		if(!flag){
			exit(-1);
		}
	
		ready=*buffer-48;
		while((*buffer>='0' && *buffer<='9') && flag){
			flag=fread(buffer, sizeof(char), 1, instance);
			if(!flag){
				exit(-1);
			}else if((*buffer>='0' && *buffer<='9') && flag){
				ready=ready*10+(*buffer-48);
			}
		}
		
		//Leitura Due Time
		while((*buffer<'0' || *buffer>'9') && flag)
			flag=fread(buffer, sizeof(char), 1, instance);
		if(!flag){
			exit(-1);
		}
	
		due=*buffer-48;
		while((*buffer>='0' && *buffer<='9') && flag){
			flag=fread(buffer, sizeof(char), 1, instance);
			if(!flag){
				exit(-1);
			}else if((*buffer>='0' && *buffer<='9') && flag){
				due=due*10+(*buffer-48);
			}
		}
		
		//Leitura Service Time
		while((*buffer<'0' || *buffer>'9') && flag)
			flag=fread(buffer, sizeof(char), 1, instance);
		if(!flag){
			exit(-1);
		}
	
		service=*buffer-48;
		while((*buffer>='0' && *buffer<='9') && flag){
			flag=fread(buffer, sizeof(char), 1, instance);
			if(!flag){
				exit(-1);
			}else if((*buffer>='0' && *buffer<='9') && flag){
				service=service*10+(*buffer-48);
			}
		}
		
		matrix[i][0]=x;
		matrix[i][1]=y;
		matrix[i][2]=demand;
		matrix[i][3]=ready;
		matrix[i][4]=due;
		matrix[i][5]=service;
		//printf("\n i: %i\n x: %i \t y: %i \t demand: %i\n ready: %i \t due: %i \t service: %i\n", i,x,y,demand,ready,due,service);
		i+=1;
		if(i==clients)
			flag=0;
	}
	free(buffer);
	//printf("\nA\n\n");
	return matrix;
}

double distanceAB(int x1, int y1, int x2, int y2){
	double realX, realY;
	realX=(double)(x1-x2);
	realY=(double)(y1-y2);
	return sqrt(realX*realX + realY*realY);
}

Solomon readInstance(char* instanceName){
	Solomon instance;
	/*unsigned*/ int i=0, j;
	
	if(instanceName==NULL){
		printf("\nInvalid file. Operation aborted.");
		exit(-1);
	}
	
	while(instanceName[i]!='\0'){
		i++;
	}
	
	instance.src=(char*)malloc(sizeof(char)*(i+1));
	memcpy ( instance.src, instanceName, i);
	instance.src[i]='\0';
	
	FILE* instance_file = fopen(instance.src,"rt");
	if(instance_file==NULL){
		printf("\nCouldn't open file. Operation aborted.");
		exit(-1);
	}
	instance.nDestination=clientCheck(instance_file);
	rewind(instance_file);
	readHeader(instance_file, &instance.nVehicle, &instance.capacity);
	//printf("\n instance.nVehicle: %i \t instance.capacity: %i\n", instance.nVehicle, instance.capacity);
	instance.dest_info = readRow(instance_file, instance.nDestination);
	
	fclose(instance_file);
	//printf("\nB\n\n");
	
	instance.cost_Matrix=(double**)malloc(sizeof(double*)*instance.nDestination);
	
	for(i=0; i<instance.nDestination; i++){
		instance.cost_Matrix[i]=(double*)malloc(sizeof(double)*instance.nDestination);
		for(j=0; j<instance.nDestination; j++){
			if(i!=j)	instance.cost_Matrix[i][j]=distanceAB(instance.dest_info[i][0], instance.dest_info[i][1], instance.dest_info[j][0], instance.dest_info[j][1]);
			else instance.cost_Matrix[i][j]=0;
		}
	}

	//printf("\nC\n\n");

	return instance;
}

Vrp* Vrp_init(char* src){
	int size = strlen(src)+1;
	Vrp* problem=(Vrp*)malloc(sizeof(Vrp));
	problem->source = (char*)malloc(sizeof(char)*size);
	Solomon instance_model=readInstance(src);
	//memcpy (problem->source, src, size );
	strcpy (problem->source, src);
	//printf("\nD %s\n\n", src);
	problem->vehicle = instance_model.nVehicle;
	problem->client = instance_model.nDestination;
	problem->capacity = instance_model.capacity;
	problem->cost=instance_model.cost_Matrix;
	problem->info=instance_model.dest_info;
	
	problem->demand      = (int*)malloc(sizeof(int)*problem->client);
	problem->readyTime   = (double*)malloc(sizeof(double)*problem->client);
	problem->dueTime     = (double*)malloc(sizeof(double)*problem->client);
	problem->serviceTime = (double*)malloc(sizeof(double)*problem->client);
	
	/*unsigned*/ int i;
	for(i=0; i<problem->client; i++){
		problem->demand[i]      = problem->info[i][2];
		problem->readyTime[i]   = problem->info[i][3];
		problem->dueTime[i]     = problem->info[i][4];
		problem->serviceTime[i] = problem->info[i][5];
	}
	//printf("\nE\n\n");
	
	return problem;
}



#endif
