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
        typedef ChromosomeType<GeneType> thisChromosomeType;
        
        // Atributes
        double sumCosts, maxCost, minCost;
        int countGeneration, countSameGenBest, reportCountGeneration;
        double lastGenBest;
        double currentTime, totalTime;
        int countMut, countCross;
        vector<double> rankCost;

        //Methods
        void prepareRouletteClassic()
        {
            maxCost = population[0].cost;
            minCost = population[0].cost;

            for(int i = 1; i < (int)population.size(); i++)
            {
                if(population[i].cost > maxCost)
                {
                    maxCost = population[i].cost;
                }
                if(population[i].cost < minCost)
                {
                    minCost = population[i].cost;
                }
            }

            sumCosts = 0;
            for(int i = 0; i < (int)population.size(); i++)
            {
                sumCosts += (maxCost + minCost - population[i].cost);
            }
        }

        void prepareRouletteRanking()
        {
            maxCost = 1.1;
            minCost = 0.9;

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
                    GeneType cleanGenes;
                    chromosome.genes = cleanGenes;
                    chromosome.cost = 0;
                    init_genes(chromosome.genes);
                }
                population.push_back(chromosome);
            }

            prepareRoulette();
        }

        void prepareRoulette()
        {
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
                double currentCost = (selectionType == 0 ? population[i].cost : rankCost[i]);
                rouletteProgress += (maxCost + minCost - currentCost);
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
                
                // crossover chance
                if( random01() <= (initialProbCross + ((finalProbCross - initialProbCross) * (double)((double)countGeneration/(double)minGenerationSize))) )
                {
                    newChromossome.genes = crossover(c1.genes,c2.genes);
                    countCross++;
                }else
                {
                    newChromossome = selectParent();
                }
                if(!eval_solution(newChromossome.genes, newChromossome.cost))
                {
                    continue;
                }

                // mutation chance
                if( random01() <= (initialProbMut + ((finalProbMut - initialProbMut) * (double)((double)countGeneration/(double)minGenerationSize))) )
                {
                    newChromossome.genes = mutate(newChromossome.genes);
                    countMut++;
                }
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
            cout
            <<endl<<endl<<"Best solution:"
            <<endl<<"\tCost: "<<population.front().cost
            <<endl<<"\tChromossome:"
            <<endl<<population.front().genes.to_string()
            <<endl<<"Exec_time: "<<totalTime/1000000<<"s"
            <<endl;
        }

        void reportGeneration()
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

            if(best.cost == lastGenBest && countGeneration >= minGenerationSize)
            {
                countSameGenBest++;
            }
            else
            {
                countSameGenBest = 0;
                lastGenBest = best.cost;
            }

            // cout
            // <<endl<<reportCountGeneration<<" "<<countSameGenBest<<" "<<best.cost<<" "<<average
            // <<endl<<best.cost<<" "<<lastGenBest
            // <<endl;

            cout
            <<endl<<"Generation ["<<reportCountGeneration<<"], "
            <<"Best="<<best.cost<<", "
            <<"Average="<<average<<", "
            <<endl<<"Crossover: Chance "<<(initialProbCross + ((finalProbCross - initialProbCross) * (double)((double)countGeneration/(double)minGenerationSize)))
            <<", Count "<<countCross
            <<endl<<"Mutation: Chance "<<(initialProbMut + ((finalProbMut - initialProbMut) * (double)((double)countGeneration/(double)minGenerationSize)))
            <<", Count "<<countMut
            <<endl<<"Best genes=("<<best.genes.to_string()<<")"<<", "
            <<endl<<"Exe_time="<<currentTime/1000000<<"s"
            <<endl;
        }

        void solve()
        {
            countGeneration = 0;
            countSameGenBest = 0;
            reportCountGeneration = 0;
            lastGenBest = 0;
            totalTime = 0;
            countCross = 0;
            countMut = 0;

            auto start = high_resolution_clock::now();
            populate();
            auto stop = high_resolution_clock::now();
            currentTime = duration_cast<microseconds>(stop - start).count();
            totalTime += currentTime;
            reportGeneration();
            
            while (countGeneration < minGenerationSize || countSameGenBest < minGenerationSize/4)
            {
                countCross = 0;
                countMut = 0;
                start = high_resolution_clock::now();
                newGeneration();
                stop = high_resolution_clock::now();
                if(countGeneration < minGenerationSize) countGeneration++;
                reportCountGeneration++;
                currentTime = duration_cast<microseconds>(stop - start).count();
                totalTime += currentTime;
                reportGeneration();
            }
            displayBest();
        }
};