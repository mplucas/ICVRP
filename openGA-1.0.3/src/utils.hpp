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

    return problem;
}

void printVrp(vrp problem){

    cout << "\nSource: " << problem.sourceName;
    cout << "\nNum. vehicle: " << problem.numVehicles;
    cout << "\nCapacity: " << problem.capacity;
    cout << "\nNum. clients: " << problem.numClients;

    for(int i = 0; i < problem.numClients; i++){

        cout << "\n\nIndex: " << i;
        cout << "\nX: " << problem.locations[i].x;
        cout << "\nY: " << problem.locations[i].y;
        cout << "\nDemand: " << problem.demand[i];
        cout << "\nReady Time: " << problem.readyTime[i];
        cout << "\nDue Time: " << problem.dueTime[i];
        cout << "\nService Time: " << problem.serviceTime[i];

    }

}