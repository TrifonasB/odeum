#include <pthread.h>

#define N_SEAT 250
#define N_TEL 8
#define N_SEATLOW 1
#define N_SEATHIGH 5
#define T_SEATLOW 5
#define T_SEATHIGH 10
#define P_CARDSUCCES 0.9
#define C_SEAT 20

#define FLAG_INIT 0
#define FLAG_DESTROY 1
#define FLAG_LOCK 2
#define FLAG_UNLOCK 3

#define SEAT_EMPTY 0
#define SEAT_OCCUPIED 3

#define SUCCESS 0
#define FAIL 1

typedef struct transaction_info{
    int transaction_no;
    int seats[N_SEATHIGH];
    int requested_seats;
    int cost;
}TRANSACTION_INFO;

void mutex_handle(pthread_mutex_t* mutex, int flag);

void condition_handle (pthread_cond_t* cond, int flag);

int rand_prob(float percent, int min, int max);

void wait_operator(TRANSACTION_INFO* info);

int request_seats(int* clientID);

int find_seats (TRANSACTION_INFO* info);

int pay_seats(int amount);

void change_seats_state (int new_state, TRANSACTION_INFO* info);

void change_seats_availability (TRANSACTION_INFO* info);

void* transaction (void* clientID);

void arguments_check(int argc, char* argv[]);

