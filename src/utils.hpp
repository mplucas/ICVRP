#include <bits/stdc++.h>
#include "openga.hpp"

struct MySolution
{
	std::vector<int> route;
	std::string to_string() const
	{
		std::ostringstream out;
		out << "{";
		for(unsigned long i = 0;i < route.size(); i++)
			out << ( i?",":"" ) << std::setprecision(10) << route[i];
		out << "}";
		return out.str();
	}
};

struct MyMiddleCost
{
	// This is where the results of simulation
	// is stored but not yet finalized.
	double cost;
};

typedef EA::Genetic<MySolution,MyMiddleCost> GA_Type;
typedef EA::GenerationType<MySolution,MyMiddleCost> Generation_Type;

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

string printVrpString(vrp problem, bool nodesDetails, bool matrix){

    string info;

    info = "\n\nSource: " + problem.sourceName;
    info += "\nNum. vehicle: " + problem.numVehicles;
    info += "\nCapacity: " + problem.capacity;
    info += "\nNum. nodes: " + problem.numNodes;

    if(nodesDetails){

        for(int i = 0; i < problem.numNodes; i++){

            info += "\n\nIndex: " + i;
            info += "\nX: " + problem.locations[i].x;
            info += "\nY: " + problem.locations[i].y;
            info += "\nDemand: " + problem.demand[i];
            info += "\nReady Time: " + to_string( problem.readyTime[i] );
            info += "\nDue Time: " + to_string( problem.dueTime[i] );
            info += "\nService Time: " + to_string( problem.serviceTime[i] );
        }
    }
    
    if(matrix){

        info += "\n\nCost Matrix:\n";

        for(int i = 0; i < problem.numNodes; i++){
            
            info += "\n";
            
            for(int j = 0; j < problem.numNodes; j++){
                info += to_string( problem.cost[i][j] ) + " ";
            }
        }
    }

    return info;
}

void printVrp(vrp problem, bool nodesDetails, bool matrix){

    cout << printVrpString(problem, nodesDetails, matrix);
}

