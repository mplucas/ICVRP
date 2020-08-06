#include <algorithm>
#include<bits/stdc++.h>
#include "general.hpp"

using std::vector;
using std::cout;
using std::endl;
using std::function;
using namespace std::chrono;

template<typename GeneType>
struct ChromosomeType
{
	GeneType genes;
	double cost;

    bool operator < (const ChromosomeType& ct) const
    {
        return (cost < ct.cost);
    }
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
        int generationSize;
        int generationCount;
        int eliteSize;

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

            prepareRoulette();
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

        void newGeneration()
        {
            vector<thisChromosomeType> newPopulation;
            
            // transfering elite chromossomes
            sort(population.begin(), population.end());
            for(int i = 0; i < eliteSize; i++)
            {
                newPopulation.push_back(population[i]);
            }

            // filling the rest of new population with children of the current population
            while((int)newPopulation.size() < populationSize)
            {
                thisChromosomeType c1 = selectParent();
                thisChromosomeType c2 = selectParent();
                thisChromosomeType newChromossome;
                
                newChromossome.genes = crossover(c1.genes,c2.genes);
                if(!eval_solution(newChromossome.genes, newChromossome.cost))
                {
                    continue;
                }

                newChromossome.genes = mutate(newChromossome.genes);
                if(!eval_solution(newChromossome.genes, newChromossome.cost))
                {
                    continue;
                }

                newPopulation.push_back(newChromossome);
            }

            population = newPopulation;

            prepareRoulette();
        }

        void displayBest()
        {
            sort(population.begin(), population.end());
            cout<<endl<<endl<<"Best solution:"<<endl
            <<"\tCost: "<<population.front().cost<<endl
            <<"\tChromossome:"<<endl
            <<population.front().genes.to_string();
        }

        void reportGeneration(int index, double time)
        {
            double average = 0;
            thisChromosomeType best;
            best.cost = DBL_MAX;

            for(int i = 0; i < (int)population.size(); i++)
            {
                average += population[i].cost;
                
                if(population[i].cost < best.cost)
                {
                    best = population[i];
                }
            }
            average /= (int)population.size();

            cout
            <<"Generation ["<<index<<"], "
            <<"Best="<<best.cost<<", "
            <<"Average="<<average<<", "
            <<"Best genes=("<<best.genes.to_string()<<")"<<", "
            <<"Exe_time="<<time<<"s"
            <<endl;
        }

        void solve()
        {
            generationCount = 0;

            auto start = high_resolution_clock::now();
            populate();
            auto stop = high_resolution_clock::now();
            reportGeneration(generationCount, duration_cast<seconds>(stop - start).count());

            while (generationCount < generationSize)
            {
                start = high_resolution_clock::now();
                newGeneration();
                stop = high_resolution_clock::now();
                generationCount++;
                reportGeneration(generationCount, duration_cast<seconds>(stop - start).count());
            }
            displayBest();
        }
};