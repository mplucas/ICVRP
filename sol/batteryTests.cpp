#include <algorithm>
#include <bits/stdc++.h>
#include "src/ga_vrp.hpp"

// void batteryTests(GA_Type ga_obj, vrp &problem, string entry, int timesToRepeat, int fitCriterion, void (*resetGlobals)(), ofstream &output){

//     double avgTime = 0;
//     double bestTime = DBL_MAX; // infinity
//     double avgValue = 0;
//     double bestValue = DBL_MAX;
//     MySolution bestSolution;
//     double samples[timesToRepeat];

//     for(int i = 0; i < timesToRepeat; i++){
    
//         problem = readFile(entry);
//         problem.fitCriterion = fitCriterion;

//         EA::Chronometer timer;
//         timer.tic();
        
//         ga_obj.solve();

//         // std::cout<<"The problem is optimized in "<<timer.toc()<<" seconds."<<std::endl;
//         double currentTime = timer.toc();
        
//         avgTime += currentTime;
        
//         bestTime = min(currentTime, bestTime);

//         avgValue += ga_obj.last_generation.best_total_cost;

//         if(ga_obj.last_generation.best_total_cost < bestValue){
//             bestValue = ga_obj.last_generation.best_total_cost;
//             bestSolution = ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes;
//         }

//         samples[i] = ga_obj.last_generation.best_total_cost;

//         // reseting global variables
//         (*resetGlobals)();
//     }

//     avgTime /= timesToRepeat;

//     avgValue /= timesToRepeat;

//     // calculating standard error
//     double stdDeviation = 0;
//     double stdError;
	
// 	for( int i = 0; i < timesToRepeat; i++ ){
// 		stdDeviation += pow( samples[i] - avgValue, 2 );
// 	}

// 	stdDeviation = sqrt( stdDeviation / (timesToRepeat - 1) );
// 	stdError = stdDeviation / sqrt(timesToRepeat);

//     // printf("\nAfter %d executions using %s:\n", timesToRepeat, entry.c_str());
// 	// printf("Average Time: %.2f seconds\n", avgTime);
// 	// printf("Best Time: %.2f seconds\n", bestTime);
// 	// printf("Average Value: %.2f\n", avgValue);
// 	// printf("Best Value: %.2f\n", bestValue);
// 	// printf("Standard Error: %.2f\n", stdError);
// 	// printf("Exiting code\n");

//     string results;
//     results = "\nAfter " + to_string(timesToRepeat) + " executions using " + entry + ":\n";
// 	results += "Average Time: " + to_string(avgTime) + " seconds\n";
// 	results += "Best Time: " + to_string(bestTime) + " seconds\n";
// 	results += "Average Value: " + to_string(avgValue) + "\n";
// 	results += "Best Value: " + to_string(bestValue) + "\n";
// 	results += "Standard Error: " + to_string(stdError) + "\n";
// 	results += "Best Solution:\n" + bestSolution.to_string() + "\n";

//     cout << results;
//     output << results;
// }


