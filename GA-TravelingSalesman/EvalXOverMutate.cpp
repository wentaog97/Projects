#include <iostream>  // cout
#include <stdlib.h>  // rand
#include <math.h>    // sqrt, pow
#include <omp.h>     // OpenMP
#include <string.h>  // memset
#include <algorithm>
#include "Timer.h"
#include "Trip.h"

// DEBUG
#define TEST_CHROMOSOMES_N 12
#define TEST_TOP_X         6
// DEBUG

#define CHROMOSOMES    50000 // 50000 different trips
#define CITIES         36    // 36 cities = ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
#define TOP_X          25000 // top optimal 25%
#define MUTATE_RATE    50    // optimal 50%

using namespace std;

// HELPERS
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
  char city = rand( ) % CITIES;
  if ( city < 26 )
    city += 'A';
  else
    city = city - 26 + '0';
  return city;
}

float getDistance(char a, char b, int coordinates[CITIES][2]){
   int ia = getIndex(a);
   int ib = getIndex(b);
   return sqrt(pow(coordinates[ia][0] - coordinates[ib][0], 2) + pow(coordinates[ia][1] - coordinates[ib][1], 2));
}

void getComplement(const char* ori, char* tar){
   string compliment = "9876543210ZYXWVUTSRQPONMLKJIHGFEDCBA";
   for(int i=0; i<CITIES; i++){
      char c = ori[i];
      int index = ( c >= 'A' ) ? c - 'A' : c - '0' + 26;
      tar[i]=compliment[index];
   }
}
// HELPERS


/*
 * Evaluates each trip (or chromosome) and sort them out

   Load in the chromosomes, coordinates
   we need to findout the value of the chromosomes

   Evaluates the distance of each trip and sorts out all the trips in the shortest-first
   order. Memorize the current shortest trip as a tentative answer if it is shorter the
   previous.
 */
void evaluate( Trip trip[CHROMOSOMES], int coordinates[CITIES][2] ) {
   // cout << "Evaluating" << endl;

   // Iterating through all trips
   #pragma omp parallel
   {
      #pragma omp for
      for(int i_c = 0; i_c < CHROMOSOMES; i_c++){
         char prevCity = trip[i_c].itinerary[0];
         double distance = 0;

         // Start with the second city, and calculate its distance from the prev
         for(int i=1; i<CITIES; i++){
            char city = trip[i_c].itinerary[i];
            float d = getDistance(city, prevCity, coordinates);
            distance += d;
            prevCity = city;
         }

         // After reaching the last city, calculate the last to the first city.
         char city = trip[i_c].itinerary[0];
         float d = getDistance(city, prevCity, coordinates);
         distance += d;
         
         // Assign the fitness of this trip with the total calculated distance
         trip[i_c].fitness = distance;
      }  
   }

   // Sort all trips based on distance using a custome comparator
   std::sort(trip, trip+CHROMOSOMES, CompareTrips);
   
   // DEBUG
   if(DEBUG){
      for(int i=0; i<CHROMOSOMES; i++){
         for(int index=0; index<CITIES; index++){
            cout << trip[i].itinerary[index];
         }
         cout << endl;
         cout << trip[i].fitness << endl;
      }
   }
   // DEBUG
}

/*
 * Generates new TOP_X offsprings from TOP_X parents.
 * Noe that the i-th and (i+1)-th offsprings are created from the i-th and (i+1)-th parents
 */

 /*
    Generates 25,000 off-springs from the parents. More specifically, we spawn a pair 
    of child[i] and [i+1] from parent[i] and [i+1] 
  */

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

void crossover( Trip parents[TOP_X], Trip offsprings[TOP_X], int coordinates[CITIES][2] ) {
   // cout << "Crossing over" << endl;
   #pragma omp parallel
   {
      #pragma omp for
      for(int i=0; i<TOP_X; i+=2){
         Trip p1 = parents[i];
         Trip p2 = parents[i+1];

         // Hash set for visited cities
         bool visited[36] = {false};
         
         offsprings[i].itinerary[0] = p1.itinerary[0];
         visited[getIndex(offsprings[i].itinerary[0])] = true;
         
         // Greedy cross over
         for(int j=1; j<36; j++){
            char startingCity = offsprings[i].itinerary[j-1];

            char p1Next = getNextCity(p1.itinerary, startingCity);
            char p2Next = getNextCity(p2.itinerary, startingCity);

            float d1 = getDistance(startingCity, p1Next, coordinates);
            float d2 = getDistance(startingCity, p2Next, coordinates);

            if(min(d1,d2) == d1){
               if(!visited[getIndex(p1Next)]) {
                  offsprings[i].itinerary[j] = p1Next;
                  visited[getIndex(p1Next)] = true;
               } else if(!visited[getIndex(p2Next)]){
                  offsprings[i].itinerary[j] = p2Next;
                  visited[getIndex(p2Next)] = true;
               } else {
                  char randomCity = getRandomCity();
                  while(visited[getIndex(randomCity)]){
                     randomCity = getRandomCity();
                  }
                  offsprings[i].itinerary[j] = randomCity;
                  visited[getIndex(randomCity)] = true;
               }
            } else {
               if(!visited[getIndex(p2Next)]) {
                  offsprings[i].itinerary[j] = p2Next;
                  visited[getIndex(p2Next)] = true;
               } else if(!visited[getIndex(p1Next)]){
                  offsprings[i].itinerary[j] = p1Next;
                  visited[getIndex(p1Next)] = true;
               } else {
                  char randomCity = getRandomCity();
                  while(visited[getIndex(randomCity)]){
                     randomCity = getRandomCity();
                  }
                  offsprings[i].itinerary[j] = randomCity;
                  visited[getIndex(randomCity)] = true;
               }
            }
         }

         getComplement(offsprings[i].itinerary, offsprings[i+1].itinerary);      
      }
   }

}

/*
 * Mutate a pair of genes in each offspring.
 */

 /*
    Randomly chooses two distinct cities (or genes) in each trip (or chromosome) with a
    given probability, and swaps them.
 */

void swap(char * arr, int a, int b){
   char temp = arr[a];
   arr[a] = arr[b];
   arr[b] = temp;
}

void mutate( Trip offsprings[TOP_X] ) {
   // cout << "Mutating" << endl;
   #pragma omp parallel
   {
      #pragma omp for
      for(int i=0; i<TOP_X; i++){
         int a=0, b=0;
         a = rand()%CITIES;
         b = rand()%CITIES;
         while(b==a) b=rand()%CITIES;
         int prob = rand()%100;
         if(prob > MUTATE_RATE) swap(offsprings[i].itinerary, a, b);
      }
   }
}

