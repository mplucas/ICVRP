#include<bits/stdc++.h>
#include "general.hpp"

using std::vector;
using std::cout;
using std::endl;
using std::function;

template<typename GeneType>
struct ChromosomeType
{
	GeneType genes;
	double cost;
};

template<typename GeneType>
class Genetic
{
    private:
        
    public:

        typedef ChromosomeType<GeneType> thisChromosomeType;

        // Atributes    
        int populationSize;
        vector<thisChromosomeType> population;

        function<void(GeneType&)> init_genes;
        function<bool(const GeneType&, double &cost)> eval_solution;

        // Methods
        void populate()
        {
            population.clear();
            for(int i = 0; i < populationSize; i++){
                thisChromosomeType chromosome;
                init_genes(chromosome.genes);
                while(!eval_solution(chromosome.genes, chromosome.cost)){
                    init_genes(chromosome.genes);
                }
                population.push_back(chromosome);
            }
        }
};