#include <algorithm>
#include <bits/stdc++.h>
#include "src/utils.hpp"

// global vrp problem variable
vrp problem;

// variable for fractional delivery
bool isFractionalDelivery;

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

bool eval_solution(const MySolution& p, double &cost);

void init_genes(MySolution& p)
{
    if ((int)nnPopParameters.size() > 0) {
		int choosenParameters = (int)nnPopParameters.size() - 1;
		nearestNeighborPop( p.route, problem, nnPopParameters[choosenParameters][0], nnPopParameters[choosenParameters][1], nnPopParameters[choosenParameters][2] );
		nnPopParameters.pop_back();
        
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
		nearestNeighborPop( p.route, problem, parameters[0], parameters[1], parameters[2] );
    }else if(popCount < (int)(popSize * 0.7)){
        p.route = k_means( problem, (int)(popSize*0.05) + (int)(random01() * (popSize*0.08)) );
    }else {
        p.route = randomPopImproved( problem, random01 );
    }

	if(isFractionalDelivery){
		p.route = fixFDRoute(p.route, problem);
	}
	popCount++;
}

bool eval_solution(const MySolution& p, double &cost)
{
	bool isFeasible = true; // if true accepts gene, if false rejects gene

	// VARIABLES TO CONTROL VEHICLE BEING USED
	int choosenVehicle = 0; // vehicle wich route belongs to, if it turns equal to problem.numVehicles than its unfeasible
	double vehicleTimer = 0;
	int vehicleUsedCapacity = 0;
	int originNode = 0;

    // VARIABLES TO CALCULATE FIT ACCORDING TO FITCRITERION
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
    // cout << cost << endl; //lll

	return isFeasible;
}

MySolution mutate(const MySolution& baseGene)
{
	MySolution mutatedGene = baseGene;

	if( random01() > (initialProbMut + ((finalProbMut - initialProbMut) * (double)((double)generationCount/(double)generationSize))) ){
		return mutatedGene;
	}

	vector<int> points;

	do{
		int choosenNode = (int)(random01() * (double)baseGene.route.size());
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
			iPoint1 = (unsigned int)((int)(random01() * (double)points.size()) % points.size());
		}while(usedPoints[iPoint1]);

		unsigned int iPoint2;
		do{
			iPoint2 = (unsigned int)((int)(random01() * (double)points.size()) % points.size());
		}while(usedPoints[iPoint2] && iPoint2 == iPoint1);
		
		int auxNode = mutatedGene.route[points[iPoint1]];
		mutatedGene.route[points[iPoint1]] = mutatedGene.route[points[iPoint2]];
		mutatedGene.route[points[iPoint2]] = auxNode;

		usedPoints[iPoint1] = true;
		usedPoints[iPoint2] = true;

		i += 2;
	}

	mutCount++;

	if(isFractionalDelivery){
		mutatedGene.route = fixFDRoute(mutatedGene.route, problem);
	}

	// cout<<endl<<"Points:"<<endl;
	// printRoute(points);
	// cout<<endl<<"g:"<<endl;
	// printRoute(baseGene.route);
	// cout<<endl<<"new:"<<endl;
	// printRoute(mutatedGene.route);

	return mutatedGene;
}

