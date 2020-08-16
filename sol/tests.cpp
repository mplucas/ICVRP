#include <algorithm>
#include <bits/stdc++.h>
#include "src/ga_vrp.hpp"

int main()
{
    setbuf(stdout, NULL);

    GA_Type ga;
    init_variables(ga);

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
    vector<double> costsDrawed;

    for(int i = 0; i < 100; i++)
    {
        costsDrawed.push_back( ga.selectParent().cost );
    }

    vector<pair<double,int>> timesCostsDrawed;
    sort(costsDrawed.begin(), costsDrawed.end());
    pair<double,int> aux;
    aux.first = costsDrawed[0];
    aux.second = 1;
    timesCostsDrawed.push_back(aux);

    for(int i = 1; i < costsDrawed.size(); i++)
    {
        if(costsDrawed[i] == timesCostsDrawed.back().first)
        {
            timesCostsDrawed.back().second++;
        }
        else
        {
            pair<double,int> aux;
            aux.first = costsDrawed[i];
            aux.second = 1;
            timesCostsDrawed.push_back(aux);
        }
    }

    for(int i = 0; i < timesCostsDrawed.size(); i++)
    {
        cout << endl << timesCostsDrawed[i].first << ": " << timesCostsDrawed[i].second;
    }
    // ### END TEST SELECTION	###############################################################################################

	// ### TEST CROSSOVER		###############################################################################################
	// ChromosomeType<MySolution> chromossome1, chromossome2;

	// chromossome1 = ga.selectParent();
	
	// do{
	// 	chromossome2 = ga.selectParent();
	// }while(chromossome1.cost == chromossome2.cost);
	// cout<<endl;
	// printRoute(chromossome1.genes.route);
	// cout<<endl;
	// printRoute(chromossome2.genes.route);
	// ga.crossover(chromossome1.genes, chromossome2.genes);
	// ### END TEST CROSSOVER	###############################################################################################

	// ### TEST CROSSOVER		###############################################################################################
	// ga.mutate(chromossome1.genes);
	// ### END TEST CROSSOVER	###############################################################################################

    return 0;
}
