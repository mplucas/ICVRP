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
	vector<double> costs{0, 0};
    double generalCost = 0;

    bool operator < (const ChromosomeType& otherCT) const
    {
        if(costs[0] == otherCT.costs[0])
            return (costs[1] < otherCT.costs[1]);

        return (costs[0] < otherCT.costs[0]);
    }

    void cleanChromosome()
    {
        GeneType cleanGenes;
        genes = cleanGenes;
        costs = {0,0};
        generalCost = 0;
    }
};

template<typename GeneType>
class Genetic
{
    private:
        typedef ChromosomeType<GeneType> thisChromosomeType;
        
        // Atributes
        double sumCosts, maxCost, minCost, maxSecondaryCost;
        int countGeneration, countSameGenBest;
        vector<double> lastGenBestCosts;
        double currentTime;
        int countMut, countCross;
        vector<double> rankCost;

        //Methods
        void prepareGeneralCosts()
        {
            maxSecondaryCost = 0;
            
            for(int i = 0; i < (int)population.size(); i++)
            {
                if(population[i].costs[1] > maxSecondaryCost)
                    maxSecondaryCost = population[i].costs[1];
            }

            maxCost = 0;
            minCost = DBL_MAX;

            for(int i = 0; i < (int)population.size(); i++)
            {
                population[i].generalCost = population[i].costs[1] + (population[i].costs[0] * maxSecondaryCost);
                if(population[i].generalCost > maxCost)
                {
                    maxCost = population[i].generalCost;
                }
                if(population[i].generalCost < minCost)
                {
                    minCost = population[i].generalCost;
                }
            }
        }

        void prepareRouletteClassic()
        {
            sumCosts = 0;
            for(int i = 0; i < (int)population.size(); i++)
            {
                sumCosts += (maxCost + minCost - population[i].generalCost);
            }
        }

        void prepareRouletteRanking()
        {
            sort(population.begin(), population.end());
            rankCost.clear();

            for(int i = 1; i <= (int)population.size(); i++)
            {
                rankCost.push_back(minCost + (maxCost - minCost) * (i - 1) / ((int)population.size() - 1));
            }

            sumCosts = 0;
            for(int i = 0; i < (int)rankCost.size(); i++)
            {
                sumCosts += (maxCost + minCost - rankCost[i]);
                // cout<<endl<<rankCost[i];
            }

            // cout<<endl<<sumCosts<<endl;
        }
        
    public:
        // Atributes    
        int populationSize;
        vector<thisChromosomeType> population;
        int minGenerationSize;
        int eliteSize;
        int selectionType;  // 0: Classic, 1: By Rank
        double initialProbMut, finalProbMut;
        double initialProbCross, finalProbCross;

        // Atributes for measuring
        bool debug;
        double totalTime, setupTime = 0;
        int reportCountGeneration;
        thisChromosomeType best;

