#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

// PART 2 SANITY CHECK

//Fibonacci sequence
int fib(int n) {
    if (n <= 1) {
        return 1;
    }
    return fib(n-1) + fib(n-2);
}
//Use this for long computation
//fib(39) takes roughly 30 secs
void longComputation() {
    // fib(39);
    fib(39);
}

int main(int argc, char *argv[]) {
    int pid;                                        // Process ID
    int ctime, ttime, retime, rutime, stime;        // initialize times for: creation, termination, ready, run, sleep
    int ready = 0;                                  // ready time total 
    int run = 0;                                    // run time total
    int sleep = 0;                                  // sleep time total
    int turnAround = 0;                             // turnAround = sleep + run +ready
    int numChildren = 20;                           // num of children processes, default val = 20

    for (int i = 0; i < numChildren; i++) {         // for loop to create child processes
        pid = fork();                               // fork execution
        if (pid == 0) {                             // CHILD PROCESS
            longComputation();                      // long computation action
            exit();                                 // exiting
        }           
        continue;                                   // continue
    }

    for (int i = 0; i < numChildren; i++) {         // for each child process
        // using new PART 1 wait stat function
        pid = wait_stat(&ctime, &ttime, &retime, &rutime, &stime);
        sleep += stime;                             // keep track of total sleep time
        ready += retime;                            // keep track of total ready time
        run += rutime;                              // keep track of total run time
        turnAround = (turnAround + retime + rutime + stime);    // keep track of total turnAround time

        printf(1,"PID[%d] Creation[%d] Termination[%d] Ready Time[%d] Running Time[%d] Sleep Time[%d]\n",
        pid, ctime, ttime, retime, rutime, stime); // printing stats for this process
    }

    printf(1,"Avg. Wait Time[%d] Avg. Running Time[%d] Avg. Sleep Time[%d] Avg. Turnaround Time[%d]\n",
    ready/20, run/20, sleep/20, turnAround/20); // print total avg stats

    exit();                                     // exiting
}