MySolution crossover(const MySolution& gene1, const MySolution& gene2)
{
	MySolution newGene1, newGene2;

	// chance to do crossover starts 80% and rises to 100%
	if( random01() > (initialProbCross + ((finalProbCross - initialProbCross) * (double)((double)generationCount/(double)generationSize))) ){
		if(random01() >= 0.5)
			return gene2;
		else
			return gene1;
	}

	vector<int> cuts;

	do{
		int choosenNode = (int)((int)(random01() * (double)gene1.route.size()) % gene1.route.size());
		// if is not the same
		if(find(cuts.begin(), cuts.end(), choosenNode) == cuts.end()){
			cuts.push_back( choosenNode );
		}
	}while((int)cuts.size() < numCuts);
	
	sort(cuts.begin(), cuts.end());
	
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
	double c1, c2;

	if(!eval_solution( newGene1, c1 )){
		newGene = newGene2;
	}else if(!eval_solution( newGene2, c2 )){
		newGene = newGene1;
	}else{
		if(c1 < c2){
			newGene = newGene1;
		}else{
			newGene = newGene2;
		}
	}

	crossCount++;

	if(isFractionalDelivery){
		newGene.route = fixFDRoute(newGene.route, problem);
	}

	// cout<<endl<<"CUTS:"<<endl;
	// printRoute(cuts);
	// cout<<endl<<"g1:"<<endl;
	// printRoute(gene1.route);
	// cout<<endl<<"g2:"<<endl;
	// printRoute(gene2.route);
	// cout<<endl<<"new:"<<endl;
	// printRoute(newGene.route);

	return newGene;
}

int main()
{
    setbuf(stdout, NULL);

    // globals
    problem = readFile("entrada.txt");
    problem.fitCriterion = 1; // Distance

    debug = false;
    popSize = 100;
	generationSize = 100;
    isFractionalDelivery = false;

	// variables to control crossover
	numCuts = 2;
	initialProbCross = 1;
	finalProbCross = 0.8;

	// variables to control mutation
	numPoints = (int)(popSize*0.05)*2;
	initialProbMut = 0;
	finalProbMut = 0.1;

    //GA
    GA_Type ga;
    ga.populationSize = popSize;
	ga.generationSize = generationSize;
	ga.eliteSize = (int)((double)popSize*0.05);
    ga.init_genes = init_genes;
    ga.eval_solution = eval_solution;
	ga.crossover = crossover;
	ga.mutate = mutate;

    // ### TEST POPULATE		###############################################################################################
    ga.populate();

	// cout<<endl<<ga.population.size();
    // for(int i = 0; i < ga.population.size(); i++)
    // {
    //     cout << ga.population[i].genes.to_string() << endl << ga.population[i].cost << endl;
    // }
    // cout << endl;
    // ### END TEST POPULATE	###############################################################################################

    // ### TEST SELECTION		###############################################################################################
    ga.prepareRoulette();
    // vector<double> costsDrawed;

    // for(int i = 0; i < 100; i++)
    // {
    //     costsDrawed.push_back( ga.selectParent().cost );
    // }

    // vector<pair<double,int>> timesCostsDrawed;
    // sort(costsDrawed.begin(), costsDrawed.end());
    // pair<double,int> aux;
    // aux.first = costsDrawed[0];
    // aux.second = 1;
    // timesCostsDrawed.push_back(aux);

    // for(int i = 1; i < costsDrawed.size(); i++)
    // {
    //     if(costsDrawed[i] == timesCostsDrawed.back().first)
    //     {
    //         timesCostsDrawed.back().second++;
    //     }
    //     else
    //     {
    //         pair<double,int> aux;
    //         aux.first = costsDrawed[i];
    //         aux.second = 1;
    //         timesCostsDrawed.push_back(aux);
    //     }
    // }

    // for(int i = 0; i < timesCostsDrawed.size(); i++)
    // {
    //     cout << endl << timesCostsDrawed[i].first << ": " << timesCostsDrawed[i].second;
    // }
    // ### END TEST SELECTION	###############################################################################################

	// ### TEST CROSSOVER		###############################################################################################
	ChromosomeType<MySolution> chromossome1, chromossome2;

	chromossome1 = ga.selectParent();
	
	do{
		chromossome2 = ga.selectParent();
	}while(chromossome1.cost == chromossome2.cost);
	// cout<<endl;
	// printRoute(chromossome1.genes.route);
	// cout<<endl;
	// printRoute(chromossome2.genes.route);
	ga.crossover(chromossome1.genes, chromossome2.genes);
	// ### END TEST CROSSOVER	###############################################################################################

	// ### TEST CROSSOVER		###############################################################################################
	ga.mutate(chromossome1.genes);
	// ### END TEST CROSSOVER	###############################################################################################

    return 0;
}
