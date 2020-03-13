#include <bits/stdc++.h>

using namespace std;

typedef struct{
	int x, y;
}location;

typedef struct{

	string sourceName;		        // sourcefile
	int numVehicles;	            // number of vehicles
	int numClients;                 // number of clients
	int capacity;	                // capacity os the vehicles

    // vectors withinformations about nodes:
        // 0 is the depot,
        // rest are clients
	vector<int> demand;		        // demands of clients
	vector<double> readyTime;	    // ready times of clients
	vector<double> dueTime;	        // due times of clients
	vector<double> serviceTime;     // service times of clients
	vector<vector<double>> cost;    // matrix with the costs between all the clients
	vector<location> locations;     // array with the location of each client:
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

        problem.numClients = 0;

        // skipping first client index
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
            
            // reading demand of client
            fileStream >> line;
            problem.demand.push_back( stoi(line) );

            // reading ready time of client
            fileStream >> line;
            problem.readyTime.push_back( stoi(line) );

            // reading due time of client
            fileStream >> line;
            problem.dueTime.push_back( stoi(line) );

            // reading service time of client
            fileStream >> line;
            problem.serviceTime.push_back( stoi(line) );

            problem.numClients++;

            // skipping next client index
            fileStream >> line;
        }
    }

    fileStream.close();

    // calculating costs
    
    // resizing matrix
    problem.cost.resize(problem.numClients);
    for(int i = 0; i < problem.numClients; i++){
        problem.cost[i].resize(problem.numClients);
    }

    for(int i = 0; i < problem.numClients; i++){

        for(int j = i; j < problem.numClients; j++){

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
    info += "\nNum. clients: " + problem.numClients;

    for(int i = 0; i < problem.numClients; i++){

        info += "\n\nIndex: " + i;
        info += "\nX: " + problem.locations[i].x;
        info += "\nY: " + problem.locations[i].y;
        info += "\nDemand: " + problem.demand[i];
        info += "\nReady Time: " + to_string( problem.readyTime[i] );
        info += "\nDue Time: " + to_string( problem.dueTime[i] );
        info += "\nService Time: " + to_string( problem.serviceTime[i] );
    }

    info += "\n\nCost Matrix:\n";

    for(int i = 0; i < problem.numClients; i++){
        
        info += "\n";
        
        for(int j = 0; j < problem.numClients; j++){
            info += to_string( problem.cost[i][j] ) + " ";
        }
    }

    return info;
}

void printVrp(vrp problem){

    cout << printVrpString(problem);
}