#include <iostream>
#include "ag.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char** argv) {
	int i, j;
	srand(time(NULL));
	int pop_size      = 60;
	int select        = 97;
	int cross_percent = 100;
	int generations   = 200;
	int repeat = 1;

	double sum= 0.0;
	char source[] ={"C103.txt"};
	for(i=0; i<repeat; i++){
		Genetic alg(pop_size, select, cross_percent, 50, generations, source, false, 2);
		alg.solve();
		sum += alg.getImprovRate();
		alg.destroy();
	}
	printf("\nAverage Improvement: %.2f\n", sum/(double)i);
	printf("\nExiting code\n");
	return 0;
}
