#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N_SEAT 250
#define N_TEL 8
#define N_SEATLOW 1
#define N_SEATHIGH 5
#define T_SEATLOW 5
#define T_SEATHIGH 10
#define P_CARDSUCCES 0.9
#define C_SEAT 20

#define FLAG_ON 1
#define FLAG_OFF 0

int n_cust;
unsigned int seed;

int account_remain = 0;
int available_operators = N_TEL;

int sum_transactions = 0;
int sum_transaction_time = 0;
int sum_waiting_time = 0;

int theater_seats [N_SEAT] = {0};

pthread_mutex_t operators;
pthread_mutex_t theater;


typedef struct transaction_info{
    int transaction_no;
    int* seats[N_SEATHIGH];
    int cost;
}TRASACTION_INFO;

void mutex_init(pthread_mutex_t* mutex){
    int rc;
    rc = pthread_mutex_init(mutex, NULL);
	if (rc != 0) {	
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nPromblem at mutex init.");
		exit(-1);
	}
}

void mutex_handle(pthread_mutex_t* mutex, int flag){
    int rc;
    if(flag){
        //mutex lock
        rc = pthread_mutex_lock (mutex);
        if (rc != 0) {	
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe rise of the Mutex.");
            pthread_exit(&rc);
        }
    }
    else{
        //mutex unlock
        rc = pthread_mutex_unlock(mutex);
        if (rc != 0) {	
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe fall of the Mutex.");
            pthread_exit(&rc);
        }
    }
} 

void wait_operator(TRASACTION_INFO* info) {
    mutex_handle(&operators, FLAG_ON);
    while(1){
        if(available_operators){
            available_operators--;
            info->transaction_no = ++sum_transactions;
            break;
        }else{
            sleep(1);
        }
    }
    mutex_handle(&operators, FLAG_OFF);
}

int request_seats_from_client(){
    int seats;

    printf("Welcome to Lucas Film Theater! \nYou can pick from #%d to #%d. \nHow many seats do you want? ", N_SEATLOW, N_SEATHIGH);
    seats = rand_r(&seed) % (N_SEATHIGH + 1);
    if(seats < N_SEATLOW)
        seats = N_SEATLOW;
    
    return seats;
}

void* transaction (void* clientID){
    int* tid = (int*) clientID;
    printf("Client #%d just called. \n", *tid);

    TRASACTION_INFO* info;
    struct timespec req_start, req_end;

    //init clock
    //clock_gettime(CLOCK_REALTIME, &req_start);
    
    //Await available operator
    wait_operator(info);
    int seats;
    seats = request_seats_from_client();

    printf("So you want #%d seats..hmm let me check..", seats);
    
    //check seats availability in theater
    //pay
    //update mean values    

    pthread_exit (clientID);
}


int main (int argc, char* argv[]){
    
    if (argc != 3){
        printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nGive 2 arguments.");
        exit (-1);
    }
    n_cust = atoi(argv[1]);
    seed = abs(atoi(argv[2]));
    if(n_cust < 0){
        printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe number of customers must be positive.");
        exit (-1);
    }

    //thread memory
    int rc;
    pthread_t* clients;
    int clientCount[n_cust];

    clients = malloc(n_cust*sizeof(pthread_t));
    if (clients == NULL){
        printf("E R R O R ! ! !\nOut of memory\n");
        return -1;
    }

    //mutex initialization
    mutex_init(&operators);
    mutex_init(&theater);

    //thread loop
    int i;
    for (i = 0; i < n_cust; i++){
        clientCount[i] = i + 1;
        //printf("Client #%d just called. \n", clientCount[i]);
        rc = pthread_create(&clients[i], NULL, transaction, &clientCount[i]);
        if (rc != 0){
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nPromblem at thread creation.");
            exit(-1);
        }
    }

    //thread join
    void* status;
    for(i = 0; i < n_cust; i ++){
        rc = pthread_join(clients[i], &status);
        if (rc != 0){
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nPromblem at thread join.");
            exit(-1);
        }
    }

    rc = pthread_mutex_destroy(&operators);
	if (rc != 0) {
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe mutex survived.");
      		exit(-1);
	}

    return 0;
}