#include "../ext/vec/src/vec.h"
// this code is for process printing through the interface
// process structure
struct process_t{
    // state = 0 for killed , state = 1 for active, state = 2 for inactive
    // this cnvention may change
    int status ;
    int pid ; 
    int ppid ; 
    int gid   ;  
    int num_threads ;   // number of threads in the process
    int memory ;           // memory used by the process
};