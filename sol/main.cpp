#include <algorithm>
#include <bits/stdc++.h>
#include "src/ga_vrp.hpp"

int main()
{
    setbuf(stdout, NULL);

	GA_Type ga;
    init_variables(ga);

    ga.solve();
	
	if(isSplitDelivery)
	{
		cout<<endl<<"RealNodes:"<<endl;
		printRealRoute(ga.population.front().genes, problem);
		cout<<endl<<"DebugRealNodes:"<<endl;//lll
		debugRealRoute(ga.population.front().genes, problem);//lll
		cout<<endl;
	}

    return 0;
}
// g++ main.cpp -Wall -O3 -I/src -std=c++11