        // Functions that the user will pass
        function<void(GeneType&)> init_genes;
        function<bool(GeneType&, vector<double>&)> eval_solution;
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
                while(!eval_solution(chromosome.genes, chromosome.costs))
                {
                    chromosome.cleanChromosome();
                    init_genes(chromosome.genes);
                }
                population.push_back(chromosome);
            }

            prepareRoulette();
        }

        void prepareRoulette()
        {
            prepareGeneralCosts();
            if(selectionType == 0)
            {
                prepareRouletteClassic();
            }else
            {
                prepareRouletteRanking();
            }
        }

        // addicted roulette
        thisChromosomeType selectParent()
        {
            double drawn = sumCosts * random01();
            double rouletteProgress = 0;
            int choosenParent = 0;

            for(int i = 0; i < (int)population.size(); i++)
            {
                double currentCost = (selectionType == 0 ? population[i].generalCost : rankCost[i]);
                rouletteProgress += (maxCost + minCost - currentCost);
                // cout<<endl<<currentCost<<" "<<population[i].costs[0]<<" "<<population[i].costs[1]<<" "<<(maxCost + minCost - currentCost); // lll
                
                if(rouletteProgress > drawn)
                {
                    choosenParent = i;
                    break;
                }
            }
            // cout<<endl;//lll
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
                
                // crossover chance
                if( random01() <= (initialProbCross + ((finalProbCross - initialProbCross) * (double)((double)countGeneration/(double)minGenerationSize))) )
                {
                    newChromossome.genes = crossover(c1.genes,c2.genes);
                    countCross++;
                }else
                {
                    newChromossome = selectParent();
                }
                if(!eval_solution(newChromossome.genes, newChromossome.costs))
                {
                    continue;
                }

                // mutation chance
                if( random01() <= (initialProbMut + ((finalProbMut - initialProbMut) * (double)((double)countGeneration/(double)minGenerationSize))) )
                {
                    newChromossome.genes = mutate(newChromossome.genes);
                    countMut++;
                }
                if(!eval_solution(newChromossome.genes, newChromossome.costs))
                {
                    continue;
                }

                newPopulation.push_back(newChromossome);
            }

            population = newPopulation;

            prepareRoulette();
        }

        void reportGeneration()
        {
            vector<double> average {0, 0};
            thisChromosomeType best = population[0];

            for(int i = 1; i < (int)population.size(); i++)
            {
                average[0] += population[i].costs[0];
                average[1] += population[i].costs[1];
                
                if(population[i].generalCost < best.generalCost)
                {
                    best = population[i];
                }
            }
            average[0] /= (int)population.size();
            average[1] /= (int)population.size();

            // cout //lll
            // <<endl<<best.generalCost<<" "
            // <<best.costs[0]<<" "<<lastGenBestCosts[0]<<" "
            // <<best.costs[1]<<" "<<lastGenBestCosts[1]<<" "
            // <<countGeneration<<" "<<minGenerationSize
            // <<endl<<countSameGenBest
            // <<endl; // lll

            if(best.costs[0] == lastGenBestCosts[0] && best.costs[1] == lastGenBestCosts[1])
            {
                countSameGenBest++;
            }
            else
            {
                countSameGenBest = 0;
                lastGenBestCosts = best.costs;
            }

            if(debug)
            {
                cout
                <<endl<<"Generation ["<<reportCountGeneration<<"], "
                <<"BestCosts: { ";
                for(int i = 0; i < (int)best.costs.size(); i++)
                    cout<<(i?", ":"")<< best.costs[i];
                cout<<" }, "
                <<"Averages: { ";
                for(int i = 0; i < (int)average.size(); i++)
                    cout<<(i?", ":"")<< average[i];
                cout<<" }, "
                <<endl<<"Crossover: Chance "<<(initialProbCross + ((finalProbCross - initialProbCross) * (double)((double)countGeneration/(double)minGenerationSize)))
                <<", Count "<<countCross
                <<endl<<"Mutation: Chance "<<(initialProbMut + ((finalProbMut - initialProbMut) * (double)((double)countGeneration/(double)minGenerationSize)))
                <<", Count "<<countMut
                <<endl<<"Best genes=("<<best.genes.to_string()<<")"<<", "
                <<endl<<"Exe_time="<<currentTime<<"s"
                <<endl;
            }
        }

        void displayBest()
        {
            sort(population.begin(), population.end());
            best = population.front();

            if(debug)
            {
                cout
                <<endl<<"Best solution:"
                <<endl<<"Costs: { ";
                for(int i = 0; i < (int)best.costs.size(); i++)
                    cout<<(i?", ":"")<< best.costs[i];
                cout<<" }"
                <<endl<<"Chromossome:"
                <<endl<<best.genes.to_string()
                <<endl<<"Exec_time: "<<totalTime/1000000<<"s (Setup: "<<setupTime<<"s)"
                <<", Number of Generations: "<<reportCountGeneration
                <<endl;
            }
        }

        void solve()
        {
            countGeneration = 0;
            countSameGenBest = 0;
            reportCountGeneration = 0;
            lastGenBestCosts = vector<double>{DBL_MAX, DBL_MAX};
            totalTime = setupTime;
            countCross = 0;
            countMut = 0;
            auto start = high_resolution_clock::now();
            populate();
            auto stop = high_resolution_clock::now();
            currentTime = (double)duration_cast<microseconds>(stop - start).count()/1000000;
            totalTime += currentTime;
            reportGeneration();
            
            while (countGeneration < minGenerationSize || countSameGenBest < (minGenerationSize/4))
            {
                countCross = 0;
                countMut = 0;
                
                start = high_resolution_clock::now();
                newGeneration();
                stop = high_resolution_clock::now();
                currentTime = (double)duration_cast<microseconds>(stop - start).count()/1000000;
                totalTime += currentTime;

                if(countGeneration < minGenerationSize) countGeneration++;
                reportCountGeneration++;
                reportGeneration();
            }
            displayBest();
        }
};