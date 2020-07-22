#include <bits/stdc++.h>

bool srandOK = false;
double random01(){
    if(!srandOK){
        srand(time(NULL));
        srandOK = true;
    }
    return (double)rand()/(double)RAND_MAX;
}