bool addIsFeasible( vector<int> route, int nodeToAdd, int addBeforeThisNode, vrp problem ){

    // cout << "\nd " << nodeToAdd  << " " << addBeforeThisNode; //lll
    bool isFeasible = true;

    // VARIABLES TO CHECK CAPACITY
    int routeUsedCapacity = 0;
    
    // CHECKING CAPACITY
    for(int node : route){
        routeUsedCapacity += problem.demand[node];
    }

    if(problem.demand[nodeToAdd] + routeUsedCapacity > problem.capacity){
        // cout << "\nc"; //lll
        isFeasible = false;
        return isFeasible;
    }

    // VARIABLES TO CHECK TIME
    double oldBeginOfNext = 0;
    int oldOriginNode = 0;
    
    // CHECKING TIME
    // adding return no depot to verify feasibility
    route.push_back(0);

    // calculating begin time of until the route reaches addBeforeThisNode
    for( int i = 0; i < addBeforeThisNode; i++ ){

        int destinyNode = route[i];

        oldBeginOfNext += problem.cost[oldOriginNode][destinyNode];
        // if vehicle arrives earlier than start of TW, it waits until the start
        if( oldBeginOfNext < problem.readyTime[destinyNode] ){
            oldBeginOfNext = problem.readyTime[destinyNode];
        }
        // adds service time
        oldBeginOfNext += problem.serviceTime[destinyNode];

        // update next origin
    	oldOriginNode = destinyNode;
    }

    // calculate begin of service service in nodeToAdd
    double newBeginOfNext = oldBeginOfNext;

    newBeginOfNext += problem.cost[oldOriginNode][nodeToAdd];
    // if vehicle arrives earlier than start of TW, it waits until the start
    if( newBeginOfNext < problem.readyTime[nodeToAdd] ){
        newBeginOfNext = problem.readyTime[nodeToAdd];
    }else if( newBeginOfNext > problem.dueTime[nodeToAdd] ){
        isFeasible = false;
        return isFeasible;
    }
    // adds service time
    newBeginOfNext += problem.serviceTime[nodeToAdd];

    // saves origin of new partial path
    int newOriginNode = nodeToAdd;

    for( int i = addBeforeThisNode; i < (int)route.size(); i++ ){
    
        int destinyNode = route[i];

        // CALCULATING OLDBEGIN
        oldBeginOfNext += problem.cost[oldOriginNode][destinyNode];
        // if vehicle arrives earlier than start of TW, it waits until the start
        if( oldBeginOfNext < problem.readyTime[destinyNode] ){
            oldBeginOfNext = problem.readyTime[destinyNode];
        }

        // CALCULATING NEWBEGIN
        newBeginOfNext += problem.cost[newOriginNode][destinyNode];
        // if vehicle arrives earlier than start of TW, it waits until the start
        if( newBeginOfNext < problem.readyTime[destinyNode] ){
            newBeginOfNext = problem.readyTime[destinyNode];
        }
        
        // CALCULATING PUSH-FOWARD
        double pf = newBeginOfNext - oldBeginOfNext;
        // cout << endl << newBeginOfNext << " " << oldBeginOfNext << " " << destinyNode; //lll
        if( pf <= 0 ){ // success
            // cout << "\ns" << oldBeginOfNext << " " << pf << " " << problem.dueTime[destinyNode]; //lll
            break;
        }else if( oldBeginOfNext + pf > problem.dueTime[destinyNode] ){ // fail
            // cout << "\nf" << oldBeginOfNext << " " << pf << " " << problem.dueTime[destinyNode]; //lll
            isFeasible = false;
            break;
        }

        // adds service time
        oldBeginOfNext += problem.serviceTime[destinyNode];

        // update next origin
        oldOriginNode = destinyNode;

        // adds service time
        newBeginOfNext += problem.serviceTime[destinyNode];

        // update next origin (newOrigin and oldOrigin stays the same after the first iteration, it's better to understand like this)
        newOriginNode = destinyNode;
    }

    return isFeasible;
}

