#include <bits/stdc++.h>

using namespace std;

typedef struct{
	int x, y;
}location;

typedef struct{

	double timer = 0;
    int usedCapacity = 0;

}vehicle;

typedef struct{

	string sourceName;		        // sourcefile
	int numVehicles;	            // number of vehicles
	int numNodes;                   // number of nodes
	int capacity;	                // capacity of the vehicles

    // vectors with informations about nodes:
        // 0 is the depot,
        // rest are clients
	vector<int> demand;		        // demands of nodes
	vector<double> readyTime;	    // ready times of nodes
	vector<double> dueTime;	        // due times of nodes
	vector<double> serviceTime;     // service times of nodes
	vector<vector<double>> cost;    // matrix with the costs between all the nodes
	vector<location> locations;     // array with the location of each node:
}vrp;

double distanceAB(int x1, int y1, int x2, int y2){

	double realX, realY;
	
    realX = (double)(x1 - x2);
	realY = (double)(y1 - y2);
	
    return sqrt(realX * realX + realY * realY);
}

vrp readFile(string fileName){

    vrp problem;
    problem.sourceName = fileName;
    
    ifstream fileStream;
    fileStream.open(fileName);
    string line;

    if (fileStream.is_open()) {
        
        // skipping 4 words
        for(int i = 0; i < 4; i++)
            fileStream >> line;
        
        // reading number of vehicles
        fileStream >> line;
        problem.numVehicles = stoi(line);

        // reading capacity
        fileStream >> line;
        problem.capacity = stoi(line);

        // skipping 12 words
        for(int i = 0; i < 12; i++)
            fileStream >> line;

        problem.numNodes = 0;

        // skipping first node index
            fileStream >> line;

        while (!fileStream.eof()) {

            location auxLocation;

            // reading x coord
            fileStream >> line;
            auxLocation.x = stoi(line);

            // reading y coord
            fileStream >> line;
            auxLocation.y = stoi(line);

            problem.locations.push_back(auxLocation);
            
            // reading demand of node
            fileStream >> line;
            problem.demand.push_back( stoi(line) );

            // reading ready time of node
            fileStream >> line;
            problem.readyTime.push_back( stoi(line) );

            // reading due time of node
            fileStream >> line;
            problem.dueTime.push_back( stoi(line) );

            // reading service time of node
            fileStream >> line;
            problem.serviceTime.push_back( stoi(line) );

            problem.numNodes++;

            // skipping next node index
            fileStream >> line;
        }
    }

    fileStream.close();

    // calculating costs
    
    // resizing matrix
    problem.cost.resize(problem.numNodes);
    for(int i = 0; i < problem.numNodes; i++){
        problem.cost[i].resize(problem.numNodes);
    }

    for(int i = 0; i < problem.numNodes; i++){

        for(int j = i; j < problem.numNodes; j++){

            if( j == i ){

                problem.cost[i][j] = 0;
                continue;
            }

            double distance = distanceAB( problem.locations[i].x, problem.locations[i].y, problem.locations[j].x, problem.locations[j].y );
            problem.cost[i][j] = distance;
            problem.cost[j][i] = distance;
        }
    }

    return problem;
}

string printVrpString(vrp problem){

    string info;

    info = "\n\nSource: " + problem.sourceName;
    info += "\nNum. vehicle: " + problem.numVehicles;
    info += "\nCapacity: " + problem.capacity;
    info += "\nNum. nodes: " + problem.numNodes;

    for(int i = 0; i < problem.numNodes; i++){

        info += "\n\nIndex: " + i;
        info += "\nX: " + problem.locations[i].x;
        info += "\nY: " + problem.locations[i].y;
        info += "\nDemand: " + problem.demand[i];
        info += "\nReady Time: " + to_string( problem.readyTime[i] );
        info += "\nDue Time: " + to_string( problem.dueTime[i] );
        info += "\nService Time: " + to_string( problem.serviceTime[i] );
    }

    info += "\n\nCost Matrix:\n";

    for(int i = 0; i < problem.numNodes; i++){
        
        info += "\n";
        
        for(int j = 0; j < problem.numNodes; j++){
            info += to_string( problem.cost[i][j] ) + " ";
        }
    }

    return info;
}

void printVrp(vrp problem){

    cout << printVrpString(problem);
}