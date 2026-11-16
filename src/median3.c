#include "median3.h"
int16_t median3(int16_t a,int16_t b,int16_t c){
    int16_t t;
    if(a>b){t=a;a=b;b=t;} if(b>c){t=b;b=c;c=t;} if(a>b){t=a;a=b;b=t;}
    return b;
}
