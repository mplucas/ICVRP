#include <bits/stdc++.h>
#include "src/utils.hpp"

// global vrp problem variable
vrp problem;

// variable for fractional delivery
bool isFractionalDelivery;

// easy debug variable
bool debug;

// variables to control pop creation
int popSize;
int popCount = 0;
std::vector<std::vector<double>> nnPopParameters{
    {0.4, 0.4, 0.2},
    {0, 1, 0},
    {0.5, 0.5, 0},
    {0.3, 0.3, 0.4}
};

bool eval_solution(const MySolution& p, double &cost);

void init_genes(MySolution& p)
{
    if ((int)nnPopParameters.size() > 0) {
		int choosenParameters = (int)nnPopParameters.size() - 1;
		nearestNeighborPop( p.route, problem, nnPopParameters[choosenParameters][0], nnPopParameters[choosenParameters][1], nnPopParameters[choosenParameters][2] );
		nnPopParameters.pop_back();
		// popCount++;
            
    // } else if((int)si1PopParameters.size() > 0){
	// 	int choosenParameters = (int)si1PopParameters.size() - 1;
    //     solomonInsertion1( p.route, problem, (int)si1PopParameters[choosenParameters][0], si1PopParameters[choosenParameters][1], si1PopParameters[choosenParameters][2], si1PopParameters[choosenParameters][3], si1PopParameters[choosenParameters][4]);
	// 	si1PopParameters.pop_back();
        
	}else if(popCount < (int)(popSize * 0.35)){
		double parameters[3] = {
			-1, -1, -1
		};
		double remnant = 1;
		int parametersPopulated = 0;

		while(parametersPopulated < 3){
			
			int parameterIndex = (int)((int)(3 * random01()) % 3);
			if(parameters[parameterIndex] != -1){
				continue;
			}
			if(parametersPopulated == 2){
				parameters[parameterIndex] = remnant;
			}else{
				parameters[parameterIndex] = min(random01(), remnant);
				remnant -= parameters[parameterIndex];
			}
			parametersPopulated++;
		}
        if(nearestNeighborPop( p.route, problem, parameters[0], parameters[1], parameters[2] )){
			// popCount++;
		}
    }else if(popCount < (int)(popSize * 0.7)){
        p.route = k_means( problem, (int)(popSize*0.05) + (int)(random01() * (popSize*0.08)) );
    	// popCount++;
    }else {
        p.route = randomPopImproved( problem, random01 );
		// popCount++;
		// p.route = randomPop( problem, random01 );
    }

	if(isFractionalDelivery){
		p.route = fixFDRoute(p.route, problem);
	}
	popCount++;
	// cout << endl << popCount; //lll
    // cout << "pop " << p.to_string() << endl; // lll
}

