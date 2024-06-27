/*
Question for Operating System Concepts
5 philosopheres with 2 states: thinking and eating
sits in a circular table with 5 chairs, there are
5 single chopsticks on both sides of the philosophers
a philosopher can only eat if picks up 2 chopsticks

Need to  implement in a deadlock free way and avoid starvation
*/

#include "stdio.h"
#include "pthread.h"
#include "string.h"
#include "unistd.h"
#include "stdlib.h"
#include "queue"

#define n_philosophers 5

class ThreadParam;
class Table;

std::queue<int> line;

class ThreadParam{
public:
ThreadParam(Table* table, int position, int time_eating, int time_thinking):
    table(table), 
    position(position), 
    time_eating(time_eating),
    time_thinking(time_thinking),
    n_food_ate(0) {};

    Table* table;         
    int position;   
    int time_eating;
    int time_thinking;
    int n_food_ate;
};

class Table{
public:
    int foodCount[n_philosophers];
    bool isChopsticksAvailable[n_philosophers];

    pthread_mutex_t mutex_;
    pthread_cond_t cv_ready;
    pthread_cond_t cv_eating;

    bool isEating[n_philosophers];

    Table(){
        for(int i=0; i<n_philosophers; i++){
            isChopsticksAvailable[i] = true;
            foodCount[i] = 0;
        }

        pthread_mutex_init(&mutex_,NULL);
        pthread_cond_init(&cv_ready, NULL);
        pthread_cond_init(&cv_eating, NULL);
    }

    ~Table(){
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&cv_ready);
        pthread_cond_destroy(&cv_eating);
    }
};

void lineUpToEat(Table* table, int position){
    pthread_mutex_lock(&(table->mutex_));
    line.push(position);
    pthread_mutex_unlock(&(table->mutex_));
}

void think(Table* table, int position, int time_thinking){
    printf("Philosopher [%d] thinking!\n", position);
    usleep(time_thinking);
    lineUpToEat(table, position);
}

void pickUpChopsticks(Table* table, int position, int l, int r){
    pthread_mutex_lock(&(table->mutex_));

    // For fairness sake, check if in front of the line, else wait
    while(line.front()!=position){
        pthread_cond_wait(&table->cv_ready, &table->mutex_);
    }
    line.pop();
    pthread_cond_broadcast(&table->cv_ready);    

    // Wait if neighbors are using chopsticks
    while (!table->isChopsticksAvailable[l]||!table->isChopsticksAvailable[r]){
        pthread_cond_wait(&table->cv_eating, &table->mutex_);
    }

    // Picks up both chopsticks
    table->isChopsticksAvailable[l] = false;
    table->isChopsticksAvailable[r] = false;
    printf("Philosopher [%d] picks up the chopsticks: left = [%d], right = [%d]\n", position, l, r);

    pthread_mutex_unlock(&(table->mutex_));
}

void putDownChopsticks(Table* table, int position, int l, int r){
    pthread_mutex_lock(&(table->mutex_));
    table->foodCount[position]++;

    // Put down the chopsticks
    printf("Philosopher [%d] puts down the chopsticks: left = [%d], right = [%d]\n", position, l, r);
    table->isChopsticksAvailable[l] = true;
    table->isChopsticksAvailable[r] = true;
    pthread_cond_broadcast(&table->cv_eating);

    pthread_mutex_unlock(&(table->mutex_));
}


void eat(Table* table, int position, int time_eating){
    int l = position;
    int r = (position+1)%n_philosophers;

    // Part 1: Check chopsticks availability
    pickUpChopsticks(table, position, l, r);

    // Part 2: Eat for designated time
    printf("Philosopher [%d] eating!\n", position);
    usleep(time_eating);

    // Part 3: Restore chopsticks availability
    putDownChopsticks(table, position, l, r);
    
}

void* philosopher(void* arg){
    ThreadParam* philosopherParam = (ThreadParam*) arg;
    Table* table = philosopherParam->table;
    int time_eating = philosopherParam->time_eating;
    int time_thinking = philosopherParam->time_thinking;
    int position = philosopherParam->position;
    delete philosopherParam;

    while(1){
        think(table, position, time_thinking);
        eat(table, position, time_eating);
    }
}

int main(int argc, char** argv){
    if(argc!=4){
        printf("Usage: ./a.out total_running_time_seconds time_eating time_thinking");
        return 1;
    }

    int total_running_time_seconds = atoi(argv[1]);
    int time_eating = atoi(argv[2]);
    int time_thinking = atoi(argv[3]);

    Table table;

    pthread_t threads[n_philosophers];

    for(int i=0; i<n_philosophers; i++){
        ThreadParam* philosopherParam = new ThreadParam(&table, i, time_eating, time_thinking);
        pthread_create(&threads[i], NULL, philosopher, philosopherParam);
    }

    // Running time
    sleep(total_running_time_seconds);

    for(int i=0; i<n_philosophers; i++){
        pthread_cancel(threads[i]);
    }

    sleep(1);

    // Print Results
    for(int i=0; i<n_philosophers; i++){
        printf("Philosopher [%d] has ate [%d] times!\n", i, table.foodCount[i]);
    }
    
    return 0;
}
