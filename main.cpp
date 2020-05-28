#include <bits/stdc++.h>
#include "src/utils.hpp"

// global vrp problem variable
vrp problem;

// easy debug variable
bool debug = false;

// variable to generate tests
bool test = true;

// variables to control pop creation
int popSize = 100;
int popCount = 0;
std::vector<std::vector<double>> nnPopParameters{
    {0.4, 0.4, 0.2},
    {0, 1, 0},
    {0.5, 0.5, 0},
    {0.3, 0.3, 0.4}
};
// std::vector<std::vector<double>> si1PopParameters{
//     {0, 1, 1, 1, 0},
// 	{0, 1, 2, 1, 0},
// 	{0, 1, 1, 0, 1},
// 	{0, 1, 2, 0, 1},
// 	{1, 1, 1, 1, 0},
// 	{1, 1, 2, 1, 0},
// 	{1, 1, 1, 0, 1},
// 	{1, 1, 2, 0, 1}
// };

bool eval_solution(
	const MySolution& p,
	MyMiddleCost &c);
void init_genes(MySolution& p,const std::function<double(void)> &rnd01)
{

    // cout << "\n\n chosens: " << popCount << "\n"; // lll
    if ((int)nnPopParameters.size() > 0) {
		// cout << "\na"; //lll
		int choosenParameters = (int)nnPopParameters.size() - 1;
		nearestNeighborPop( p.route, problem, nnPopParameters[choosenParameters][0], nnPopParameters[choosenParameters][1], nnPopParameters[choosenParameters][2] );
		nnPopParameters.pop_back();
		popCount++;
            
    // } else if((int)si1PopParameters.size() > 0){
	// 	// cout << "\nb"; //lll
	// 	int choosenParameters = (int)si1PopParameters.size() - 1;
    //     solomonInsertion1( p.route, problem, (int)si1PopParameters[choosenParameters][0], si1PopParameters[choosenParameters][1], si1PopParameters[choosenParameters][2], si1PopParameters[choosenParameters][3], si1PopParameters[choosenParameters][4]);
	// 	si1PopParameters.pop_back();
        
	}else if(popCount < popSize / 2){
		// cout << "\nc"; //lll
		double parameters[3] = {
			-1, -1, -1
		};
		double remnant = 1;
		int parametersPopulated = 0;

		while(parametersPopulated < 3){
			
			int parameterIndex = (int)((int)(3 * rnd01()) % 3);
			if(parameters[parameterIndex] != -1){
				continue;
			}
			if(parametersPopulated == 2){
				parameters[parameterIndex] = remnant;
			}else{
				parameters[parameterIndex] = min(rnd01(), remnant);
				remnant -= parameters[parameterIndex];
			}
			parametersPopulated++;
		}
        if(nearestNeighborPop( p.route, problem, parameters[0], parameters[1], parameters[2] )){
			popCount++;
		}
    }else {
		// cout << "\n0"; //lll
        p.route = randomPopImproved( problem, rnd01 );
		// p.route = randomPop( problem, rnd01 );
    	popCount++;
    }
    // cout << "pop " << p.to_string() << endl; // lll
}

bool eval_solution(
	const MySolution& p,
	MyMiddleCost &c)
{
	// cout << "\na\n"; // lll
	bool isFeasible = true; // if true accepts gene, if false rejects gene
	
	// cout << "\n"; //lll
	// printRoute(p.route); // lll
	if((int)p.route.size() != problem.numNodes - 1){
		// cout << "\ntamanho diferente " << (int)p.route.size() << endl; // lll
		isFeasible = false;
		return isFeasible;
	}
	// else{
	// 	// cout << "aceito " << popCount << endl; // lll
	// }
	// for(int i = 0; i < (int)p.route.size(); i++){
	// 	for(int j = i+1; j < (int)p.route.size(); j++){
	// 		if(p.route[i] == p.route[j]){
	// 			cout << "\nigual: " << p.route[i] << " [" << i << "] e [" << j << "]";
	// 		}
	// 		if(p.route[i] < 1 || p.route[i] > (int)p.route.size()){
	// 			cout << "inconsistente " << p.route[i];
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
        
        c.cost = biggestTimer;
    
    }else if(problem.fitCriterion == 1){
        
        // Adding distance to return to depot of the last vehicle
        c.cost = totalDistance + problem.cost[originNode][0];
    }

	if(debug){
		vehicleDebugger.back().timer += problem.cost[originNode][0];
		vehicleDebugger.back().distance += problem.cost[originNode][0];

		for(unsigned int i = 0; i < vehicleDebugger.size(); i++){
			cout << "\nVehicle " << i << ":\n" << vehicleDebugger[i].to_string();
		}
        // cout << "\n Last node visited (or tried): p.route[" << i << "] = " << p.route[i];
	}

    // if(isFeasible) cout << "FEASIBLE " << popCount << endl; //lll
	// else cout << "NOT FEASIBLE " << popCount << " " << p.route.size() << " " << endl; //lll

	return isFeasible;
}

