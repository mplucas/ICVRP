#include <bits/stdc++.h>
#include "src/utils.hpp"

// global vrp problem variable
vrp problem;

// easy debug variable
bool debug;

// variable to generate tests
bool test;
int mutCount = 0;
int crossCount = 0;

// variables to control crossover
int numCuts;
double initialProbCross;
double finalProbCross;

// variables to control mutation
int numPoints;
double initialProbMut;
double finalProbMut;

// variables to control generation
int generationSize;
int generationCount = 0;

// variables to control pop creation
int popSize;
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
    if ((int)nnPopParameters.size() > 0) {
		int choosenParameters = (int)nnPopParameters.size() - 1;
		nearestNeighborPop( p.route, problem, nnPopParameters[choosenParameters][0], nnPopParameters[choosenParameters][1], nnPopParameters[choosenParameters][2] );
		nnPopParameters.pop_back();
		popCount++;
            
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
    }else if(popCount < (int)(popSize * 0.7)){
        p.route = k_means( problem, (int)(popSize*0.05) + (int)(rnd01() * (popSize*0.08)) );
    	popCount++;
    }else {
        p.route = randomPopImproved( problem, rnd01 );
		popCount++;
		// p.route = randomPop( problem, rnd01 );
    }
    // cout << "pop " << p.to_string() << endl; // lll
}

bool eval_solution(
	const MySolution& p,
	MyMiddleCost &c)
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
	if(originNode != 0){
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

	return isFeasible;
}

MySolution mutate(
	const MySolution& baseGene,
	const std::function<double(void)> &rnd01,
	double shrink_scale)
{
	// cout << "\nb";// lll
	MySolution mutatedGene = baseGene;

	if( rnd01() > (initialProbMut + ((finalProbMut - initialProbMut) * (double)((double)generationCount/(double)generationSize))) ){
		return mutatedGene;
	}

	vector<int> points;

	do{
		int choosenNode = (int)((int)(rnd01() * (double)baseGene.route.size()) % baseGene.route.size());
		// if is not the same
		if(find(points.begin(), points.end(), choosenNode) == points.end()){
			points.push_back( choosenNode );
		}
	}while((int)points.size() < numPoints);

	int i = 0;
	vector<bool> usedPoints((int)points.size(), false);
		
	while(i < numPoints){

		unsigned int iPoint1;
		do{
			iPoint1 = (unsigned int)((int)(rnd01() * (double)points.size()) % points.size());
		}while(usedPoints[iPoint1]);

		unsigned int iPoint2;
		do{
			iPoint2 = (unsigned int)((int)(rnd01() * (double)points.size()) % points.size());
		}while(usedPoints[iPoint2] && iPoint2 == iPoint1);
		
		int auxNode = mutatedGene.route[points[iPoint1]];
		mutatedGene.route[points[iPoint1]] = mutatedGene.route[points[iPoint2]];
		mutatedGene.route[points[iPoint2]] = auxNode;

		usedPoints[iPoint1] = true;
		usedPoints[iPoint2] = true;

		i += 2;
	}
	
	//lll
	// cout << endl << shrink_scale << " " << (shrink_scale + (0.07 * (double)((double)generationCount/(double)generationSize)) );
	// for(int i = 0; i < (int)mutatedGene.route.size(); i++){
	// 	for(int j = i+1; j < (int)mutatedGene.route.size(); j++){
	// 		if(mutatedGene.route[i] == mutatedGene.route[j]){
	// 			cout << "---------------------------------------------------------------------\nigual: " << mutatedGene.route[i] << " [" << i << "] e [" << j << "]";
	// 		}
	// 		if(mutatedGene.route[i] < 1 || mutatedGene.route[i] > (int)mutatedGene.route.size()){
	// 			cout << "---------------------------------------------------------------------\ninconsistente " << mutatedGene.route[i];
	// 		}
	// 	}
	// }

	mutCount++;

	return mutatedGene;
}