// teste
bool calculateFit(vector<int> route, vrp problem){
    bool isFeasible = true; // if true accepts gene, if false rejects gene
    double cost;
    bool debug = true;

	// VARIABLES TO CONTROL VEHICLE BEING USED
	int choosenVehicle = 0; // vehicle wich route belongs to, if it turns equal to problem.numVehicles than its unfeasible
	double vehicleTimer = 0;
	int vehicleUsedCapacity = 0;
	int originNode = 0;

    // VARIABLES TO CALCULATE FIT ACCORDING TO FITCRITERION
	// if problem.fitCriterion == 0 (Time)
    double biggestTimer = 0; // biggest cost in all routes, since it determines the cost of whole operation

    // if problem.fitCriterion == 1 (Distance)
    double totalDistance = 0;

	// VARIABLES TO DEBUG
	vector<vehicle> vehicleDebugger(1);
    
	for(unsigned int i = 0; i < route.size(); i++){
		
		int destinyNode = route[i];

		if( vehicleTimer + problem.cost[originNode][destinyNode] <= problem.dueTime[destinyNode]
			&& vehicleUsedCapacity + problem.demand[destinyNode] <= problem.capacity )
		{
			// add cost and demand to vehicle
			vehicleUsedCapacity += problem.demand[destinyNode];
			vehicleTimer += problem.cost[originNode][destinyNode];

			// if vehicle arrives earlier than start of TW, it waits until the start
			if( vehicleTimer < problem.readyTime[destinyNode] ){
				vehicleTimer = problem.readyTime[destinyNode];
			}

			// adds service time
			vehicleTimer += problem.serviceTime[destinyNode];

			if(problem.fitCriterion == 0){
                // save fit if it is bigger
                if( vehicleTimer > biggestTimer ){
                    biggestTimer = vehicleTimer;
                }
            }else if(problem.fitCriterion == 1){
				// add distance
                totalDistance += problem.cost[originNode][destinyNode];
            }

			if(debug){
                vehicleDebugger[choosenVehicle].timer = vehicleTimer;
                vehicleDebugger[choosenVehicle].usedCapacity = vehicleUsedCapacity;
				vehicleDebugger[choosenVehicle].distance += problem.cost[originNode][destinyNode];
				vehicleDebugger[choosenVehicle].route.push_back(destinyNode);
			}

			// update next origin
			originNode = destinyNode;
		}
		else
		{   
			// send vehicle back to depot
			vehicleTimer += problem.cost[originNode][0];

			if(problem.fitCriterion == 0){
                // save fit if it is bigger
                if( vehicleTimer > biggestTimer ){
                    biggestTimer = vehicleTimer;
                }
            }else if(problem.fitCriterion == 1){
				totalDistance += problem.cost[originNode][0];
			}

			if(debug){

				vehicleDebugger[choosenVehicle].timer = vehicleTimer;
				vehicleDebugger[choosenVehicle].usedCapacity = vehicleUsedCapacity;
				vehicle newVehicle;
				vehicleDebugger.push_back(newVehicle);

                // cout << "\nQuebra por: "; //lll
                // if(vehicleTimer + problem.cost[originNode][destinyNode] > problem.dueTime[destinyNode]){
                //     cout << "Time "; //lll
                // }
                // if(vehicleUsedCapacity + problem.demand[destinyNode] > problem.capacity){
                //     cout << "Capacity"; // lll
                // }
                // cout << endl;
			}

            // try to assign this node to the next vehicle route
            choosenVehicle++;
            
			if(choosenVehicle == problem.numVehicles){
				isFeasible = false;
				break;
			}

            // resets variables of vehicle
			vehicleTimer = 0;
			vehicleUsedCapacity = 0;
			originNode = 0;

			// returns to same client
			i--;
		}
	}

	// finalizing fit
    if(problem.fitCriterion == 0){

		// send vehicle back to depot
		vehicleTimer += problem.cost[originNode][0];

		// save fit if it is bigger
		if( vehicleTimer > biggestTimer ){
			biggestTimer = vehicleTimer;
		}
        
        cost = biggestTimer;
    
    }else if(problem.fitCriterion == 1){
        
        // Adding distance to return to depot of the last vehicle
        cost = totalDistance + problem.cost[originNode][0];
    }

	if(debug){
		vehicleDebugger.back().timer += problem.cost[originNode][0];
		vehicleDebugger.back().distance += problem.cost[originNode][0];

		for(unsigned int i = 0; i < vehicleDebugger.size(); i++){
			cout << "\nVehicle " << i << ":\n" << vehicleDebugger[i].to_string();
		}
        // cout << "\n Last node visited (or tried): p.route[" << i << "] = " << p.route[i];
	}

    if(isFeasible) cout << "FEASIBLE " << endl; //lll
	else cout << "NOT FEASIBLE " << endl; //lll
    cout << cost << endl; // lll

	return isFeasible;
}

void printRoute(vector<int> route){
    cout << "{";
    for(int i = 0; i < (int)route.size(); i++){
        cout << ( i?",":"" ) << std::setprecision(10) << route[i];
    }
	cout << "}";
}

