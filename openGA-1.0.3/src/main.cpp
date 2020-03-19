#include <bits/stdc++.h>
#include "openga.hpp"
#include "utils.hpp"

// global vrp problem variable
vrp problem;

struct MySolution
{
	std::vector<int> route;

	std::string to_string() const
	{
		std::ostringstream out;
		out << "{";
		for(unsigned long i = 0;i < route.size(); i++)
			out << ( i?",":"" ) << std::setprecision(10) << route[i];
		out << "}";
		return out.str();
	}
};

struct MyMiddleCost
{
	// This is where the results of simulation
	// is stored but not yet finalized.
	double cost;
};

typedef EA::Genetic<MySolution,MyMiddleCost> GA_Type;
typedef EA::GenerationType<MySolution,MyMiddleCost> Generation_Type;

void init_genes(MySolution& p,const std::function<double(void)> &rnd01)
{
	// cout << "\n\na chosens:\n"; // lll
	vector<bool> visited(problem.numNodes, false);

	// marking depot as visited because it do not enter de solution
	visited[0] = true;
	
	for(int i = 1; i < problem.numNodes; i++){
		
		int choosen = (int)(rand() % problem.numNodes);

		// cout << choosen << " ";

		while(visited[choosen]){

			choosen++;
			choosen %= problem.numNodes;
		}

		p.route.push_back(choosen);
		visited[choosen] = true;
	}
}

bool eval_solution(
	const MySolution& p,
	MyMiddleCost &c)
{
	// cout << "b";// lll
	vector<vehicle> vehicles( problem.numVehicles );
	// vector<double> distances( problem.numVehicles, 0 );
	int choosenVehicle = 0;
	int originNode = 0;
	int countRejected = 0;  // counter of vehicles that were rejected, if it reaches problem.numVehicles, then the solution is unfeaseble
	double bestFit = 0;     // biggest cost in all routes, since it determines the cost of whole operation
	bool isFeasible = true; // if true accepts gene, if false rejects gene

	for(int i = 0; i < p.route.size(); i++){
		
		int destinyNode = p.route[i];

		if( vehicles[choosenVehicle].timer + problem.cost[originNode][destinyNode] <= problem.dueTime[destinyNode]
			&& vehicles[choosenVehicle].usedCapacity + problem.demand[destinyNode] <= problem.capacity )
		{
			// add cost and demand to vehicle
			vehicles[choosenVehicle].usedCapacity += problem.demand[destinyNode];
			vehicles[choosenVehicle].timer += problem.cost[originNode][destinyNode];
			// distances[choosenVehicle] += problem.cost[originNode][destinyNode];

			// if vehicle arrives earlier than start of TW, it waits until the start
			if( vehicles[choosenVehicle].timer < problem.readyTime[destinyNode] ){
				vehicles[choosenVehicle].timer = problem.readyTime[destinyNode];
			}

			// adds service time
			vehicles[choosenVehicle].timer += problem.serviceTime[destinyNode];

			// save fit if it is bigger
			// if( distances[choosenVehicle] > bestFit ){
			// 	bestFit = distances[choosenVehicle];
			// }
			if( vehicles[choosenVehicle].timer > bestFit ){
				bestFit = vehicles[choosenVehicle].timer;
			}

			// update destiny
			originNode = destinyNode;

			// reset rejected vehicles
			countRejected = 0;
		}
		else
		{
			// send vehicle back to depot, add cost and reset capacity
			vehicles[choosenVehicle].usedCapacity = 0;
			vehicles[choosenVehicle].timer += problem.cost[originNode][0];
			// distances[choosenVehicle] += problem.cost[originNode][0];

			// resets originNode to depot
			originNode = 0;

			// increments rejected vehicles
			countRejected++;

			if( countRejected == problem.numVehicles ){
				isFeasible = false;
				break;
			}

			// try to assign this node to the next vehicle route
			choosenVehicle++;
			choosenVehicle %= problem.numVehicles;
			i--;
		}
	}

	c.cost = bestFit;

	return isFeasible;
}

MySolution mutate(
	const MySolution& baseGene,
	const std::function<double(void)> &rnd01,
	double shrink_scale)
{
	// cout << "c";// lll

	MySolution mutatedGene = baseGene;

	if( rnd01() < shrink_scale ){
		
		int choosenNode1 = (int)(rand() % baseGene.route.size());
		int choosenNode2;

		do{
			choosenNode2 = (int)(rand() % baseGene.route.size());
		}while(choosenNode2 == choosenNode1);

		mutatedGene.route[choosenNode1] = baseGene.route[choosenNode2];
		mutatedGene.route[choosenNode2] = baseGene.route[choosenNode1];
	}

	return mutatedGene;
}

MySolution crossover(
	const MySolution& gene1,
	const MySolution& gene2,
	const std::function<double(void)> &rnd01)
{
	// cout << "d"; // lll
	MySolution newGene;
	
	int choosenNode1 = (int)(rand() % gene1.route.size());
	int choosenNode2;

	do{
		choosenNode2 = (int)(rand() % gene1.route.size());
	}while(choosenNode2 == choosenNode1);

	int smallerIndex, biggerIndex;

	if( choosenNode1 > choosenNode2 ){
		
		biggerIndex  = choosenNode1;
		smallerIndex = choosenNode2;
	}else{

		biggerIndex  = choosenNode2;
		smallerIndex = choosenNode1;
	}

	for(int i = 0; i < smallerIndex; i++){
		newGene.route.push_back( gene1.route[i] );
	}
	for(int i = smallerIndex; i < biggerIndex; i++){
		newGene.route.push_back( gene2.route[i] );
	}
	for(int i = biggerIndex; i < gene1.route.size(); i++){
		newGene.route.push_back( gene1.route[i] );
	}

	return newGene;
}

double calculate_SO_total_fitness(const GA_Type::thisChromosomeType &X)
{
	// cout << "e"; // lll
	// finalize the cost
	return X.middle_costs.cost;
}

std::ofstream output_file;

void SO_report_generation(
	int generation_number,
	const EA::GenerationType<MySolution,MyMiddleCost> &last_generation,
	const MySolution& best_genes)
{
	std::cout
		<<"Generation ["<<generation_number<<"], "
		<<"Best="<<last_generation.best_total_cost<<", "
		<<"Average="<<last_generation.average_cost<<", "
		<<"Best genes=("<<best_genes.to_string()<<")"<<", "
		<<"Exe_time="<<last_generation.exe_time
		<<std::endl;

	output_file
		<<generation_number<<"\t"
		<<last_generation.average_cost<<"\t"
		<<last_generation.best_total_cost
		<<"\n";
}

int main()
{

	srand(time(NULL));
	problem = readFile("entrada.txt");	

	output_file.open("./bin/result_.txt");
	output_file
		<<"step"<<"\t"
		<<"cost_avg"<<"\t"
		<<"cost_best"
		<<"\n";

	EA::Chronometer timer;
	timer.tic();

	GA_Type ga_obj;
	ga_obj.problem_mode=EA::GA_MODE::SOGA;
	ga_obj.multi_threading=true;
	ga_obj.dynamic_threading=false;
	ga_obj.idle_delay_us=0; // switch between threads quickly
	ga_obj.verbose=false;
	ga_obj.population=10000;
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
	ga_obj.solve();

	std::cout<<"The problem is optimized in "<<timer.toc()<<" seconds."<<std::endl;

	output_file.close();
	return 0;
}
// g++ -O3 -s -DNDEBUG -std=c++11 -pthread -I./src -Wall -Wconversion -Wfatal-errors -Wextra main.cpp