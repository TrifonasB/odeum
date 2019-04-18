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

#define SEAT_EMPTY 0
#define SEAT_OCCUPIED 3

#define SUCCESS 0
#define FAIL 1

int n_cust;
unsigned int seed;

int account_remain = 0;
int available_operators = N_TEL;

int sum_transactions = 0;
int sum_transaction_time = 0;
int sum_waiting_time = 0;

int theater_seats [N_SEAT] = {0};
int available_seats = N_SEAT;
int seats_threshold = 0;

pthread_mutex_t operators;
pthread_mutex_t expression_of_interest;
pthread_mutex_t payment;
pthread_mutex_t seats_availability;


typedef struct transaction_info{
    int transaction_no;
    int seats[N_SEATHIGH];
    int requested_seats;
    int cost;
}TRASACTION_INFO;

void mutex_handle(pthread_mutex_t* mutex, int flag){
    int rc;
    if (flag == FLAG_INIT) {
        rc = pthread_mutex_init(mutex, NULL);
	    if (rc != 0) {
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nPromblem at mutex init.");
		    exit(-1);
        }
	}else if (flag == FLAG_DESTROY){
        rc = pthread_mutex_destroy(&operators);
	    if (rc != 0) {
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe mutex survived.");
      	    exit(-1);
        }
    }else if (flag == FLAG_LOCK){
        rc = pthread_mutex_lock (mutex);
        if (rc != 0) {	
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe rise of the Mutex.");
            pthread_exit(&rc);
        }
    }else if (flag == FLAG_UNLOCK){
        rc = pthread_mutex_unlock(mutex);
        if (rc != 0) {	
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe fall of the Mutex.");
            pthread_exit(&rc);
        }
    }
}