// Population functions
vector<int> randomPop( vrp problem, const std::function<double(void)> &rnd01 ){

    // cout << "\na"; //lll
    vector<int> newPop;
    vector<bool> visited(problem.numNodes, false);
    int vehicleRouteStart = 0;

	// marking depot as visited because it do not enter de solution
	visited[0] = true;
	
	while( (int)newPop.size() < problem.numNodes - 1){

        // cout << endl << "a " << (int)newPop.size() << " " << problem.numNodes;// lll
		
		int choosenToAdd = (int)((int)(rnd01() * problem.numNodes) % problem.numNodes);

		// cout << choosenToAdd << " ";

		while(visited[choosenToAdd]){
			choosenToAdd++;
			choosenToAdd %= problem.numNodes;
		}
        // cout << endl << choosenToAdd;// lll

        vector<int> :: const_iterator first = newPop.begin() + vehicleRouteStart;
        vector<int> :: const_iterator last = newPop.end();
        vector<int> testRoute(first, last);

        // if insertion is not feasible, injects feasible nodes in the partial route
        if(addIsFeasible( testRoute, choosenToAdd, (int)testRoute.size(), problem )){
            // cout << endl;//lll
            // printRoute(newPop); //lll
            // cout << "\ntest route:\n"; //lll
            // printRoute(testRoute); //lll
            // cout << "\ninsert " << choosenToAdd << " in the back" << endl; //lll

            newPop.push_back(choosenToAdd);
		    visited[choosenToAdd] = true;

            // printRoute(newPop);//lll
            // calculateFit(newPop, problem); //lll
        }else{
            // getting not visited nodes
            vector<int> notVisited;
            for(int i = 0; i < problem.numNodes; i++){
                if(!visited[i]){
                    notVisited.push_back(i);
                }
            }

            // assigning not visited nodes to partial route
            while(notVisited.size() > 0){

                choosenToAdd = (int)((int)(rnd01() * (double)notVisited.size()) % notVisited.size());
                // cout << endl << "b " << (int)notVisited.size() << " " << choosenToAdd;// lll

                vector<bool> testedNodes((int)testRoute.size() + 1, false);

                for(int i = 0; i <= (int)testRoute.size(); i++){

                    int choosenToTest = (int)((int)(rnd01() * ((int)testRoute.size() + 1)) % ((int)testRoute.size() + 1));

                    // cout << choosenToTest << " ";

                    while(testedNodes[choosenToTest]){
                        choosenToTest++;
                        choosenToTest %= ((int)testRoute.size() + 1);
                    }

                    testedNodes[choosenToTest] = true;
                    
                    if(addIsFeasible( testRoute, notVisited[choosenToAdd], i, problem )){

                        // inserting node

                        // printRoute(newPop);//lll
                        // cout << "\ntest route:\n";//lll
                        // printRoute(testRoute);//lll
                        // cout << "\ninsert " << notVisited[choosenToAdd] << " before " << newPop[vehicleRouteStart + i] << endl;//lll

                        newPop.insert(newPop.begin() + vehicleRouteStart + i, notVisited[choosenToAdd]);
                        visited[notVisited[choosenToAdd]] = true;

                        // updating partial route
                        first = newPop.begin() + vehicleRouteStart;
                        last = newPop.end();
                        testRoute = vector<int>(first, last);

                        // printRoute(newPop); //lll
                        // calculateFit(newPop, problem); //lll
                        break;
                    }
                }

                // updating not visited nodes
                notVisited.erase(notVisited.begin() + choosenToAdd);
            }
            // updating partial route start
            vehicleRouteStart = (int)newPop.size();
        }
	}

    return newPop;

}