MySolution mutate(
	const MySolution& baseGene,
	const std::function<double(void)> &rnd01,
	double shrink_scale)
{
	// cout << "\nc\n"; // lll
	MySolution mutatedGene = baseGene;

	if( rnd01() < shrink_scale ){

		unsigned int choosenNode1 = (unsigned int)((int)(rnd01() * (double)baseGene.route.size()) % baseGene.route.size());
		unsigned int choosenNode2;

		do{
			choosenNode2 = (unsigned int)((int)(rnd01() * (double)baseGene.route.size()) % baseGene.route.size());
		}while(choosenNode2 == choosenNode1);

		mutatedGene.route[choosenNode1] = baseGene.route[choosenNode2];
		mutatedGene.route[choosenNode2] = baseGene.route[choosenNode1];
        // cout << "\nMutation " << choosenNode1 << " " << choosenNode2 << "\nBase: " << baseGene.to_string();// lll
	    // cout << "\n Mut: " << mutatedGene.to_string() << endl;// lll
	}    

	return mutatedGene;
}

MySolution crossover(
	const MySolution& gene1,
	const MySolution& gene2,
	const std::function<double(void)> &rnd01)
{
	MySolution newGene;
	
	unsigned int choosenNode1 = (unsigned int)((int)(rnd01() * (double)gene1.route.size()) % gene1.route.size());
	unsigned int choosenNode2;

	do{
		choosenNode2 = (unsigned int)((int)(rnd01() * (double)gene1.route.size()) % gene1.route.size());
	}while(choosenNode2 == choosenNode1);
	
	int smallerIndex, biggerIndex;

	if( choosenNode1 > choosenNode2 ){
		
		biggerIndex  = choosenNode1;
		smallerIndex = choosenNode2;
	}else{

		biggerIndex  = choosenNode2;
		smallerIndex = choosenNode1;
	}

	// cout << "\nCrossOver " << smallerIndex << " " << biggerIndex << "\n G1: " << gene1.to_string() << " [" << gene1.route.size() << "]\n G2: " << gene2.to_string() << " [" << gene1.route.size() << "]\n";// lll

	for(int i = 0; i < smallerIndex; i++){
		// cout << "\n 1 pushando " << gene1.route[i] << " " << i << " " << gene1.route.size(); // lll
		newGene.route.push_back( gene1.route[i] );
	}
	for(int i = smallerIndex; i < biggerIndex; i++){
		// cout << "\n 2 pushando " << gene2.route[i] << " " << i << " " << gene2.route.size(); // lll
		newGene.route.push_back( gene2.route[i] );
	}
	for(int i = biggerIndex; i < (int)gene1.route.size(); i++){
		// cout << "\n 3 pushando " << gene1.route[i] << " " << i << " " << gene1.route.size(); // lll
		newGene.route.push_back( gene1.route[i] );
	}

    // Correcting cross over
    // Finding duplicated and missing nodes
    vector<int> duplicatedNodes;
    vector<int> missingNodes;

    for(int i = smallerIndex; i < biggerIndex; i++){
        
        bool isDuplicated = true;
        bool isMissing = true;
		
        for(int j = smallerIndex; j < biggerIndex; j++){
        
            // if not found in gene1 than it is duplicated in newGene
            if(gene2.route[i] == gene1.route[j]){
                isDuplicated = false;
            }
            // if not found in gene2 than it is missing in newGene
            if(gene1.route[i] == gene2.route[j]){
                isMissing = false;
            }
        }

        if(isDuplicated){
            duplicatedNodes.push_back(gene2.route[i]);
        }
        if(isMissing){
            missingNodes.push_back(gene1.route[i]);
        }

	}

    // cout << "\nSizes d:" << duplicatedNodes.size() << " m:" << missingNodes.size() << endl;

    // Correcting duplicated nodes
    for(int i = 0; i < smallerIndex; i++){

        // if node is duplicated, replace it with a missing one
        vector<int> :: iterator itDuplicatedNodes = find(duplicatedNodes.begin(), duplicatedNodes.end(), newGene.route[i]);
        
        if( itDuplicatedNodes != duplicatedNodes.end() ){
            
            vector<int> :: iterator itMissingNodes = missingNodes.begin() + (itDuplicatedNodes - duplicatedNodes.begin());
			// cout << "\ntroca " << newGene.route[i] << " por " << *itMissingNodes; //lll
            newGene.route[i] = *itMissingNodes;
            duplicatedNodes.erase( itDuplicatedNodes );
            missingNodes.erase( itMissingNodes );
        }

    }

    for(unsigned int i = biggerIndex; i < newGene.route.size(); i++){

        // if node is duplicated, replace it with a missing one
        vector<int> :: iterator itDuplicatedNodes = find(duplicatedNodes.begin(), duplicatedNodes.end(), newGene.route[i]);
        
        if( itDuplicatedNodes != duplicatedNodes.end() ){
            
            vector<int> :: iterator itMissingNodes = missingNodes.begin() + (itDuplicatedNodes - duplicatedNodes.begin());
			// cout << "\ntroca " << newGene.route[i] << " por " << *itMissingNodes; //lll
            newGene.route[i] = *itMissingNodes;
            duplicatedNodes.erase( itDuplicatedNodes );
            missingNodes.erase( itMissingNodes );
        }

    }

	// cout << "\nRes: " << newGene.to_string() << endl;// lll

	return newGene;
}