MySolution crossover(
	const MySolution& gene1,
	const MySolution& gene2,
	const std::function<double(void)> &rnd01)
{
	// cout << "\na";// lll
	MySolution newGene1, newGene2;

	// chance to do crossover starts 80% and rises to 100%
	if( rnd01() > (initialProbCross + ((finalProbCross - initialProbCross) * (double)((double)generationCount/(double)generationSize))) ){
		if(rnd01() >= 0.5)
			return gene2;
		else
			return gene1;
	}

	vector<int> cuts;

	do{
		int choosenNode = (int)((int)(rnd01() * (double)gene1.route.size()) % gene1.route.size());
		// if is not the same
		if(find(cuts.begin(), cuts.end(), choosenNode) == cuts.end()){
			cuts.push_back( choosenNode );
		}
	}while((int)cuts.size() < numCuts);
	
	sort(cuts.begin(), cuts.end());
	
	// lll
	// cout << endl << "Cross:" << endl;
	// cout << endl << "Cuts: ";
	// printRoute(cuts);
	// cout << endl << "G1 : ";
	// printRoute(gene1.route);
	// cout << endl << "G2 : ";
	// printRoute(gene2.route);

	int iCut = 0;
	for(int i = 0; i < (int)gene1.route.size(); i++){
		int choosenNode1, choosenNode2;
		if(i >= cuts[iCut] && i < cuts[iCut + 1]){
			choosenNode1 =  gene2.route[i];
			choosenNode2 =  gene1.route[i];
		}else{
			choosenNode1 = gene1.route[i];
			choosenNode2 = gene2.route[i];
		}
		newGene1.route.push_back(choosenNode1);
		newGene2.route.push_back(choosenNode2);
		if(iCut + 1 < (int)cuts.size() - 1 && i >= cuts[iCut + 1]){
			iCut += 2;
		}
	}

	// lll
	// cout << endl << "G3 before correction: ";
	// cout << endl << "G3 : ";
	// printRoute(newGene.route);

    // Correcting cross over
    // Finding duplicated and missing nodes
    vector<int> duplicatedNodes;
    vector<int> missingNodes;
	vector<int> positionsToVerify;

	for(int i = 0; i < (int)cuts.size(); i += 2){
		int smallerIndex = cuts[i];
		int biggerIndex = cuts[i + 1];
		for(int j = smallerIndex; j < biggerIndex; j++){
			positionsToVerify.push_back(j);
		}
	}

	for(int i = 0; i < (int)positionsToVerify.size(); i++){
	
		bool isDuplicated = true;
		bool isMissing = true;
		
		for(int j = 0; j < (int)positionsToVerify.size(); j++){
		
			// if not found gene 2 in gene1 than it is duplicated in newGene1 and missing in newGene2
			if(gene2.route[positionsToVerify[i]] == gene1.route[positionsToVerify[j]]){
				isDuplicated = false;
			}
			// if not found gene1 in gene2 than it is missing in newGene1 and duplicated in newGene1
			if(gene1.route[positionsToVerify[i]] == gene2.route[positionsToVerify[j]]){
				isMissing = false;
			}
		}

		if(isDuplicated){
			duplicatedNodes.push_back(gene2.route[positionsToVerify[i]]);
		}
		if(isMissing){
			missingNodes.push_back(gene1.route[positionsToVerify[i]]);
		}

	}

	// lll
	// cout << endl << "G3 duplicated nodes: ";
	// cout << endl << "G3 : ";
	// printRoute(duplicatedNodes);
	// cout << endl << "G3 missing nodes: ";
	// cout << endl << "G3 : ";
	// printRoute(missingNodes);

    // Correcting duplicated nodes
	vector<int> reverseCuts = cuts;
	reverseCuts.insert(reverseCuts.begin(), 0);
	reverseCuts.push_back((int)gene1.route.size());
	positionsToVerify.clear();

	for(int i = 0; i < (int)reverseCuts.size(); i += 2){
		int smallerIndex = reverseCuts[i];
		int biggerIndex = reverseCuts[i + 1];
		for(int j = smallerIndex; j < biggerIndex; j++){
			positionsToVerify.push_back(j);
		}
	}

	for(int i = 0; i < (int)positionsToVerify.size(); i++){

		vector<int> :: iterator itDuplicatedNodes;
		vector<int> :: iterator itMissingNodes;
		// if node is duplicated in newGene1, replace it with a missing one
		itDuplicatedNodes = find(duplicatedNodes.begin(), duplicatedNodes.end(), newGene1.route[positionsToVerify[i]]);

		if( itDuplicatedNodes != duplicatedNodes.end() ){
			itMissingNodes = missingNodes.begin() + (itDuplicatedNodes - duplicatedNodes.begin());
			newGene1.route[positionsToVerify[i]] = *itMissingNodes;
		}

		// the reverse is done in newGene2
		itMissingNodes = find(missingNodes.begin(), missingNodes.end(), newGene2.route[positionsToVerify[i]]);
		
		if( itMissingNodes != missingNodes.end() ){
			itDuplicatedNodes = duplicatedNodes.begin() + (itMissingNodes - missingNodes.begin());
			newGene2.route[positionsToVerify[i]] = *itDuplicatedNodes;
		}

	}

	MySolution newGene;
	MyMiddleCost c1, c2;

	if(!eval_solution( newGene1, c1 )){
		newGene = newGene2;
	}else if(!eval_solution( newGene2, c2 )){
		newGene = newGene1;
	}else{
		if(c1.cost < c2.cost){
			newGene = newGene1;
		}else{
			newGene = newGene2;
		}
	}

	// lll
	// bool error = false;
	// for(int i = 0; i < (int)newGene.route.size(); i++){
	// 	for(int j = i+1; j < (int)newGene.route.size(); j++){
	// 		if(newGene.route[i] == newGene.route[j]){
	// 			cout << "\nigual: "
	// 			<< newGene.route[i] << " [" << i << "] e [" << j << "]";
	// 			error = true;
	// 		}
	// 		if(newGene.route[i] < 1 || newGene.route[i] > (int)newGene.route.size()){
	// 			cout << "\ninconsistente "
	// 			<< newGene.route[i];
	// 			error = true;
	// 		}
	// 	}
	// }
	// if(error){
	// 	cout << endl
	// 	<< "cuts"
	// 	<< endl;
	// 	printRoute(cuts);
	// 	cout << endl
	// 	<< "route"
	// 	<< endl;
	// 	printRoute(newGene.route);
	// 	cout << endl << endl;
	// }

	crossCount++;

	return newGene;
}

