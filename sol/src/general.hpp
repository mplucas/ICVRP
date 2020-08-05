#include <bits/stdc++.h>

bool srandOK = false;
double random01(){
    if(!srandOK){
        srand(time(NULL));
        srandOK = true;
    }
    // double aux = (double)rand()/(double)RAND_MAX;
    // std::cout<<std::endl<<aux;
    // return aux;
    return (double)rand()/(double)RAND_MAX;
}