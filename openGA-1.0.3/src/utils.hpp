#include <bits/stdc++.h>
#include <cfloat>

using namespace std;

typedef struct{
	int x, y;
}location;

typedef struct Vehicle{

	double timer = 0;
    int usedCapacity = 0;
    double distance = 0;
    vector<int> route;

    std::string to_string() const
    {
        std::ostringstream out;
		out << "{" << endl;
		out << "\tCap: " << usedCapacity << ", Timer: " << timer << ", Distance: " << distance << endl;
        out << "\tRoute: [ ";
		for(unsigned long i = 0;i < route.size(); i++)
			out << ( i?", ":"" ) << std::setprecision(10) << route[i];
		out << " ]";
		out << endl << "}";
		return out.str();
    }

}vehicle;

typedef struct Vrp{

	string sourceName;		        // sourcefile
	int numVehicles;	            // number of vehicles
	int numNodes;                   // number of nodes
	int capacity;	                // capacity of the vehicles
    int fitCriterion;               // criterion utilized in fit calculation:
                                        // 0 - Time
                                        // 1 - Distance

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

// Population functions
vector<int> randomPop( vrp problem, const std::function<double(void)> &rnd01 ){

    vector<int> newPop;
    vector<bool> visited(problem.numNodes, false);

	// marking depot as visited because it do not enter de solution
	visited[0] = true;
	
	for(int i = 1; i < problem.numNodes; i++){
		
		int choosen = (int)((int)(rnd01() * problem.numNodes) % problem.numNodes);

		// cout << choosen << " ";

		while(visited[choosen]){

			choosen++;
			choosen %= problem.numNodes;
		}

		newPop.push_back(choosen);
		visited[choosen] = true;
	}

    return newPop;

}

bool addIsFeasible( vector<int> route, int nodeToAdd, int addBeforeThisNode, vrp problem ){

    bool isFeasible = true;
    double oldBegin = 0;
    int oldOriginNode = 0;

    // adding return no depot to verify feasibility
    route.push_back(0);

    // calculating begin time of until the route reaches addBeforeThisNode
    for( int i = 0; i < addBeforeThisNode; i++ ){

        int destinyNode = route[i];

        oldBegin += problem.cost[oldOriginNode][destinyNode];

        // if vehicle arrives earlier than start of TW, it waits until the start
        if( oldBegin < problem.readyTime[destinyNode] ){
            oldBegin = problem.readyTime[destinyNode];
        }

        // adds service time
        oldBegin += problem.serviceTime[destinyNode];

        // update next origin
    	oldOriginNode = destinyNode;

    }

    // calculate begin of service service in nodeToAdd
    double newBegin = oldBegin;

    newBegin += problem.cost[oldOriginNode][nodeToAdd];

    // if vehicle arrives earlier than start of TW, it waits until the start
    if( newBegin < problem.readyTime[nodeToAdd] ){
        newBegin = problem.readyTime[nodeToAdd];
    }

    // saves origin of new partial path
    int newOriginNode = nodeToAdd;

    if( newBegin > problem.dueTime[nodeToAdd] ){ // fail

        isFeasible = false;

    }else{
        
        for( int i = addBeforeThisNode; i < route.size(); i++ ){
        
            int destinyNode = route[i];

            // CALCULATING OLDBEGIN
            oldBegin += problem.cost[oldOriginNode][destinyNode];

            // if vehicle arrives earlier than start of TW, it waits until the start
            if( oldBegin < problem.readyTime[destinyNode] ){
                oldBegin = problem.readyTime[destinyNode];
            }

            // adds service time
            oldBegin += problem.serviceTime[destinyNode];

            // update next origin
            oldOriginNode = destinyNode;

            // CALCULATING NEWBEGIN
            newBegin += problem.cost[newOriginNode][destinyNode];

            // if vehicle arrives earlier than start of TW, it waits until the start
            if( newBegin < problem.readyTime[destinyNode] ){
                newBegin = problem.readyTime[destinyNode];
            }

            // adds service time
            newBegin += problem.serviceTime[destinyNode];

            // update next origin (newOrigin and oldOrigin stays the same after the first iteration, it's better to understand like this)
            newOriginNode = destinyNode;
            
            // CALCULATING PUSH-FOWARD
            double pf = newBegin - oldBegin;
            
            if( pf <= 0 ){ // success
                break;
            }else if( newBegin + pf > problem.dueTime[destinyNode] ){ // fail
                isFeasible = false;
                break;
            }

        }

    }

    return isFeasible;

}

bool nearestNeighborPop( vector<int> &newPop, vrp problem, double distanceWeight, double timeWeight, double urgencyWeight ){

    bool isFeasible = true;
    vector<bool> routedNodes( problem.numNodes, false );
    int originNode = 0;
    int destinationIndex;
    double destinationCost;
    double timer = 0;
    int vehicleRouteStart = 0;
    int vehiclesUsed = 1;

    routedNodes[0] = true;

    for( int i = 1; i < problem.numNodes; i++ ){

        destinationCost = DBL_MAX; // infinity
        
        vector<int> :: const_iterator first = newPop.begin() + vehicleRouteStart;
        vector<int> :: const_iterator last = newPop.end();
        vector<int> testRoute(first, last);
        
        for( int j = 1; j < problem.numNodes; j++ ){
        
            if( !routedNodes[j] && addIsFeasible( testRoute, j, testRoute.size(), problem ) ){
                
                // the time difference between the completion of service at originNode and the beginning of service at j
                double time = max( problem.cost[originNode][j], problem.readyTime[j] - timer );
                double urgency = problem.dueTime[j] - ( timer + problem.cost[originNode][j] );
                double currentCost = distanceWeight * problem.cost[originNode][j] + timeWeight * time + urgencyWeight * urgency;

                if( currentCost < destinationCost ){
                    destinationCost = currentCost;
                    destinationIndex = j;
                }
            }

        }

        if( destinationCost != DBL_MAX ){ // if found a node to add

            newPop.push_back( destinationIndex );
            routedNodes[destinationIndex] = true;
            timer = max( timer + problem.cost[originNode][destinationIndex] + problem.serviceTime[destinationIndex], problem.readyTime[destinationIndex] );
            originNode = destinationIndex;

        }else{

            vehicleRouteStart = newPop.size();
            timer = 0;
            originNode = 0;

            vehiclesUsed++;
            if(vehiclesUsed > problem.numVehicles){
                isFeasible = false;
                break;
            }

            // returns to same node
            i--;
        }

    }    

    return isFeasible;
}