int rand_prob(float percent, int min, int max){
    int r  = rand();
    if(r < RAND_MAX * percent)
        return SUCCESS;
    else
        return FAIL;
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

int request_seats(int* clientID){
    int seats;

    printf("\nCLient #%d:: Welcome to Lucas Film Theater! \nYou can pick from #%d to #%d. \nHow many seats do you want? ", *clientID, N_SEATLOW, N_SEATHIGH);
    seats = rand_r(&seed) % (N_SEATHIGH + 1);
    if(seats < N_SEATLOW)
        seats = N_SEATLOW;
    
    return seats;
}

int find_seats (TRASACTION_INFO* info){
    sleep(2); //pretend operator is looking
    int i;
    int j = 0;
    int new_threshold = seats_threshold;
    for (i = seats_threshold; i < N_SEAT; i ++){
        /*if seat == 0 (empty), mark it as occupied*/
        if(theater_seats[i] == SEAT_EMPTY){
            mutex_handle(&expression_of_interest, FLAG_LOCK);    
            if(theater_seats[i] != 0)
                continue;
            theater_seats[i] = SEAT_OCCUPIED;
            info->seats[j++] = i;
            mutex_handle(&expression_of_interest, FLAG_UNLOCK);
        /*if all the previous seats are sold, we have a new threshold*/
        }
        else if(theater_seats[i] > 0){
            if (new_threshold == i-1){
                new_threshold = i;
            }
        }
        if (j == info->requested_seats){
            break;
        }
    }
    if (j < info->requested_seats){
        /*If not enough seats are found then free those which are found*/
        mutex_handle(&expression_of_interest, FLAG_LOCK); 
        for(i = 0; i < j; i ++){
            theater_seats[info->seats[i]] = SEAT_EMPTY;
        }
        mutex_handle(&expression_of_interest, FLAG_UNLOCK);
        return FAIL;
    }
    seats_threshold = new_threshold;
    return SUCCESS;
}

int pay_seats(int amount) {
    int res = rand_prob(0.9F, 0, 1);
    if(res == SUCCESS){
        mutex_handle(&payment, FLAG_LOCK);
        account_remain += amount;
        mutex_handle(&payment, FLAG_UNLOCK);
        return SUCCESS;
    }
    return FAIL;
}

void change_seats_state (int new_state, TRASACTION_INFO* info){
    mutex_handle(&expression_of_interest, FLAG_LOCK);
    for(int i = 0; i < info->requested_seats; i++)
    {
        theater_seats[info->seats[i]] = new_state;
    }
    mutex_handle(&expression_of_interest, FLAG_LOCK);
}

void change_seats_availability (TRASACTION_INFO* info){
    mutex_handle(&seats_availability, FLAG_LOCK);
    available_seats -= info->requested_seats;
    mutex_handle(&seats_availability, FLAG_LOCK);
}

void* transaction (void* clientID){
    int res, i;
    int* tid = (int*) clientID;
    printf("\nClient #%d just called. \n", *tid);

    TRASACTION_INFO* info;

    struct timespec req_start, req_end, trans_end;
    
    clock_gettime( 0, &req_start);
    
    //Await available operator
    wait_operator(info);

    clock_gettime( 0, &req_end);

    if(available_seats == 0){
        printf("\nClient #%d:: Sorry but the theater is full...", *tid);
    }
    else{
        info->requested_seats = request_seats(tid);
        printf("\nClient #%d:: So you want #%d seats..hmm let me check..", *tid, info->requested_seats);
        
        //check seats availability in theater
        res = find_seats(info);
        if(res == SUCCESS){
            printf("\nClient #%d:: We successfully found the following seats:", *tid);
            for(i = 0; i < info->requested_seats; i++)
            {
                printf("\nClient #%d:: No. %d. ", *tid, info->seats[i]);
            }

            printf("\nClient #%d:: Transaction Cost: %d", *tid, info->cost);
            info->cost = info->requested_seats * C_SEAT;
            res = pay_seats(info->cost);
            if(res == SUCCESS){
                change_seats_state(*tid, info);

                printf("\nClient #%d:: You booked %d seats successfull! Enjoy the play!", *tid, info->requested_seats);
                printf("\nClient #%d:: Transaction ID: %d", *tid, info->transaction_no);
                printf("\nClient #%d:: Amount Paid: %d", *tid, info->cost);
                for(i = 0; i < info->requested_seats; i++)
                {
                    printf("\nClient #%d:: Booked Seat No. %d", *tid, info->seats[i]);
                }
            }
            else {
                change_seats_state(SEAT_EMPTY, info);
                printf("\nClient #%d:: Sorry, your payment wasn't successfull. Your booking is canceled", *tid);
            }
        }
        else {
            printf("\nClient #%d:: Sorry can't proceed with your booking because theater doesn't have enough seats", *tid);
        }
    }
    
    clock_gettime( 0, &trans_end);
    //update mean values
    sum_waiting_time += req_end.tv_sec - req_start.tv_sec;
    sum_transaction_time += trans_end.tv_sec - req_start.tv_sec;

    pthread_exit (clientID);
}

void arguments_check(int argc, char* argv[]){
    if (argc != 3){
        printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nGive 2 arguments.\n");
        exit (-1);
    }
    n_cust = atoi(argv[1]);
    seed = abs(atoi(argv[2]));
    if(n_cust < 0){
        printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nThe number of customers must be positive.\n");
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
    mutex_handle(&expression_of_interest, FLAG_INIT);
    mutex_handle(&payment, FLAG_INIT);
    mutex_handle(&seats_availability, FLAG_INIT);

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

    double mean_waiting_time = sum_waiting_time / n_cust;
    double mean_transaction_time = sum_transaction_time / n_cust;

    printf("\n\nSolding tickets ended!");
    printf("\nMean waiting time: %lf seconds", mean_waiting_time);
    printf("\nMean transactions time: %lf seconds", mean_transaction_time);
    printf("\n\nSeats plan:");
    for(int i = 0; i < N_SEAT; i++)
    {
        if(theater_seats[i] != SEAT_EMPTY)
            printf("\n --Seat No. %d / Client %d", i + 1, theater_seats[i]);
        else
            printf("\n --Seat No. %d / Empty", i + 1);            
    }
    

    //remove memory leaks
    free(clients);
    mutex_handle(&operators, FLAG_DESTROY);
    mutex_handle(&expression_of_interest, FLAG_DESTROY);
    mutex_handle(&payment, FLAG_DESTROY);
    mutex_handle(&seats_availability, FLAG_DESTROY);


    return 0;
}