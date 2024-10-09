#include <iostream>  // cout
#include <stdlib.h>  // rand
#include <math.h>    // sqrt, pow
#include <omp.h>     // OpenMP
#include <string.h>  // memset
#include <algorithm>
#include "Timer.h"
#include "Trip.h"

#define CHROMOSOMES    50000 // 50000 different trips
#define CITIES         36    // 36 cities = ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
#define TOP_X          25000 // top optimal 25%
#define MUTATE_RATE    52    // optimal 50%

using namespace std;

// HELPERS FUNCTIONS
bool CompareTrips(const Trip& a, const Trip& b) {
   return a.fitness < b.fitness; 
}

int getIndex(char c){
   return ( c >= 'A' ) ? c - 'A' : c - '0' + 26;
}

char getCityCh(int index){
   if(index >= 0 && index <= 26) return index+'A';
   return index - 26 + '0';
}

char getRandomCity() {
   char city = rand() % CITIES;
   if ( city < 26 ) return city + 'A';
   return city - 26 + '0';
}

float getDistance(char a, char b, int coordinates[CITIES][2]){
   int ia = getIndex(a);
   int ib = getIndex(b);
   return sqrt(pow(coordinates[ia][0] - coordinates[ib][0], 2) + pow(coordinates[ia][1] - coordinates[ib][1], 2));
}

float getOriDistance(char b, int coordinates[CITIES][2]){
   int ib = getIndex(b);
   return sqrt(pow(0 - coordinates[ib][0], 2) + pow(0 - coordinates[ib][1], 2));
}

void getComplement(const char* ori, char* tar){
   string compliment = "9876543210ZYXWVUTSRQPONMLKJIHGFEDCBA";
   for(int i=0; i<CITIES; i++){
      char c = ori[i];
      int index = ( c >= 'A' ) ? c - 'A' : c - '0' + 26;
      tar[i]=compliment[index];
   }
}

char getNextCity(const char* chromosome, char c){
   int index = -1;
   for(int i=0; i<36; i++){
      if(chromosome[i]==c) {
         index = i; break;
      }
   }
   if(index == 35) return chromosome[0];
   return chromosome[index+1];
}

char getValidCity(char p1Next, char p2Next, bool* visited) {
   if (!visited[getIndex(p1Next)]) {
      visited[getIndex(p1Next)] = true;
      return p1Next;
   }

   if (!visited[getIndex(p2Next)]) {
      visited[getIndex(p2Next)] = true;
      return p2Next;
   }

   char randomCity = getRandomCity();
   while (visited[getIndex(randomCity)]) randomCity = getRandomCity(); 
   visited[getIndex(randomCity)] = true;
   return randomCity;
}

void swap(char * arr, int a, int b){
   char temp = arr[a];
   arr[a] = arr[b];
   arr[b] = temp;
}


/*
 * Evaluates each trip (or chromosome) and sort them out
 */
void evaluate( Trip trip[CHROMOSOMES], int coordinates[CITIES][2] ) {
   // Precompute distances between cities
   float distanceMatrix[CITIES][CITIES];
   #pragma omp parallel for collapse(2)
   for (int i = 0; i < CITIES; i++) {
      for (int j = 0; j < CITIES; j++) {
         distanceMatrix[i][j] = sqrt(pow(coordinates[i][0] - coordinates[j][0], 2) + pow(coordinates[i][1] - coordinates[j][1], 2));
      }
   }

   // Iterating through all trips
   #pragma omp parallel for
   for(int i_c = 0; i_c < CHROMOSOMES; i_c++){
      char prevCity = trip[i_c].itinerary[0];
      double distance = getOriDistance(prevCity, coordinates);
      int prevCityIndex = getIndex(trip[i_c].itinerary[0]);

      // Start with the second city, and calculate its distance from the prev
      for (int i = 1; i < CITIES; i++) {
         int currentCityIndex = getIndex(trip[i_c].itinerary[i]);
         distance += distanceMatrix[prevCityIndex][currentCityIndex];
         prevCityIndex = currentCityIndex;
      }
      
      // Assign the fitness of this trip with the total calculated distance
      trip[i_c].fitness = distance;
   }  

   // Sort all trips based on distance using a custome comparator
   std::sort(trip, trip+CHROMOSOMES, CompareTrips);
}

/*
 * Generates new TOP_X offsprings from TOP_X parents.
 * Noe that the i-th and (i+1)-th offsprings are created from the i-th and (i+1)-th parents
 */
void crossover( Trip parents[TOP_X], Trip offsprings[TOP_X], int coordinates[CITIES][2] ) {
   
   // Precompute distances between cities
   float distanceMatrix[CITIES][CITIES];
   #pragma omp parallel for collapse(2)
   for (int i = 0; i < CITIES; i++) {
      for (int j = 0; j < CITIES; j++) {
         distanceMatrix[i][j] = sqrt(pow(coordinates[i][0] - coordinates[j][0], 2) + pow(coordinates[i][1] - coordinates[j][1], 2));
      }
   }

   #pragma omp parallel
   {
      // Generate a different seed for each thread
      srand(time(NULL) + omp_get_thread_num());

      #pragma omp for
      for(int i=0; i<TOP_X; i+=2){
         // Hash set for visited cities
         bool visited[36] = {false};
         
         offsprings[i].itinerary[0] = parents[i].itinerary[0];
         visited[getIndex(offsprings[i].itinerary[0])] = true;
         
         // Greedy cross over
         for(int j=1; j<36; j++){
            char startingCity = offsprings[i].itinerary[j-1];

            char p1Next = getNextCity(parents[i].itinerary, startingCity);
            char p2Next = getNextCity(parents[i+1].itinerary, startingCity);

            float d1 = distanceMatrix[getIndex(startingCity)][getIndex(p1Next)];
            float d2 = distanceMatrix[getIndex(startingCity)][getIndex(p2Next)];

            if (d1 <= d2) {
               offsprings[i].itinerary[j] = getValidCity(p1Next, p2Next, visited);
            } else {
               offsprings[i].itinerary[j] = getValidCity(p2Next, p1Next, visited);
            }
         }

         // Generate complement 
         getComplement(offsprings[i].itinerary, offsprings[i+1].itinerary);      
      }
   }
}

/*
 * Mutate a pair of genes in each offspring.
 */
void mutate( Trip offsprings[TOP_X] ) {
   #pragma omp parallel
   {  
      // Generate a different seed for each thread
      srand(time(NULL) + omp_get_thread_num());

      #pragma omp for 
      for (int i = 0; i < TOP_X; i++) {
         int prob = rand() % 100;
         if (prob < MUTATE_RATE){
            int a = rand() % CITIES;  
            int b = rand() % CITIES;
            while (b == a) b = rand() % CITIES; 
            swap(offsprings[i].itinerary, a, b);  
         }
      }
   }
}

