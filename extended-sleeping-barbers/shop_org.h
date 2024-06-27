#ifndef SHOP_ORG_H_
#define SHOP_ORG_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultNumBarbers 1

class Shop_org 
{
public:
   Shop_org(int num_barbers, int num_chairs) : 
      max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), 
      num_barbers((num_barbers>0)?num_barbers: kDefaultNumBarbers),
      customer_in_chair_(num_barbers, -1), // Initialize in service chairs, -1 means no one is in service
      in_service_(num_barbers, false), // For multiple barbers
      money_paid_(num_barbers, false), 
      cust_drops_(0)
   { 
      init(); 
   };
   Shop_org() : 
      max_waiting_cust_(kDefaultNumChairs), 
      num_barbers(kDefaultNumBarbers),
      customer_in_chair_(num_barbers, -1), 
      in_service_(num_barbers, false), // For multiple barbers
      money_paid_(num_barbers, false), 
      cust_drops_(0)
   { 
      init(); 
   };

   ~Shop_org(){
      pthread_mutex_destroy(&mutex_);
      pthread_cond_destroy(&cond_customers_waiting_);
      pthread_cond_destroy(&cond_customer_served_);
      pthread_cond_destroy(&cond_barber_paid_);
      pthread_cond_destroy(&cond_barber_sleeping_);
   }

   bool visitShop(int); // return true only when a customer got a service
   void leaveShop(int);
   void helloCustomer(int);
   void byeCustomer(int);
   int get_cust_drops() const;

 private:
   const int max_waiting_cust_; // the max number of threads that can wait
   queue<int> waiting_chairs_;  // includes the ids of all waiting threads
   int cust_drops_; // Num of unserved customers

   // Modified for multiple barbers
   int num_barbers;
   vector<bool> in_service_; 
   vector<bool> money_paid_;  
   vector<int> customer_in_chair_;  

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t  cond_customers_waiting_;
   pthread_cond_t  cond_customer_served_;
   pthread_cond_t  cond_barber_paid_;
   pthread_cond_t  cond_barber_sleeping_;

   // For one barber:
   // static const int barber = 0; // the id of the barber thread
   void init();
   string int2string(int i);
   void print(bool isCustomer, int id, string message);
};
#endif
