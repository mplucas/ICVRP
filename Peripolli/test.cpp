#include<bits/stdc++.h>

int main(int argc, char const *argv[])
{
    int test = 5;

    float t1 = ceil(float(test)*0.3);
    float t2 = floor(float(test)*0.7);

    printf("\n %f %f %f", t1,t2,t1+t2);
    return 0;
}
