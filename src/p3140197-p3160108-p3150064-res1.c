#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "p3140197-p3160108-p3150064-res1.h"

int n_cust;
unsigned int seed;

int account_remain = 0;
int available_cashiers = N_CASH;
int available_operators = N_TEL;

int sum_transactions = 0;
int sum_transaction_time = 0;
int sum_waiting_time = 0;

int theater_seats [N_SEAT] = {0};
int available_seats = N_SEAT;
int a_threshold = 0;
int b_threshold = 0;
int c_threshold = 0;

pthread_mutex_t operators;
pthread_mutex_t expression_of_interest;
pthread_mutex_t payment;
pthread_mutex_t cashiers;

pthread_cond_t condition_op;
pthread_cond_t condition_cash;

void mutex_handle(pthread_mutex_t* mutex, int flag){
    int rc;
    if (flag == FLAG_INIT) {
        rc = pthread_mutex_init(mutex, NULL);
	    if (rc != 0) {
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nPromblem at mutex init.");
		    exit(-1);
        }
	}else if (flag == FLAG_DESTROY){
        rc = pthread_mutex_destroy(mutex);
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

void condition_handle (pthread_cond_t* cond, int flag){

    int rc;
    if (flag == FLAG_INIT){
        rc = pthread_cond_init(cond, NULL);
        if (rc != 0) {
            printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
            exit(-1);
	    }
    }else if (flag == FLAG_DESTROY){
        rc = pthread_cond_destroy(cond);
	    if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
       		exit(-1);
	    }
    }

}

int rand_prob(float percent){
    int r  = rand();
    if(r < RAND_MAX * percent)
        return SUCCESS;
    else
        return FAIL;
}

void handle_operator(TRANSACTION_INFO* info, int flag) {
    if(flag == FLAG_LOCK){
        mutex_handle(&operators, FLAG_LOCK);
        while(1){
            if(available_operators){
                available_operators--;
                (*info).transaction_no = sum_transactions++;
                break;
            }else{
                pthread_cond_wait(&condition_op, &operators);
            }
        }
        mutex_handle(&operators, FLAG_UNLOCK);
    }
    else if (flag == FLAG_UNLOCK) {
        mutex_handle(&operators, FLAG_LOCK);
        available_operators++;
        mutex_handle(&operators, FLAG_UNLOCK);
        pthread_cond_broadcast(&condition_op);
    }
    
}

void handle_cashier(int flag) {
    if(flag == FLAG_LOCK){
        mutex_handle(&cashiers, FLAG_LOCK);
        while(1){
            if(available_cashiers){
                available_cashiers--;
                break;
            }else{
                pthread_cond_wait(&condition_cash, &cashiers);
            }
        }
        mutex_handle(&cashiers, FLAG_UNLOCK);
    }
    else if (flag == FLAG_UNLOCK) {
        mutex_handle(&cashiers, FLAG_LOCK);
        available_cashiers++;
        mutex_handle(&cashiers, FLAG_UNLOCK);
        pthread_cond_broadcast(&condition_cash);
    }
}

int request_seats(int* clientID){
    int seats;
    printf("\nCLient #%d:: Welcome to Lucas Film Theater! \nYou can pick from #%d to #%d. \nHow many seats do you want? ", clientID, N_SEATLOW, N_SEATHIGH);
    seats = rand_r(&seed) % (N_SEATHIGH + 1);
    if(seats < N_SEATLOW)
        seats = N_SEATLOW;
    
    return seats;
}

int request_zone(int* clientID){
    int prob;
    printf("\nCLient #%d:: Which zone would you like your seats in A,B or C? ", clientID);
    prob = rand_r(&seed) % (100);
    if(prob < P_ZONE_A * 100){
        printf("\nClient #%d: SO it's Zone A. Great pick!", clientID);
        return ZONE_A;
    }
    else if(prob < (P_ZONE_A + P_ZONE_B) * 100){
        printf("\nClient #%d: SO it's Zone B. OK pick!", clientID);
        return ZONE_B;
    }
    else{
        printf("\nClient #%d: SO it's Zone C. Thanks!", clientID);
        return ZONE_C;
    }
}

int find_seats (TRANSACTION_INFO* info){
    int waiting = rand_r(&seed) % (T_SEATHIGH + 1);
    if(waiting < T_SEATLOW)
        waiting = T_SEATLOW;
    
    sleep(waiting); //pretend operator is looking

    int i, i_in, start, end, j = 0;

    if(info->requested_zone == ZONE_A)
    {
        start = 0; 
        end = N_ZONE_A * N_SEAT;
    }
    else if(info->requested_zone == ZONE_B)
    {
        start = N_ZONE_A * N_SEAT; 
        end = (N_ZONE_A + N_ZONE_B) * N_SEAT;
    }
    else {
        start = (N_ZONE_A + N_ZONE_B) * N_SEAT; 
        end = (N_ZONE_A + N_ZONE_B + N_ZONE_C) * N_SEAT;
    }

    for (i = start; i < end - info->requested_seats + 1; i = i_in + 1)
    {
        mutex_handle(&expression_of_interest, FLAG_LOCK);
        if(theater_seats[i] == SEAT_EMPTY) {
            j = 0;
            for (i_in = i; i_in < i + info->requested_seats; i_in++)
            {
                if(theater_seats[i_in] != SEAT_EMPTY)
                    break;
                theater_seats[i_in] = SEAT_OCCUPIED;
                info->seats[j++] = i_in;
            }
            if(j!= info->requested_seats){
                for (i_in = i; i_in < i + j - 1; i_in++)
                {
                    theater_seats[i_in] = SEAT_EMPTY;
                }
            }
        }
        mutex_handle(&expression_of_interest, FLAG_UNLOCK);
        if(j== info->requested_seats)
            break;
    }
    
    if (j < info->requested_seats)
        return FAIL;
    else
        return SUCCESS;

    //for (i = seats_threshold; i < N_SEAT; i ++){

        /*if seat == 0 (empty), mark it as occupied*/
        // if(theater_seats[i] == SEAT_EMPTY){
        //     mutex_handle(&expression_of_interest, FLAG_LOCK);    
        //     if(theater_seats[i] != 0)
        //         continue;
        //     theater_seats[i] = SEAT_OCCUPIED;
        //     info->seats[j++] = i;
        //     mutex_handle(&expression_of_interest, FLAG_UNLOCK);
        // /*if all the previous seats are sold, we have a new threshold*/
        // }
        // else if(theater_seats[i] > 0){
        //     if (new_threshold == i-1){
        //         new_threshold = i;
        //     }
        // }
        // if (j == info->requested_seats){
        //     break;
        // }
    //}
    
}

int pay_seats(int amount) {
    int res = rand_prob(0.9F);
    if(res == SUCCESS){
        mutex_handle(&payment, FLAG_LOCK);
        account_remain += amount;
        mutex_handle(&payment, FLAG_UNLOCK);
        return SUCCESS;
    }
    return FAIL;
}

void change_seats_state (int new_state, TRANSACTION_INFO* info){
    mutex_handle(&expression_of_interest, FLAG_LOCK);
    for(int i = 0; i < info->requested_seats; i++)
    {
        theater_seats[info->seats[i]] = new_state;
    }
    if(new_state != SEAT_EMPTY){
        available_seats -= info->requested_seats;
    }
    
    mutex_handle(&expression_of_interest, FLAG_UNLOCK);
}

int handle_seats(int* clientID, TRANSACTION_INFO* info) {
    info->requested_seats = request_seats(clientID);
    info->requested_zone = request_zone(clientID);
    printf("\nClient #%d:: So you want #%d seats..hmm let me check..", clientID, info->requested_seats);
    
    //check seats availability in theater
    int res = find_seats(info);
    if(res == SUCCESS){
        printf("\nClient #%d:: We successfully found the following seats:", clientID);
        for(int i = 0; i < info->requested_seats; i++)
        {
            printf("\nClient #%d:: No. %d. ", clientID, info->seats[i]);
        }
        printf("\nClient #%d::Done", clientID);

        printf("\nClient #%d:: I'm now transfering you to mr Varoufakis department.", clientID);
        fflush(stdout);
    }
    else {
        printf("\nClient #%d:: Sorry can't proceed with your booking because theater doesn't have enough seats", clientID);
        return FAIL;
    }

    return SUCCESS;
}

void cashier_transaction(int* clientID, TRANSACTION_INFO* info) {

    handle_cashier(FLAG_LOCK);

    if(info->requested_zone == ZONE_A){
        info->cost = info->requested_seats * C_ZONE_A;
    }
    else if(info->requested_zone == ZONE_B){
        info->cost = info->requested_seats * C_ZONE_B;
    }
    else{
        info->cost = info->requested_seats * C_ZONE_C;
    }

    printf("\nClient #%d:: Transaction Cost: %d", clientID, info->cost);
    fflush(stdout);
    int res = pay_seats(info->cost);

    if(res == SUCCESS){
        change_seats_state(clientID, info);

        printf("\nClient #%d:: You booked %d seats successfull! Enjoy the play!", clientID, info->requested_seats);
        printf("\nClient #%d:: Transaction ID: %d", clientID, info->transaction_no);
        printf("\nClient #%d:: Amount Paid: %d", clientID, info->cost);
        for(int i = 0; i < info->requested_seats; i++)
        {
            printf("\nClient #%d:: Booked Seat No. %d", clientID, info->seats[i]);
        }
    }
    else {
        change_seats_state(SEAT_EMPTY, info);
        printf("\nClient #%d:: Sorry, your payment wasn't successfull. Your booking is canceled", clientID);
    }
    handle_cashier(FLAG_UNLOCK);
}

void* transaction (void* clientID){
    int res, i;
    int* tid = (int*) clientID;
    TRANSACTION_INFO info;
    struct timespec req_start, req_end, trans_end;

    printf("\nClient #%d just called. \n", *tid);

    clock_gettime(0, &req_start);

    handle_operator(&info, FLAG_LOCK);

    clock_gettime( 0, &req_end);

    res = FAIL;

    if(available_seats == 0){
        printf("\nClient #%d:: Sorry but the theater is full...", *tid);
    }
    else {
        res = handle_seats(*tid, &info);
    }

    handle_operator(&info, FLAG_UNLOCK);

    if(res == SUCCESS){
        cashier_transaction(*tid, &info);
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
    mutex_handle(&cashiers, FLAG_INIT);
    condition_handle(&condition_op, FLAG_INIT);
    condition_handle(&condition_cash, FLAG_INIT);

    //thread loop
    int i;
    for (i = 0; i < n_cust; i++){
        clientCount[i] = i + 1; 
        rc = pthread_create(&clients[i], NULL, transaction, &clientCount[i]);
        if (rc != 0){
            printf("E R R O R ! ! !\nThe core feels that darkness is strong in you...\nPromblem at thread creation.");
            exit(-1);
        }
        //sleep(1);
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

    printf("\n\nSolding tickets ended! Our revenue is %d$!", account_remain);
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
    printf("\n");
    

    //remove memory leaks
    free(clients);
    mutex_handle(&operators, FLAG_DESTROY);
    mutex_handle(&expression_of_interest, FLAG_DESTROY);
    mutex_handle(&payment, FLAG_DESTROY);
    mutex_handle(&cashiers, FLAG_DESTROY);
    condition_handle(&condition_op, FLAG_DESTROY);
    condition_handle(&condition_cash, FLAG_DESTROY);

    return 0;
}