bool nearestNeighborPop( vector<int> &newPop, vrp problem, double distanceWeight, double timeWeight, double urgencyWeight ){

    bool isFeasible = true;
    vector<bool> routedNodes( problem.numNodes, false );
    int originNode = 0;
    int destinationIndex;
    double destinationCost;
    double timer = 0;
    int vehicleRouteStart = 0;
    int vehiclesUsed = 0;

    routedNodes[0] = true;

    for( int i = 1; i < problem.numNodes; i++ ){

        destinationCost = DBL_MAX; // infinity
        
        vector<int> :: const_iterator first = newPop.begin() + vehicleRouteStart;
        vector<int> :: const_iterator last = newPop.end();
        vector<int> testRoute(first, last);
        
        for( int j = 1; j < problem.numNodes; j++ ){
        
            if( !routedNodes[j] && addIsFeasible( testRoute, j, (int)testRoute.size(), problem ) ){
                
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
            timer = max( timer + problem.cost[originNode][destinationIndex], problem.readyTime[destinationIndex] ) + problem.serviceTime[destinationIndex];
            originNode = destinationIndex;

        }else{

            vehicleRouteStart = (int)newPop.size();
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

void solomonInsertion1Injection( vector<int> &newPop, int vehicleRouteStart, vector<bool> &routedNodes, vrp problem, double mi, double lambda, double a1, double a2 ){
        
    int previousNode;
    int nextNode;

    vector<int> :: const_iterator first = newPop.begin() + vehicleRouteStart;
    vector<int> :: const_iterator last = newPop.end();
    vector<int> testRoute(first, last);

    // inserting nodes using c1 and c2
    for( int i = 1; i < problem.numNodes; i++ ){

        if( !routedNodes[i] ){
            int positionToAdd = -1;
            double maxC2 = 0;
            double timer = 0;

            for( int j = vehicleRouteStart; j <= (int)newPop.size(); j++ ){
        
                if( j == vehicleRouteStart ){
                    previousNode = 0;
                }else{
                    previousNode = newPop[j-1];
                }
                if( j == (int)newPop.size() ){
                    nextNode = 0;
                }else{
                    nextNode = newPop[j];
                }
            
                if( addIsFeasible( testRoute, i, j - vehicleRouteStart, problem ) ){
                    
                    double oldBegin = max(timer + problem.cost[previousNode][nextNode], problem.readyTime[nextNode]);
                    double newBegin = max(timer + problem.cost[previousNode][i], problem.readyTime[i]);
                    newBegin = max(newBegin + problem.cost[i][nextNode], problem.readyTime[nextNode]);
                    double c1 = a1 * (problem.cost[previousNode][i] + problem.cost[i][nextNode] - mi * problem.cost[previousNode][nextNode])
                                + a2 * (newBegin - oldBegin);
                    double c2 = lambda * problem.cost[0][i] - c1;

                    if( c2 > maxC2 ){
                        positionToAdd = j;
                        maxC2 = c2;
                    }
                }
                // updating timer
                timer = max(timer + problem.cost[previousNode][nextNode], problem.readyTime[nextNode]) + problem.serviceTime[nextNode];
            }

            if( positionToAdd != -1 ){ // if found a feasible place to add

                // inserting node
                newPop.insert(newPop.begin() + positionToAdd, i);
                routedNodes[i] = true;

                // updating test route
                first = newPop.begin() + vehicleRouteStart;
                last = newPop.end();
                testRoute = vector<int>(first, last);
            }
        }
    }

    // inserting nodes using c1 and c2
    // for( int i = 1; i < problem.numNodes; i++ ){

    //     if( !routedNodes[i] ){
    //         int positionToAdd = -1;
    //         double minC1 = DBL_MAX; // infinity
    //         double timer = 0;

    //         for( int j = vehicleRouteStart; j <= (int)newPop.size(); j++ ){
        
    //             if( j == vehicleRouteStart ){
    //                 previousNode = 0;
    //             }else{
    //                 previousNode = newPop[j-1];
    //             }
    //             if( j == (int)newPop.size() ){
    //                 nextNode = 0;
    //             }else{
    //                 nextNode = newPop[j];
    //             }
            
    //             if( addIsFeasible( testRoute, i, j - vehicleRouteStart, problem ) ){
                    
    //                 double oldBegin = max(timer + problem.cost[previousNode][nextNode], problem.readyTime[nextNode]);
    //                 double newBegin = max(timer + problem.cost[previousNode][i], problem.readyTime[i]);
    //                 newBegin = max(newBegin + problem.cost[i][nextNode], problem.readyTime[nextNode]);
    //                 double c1 = a1 * (problem.cost[previousNode][i] + problem.cost[i][nextNode] - mi * problem.cost[previousNode][nextNode])
    //                             + a2 * (newBegin - oldBegin);

    //                 if( c1 < minC1 ){
    //                     positionToAdd = j;
    //                     minC1 = c1;
    //                 }
    //                 // if(j == (int)newPop.size()) cout << "\n ultimo " << j << " | escolhido " << positionToAdd; // lll
    //             } //else if(j == (int)newPop.size()) cout << "\n ultimo rejected " << j; // lll
    //             // updating timer
    //             timer = max(timer + problem.cost[previousNode][nextNode], problem.readyTime[nextNode]) + problem.serviceTime[nextNode];
    //         }

    //         // if(positionToAdd == (int)newPop.size()) cout << "\n adicionando na ultima posição " << i; // lll

    //         if( positionToAdd != -1 ){ // if found a feasible position
    //             int nodeToAdd = -1;
    //             double maxC2 = 0;

    //             for( int j = 1; j < problem.numNodes; j++ ){

    //                 if( !routedNodes[j] && addIsFeasible( testRoute, j, positionToAdd - vehicleRouteStart, problem ) ){
    //                     double c2 = lambda * problem.cost[0][j] - minC1;

    //                     if( c2 > maxC2 ){
    //                         nodeToAdd = j;
    //                         maxC2 = c2;
    //                     }
    //                 }
    //             }

    //             if(nodeToAdd != -1){ // if found a feasible node

    //                 // inserting node
    //                 newPop.insert(newPop.begin() + positionToAdd, nodeToAdd);
    //                 routedNodes[nodeToAdd] = true;

    //                 // updating test route
    //                 first = newPop.begin() + vehicleRouteStart;
    //                 last = newPop.end();
    //                 testRoute = vector<int>(first, last);
    //             }
    //         }
    //     }
    // }
    // cout << "\nvirou " << newPop.size() << endl; //lll
    // printRoute(newPop); // lll

    // // inserting nodes using c1
    // for( int i = vehicleRouteStart; i <= (int)newPop.size(); i++ ){
        
    //     double minCost = DBL_MAX; // infinity
    //     int choosenToAdd = -1;
    //     if( i == vehicleRouteStart ){
    //         previousNode = 0;
    //     }else{
    //         previousNode = newPop[i-1];
    //     }
    //     if( i == (int)newPop.size() ){
    //         nextNode = 0;
    //     }else{
    //         nextNode = newPop[i];
    //     }

    //     vector<int> :: const_iterator first = newPop.begin() + vehicleRouteStart;
    //     vector<int> :: const_iterator last = newPop.end();
    //     vector<int> testRoute(first, last);
    //     // finding node to add
    //     for( int j = 1; j < problem.numNodes; j++ ){
        
    //         if( !routedNodes[j] && addIsFeasible( testRoute, j, i - vehicleRouteStart, problem ) ){
                
    //             double oldBegin = max(timer + problem.cost[previousNode][nextNode], problem.readyTime[nextNode]);
    //             double newBegin = max(timer + problem.cost[previousNode][j], problem.readyTime[j]);
    //             newBegin = max(newBegin + problem.cost[j][nextNode], problem.readyTime[nextNode]);
    //             double c1 = a1 * (problem.cost[previousNode][j] + problem.cost[j][nextNode] - mi * problem.cost[previousNode][nextNode])
    //                         + a2 * (newBegin - oldBegin);

    //             if( c1 < minCost ){
    //                 choosenToAdd = j;
    //                 minCost = c1;
    //             }
    //         }
    //     }

    //     if( choosenToAdd != -1 ){ // if found a node to add

    //         // inserting node
    //         newPop.insert(newPop.begin() + i, choosenToAdd);
    //         routedNodes[choosenToAdd] = true;
        
    //         // updating timer
    //         timer += problem.cost[previousNode][choosenToAdd] + problem.serviceTime[choosenToAdd];
    //         // returning i to test insertion before node added
    //         i--;
    //     }
    // }

    // // inserting nodes using c2
    // for( int i = vehicleRouteStart; i <= (int)newPop.size(); i++ ){
        
    //     double minCost = DBL_MAX; // infinity
    //     int choosenToAdd = -1;
    //     if( i == vehicleRouteStart ){
    //         previousNode = 0;
    //     }else{
    //         previousNode = newPop[i-1];
    //     }
    //     if( i == (int)newPop.size() ){
    //         nextNode = 0;
    //     }else{
    //         nextNode = newPop[i];
    //     }

    //     vector<int> :: const_iterator first = newPop.begin() + vehicleRouteStart;
    //     vector<int> :: const_iterator last = newPop.end();
    //     vector<int> testRoute(first, last);
    //     // finding node to add
    //     for( int j = 1; j < problem.numNodes; j++ ){
        
    //         if( !routedNodes[j] && addIsFeasible( testRoute, j, i - vehicleRouteStart, problem ) ){
                
    //             double oldBegin = max(timer + problem.cost[previousNode][nextNode], problem.readyTime[nextNode]);
    //             double newBegin = max(timer + problem.cost[previousNode][j], problem.readyTime[j]);
    //             newBegin = max(newBegin + problem.cost[j][nextNode], problem.readyTime[nextNode]);
    //             double c1 = a1 * (problem.cost[previousNode][j] + problem.cost[j][nextNode] - mi * problem.cost[previousNode][nextNode])
    //                         + a2 * (newBegin - oldBegin);
    //             double c2 = lambda * problem.cost[0][j] - c1;

    //             if( c2 < minCost ){
    //                 choosenToAdd = j;
    //                 minCost = c2;
    //             }
    //         }
    //     }

    //     if( choosenToAdd != -1 ){ // if found a node to add

    //         // inserting node
    //         newPop.insert(newPop.begin() + i, choosenToAdd);
    //         routedNodes[choosenToAdd] = true;
        
    //         // updating timer
    //         timer += problem.cost[previousNode][choosenToAdd] + problem.serviceTime[choosenToAdd];
    //         // returning i to test insertion before node added
    //         i--;
    //     }
    // }
}

bool solomonInsertion1( vector<int> &newPop, vrp problem, int initType, double mi, double lambda, double a1, double a2 ){

    bool isFeasible = true;
    vector<bool> routedNodes( problem.numNodes, false );
    int originNode = 0;
    int destinationIndex;
    int vehicleRouteStart = 0;
    int vehiclesUsed = 0;

    routedNodes[0] = true;

    while( (int)newPop.size() < problem.numNodes ){
        
        vector<int> :: const_iterator first = newPop.begin() + vehicleRouteStart;
        vector<int> :: const_iterator last = newPop.end();
        vector<int> testRoute(first, last);
        
        // creating initial partial route based on initType (0 - farthest unrouted customer | 1 - earliest deadline unrouted customer)
        if(initType == 0){
            // cout << "\na"; //lll
            double maxDistance = 0;

            for( int i = 1; i < problem.numNodes; i++ ){
        
                if( !routedNodes[i] && addIsFeasible( testRoute, i, (int)testRoute.size(), problem ) && problem.cost[originNode][i] > maxDistance){
                    maxDistance = problem.cost[originNode][i];
                    destinationIndex = i;
                }
            }

            if( maxDistance != 0 ){ // if found a node to add

                newPop.push_back( destinationIndex );
                routedNodes[destinationIndex] = true;
                originNode = destinationIndex;
            }else{
                // cout << "\nera " << newPop.size() << endl; //lll
                // printRoute(newPop); // lll
                solomonInsertion1Injection(newPop, vehicleRouteStart, routedNodes, problem, mi, lambda, a1, a2);
                vehicleRouteStart = (int)newPop.size();
                originNode = 0;

                vehiclesUsed++;
                if(vehiclesUsed > problem.numVehicles){
                    isFeasible = false;
                    // cout << " - reject"; // lll
                    break;
                }
            }
        }else{
            // cout << "\nb"; //lll
            double earliestDeadline = DBL_MAX; // infinity

            for( int i = 1; i < problem.numNodes; i++ ){
        
                if( !routedNodes[i] && addIsFeasible( testRoute, i, (int)testRoute.size(), problem ) && problem.dueTime[i] < earliestDeadline ){
                    earliestDeadline = problem.dueTime[i];
                    destinationIndex = i;
                }
            }

            if( earliestDeadline != DBL_MAX ){ // if found a node to add
                // cout << "\nc"; //lll
                newPop.push_back( destinationIndex );
                routedNodes[destinationIndex] = true;
                originNode = destinationIndex;
            }else{
                // cout << "\nera " << newPop.size() << endl; //lll
                // printRoute(newPop); //lll
                solomonInsertion1Injection(newPop, vehicleRouteStart, routedNodes, problem, mi, lambda, a1, a2);
                vehicleRouteStart = (int)newPop.size();
                originNode = 0;

                vehiclesUsed++;
                if(vehiclesUsed > problem.numVehicles){
                    isFeasible = false;
                    // cout << " - reject"; // lll
                    break;
                }
            }
        }
    }    

    // cout << "\ntoma " << newPop.size(); //lll
    return isFeasible;
}

void batteryTests(GA_Type ga_obj, vrp &problem, string entry, int timesToRepeat, int fitCriterion, void (*resetGlobals)(), ofstream &output){

    double avgTime = 0;
    double bestTime = DBL_MAX; // infinity
    double avgValue = 0;
    double bestValue = DBL_MAX;
    MySolution bestSolution;
    double samples[timesToRepeat];

    for(int i = 0; i < timesToRepeat; i++){
    
        problem = readFile(entry);
        problem.fitCriterion = fitCriterion;

        EA::Chronometer timer;
        timer.tic();
        
        ga_obj.solve();

        // std::cout<<"The problem is optimized in "<<timer.toc()<<" seconds."<<std::endl;
        double currentTime = timer.toc();
        
        avgTime += currentTime;
        
        bestTime = min(currentTime, bestTime);

        avgValue += ga_obj.last_generation.best_total_cost;

        if(ga_obj.last_generation.best_total_cost < bestValue){
            bestValue = ga_obj.last_generation.best_total_cost;
            bestSolution = ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes;
        }

        samples[i] = ga_obj.last_generation.best_total_cost;

        // reseting global variables
        (*resetGlobals)();
    }

    avgTime /= timesToRepeat;

    avgValue /= timesToRepeat;

    // calculating standard error
    double stdDeviation = 0;
    double stdError;
	
	for( int i = 0; i < timesToRepeat; i++ ){
		stdDeviation += pow( samples[i] - avgValue, 2 );
	}

	stdDeviation = sqrt( stdDeviation / (timesToRepeat - 1) );
	stdError = stdDeviation / sqrt(timesToRepeat);

    // printf("\nAfter %d executions using %s:\n", timesToRepeat, entry.c_str());
	// printf("Average Time: %.2f seconds\n", avgTime);
	// printf("Best Time: %.2f seconds\n", bestTime);
	// printf("Average Value: %.2f\n", avgValue);
	// printf("Best Value: %.2f\n", bestValue);
	// printf("Standard Error: %.2f\n", stdError);
	// printf("Exiting code\n");

    string results;
    results = "\nAfter " + to_string(timesToRepeat) + " executions using " + entry + ":\n";
	results += "Average Time: " + to_string(avgTime) + " seconds\n";
	results += "Best Time: " + to_string(bestTime) + " seconds\n";
	results += "Average Value: " + to_string(avgValue) + "\n";
	results += "Best Value: " + to_string(bestValue) + "\n";
	results += "Standard Error: " + to_string(stdError) + "\n";
	results += "Best Solution:\n" + bestSolution.to_string() + "\n";

    cout << results;
    output << results;
}