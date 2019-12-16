#include <bits/stdc++.h>
#include <time.h>
#include "ag.h"

using namespace std;

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char** argv) {
	int i, j;
	srand(time(NULL));
	int pop_size      = 60;
	int select        = 97;
	int cross_percent = 100;
	int generations   = 200;
	int repeat = 5;
	char source[] ={"C103.txt"};

	// variable to measure improvement
	double avgImprovement = 0;
	double auxImprovement;
	double bestImprovement = -1;

	// variables to measure time
	clock_t t;
	double avgTime = 0;
	double auxTime;
	double bestTime = INT_MAX;

	for(i=0; i<repeat; i++){

		// measuring time
		t = clock();

		// processing
		Genetic alg(pop_size, select, cross_percent, 50, generations, source, false, 0);
		alg.solve();
		auxImprovement = alg.getImprovRate();
		alg.destroy();

		t = clock() - t;
		// finished measuring time and printing time ans improvement of this iteration

		auxTime = ((float)t)/CLOCKS_PER_SEC;
		printf ("\nTime: %f seconds.\n", auxTime);
		avgTime += auxTime;
		bestTime = min(bestTime, auxTime);
		
		printf ("Improvement: %f.\n", auxImprovement );
		avgImprovement += auxImprovement;
		bestImprovement = max(bestImprovement, auxImprovement);

	}
	avgTime /= repeat;
	avgImprovement /= repeat;

	printf("\nAfter %d executions:\n", repeat);
	printf("Average Time: %.2f seconds\n", avgTime);
	printf("Best Time: %.2f seconds\n", bestTime);
	printf("Average Improvement: %.2f\n", avgImprovement);
	printf("Best Improvement: %.2f\n", bestImprovement);
	printf("Exiting code\n");
	return 0;
}