bool eval_solution(const MySolution& p, double &cost)
{
	bool isFeasible = true; // if true accepts gene, if false rejects gene
	
	// lll
	// if((int)p.route.size() != problem.numNodes - 1){
	// 	cout << "\ntamanho diferente " << (int)p.route.size() << endl;
	// 	isFeasible = false;
	// 	return isFeasible;
	// }
	// else{
	// 	// cout << "aceito " << popCount << endl;
	// }
	// for(int i = 0; i < (int)p.route.size(); i++){
	// 	for(int j = i+1; j < (int)p.route.size(); j++){
	// 		if(p.route[i] == p.route[j]){
	// 			cout << "\nigual: " << p.route[i] << " [" << i << "] e [" << j << "]";
	// 			isFeasible = false;
	// 			return isFeasible;
	// 		}
	// 		if(p.route[i] < 1 || p.route[i] > (int)p.route.size()){
	// 			cout << "inconsistente " << p.route[i];
	// 			isFeasible = false;
	// 			return isFeasible;
	// 		}
	// 	}
	// }

	// VARIABLES TO CONTROL VEHICLE BEING USED
	int choosenVehicle = 0; // vehicle wich route belongs to, if it turns equal to problem.numVehicles than its unfeasible
	double vehicleTimer = 0;
	int vehicleUsedCapacity = 0;
	int originNode = 0;

    // VARIABLES TO CALCULATE FIT ACCORDING TO FITCRITERION
	// if problem.fitCriterion == 0 (Time)
    double biggestTimer = 0; // biggest cost in all routes, since it determines the cost of whole operation

    // if problem.fitCriterion == 1 (Distance)
    double totalDistance = 0;

	// VARIABLES TO DEBUG
	vector<vehicle> vehicleDebugger(1);
    
	for(unsigned int i = 0; i < p.route.size(); i++){
		
		int destinyNode = p.route[i];

		if( vehicleTimer + problem.cost[originNode][destinyNode] <= problem.dueTime[destinyNode]
			&& vehicleUsedCapacity + problem.demand[destinyNode] <= problem.capacity )
		{
			// add cost and demand to vehicle
			vehicleUsedCapacity += problem.demand[destinyNode];
			vehicleTimer += problem.cost[originNode][destinyNode];

			// if vehicle arrives earlier than start of TW, it waits until the start
			if( vehicleTimer < problem.readyTime[destinyNode] ){
				vehicleTimer = problem.readyTime[destinyNode];
			}

			// adds service time
			vehicleTimer += problem.serviceTime[destinyNode];

			if(problem.fitCriterion == 0){
                // save fit if it is bigger
                if( vehicleTimer > biggestTimer ){
                    biggestTimer = vehicleTimer;
                }
            }else if(problem.fitCriterion == 1){
				// add distance
                totalDistance += problem.cost[originNode][destinyNode];
            }

			if(debug){
                vehicleDebugger[choosenVehicle].timer = vehicleTimer;
                vehicleDebugger[choosenVehicle].usedCapacity = vehicleUsedCapacity;
				vehicleDebugger[choosenVehicle].distance += problem.cost[originNode][destinyNode];
				vehicleDebugger[choosenVehicle].route.push_back(destinyNode);
			}

			// update next origin
			originNode = destinyNode;
		}
		else
		{   
			// send vehicle back to depot
			vehicleTimer += problem.cost[originNode][0];

			if(problem.fitCriterion == 0){
                // save fit if it is bigger
                if( vehicleTimer > biggestTimer ){
                    biggestTimer = vehicleTimer;
                }
            }else if(problem.fitCriterion == 1){
				totalDistance += problem.cost[originNode][0];
			}

			if(debug){

				vehicleDebugger[choosenVehicle].timer = vehicleTimer;
				vehicleDebugger[choosenVehicle].usedCapacity = vehicleUsedCapacity;
				vehicle newVehicle;
				vehicleDebugger.push_back(newVehicle);
			}

            // try to assign this node to the next vehicle route
            choosenVehicle++;
            
			if(choosenVehicle == problem.numVehicles){
				isFeasible = false;
				break;
			}

            // resets variables of vehicle
			vehicleTimer = 0;
			vehicleUsedCapacity = 0;
			originNode = 0;

			// returns to same client
			i--;
		}
	}

	// finalizing fit
    if(problem.fitCriterion == 0){

        // send vehicle back to depot
        vehicleTimer += problem.cost[originNode][0];

        // save fit if it is bigger
        if( vehicleTimer > biggestTimer ){
            biggestTimer = vehicleTimer;
        }
        
        cost = biggestTimer;
    
    }else if(problem.fitCriterion == 1){
        
        // Adding distance to return to depot of the last vehicle
        cost = totalDistance + problem.cost[originNode][0];
	}

	if(debug){

		if(originNode != 0){
			vehicleDebugger.back().timer += problem.cost[originNode][0];
			vehicleDebugger.back().distance += problem.cost[originNode][0];
		}
		
		for(unsigned int i = 0; i < vehicleDebugger.size(); i++){
			cout << "\nVehicle " << i << ":\n" << vehicleDebugger[i].to_string();
		}
	}

    // if(isFeasible) cout << "FEASIBLE " << popCount << endl; //lll
	// else cout << "NOT FEASIBLE " << popCount << " " << p.route.size() << " " << endl; //lll
    cout << cost << endl; //lll

	return isFeasible;
}

int main()
{
    setbuf(stdout, NULL);

    // globals
    problem = readFile("entrada.txt");
    problem.fitCriterion = 1; // Distance
    isFractionalDelivery = false;
    debug = false;
    popSize = 100;

    //GA
    GA_Type ga;
    ga.populationSize = popSize;
    ga.init_genes = init_genes;
    ga.eval_solution = eval_solution;

    ga.populate();

    for(int i = 0; i < popSize; i++){
        cout << ga.population[i].genes.to_string() << endl << ga.population[i].cost << endl;
    }

    return 0;
}