double calculate_SO_total_fitness(const GA_Type::thisChromosomeType &X)
{
	// finalize the cost
	return X.middle_costs.cost;
}

// std::ofstream output_file;

void SO_report_generation(
	int generation_number,
	const EA::GenerationType<MySolution,MyMiddleCost> &last_generation,
	const MySolution& best_genes)
{
	if(!test){
		std::cout
		<<"Generation ["<<generation_number<<"], "
		<<"Best="<<last_generation.best_total_cost<<", "
		<<"Average="<<last_generation.average_cost<<", "
		<<"Best genes=("<<best_genes.to_string()<<")"<<", "
		<<"Exe_time="<<last_generation.exe_time
		<<std::endl;
		
		cout << "Cross chance: " << (initialProbCross + ((finalProbCross - initialProbCross) * (double)((double)generationCount/(double)generationSize)))
		<< " numCuts: " << numCuts
		<< " Cross count: " << crossCount
		<< endl;

		cout << "Mut chance: " << (initialProbMut + ((finalProbMut - initialProbMut) * (double)((double)generationCount/(double)generationSize)))
		<< " numMut: " << numPoints
		<< " Mut count: " << mutCount
		<< endl << endl;
	}
	generationCount++;
	mutCount = 0;
	crossCount = 0;
	// output_file
	// 	<<generation_number<<"\t"
	// 	<<last_generation.average_cost<<"\t"
	// 	<<last_generation.best_total_cost
	// 	<<"\n";
}