void batteryTestsSplitDelivery(GA_Type ga_obj, vrp &problem, string entry, int timesToRepeat, ofstream &output, double l, double u)
{
    double avgTime = 0;
    double bestTime = DBL_MAX; // infinity
    vector<double> avgValue {0, 0};
    double avgGeneralValue = 0;
    vector<double> bestValue {DBL_MAX, DBL_MAX};
    MySolution bestSolution;
    double samples[timesToRepeat];
    double timeSpentOnAdaptation = 0;

    for(int i = 0; i < timesToRepeat; i++)
    {    
        problem = readAndAdaptFileSplitDelivery(entry, 0.5, 0.75, l, u, 0.3, timeSpentOnAdaptation);
        problem.fitCriterion = 1;
        ga_obj.setupTime = timeSpentOnAdaptation;

        ga_obj.solve();

        avgTime += ga_obj.totalTime;
        
        bestTime = min(ga_obj.totalTime, bestTime);

        avgValue[0] += ga_obj.best.costs[0];
        avgValue[1] += ga_obj.best.costs[1];
        avgGeneralValue += ga_obj.best.generalCost;

        if(ga_obj.best.costs[0] < bestValue[0] || (ga_obj.best.costs[0] == bestValue[0] && ga_obj.best.costs[1] < bestValue[1])){
            bestValue[0] = ga_obj.best.costs[0];
            bestValue[1] = ga_obj.best.costs[1];
            bestSolution = ga_obj.best.genes;
        }

        samples[i] = ga_obj.best.generalCost;
    }

    avgTime /= timesToRepeat;

    avgValue[0] /= timesToRepeat;
    avgValue[1] /= timesToRepeat;
    avgGeneralValue /= timesToRepeat;

    // calculating standard error
    double stdDeviation = 0;
    double stdError;
	
	for( int i = 0; i < timesToRepeat; i++ ){
		stdDeviation += pow( samples[i] - avgGeneralValue, 2 );
	}

	stdDeviation = sqrt( stdDeviation / (timesToRepeat - 1) );
	stdError = stdDeviation / sqrt(timesToRepeat);

    string results;
    results = "\nAfter " + to_string(timesToRepeat) + " executions using " + entry + ":\n";
	results += "Average Time: " + to_string(avgTime) + " seconds\n";
	results += "Best Time: " + to_string(bestTime) + " seconds\n";
	results += "Average Value: { ";
    for(int i = 0; i < (int)avgValue.size(); i++)
    {
        results += (i?", ":"") + to_string(avgValue[i]);
    }
    results += " }\n";
	results += "Best Value: { ";
    for(int i = 0; i < (int)bestValue.size(); i++)
    {
        results += (i?", ":"") + to_string(bestValue[i]);
    }
    results += " }\n";
	results += "Standard Error: " + to_string(stdError) + "\n";
	results += "Best Solution:\n" + bestSolution.to_string() + "\n";
    results += "RealNodes:\n";
	results += printRealRouteString(ga_obj.best.genes, problem) + "\n";

    cout << results;
    output << results;
}

int main()
{
    setbuf(stdout, NULL);

	GA_Type ga;

    // globals
    debug = false;
    popSize = 100;
    
	// variables to control crossover
	numCuts = 2;

	// variables to control mutation
	numPoints = (int)(popSize*0.05)*2;

    //GA
    ga.populationSize = popSize;
	ga.minGenerationSize = 100;
	ga.eliteSize = (int)((double)popSize*0.05);
	ga.selectionType = 1;
    ga.init_genes = init_genes;
    ga.eval_solution = eval_solution;
	ga.crossover = crossover;
	ga.initialProbCross = 1;
	ga.finalProbCross = 0.8;
	ga.mutate = mutate;
	ga.initialProbMut = 0;
	ga.finalProbMut = 0.1;
	ga.debug = debug;

    int timesToTest = 2;
    
    // split delivery battery tests
    isSplitDelivery = true;	
    string instancesBasePath = "";
    vector<string> instances {"rc101"};
    vector<vector<double>> splitParameters
    {
        {0.01, 0.5},
        // {0.02, 1.0},
        // {0.50, 1.0},
        // {0.70, 1.0},
    };

    for(auto instance:instances)
    {
        for(auto splitParameter:splitParameters)
        {
            string fileName = "results/";
            fileName += instance;
            for(auto sp:splitParameter)
            {
                fileName += "_" + to_string(sp);
            }
            fileName += ".txt";
            ofstream outputTests;
            outputTests.open(fileName);

            string results;
            results = "\nInitiating tests:\n";
            results += fileName + "\n";

            cout << results;
            outputTests << results;

            batteryTestsSplitDelivery(ga, problem, "../instances/solomon100/" + instance + ".txt", timesToTest, outputTests, splitParameter[0], splitParameter[1]);

            outputTests.close();
        }
    }

    return 0;
}
// g++ batteryTests.cpp -Wall -O3 -I/src -std=c++11