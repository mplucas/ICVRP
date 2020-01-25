#include "ag.hpp"

using namespace std;

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char** argv) {

	setbuf(stdout, NULL);

	srand(time(NULL));
	int pop_size      = 60;
	int select        = 97;
	int cross_percent = 100;
	int generations   = 500;
	int repeat = 15;
	char source[] = {"entrada.txt"};

	// variable to measure value
	double avgValue = 0;
	double auxValue;
	double bestValue = INT_MAX;

	// variables to measure time
	clock_t t;
	double avgTime = 0;
	double auxTime;
	double bestTime = INT_MAX;

	// variables to calculate the standard error of the value
	double samples[repeat];
	double mediaValue = 0;
	double stdDeviation = 0;
	double stdError;

	for( int i = 0; i < repeat; i++ ){

		// measuring time
		t = clock();

		// processing
		Genetic alg(pop_size, select, cross_percent, 50, generations, source, false, 2);
		alg.solve();
		//printf("\nG\n\n");
		auxValue = alg.getFitness();
		//printf("\nH\n\n");
		alg.destroy();
		//printf("\nI\n\n");

		t = clock() - t;
		// finished measuring time and printing time ans value of this iteration

		auxTime = ((float)t)/CLOCKS_PER_SEC;
		printf ("\nTime: %f seconds.\n", auxTime);
		avgTime += auxTime;
		bestTime = min(bestTime, auxTime);
		
		printf ("Value: %f.\n", auxValue );
		avgValue += auxValue;
		bestValue = min(bestValue, auxValue);
		samples[i] = auxValue;

	}
	avgTime /= repeat;
	avgValue /= repeat;

	// calculating standard error
	for( int i = 0; i < repeat; i++ ){
		mediaValue += samples[i];
	}

	mediaValue /= repeat;

	for( int i = 0; i < repeat; i++ ){
		stdDeviation += pow( samples[i] - mediaValue, 2 );
	}

	stdDeviation = sqrt( stdDeviation / (repeat - 1) );
	stdError = stdDeviation / sqrt(repeat);

	// results
	printf("\nAfter %d executions:\n", repeat);
	printf("Average Time: %.2f seconds\n", avgTime);
	printf("Best Time: %.2f seconds\n", bestTime);
	printf("Average Value: %.2f\n", avgValue);
	printf("Best Value: %.2f\n", bestValue);
	printf("Standard Error: %.2f\n", stdError);
	printf("Exiting code\n");

	return 0;
}
