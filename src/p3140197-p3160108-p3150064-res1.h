#include <pthread.h>

#define N_SEAT 10

#define N_ZONE_A 5
#define N_ZONE_B 10
#define N_ZONE_C 10

#define C_ZONE_A 30
#define C_ZONE_B 25
#define C_ZONE_C 20

#define C_SEAT 20 // TODO To be removed

#define P_ZONE_A 0.2
#define P_ZONE_B 0.4
#define P_ZONE_C 0.4

#define N_TEL 8
#define N_CASH 4

#define N_SEATLOW 1
#define N_SEATHIGH 5
#define T_SEATLOW 5
#define T_SEATHIGH 10
#define T_CASHLOW 2
#define T_CASHHIGH 4

#define P_CARDSUCCES 0.9

#define FLAG_INIT 0
#define FLAG_DESTROY 1
#define FLAG_LOCK 2
#define FLAG_UNLOCK 3

#define ZONE_A 0
#define ZONE_B 1
#define ZONE_C 2

#define SEAT_EMPTY 0
#define SEAT_OCCUPIED 3

#define SUCCESS 0
#define FAIL 1

typedef struct transaction_info{
    int transaction_no;
    int seats[N_SEATHIGH];
    int requested_seats;
    int requested_zone;
    int cost;
}TRANSACTION_INFO;

void mutex_handle(pthread_mutex_t* mutex, int flag);

void condition_handle (pthread_cond_t* cond, int flag);

int rand_prob(float percent);

void handle_operator(TRANSACTION_INFO* info, int flag);

void handle_cashier(int flag);

int request_seats(int* clientID);

int request_zone(int* clientID);

int find_seats (TRANSACTION_INFO* info);

int pay_seats(int amount);

void change_seats_state (int new_state, TRANSACTION_INFO* info);

void* transaction (void* clientID);

void arguments_check(int argc, char* argv[]);

