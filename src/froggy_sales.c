#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

//#define __USE_POSIX199309 
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

int n_cust;
unsigned int seed;

int account_remain = 0;
int available_operators = N_TEL;

int sum_transactions = 0;
int sum_transaction_time = 0;
int sum_waiting_time = 0;

int theater_seats [N_SEAT] = {0};
int seats_threshold = 0;

pthread_mutex_t operators;
pthread_mutex_t expression_of_interest;


typedef struct transaction_info{
    int transaction_no;
    int* seats[N_SEATHIGH];
    int cost;
}TRASACTION_INFO;

void mutex_handle(pthread_mutex_t* mutex, int flag){
    int rc;
    if (flag == 0) {//mutex init
        rc = pthread_mutex_init(mutex, NULL);
	    if (rc != 0) {
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nPromblem at mutex init.");
		    exit(-1);
        }
	}else if (flag == 1){//mutex destroy
        rc = pthread_mutex_destroy(&operators);
	    if (rc != 0) {
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe mutex survived.");
      	    exit(-1);
        }
    }else if (flag == 2){//mutex lock
        rc = pthread_mutex_lock (mutex);
        if (rc != 0) {	
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe rise of the Mutex.");
            pthread_exit(&rc);
        }
    }else{//mutex unlock
        rc = pthread_mutex_unlock(mutex);
        if (rc != 0) {	
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe fall of the Mutex.");
            pthread_exit(&rc);
        }
    }
} 

void wait_operator(TRASACTION_INFO* info) {
    mutex_handle(&operators, FLAG_LOCK);
    while(1){
        if(available_operators){
            available_operators--;
            info->transaction_no = ++sum_transactions;
            break;
        }else{
            sleep(1);
        }
    }
    mutex_handle(&operators, FLAG_UNLOCK);
}

int request_seats_from_client(){
    int seats;

    printf("Welcome to Lucas Film Theater! \nYou can pick from #%d to #%d. \nHow many seats do you want? ", N_SEATLOW, N_SEATHIGH);
    seats = rand_r(&seed) % (N_SEATHIGH + 1);
    if(seats < N_SEATLOW)
        seats = N_SEATLOW;
    
    return seats;
}

void find_seats (int number_of_seats, TRASACTION_INFO* info){
    int i;
    int j = 0;
    int new_threshold = seats_threshold;
    for (i = seats_threshold; i < N_SEAT; i ++){
        /*if seat == 0 (empty), mark it as occupied*/
        if(!theater_seats[i]){
            mutex_handle(&expression_of_interest, FLAG_LOCK);    
            theater_seats[i] = -1;      // -1 = occupied
            info->seats[j++] = i;
            mutex_handle(&expression_of_interest, FLAG_UNLOCK);
        /*if all the previous seats are sold, we have a new threshold*/
        }else if(theater_seats[i] > 0){
            if (new_threshold == i-1){
                new_threshold = i;
            }
        if (j == number_of_seats){
            break;
        }
    }
    if (j < number_of_seats){
        /* not enough seats available */
    }
    seats_threshold = new_threshold;   
    //should we dump the array?
}

void* transaction (void* clientID){
    int* tid = (int*) clientID;
    printf("Client #%d just called. \n", *tid);

    TRASACTION_INFO* info;

    struct timespec req_start, req_end;
    clock_gettime( CLOCK_REALTIME, &req_start);
    clock_gettime( 0, &req_start);
    
    //Await available operator
    wait_operator(info);
    int requested_seats = request_seats_from_client();
    printf("So you want #%d seats..hmm let me check..", requested_seats);
    
    //check seats availability in theater
    find_seats(requested_seats, info);
    //pay
    //update mean values    

    pthread_exit (clientID);
}

void arguments_check(int argc, char* argv[]){
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
}

int main (int argc, char* argv[]){
     
    arguments_check(argc, argv);

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
    mutex_handle(&operators, FLAG_INIT);
    mutex_handle(&theater, FLAG_INIT);

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


    return 0;
}