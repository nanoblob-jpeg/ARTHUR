#include <unistd.h>
#include <cstdlib>
int main(){
    int *a = new int[20000];
    short *b = new short[4836273];
    int **c = new int*[50];

    sleep(0.5);
    for(int i{}; i < 10; ++i){
        for(int j{}; j < 50; ++j){
            c[j] = new int[rand()%100+1];
            sleep(2);
        }

        for(int j{}; j < 30; ++j){
            delete c[j];
            sleep(2);
            }
    }
}
