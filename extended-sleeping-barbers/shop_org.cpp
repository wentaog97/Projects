#include "Shop_org.h"

void Shop_org::init() {
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   pthread_cond_init(&cond_customer_served_, NULL);
   pthread_cond_init(&cond_barber_paid_, NULL);
   pthread_cond_init(&cond_barber_sleeping_, NULL);
}

string Shop_org::int2string(int i) {
   stringstream out;
   out << i;
   return out.str( );
}

void Shop_org::print(bool isCustomer, int id, string message) {
   cout << (isCustomer ? "customer[" : "barber  [" ) << id << "]: " << message << endl;
}

int Shop_org::get_cust_drops() const {
   return cust_drops_;
}

bool Shop_org::visitShop(int customerID) {
   pthread_mutex_lock(&mutex_);

   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) 
   {
      print(true, customerID, "leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return false;
   }

   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service

   // Loop through to check if there are any sleeping barbers
   int SleepingBarberID = -1;
   for(int i = 0; i < num_barbers; i++){
      if(customer_in_chair_[i]==-1) { // If no customer in chair
         SleepingBarberID = i;
         break;
      }
   }
   
   // If non of the barber are asleep, add customer to the queue
   if (SleepingBarberID == -1) 
   {
      waiting_chairs_.push(customerID);
      print(true, customerID, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      for(int i = 0; i < num_barbers; i++){
         if(customer_in_chair_[i]==customerID) { // If find the barber who would take the customer
            SleepingBarberID = i;
            break;
         }
      }
   }

   print(true, customerID, "moves to the service chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_[SleepingBarberID] = customerID;
   in_service_[SleepingBarberID] = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_broadcast(&cond_barber_sleeping_);

   pthread_mutex_unlock(&mutex_); 
   return true;
}

void Shop_org::leaveShop(int customerID) {
   pthread_mutex_lock( &mutex_ );

   // Wait for service to be completed
   print(true, customerID, "wait for the hair-cut to be done");

   int barberID = -1;
   for(int i=0; i<customer_in_chair_.size(); i++){
      if(customer_in_chair_[i]==customerID){
         barberID = i;
         break;
      }
   }

   while (in_service_[barberID] == true)
   {
      pthread_cond_wait(&cond_customer_served_, &mutex_);
   }

   // Pay the barber and signal barber appropriately
   money_paid_[barberID] = true;
   pthread_cond_broadcast(&cond_barber_paid_);
   print(true, customerID, "says good-bye to the barber." );
   pthread_mutex_unlock(&mutex_);
}

void Shop_org::helloCustomer(int barberID) {
   pthread_mutex_lock(&mutex_);
   // If no customers then the barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_[barberID]==-1 ) 
   {
      print(false, barberID, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   // check if the customer, sit down.
   while (customer_in_chair_[barberID]==-1)
   {
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   print(false, barberID, "starts a hair-cut service for " + int2string( customer_in_chair_[barberID] ) );
   
   pthread_mutex_unlock( &mutex_ );
}

void Shop_org::byeCustomer(int barberID) {
   pthread_mutex_lock(&mutex_);

   // Hair Cut-Service is done so signal customer and wait for payment
   in_service_[barberID] = false;
   print(false, barberID, "says he's done with a hair-cut service for " + int2string(customer_in_chair_[barberID]));
   money_paid_[barberID] = false;
   pthread_cond_broadcast(&cond_customer_served_);
   while (money_paid_[barberID] == false)
   {
      pthread_cond_wait(&cond_barber_paid_, &mutex_);
   }

   //Signal to customer to get next one
   customer_in_chair_[barberID] = -1;
   print(false, barberID, "calls in another customer");
   pthread_cond_signal( &cond_customers_waiting_ );
   
   // Takes the first one waiting
   if(!waiting_chairs_.empty()) {
      customer_in_chair_[barberID]=waiting_chairs_.front(); 
      waiting_chairs_.pop(); // Pops the first one in line to prevent others accessing
   }

   pthread_mutex_unlock( &mutex_ );  // unlock
}
