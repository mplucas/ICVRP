#include <algorithm>
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
        double sumCosts;
        double maxCost;

        // Functions that the user will pass
        function<void(GeneType&)> init_genes;
        function<bool(const GeneType&, double&)> eval_solution;
        function<GeneType(const GeneType&, const GeneType&)> crossover;
        function<GeneType(const GeneType&)> mutate;

        // Methods
        void populate()
        {
            population.clear();
            for(int i = 0; i < populationSize; i++)
            {
                thisChromosomeType chromosome;
                init_genes(chromosome.genes);
                while(!eval_solution(chromosome.genes, chromosome.cost))
                {
                    init_genes(chromosome.genes);
                }
                population.push_back(chromosome);
            }
        }

        void prepareRoulette()
        {
            maxCost = population[0].cost;

            for(int i = 1; i < (int)population.size(); i++)
            {
                if(population[i].cost > maxCost)
                {
                    maxCost = population[i].cost;
                }
            }

            sumCosts = 0;
            for(int i = 0; i < (int)population.size(); i++)
            {
                sumCosts += (maxCost - population[i].cost);
            }
        }

        thisChromosomeType selectParent()
        {
            double drawn = sumCosts * random01();
            double rouletteProgress = 0;
            int choosenParent = 0;

            for(int i = 0; i < (int)population.size(); i++)
            {
                rouletteProgress += (maxCost - population[i].cost);
                // cout << endl << rouletteProgress << " < " << drawn; // lll
                
                if(rouletteProgress > drawn)
                {
                    choosenParent = i;
                    break;
                }
            }

            // cout << endl << drawn << " " << choosenParent; // lll

            return population[choosenParent];
        }
};