void resetGlobals(){
	// variables to control pop creation
	popCount = 0;
	nnPopParameters = {
		{0.4, 0.4, 0.2},
		{0, 1, 0},
		{0.5, 0.5, 0},
		{0.3, 0.3, 0.4}
	};
	generationCount = 0;
	mutCount = 0;
	crossCount = 0;
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

	debug = false;
	test = true;
	generationSize = 100;
	popSize = 100;

	GA_Type ga_obj;
	ga_obj.problem_mode=EA::GA_MODE::SOGA;
	ga_obj.multi_threading=false;
	ga_obj.dynamic_threading=false;
	ga_obj.idle_delay_us=0; // switch between threads quickly
	ga_obj.verbose=false;
	ga_obj.population=popSize;
	ga_obj.generation_max=generationSize;
	ga_obj.calculate_SO_total_fitness=calculate_SO_total_fitness;
	ga_obj.init_genes=init_genes;
	ga_obj.eval_solution=eval_solution;
	ga_obj.mutate=mutate;
	ga_obj.crossover=crossover;
	ga_obj.SO_report_generation=SO_report_generation;
	ga_obj.best_stall_max=(int)((double)generationSize);
	ga_obj.average_stall_max=(int)((double)generationSize);
	ga_obj.tol_stall_best=1e-6;
	ga_obj.tol_stall_average=1e-6;
	ga_obj.elite_count=(int)(popSize*0.05);
	ga_obj.crossover_fraction=1; // keep 100% because it is controlled from function
	ga_obj.mutation_rate=1; // keep 100% because it is controlled from function

	if(!test){

		// ### BEGIN CLASSIC TEST
		// variables to control crossover
		numCuts = (int)(popSize*0.1) * 2;
		initialProbCross = 0.8;
		finalProbCross = 1;

		// variables to control mutation
		numPoints = (int)(popSize*0.1) * 2;
		initialProbMut = 0.1;
		finalProbMut = 0.05;

		problem = readFile("entrada.txt");
		problem.fitCriterion = 1; // Distance

		EA::Chronometer timer;
		timer.tic();

		ga_obj.solve();

		std::cout<<"The problem is optimized in "<<timer.toc()<<" seconds."<<std::endl;

		debug = true;
		MyMiddleCost c;
		ga_obj.eval_solution( ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes, c );

		// lll
		// MySolution perfectTest;
		// perfectTest.route = {81, 78, 76, 71, 70, 73, 77, 79, 80,57, 55, 54, 53, 56, 58, 60, 59,98, 96, 95, 94, 92, 93, 97, 100, 99,32, 33, 31, 35, 37, 38, 39, 36, 34,13, 17, 18, 19, 15, 16, 14, 12,90, 87, 86, 83, 82, 84, 85, 88, 89, 91,43, 42, 41, 40, 44, 46, 45, 48, 51, 50, 52, 49, 47,67, 65, 63, 62, 74, 72, 61, 64, 68, 66, 69,5, 3, 7, 8, 10, 11, 9, 6, 4, 2, 1, 75,20, 24, 25, 27, 29, 30, 28, 26, 23, 22, 21};
		// ga_obj.eval_solution( perfectTest , c );
		// cout << endl << c.cost << endl;
		// ### END CLASSIC TEST
	}else{
		int timesToTest = 10;
		// vector<vector<double>> testParameters{
		// 	//numCuts, initialProbCross, finalProbCross, numPoints, initialProbMut, finalProbMut
		// 	{(popSize*0.1) * 2,  0.9, 1,   (popSize*0.1) * 2,  0.05, 0},
		// 	{(popSize*0.1) * 2,  1,   0.9, (popSize*0.1) * 2,  0,    0.05},
		// 	{(popSize*0.05) * 2, 0.9, 1,   (popSize*0.05) * 2, 0.05, 0},
		// 	{(popSize*0.05) * 2, 1,   0.9, (popSize*0.05) * 2, 0,    0.05},
		// 	{(popSize*0.15) * 2, 0.9, 1,   (popSize*0.15) * 2, 0.05, 0},
		// 	{(popSize*0.15) * 2, 1,   0.9, (popSize*0.15) * 2, 0,    0.05}
		// };

		vector<vector<double>> testParameters;

		srand(time(NULL));
		for(int i = 0; i < 30; i++){
			double randnumCuts, randInitialProbCross, randFinalProbCross, randNumPoints, randInitialProbMut, randFinalProbMut;
			randnumCuts = (int)(popSize * (0.025 + 0.25 * ((double) rand() / (RAND_MAX))) ) * 2; // 5% to 55%
			randInitialProbCross = 0.5 + 0.5 * ((double) rand() / (RAND_MAX)); // 50% to 100%
			randFinalProbCross = 0.5 + 0.5 * ((double) rand() / (RAND_MAX)); // 50% to 100%
			randNumPoints = (int)(popSize * (0.025 + 0.25 * ((double) rand() / (RAND_MAX))) ) * 2; // 5% to 55%
			randInitialProbMut = 0.5 + 0.5 * ((double) rand() / (RAND_MAX)); // 50% to 100%
			randFinalProbMut = 0.5 + 0.5 * ((double) rand() / (RAND_MAX)); // 50% to 100%
			//numCuts, initialProbCross, finalProbCross, numPoints, initialProbMut, finalProbMut
			cout << "generated parameters" << endl
			<< randnumCuts<<" "<< randInitialProbCross<<" "<< randFinalProbCross<<" "<< randNumPoints<<" "<< randInitialProbMut<<" "<< randFinalProbMut << endl;
			testParameters.push_back({ randnumCuts, randInitialProbCross, randFinalProbCross, randNumPoints, randInitialProbMut, randFinalProbMut });
		}

		for(int i = 0; i < (int)testParameters.size(); i++){

			// variables to control crossover
			numCuts = (int)testParameters[i][0];
			initialProbCross = testParameters[i][1];
			finalProbCross = testParameters[i][2];

			// variables to control mutation
			numPoints = (int)testParameters[i][3];
			initialProbMut = testParameters[i][4];
			finalProbMut = testParameters[i][5];

			string fileName = "results/results";
			fileName.append(to_string(i)).append(".txt");
			ofstream outputTests;
			outputTests.open(fileName);

			string results;
			results = "\nInitiating tests:\n";
			results += "numCuts: " + to_string(numCuts) + ", initialProbCross: " + to_string(initialProbCross) + ", finalProbCross: " + to_string(finalProbCross) + "\n";
			results += "numPoints: " + to_string(numPoints) + ", initialProbMut: " + to_string(initialProbMut) + ", finalProbMut: : " + to_string(finalProbMut) + "\n";

			cout << results;
			outputTests << results;

			batteryTests(ga_obj, problem, "instances/solomon100/c101.txt", timesToTest, 1, resetGlobals, outputTests);
			batteryTests(ga_obj, problem, "instances/solomon100/rc101.txt", timesToTest, 1, resetGlobals, outputTests);
			batteryTests(ga_obj, problem, "instances/solomon100/r101.txt", timesToTest, 1, resetGlobals, outputTests);
			batteryTests(ga_obj, problem, "instances/homberger200/C1_2_1.TXT", timesToTest, 1, resetGlobals, outputTests);
			batteryTests(ga_obj, problem, "instances/homberger200/RC1_2_1.TXT", timesToTest, 1, resetGlobals, outputTests);
			batteryTests(ga_obj, problem, "instances/homberger200/R1_2_1.TXT", timesToTest, 1, resetGlobals, outputTests);

			outputTests.close();
		}

	}

	return 0;
}
// g++ -O3 -s -DNDEBUG -std=c++11 -pthread -I/src -Wall -Wconversion -Wfatal-errors -Wextra main.cpp