double calculate_SO_total_fitness(const GA_Type::thisChromosomeType &X)
{
	// finalize the cost
	// cout << "\ne\n"; // lll
	return X.middle_costs.cost;
}

// std::ofstream output_file;

void SO_report_generation(
	int generation_number,
	const EA::GenerationType<MySolution,MyMiddleCost> &last_generation,
	const MySolution& best_genes)
{
	// cout << "\nf\n"; // lll
	if(!test){
		std::cout
		<<"Generation ["<<generation_number<<"], "
		<<"Best="<<last_generation.best_total_cost<<", "
		<<"Average="<<last_generation.average_cost<<", "
		<<"Best genes=("<<best_genes.to_string()<<")"<<", "
		<<"Exe_time="<<last_generation.exe_time
		<<std::endl;
	}
	// output_file
	// 	<<generation_number<<"\t"
	// 	<<last_generation.average_cost<<"\t"
	// 	<<last_generation.best_total_cost
	// 	<<"\n";
}

void resetGlobals(){
	// variables to control pop creation
	popSize = 100;
	popCount = 0;
	nnPopParameters = {
		{0.4, 0.4, 0.2},
		{0, 1, 0},
		{0.5, 0.5, 0},
		{0.3, 0.3, 0.4}
	};
	// si1PopParameters = {
	// 	{0, 1, 1, 1, 0},
	// 	{0, 1, 2, 1, 0},
	// 	{0, 1, 1, 0, 1},
	// 	{0, 1, 2, 0, 1},
	// 	{1, 1, 1, 1, 0},
	// 	{1, 1, 2, 1, 0},
	// 	{1, 1, 1, 0, 1},
	// 	{1, 1, 2, 0, 1}
	// };
}

int main()
{
    setbuf(stdout, NULL);

	// output_file.open("./bin/result_.txt");
	// output_file
	// 	<<"step"<<"\t"
	// 	<<"cost_avg"<<"\t"
	// 	<<"cost_best"
	// 	<<"\n";

	GA_Type ga_obj;
	ga_obj.problem_mode=EA::GA_MODE::SOGA;
	ga_obj.multi_threading=false;
	ga_obj.dynamic_threading=false;
	ga_obj.idle_delay_us=0; // switch between threads quickly
	ga_obj.verbose=false;
	ga_obj.population=popSize;
	ga_obj.generation_max=1000;
	ga_obj.calculate_SO_total_fitness=calculate_SO_total_fitness;
	ga_obj.init_genes=init_genes;
	ga_obj.eval_solution=eval_solution;
	ga_obj.mutate=mutate;
	ga_obj.crossover=crossover;
	ga_obj.SO_report_generation=SO_report_generation;
	ga_obj.best_stall_max=20;
	ga_obj.average_stall_max=20;
	ga_obj.tol_stall_best=1e-6;
	ga_obj.tol_stall_average=1e-6;
	ga_obj.elite_count=10;
	ga_obj.crossover_fraction=0.7;
	ga_obj.mutation_rate=0.1;

	if(!test){

		// ### BEGIN CLASSIC TEST

		problem = readFile("entrada.txt");
		problem.fitCriterion = 1; // Distance

		EA::Chronometer timer;
		timer.tic();

		ga_obj.solve();

		std::cout<<"The problem is optimized in "<<timer.toc()<<" seconds."<<std::endl;

		debug = true;
		MyMiddleCost c;
		ga_obj.eval_solution( ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes, c );
		// ### END CLASSIC TEST
	}else{

		ofstream outputTests;
		outputTests.open("results.txt");

		batteryTests(ga_obj, problem, "instances/solomon100/c101.txt", 10, 1, resetGlobals, outputTests);
		batteryTests(ga_obj, problem, "instances/solomon100/rc101.txt", 10, 1, resetGlobals, outputTests);
		batteryTests(ga_obj, problem, "instances/solomon100/r101.txt", 10, 1, resetGlobals, outputTests);
		batteryTests(ga_obj, problem, "instances/homberger200/C1_2_1.TXT", 10, 1, resetGlobals, outputTests);
		batteryTests(ga_obj, problem, "instances/homberger200/RC1_2_1.TXT", 10, 1, resetGlobals, outputTests);
		batteryTests(ga_obj, problem, "instances/homberger200/R1_2_1.TXT", 10, 1, resetGlobals, outputTests);

		outputTests.close();
	}

	// output_file.close();
	return 0;
}
// g++ -O3 -s -DNDEBUG -std=c++11 -pthread -I/src -Wall -Wconversion -Wfatal-errors -Wextra main.cpp