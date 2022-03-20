
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <sys/user.h>
#include <proc/readproc.h> // get ppid from pid 


#include <string.h>
#include <libunwind-ptrace.h>
#include <libunwind.h>
#include "libdwarf.h"
#include "dwarf.h"
#include "utilities.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <execinfo.h>
#include <sys/types.h>
#include <sys/wait.h>


#include <panel.h>
#include <menu.h>
#include <form.h>
#include "../ext/vec/src/vec.h"



//#include "deb.h"


char *choice_panel[] = { "Help", "Processes", "Memory", "Code", "Elf", (char *)NULL, } ; 
char *panel_func[] = { " ", " ", " ", " ", " ", (char *)NULL, } ; 
char *command[] = { "exec", "value", "adress", "reg",   (char *)NULL} ;
char *options[] = { "-s", "-r", "-f", "-d",   (char *)NULL} ;
// -s : singlestep
// -r : show reg
// -f : get flags
//
//
//
//

int NumDigits(int n){
    // this is a method to determine the lenght of the decimal representation of a nuumber
    // stolen from stackoverflow
    // @user:14637
    int digits = 0 ; 
    while (n){
        n /= 10 ; 
        ++digits ; 
    }
    return digits ; 
}


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

vec_t* generate_processes(void){
   // test p
    // first main process
    struct process_t p1 = {  
        .pid = 2, 
        .ppid = 1, 
        .gid = 4399,
        .status = 1, 
        .num_threads = 2, 
        .memory = 329848
    }; 
    // second main process
    struct process_t p2 = {  
        .pid = 3, 
        .ppid = 1, 
        .gid = 4399,
        .status = 1, 
        .num_threads = 2, 
        .memory = 329848
    }; 
    // processes attached to second main process
    struct process_t p3 = {  
        .pid = 4, 
        .ppid = 3, 
        .gid = 765,
        .status = 1, 
        .num_threads = 1, 
        .memory = 4344
    };     

    struct process_t p4 = {  
        .pid = 5, 
        .ppid = 3, 
        .gid = 76,
        .status = 1, 
        .num_threads = 1, 
        .memory = 143
    }; 
    struct process_t p5 = {  
        .pid = 6, 
        .ppid = 3, 
        .gid = 543,
        .status = 1, 
        .num_threads = 1, 
        .memory = 98
    }; 

    vec_t* vp = vec_new( sizeof(struct process_t)) ; 
    vec_push( vp , &p1) ; 
    vec_push( vp , &p2) ; 
    vec_push( vp , &p3) ; 
    vec_push( vp , &p4) ; 
    vec_push( vp , &p5) ; 

    return vp ;  
}






// start_interface
struct Interface{
    // first : windows

    int number_right_window  ; 
    int number_left_window  ; 
    

    WINDOW *main_window[1] ;
    WINDOW *right_window[5] ; 
    WINDOW *title_window[1] ; 
    WINDOW *selector_window[1] ;  
    PANEL *my_panels[1 + 5 + 2] ;
    ITEM **my_items ; 
    MENU *my_menus ; 
    ITEM *cur_item ; 
    
} ;

struct Window_size{
    // x, y are top left positions of the window
    // dx, dy are dimensions of the window
    int x ; 
    int y ; 
    int dx; 
    int dy ;
} ;

struct All_window_size{
    struct Window_size main ; 
    struct Window_size right ; 
    struct Window_size title ; 
    struct Window_size selector ; 
} ;

struct Interface start_interface(void){
    struct Interface inter ; 
    return inter ;
}

struct winsize compute_size_terminal(void){
    struct winsize w ; 
    ioctl(0, TIOCGWINSZ, &w);
    return w ;     
}

struct All_window_size compute_size_window(void){
    struct All_window_size size ; 
    struct winsize w = compute_size_terminal(); // get the size of the terminal
    // ratios are fromn 0 to 1 and indicaters the relative position of delimiters
    float vertical_ratio = 0.5 ; 
    float horizontal_ratio = 0.6 ; 
    // compute the integer dimensions and positions of each windows
    size.main.dx = (int)(1.0 * w.ws_row) - 3  ,  size.main.dy = (int)(vertical_ratio * w.ws_col)  ; 
    size.right.dx = (int)(1.0 * w.ws_row) - 3  ,  size.right.dy = (int)((1.0 - vertical_ratio) * w.ws_col) - 3  ; 
    size.title.dx = 1 ; 
    size.title.dy = 29 ;
    size.selector.dx = 1 ; 
    size.selector.dy = (int)(1.0 * w.ws_col) - 1 ; 

    size.main.x = 2 , size.main.y = 1 ; 
    size.right.x = 2 , size.right.y = size.main.dy + 3 ; 
    size.title.x = 1 ; 
    size.title.y = (int)(w.ws_col/2 - size.title.dy/2) ; 
    size.selector.x = (int)(1.0 * w.ws_row) - size.selector.dx  ;
    size.selector.y = 1 ;  

    return size ;

};


//struct Data {
//    struct user_regs_struct reg ; 
//
//
//};



void setup_selector(struct Interface *inter){
    int n_choice = sizeof(choice_panel) / sizeof(choice_panel[0]) ; 
    //n_choice = 6 ; 
    inter->my_items = (ITEM **)calloc(n_choice  , sizeof(ITEM *)) ; 
    for (int i = 0 ; i < n_choice ; i++){
        inter->my_items[i] = new_item(panel_func[i], choice_panel[i]);
    }
    inter->my_items[n_choice] = (ITEM *)NULL ;

    inter->my_menus= new_menu((ITEM **)inter->my_items) ;
}

void setup_window(struct Interface *inter, struct All_window_size *ws){
    inter->number_right_window = 5 ; 
    for (int i = 0 ; i < 5 ; i++){
        inter->right_window[i] = newwin(ws->right.dx, ws->right.dy, ws->right.x, ws->right.y) ; 
    }
    inter->main_window[0] = newwin(ws->main.dx, ws->main.dy, ws->main.x, ws->main.y) ; 
    inter->title_window[0] = newwin(ws->title.dx, ws->title.dy, ws->title.x, ws->title.y) ; 
    inter->selector_window[0] = newwin(ws->selector.dx, ws->selector.dy, ws->selector.x, ws->selector.y) ;    
}

void draw_box(struct Interface *inter){
     box(inter->main_window[0], 0, 0) ;     
    for (int i = 0 ; i < 5 ; i++){
        box(inter->right_window[i], 0, 0) ;     
    }   
   
}

void setup_panel(struct Interface *inter){
    inter->my_panels[0] = new_panel(inter->main_window[0]) ;    
     for(int i = 0 ; i < 5 ; i++){
        inter->my_panels[i+1] = new_panel(inter->right_window[i]) ;
    }
    inter->my_panels[6 ] = new_panel(inter->title_window[0]) ;
    inter->my_panels[7 ] = new_panel(inter->selector_window[0]) ;   
}

void setup_title(struct Interface *inter){
    char title[] = "    NCurses debugger v0.3    " ;
    wattron(inter->title_window[0], COLOR_PAIR(1));   
    mvwprintw(inter->title_window[0], 0, 0, "%s", title) ;   
    wattroff(inter->title_window[0], COLOR_PAIR(1));     
}

void setup_menu(struct Interface *inter, struct All_window_size *ws){
    int n_choice = sizeof(choice_panel) / sizeof(choice_panel[0]) ; 
    set_menu_win(inter->my_menus, inter->selector_window[0]) ; 
    set_menu_sub(inter->my_menus, derwin(inter->selector_window[0], ws->selector.dx, ws->selector.dy, 0, 0));
    set_menu_format(inter->my_menus, 1, n_choice );
    post_menu(inter->my_menus) ;
    set_current_item(inter->my_menus, inter->my_items[0]);
}

// rules for each individual window


void refresh_window_start(struct Interface *inter);
void refresh_window_processes(struct Interface *inter, vec_t *vp);
void refresh_window_memory(struct Interface *inter, struct user_regs_struct reg);
void refresh_window_code(struct Interface *inter);
void refresh_window_elf(struct Interface *inter);

int keyboard_input(struct Interface *inter, WINDOW *win, vec_t *input);


void show_specific_panel(struct Interface *inter, int panel_amount, int panel_id){
    set_current_item(inter->my_menus, inter->my_items[panel_id - 1]); //main_window has no panel
    for (int i =1 ; i < panel_id; i++){
        hide_panel(inter->my_panels[i]) ; 
    }

    for (int i = panel_id + 1 ; i < panel_amount +1; i++){
        hide_panel(inter->my_panels[i]) ;      
    }
    show_panel(inter->my_panels[panel_id]) ;  
    refresh() ;
    update_panels();
    doupdate();
}


void loop_execution(struct Interface *inter, vec_t *vp, int c, vec_t *input){

    int panel_amount = 5 ;
    // !!!!!

    while(c != KEY_F(10)){
        mvaddstr(1, 1, "while main"); 
        refresh();
        switch(c){
            //case 49:   
            case KEY_F(1):
            mvaddstr(1, 1, "F1 pressed ");  
                refresh();
                show_specific_panel(inter, panel_amount, 1);  // be careful , main window in panel[0]  
                refresh();                
                update_panels();  
                doupdate();       
                wrefresh(inter->right_window[0]);                   
                c = keyboard_input(inter, inter->main_window[0] , input)  ; 
                break;
            //case 50:
            case KEY_F(2):             
            mvaddstr(1, 1, "F2 pressed ");    
                refresh(); 
                show_specific_panel(inter, panel_amount, 2);  // be careful , main window in panel[0]       
                refresh();        
                update_panels();
                doupdate();                
                wrefresh(inter->right_window[1]);                                 
                c = keyboard_input(inter, inter->main_window[0], input)  ;
                break;
            //case 51:
            case KEY_F(3):            
            mvaddstr(1, 1, "F3 pressed ");      
                refresh();
                show_specific_panel(inter, panel_amount, 3);  // be careful , main window in panel[0]        
                refresh();        
                update_panels();  
                doupdate();        
                wrefresh(inter->right_window[2]);                                                
                c = keyboard_input(inter, inter->main_window[0], input)  ;
                break;                
            //case 52:  
            case KEY_F(4):            
                mvaddstr(1, 1, "F4 pressed ");   
                refresh(); 
                show_specific_panel(inter, panel_amount, 4);  // be careful , main window in panel[0]                      
                refresh();        
                update_panels();  
                doupdate();  
                wrefresh(inter->right_window[3]);     
                c = keyboard_input(inter, inter->main_window[0], input) ;
                break;
            //case 53:  
            case KEY_F(5):            
                mvaddstr(1, 1, "F5 pressed ");   
                refresh(); 
                show_specific_panel(inter, panel_amount, 5);  // be careful , main window in panel[0]                      
                refresh();        
                update_panels();  
                doupdate();  
                wrefresh(inter->right_window[4]);     
                c = keyboard_input(inter, inter->main_window[0], input) ;
                break;                
/* 
            case 10:
                // interprate the commande
                // void the field
                set_current_field(my_form, field[0]);
                form_driver(my_form, REQ_CLR_FIELD);
                break;
            case KEY_F(6):
                form_driver(my_form, REQ_DEL_PREV);
                break;
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;

            case KEY_RIGHT:
                form_driver(my_form, REQ_NEXT_CHAR);
                break;   

            default:
                form_driver(my_form, c) ; 
                break;
*/
            /*
            case 10:{ // enter
                ITEM *cur ; 
                if (state == 1){
                    hide_panel(my_panels[0]);
                    state = 0 ; 
                }
                else{
                    show_panel(my_panels[0]);
                    state = 1 ; 
                }
                wclear(my_windows[0]) ; 
                //wprintw(my_windows[0], choice_panel[0]) ; 
                update_panels();
                break ; 
            } */
            break;
        }

        doupdate();
        refresh();
    } 
}



void new_main_line(WINDOW *win){
            wprintw(win, "\n") ;          
	    wprintw(win, " ") ; 
	    wrefresh(win) ;   
}


int function_key(WINDOW *win, int key){
        if ((key == KEY_F(1)) || (key == KEY_F(2)) || (key == KEY_F(3)) || (key == KEY_F(4)) || (key == KEY_F(5))){
            //waddch(main_win, espace) ; 
            mvaddstr(1,1,"specialkey") ;             
            wrefresh(win);
            return 1 ;
        }	
        else{
        	return 0 ; 
        }
}

int enter_key(WINDOW *win, int key){
        if (key == 10){ //enter key
            mvaddstr(1,1,"enterkey") ; 
	    new_main_line(win) ; 
	    wrefresh(win); 
	    return 1 ;
        }	
        else{
        	return 0 ;
        }
}


struct input_thread{
    struct Interface *inter; 
    char **args ;
    char **opts ;
    int size_args ; 
    int size_opts ; 
};



void print_parsed(WINDOW *win, struct input_thread *i){
        for (int j = 0 ; j < i->size_args+1 ; j++){
	        waddstr(win , "\n vous avew saisi  les arguments :") ;            
            waddstr(win , i->args[j]) ;
            
        }
        new_main_line(win) ;        
        for (int j = 0 ; j < i->size_opts ; j++){
            waddstr(win , "\n vous avew saisi les options  :") ;
            waddstr(win , i->opts[j]) ;          
        }
}

void get_pid(struct process_t *pid){
    pid->pid = (int)getpid() ; 
    pid->ppid = (int)getppid() ; 
    pid->gid = (int)getgid() ; 

}

void print_pid(WINDOW *win, struct process_t *pid, int info){
    if (info == 0){
        waddstr(win , "Process fils : \n ") ;                      
    }
    else{
        waddstr(win , "Process père : \n ") ;                      
    }
    char tmp[10] ;
    sprintf(tmp, "%d", pid->pid) ; 
    waddstr(win , "  PID : ") ;                
    waddstr(win, tmp  ) ;   
    new_main_line(win) ;
    sprintf(tmp, "%d", pid->ppid) ; 
    waddstr(win , "  PPID : ") ;                       
    waddstr(win, tmp  ) ;   
    new_main_line(win) ;
    sprintf(tmp, "%d", pid->gid) ; 
    waddstr(win , "  GID : ") ;       
    waddstr(win, tmp  ) ;   
    new_main_line(win) ;
    refresh();    
}

void show_libraries(WINDOW *win, struct process_t *pid){
    // this function is for print the loaded libraries from /proc/pid/maps
    char *buffpid = (char*)malloc( 10 * sizeof(char)) ;
    char *buffpath = (char*)malloc( 30 * sizeof(char)) ; 
    char *buffline = (char*)malloc( 1000 * sizeof(char));
    int size_line = 1000 ; 
    int size_loaded_libs = 1000 ; 
    char **line = malloc( size_line * sizeof(char *)) ; 
    char **loaded_libs = malloc( size_loaded_libs * sizeof(char *));

    sprintf(buffpid, "%d", pid->pid) ; 
    waddstr(win, " \n maps localisation :");
    sprintf(buffpath, "/proc/%s/maps", buffpid);
    waddstr(win, buffpath) ; 
    waddstr(win, " \n ");
    FILE *fp = fopen(buffpath, "r") ; 
    if (!fp){
        waddstr(win, "impossible d'afficher les librairies chargees\n ");
    }
    //now we want to show loaded libraries
    size_t line_buf_size = 0;
    int i = 0 ; 
    while (getline(&buffline, &line_buf_size , fp )> -1){
        line[i] = malloc(( line_buf_size + 1 ) * sizeof(char)) ;         
        strcpy( line[i], buffline);
        i++;
    }
    line[i] = malloc(( line_buf_size + 1 ) * sizeof(char)) ;     
    strcpy( line[i], buffline);

    const char delim[2] = " " ;
    char slash[2] = "/\0";
    char comp[2] = "\0\0"; 
    
    char *token ;
    char *parsed_line[10] ; 
    
    int number_loaded_lib = 0 ; 
    for (int j = 0 ; j < i ; j++){

        waddstr(win, line[j]) ; 
        waddstr(win, " ");
        token = strtok((char *)line[j], delim) ;
        parsed_line[0] = malloc(strlen(token) * sizeof(char)) ; 
        strcpy(parsed_line[0] , token) ; 
        int index = 0 ; 
        while( token != NULL){
            token = strtok(NULL, delim);
            if (token != NULL){
                index++;


                parsed_line[index] = malloc( strlen(token) * sizeof(char) + 1);
                strcpy(parsed_line[index] , token);
                
                //waddstr(win, "\n"); 

                strncpy(comp, parsed_line[index], 1) ; 

                if ( strcmp( comp, slash ) == 0){    
                    //search if the loadd library already saved
                    int already_saved = 0 ; 
                    for (int k = 0 ; k < number_loaded_lib ; k++){
                        if ( strcmp( loaded_libs[k] , parsed_line[index]) == 0){
                            already_saved = 1 ; 
                        }
                    }     
                    // si le groupe commence par un backslash --> c'est une lib
                    if (already_saved == 0){
                    //if (true){
                        loaded_libs[number_loaded_lib] = malloc(( 100 + 1 ) * sizeof(char)) ; 
                        strcpy(loaded_libs[number_loaded_lib] , parsed_line[index]) ; 
                        number_loaded_lib++;                        
                    }
                }
            }  
        }	
    }
    // method to get the libs - little parser
    waddstr(win, "\n Loaded libraries : \n "); 
    for (int j = 0 ; j < number_loaded_lib ; j++){   
        waddstr(win, loaded_libs[j]) ; 
        waddstr(win, " ");       
    }
    box(win, 0, 0);
    wrefresh(win);

    fclose(fp); 

    free(buffpid) ; 
    free(buffpath);
    free(buffline);
    for (int j = 0 ; j < number_loaded_lib ; j++){
        free(loaded_libs[j]);
    }
    for (int j = 0 ; j < i ; j++){
        free(line[j]);
    }    
    free(loaded_libs);
    free(line);
}



void print_siginfo(WINDOW *win, siginfo_t *signinf, struct user_regs_struct *reg){
    char tmp2[100] ;
    char tmp3[100] ;
    char tmp4[100] ; 
    sprintf(tmp2, "%s",strsignal(signinf->si_signo)) ; 
    sprintf(tmp3, "%d",signinf->si_signo) ;             
    sprintf(tmp4, "%llx",reg->rip) ;                                          
    waddstr(win , "child stopped : ") ;                
    waddstr(win, tmp2  ) ;   
    waddstr(win, " (") ;                
    waddstr(win, tmp3  ) ;   
    waddstr(win, ") at adress 0x") ;                
    waddstr(win, tmp4  ) ;                                                  
    new_main_line(win) ;
    wrefresh(win) ;     
}


struct option_debugger{
    int singlestep ;
    int get_reg  ;      
    int get_all_sig  ;

};


void config_debugger(struct input_thread *in, struct option_debugger *opt_deb){
    opt_deb->singlestep = 0 ; 
    opt_deb->get_reg = 0 ; 
    opt_deb->get_all_sig = 0 ; 
    for (int i = 0 ; i < in->size_opts ; i++){
        if ( strcmp(options[0] , in->opts[i]) == 0 ){
            opt_deb->singlestep = 1 ; 
        }

        if ( strcmp(options[1] , in->opts[i]) == 0 ){
            opt_deb->get_reg = 1 ; 
        }
        if ( strcmp(options[2] , in->opts[i]) == 0 ){
            opt_deb->get_all_sig = 1 ; 
        }                
    }
}


static unw_addr_space_t as;
static struct UPT_info *ui;
void *spawn_thread(void* input){
    struct input_thread *i = input;
    WINDOW *main_win = i->inter->main_window[0] ; 
    WINDOW *process_win = i->inter->right_window[1] ; 
    print_parsed(i->inter->main_window[0], i) ; 

    struct option_debugger opt_deb ; 
    config_debugger(i, &opt_deb);

    as = unw_create_addr_space(&_UPT_accessors, 0);

    pid_t child_pid ; 
    int pid_status ; 
    child_pid = fork() ; 

    struct process_t process_child ; 
    struct process_t process_father ; 


    char *argprog[] = {"/home/sbstndbs/debugger/target/test", "5433", (char *)NULL} ;    
    char *argprog2[] =    {"./test", "5433", (char *)NULL} ;


    if (child_pid == 0){
        // PTRACE
        get_pid(&process_child) ;         
        if (ptrace(PTRACE_TRACEME,0,NULL,NULL) < 0) return printf("Error with ptrace, check the manual"),-2;  
  
    }
    if (child_pid != 0){
        process_child.pid = child_pid ;  
        get_pid(&process_father) ;        
        print_pid(main_win, &process_father, 1) ; 
        print_pid(main_win, &process_child, 0);    
        ui = _UPT_create(child_pid) ; 

    }  
    int is_executed = 1 ;     
    if (child_pid == 0){                          
        execvp(i->args[0], i->args);
        is_executed = 0 ; 
        _exit(-1);
        // is_executed set to -1 if execvp returned an error
    }
    if (is_executed == 0){
        waddstr(main_win , "\n Issue : cannot trace this program") ; 
    }
    if ( (child_pid != 0) & (is_executed != 0)){
        int wait_status ;   
        // fiona part   for dgb modified  
        char *temp[] = {"./test\0" } ;
        //get_dbg(i->args[0]);
        get_dbg(temp[0]);
        long long prog_offset ; 
        char fname[128];
        char buff[128];
        char buff2[128]  ;
        sprintf(fname, "/proc/%d/maps", child_pid);
        FILE *f = fopen(fname, "rb");
        fscanf(f, "%llx", &prog_offset);
        // Print proc information
        sprintf(buff, " PID: %d\n", getpid());
        waddstr(main_win ,buff) ; 
        sprintf(buff, " GID: %d\n", getgid());
        waddstr(main_win ,buff) ; 
        sprintf(buff, " PPID: %d\n", getppid());
        waddstr(main_win ,buff) ; 
        sprintf(buff, " Path : %s\n", realpath(i->args[0], NULL));
        waddstr(main_win ,buff) ; 
        sprintf(buff, " Offset = %llx\n", prog_offset);
        waddstr(main_win ,buff) ; 

        sprintf(buff, " Function count : %d\n", count_func);
        waddstr(main_win ,buff) ; 

        for (int i = 0; i < count_func; i++){
            sprintf(buff, " Function : %s\n", func[i].name);
            waddstr(main_win ,buff) ; 
        }

        for (int i = 0; i < count_var; i++){
            sprintf(buff, " Variable  : %s dans func:", var[i].name);
            waddstr(main_win ,buff) ;             
            sprintf(buff, " %s\n ", var[i].funcname);
            waddstr(main_win, buff );
        }
        // Wait for the program (first execution)

        // end fiona part 
        waitpid(child_pid, &wait_status, 0);


        vec_t* vp = generate_processes() ; 



        struct user_regs_struct reg ; 
        char tmp[10] ;        
        char tmp2[100] ;
        char tmp3[100] ;
        char tmp4[100] ; 
        unsigned long long number_of_instructions = 0 ; 
        int loop = 0 ; 
        while(true){
            loop++;
            if (opt_deb.singlestep == 1){
                ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0);
                number_of_instructions++;
            }
            else{
                ptrace(PTRACE_SYSCALL, child_pid, 0, 0);                
            }

            if (loop == 1){
                
            }

            //waddstr(i->inter->main_window[0], tmp  ) ; 
            //new_main_line(i->inter->main_window[0]) ;

            waitpid(child_pid, &wait_status,0);       
            siginfo_t signinf;
            ptrace(PTRACE_GETSIGINFO,child_pid,NULL,&signinf);


            ptrace(PTRACE_GETREGS, child_pid, NULL, &reg);;

            if(signinf.si_signo != 5)
            {
                print_siginfo(main_win, &signinf, &reg);
                break;
            }
            if ((number_of_instructions%10000 == 0) & opt_deb.singlestep == 1){
                refresh_window_memory(i->inter, reg);
            }
            else{
                refresh_window_memory(i->inter, reg);     
            }
        }
show_libraries(process_win, &process_child);        
    }
    pthread_exit(NULL) ;
}

void parse(struct Interface *inter, WINDOW *win, vec_t *input){
	    waddstr(win , "\n vous avew saisi :") ; 
	    new_main_line(win) ;         
	    waddstr(win, (char *)(&input->data)[0] ) ; // affichage de la siason en guise de verif
	    new_main_line(win) ;  
       
	    const char delim[2] = " " ; 

        /////////////////
        // etape de recuperation de l'input
        /////////////////
  
        char *token ;
        char *parsed[20] ; // we do not consider more than 20 args...      
        char *opts[20];
        char *args[20] ; 

        int i_p = 0 ; // nombre d'occurence
        int i_o = 0 ; 
        int i_a = 0 ; 

	    token = strtok((char *)(&input->data)[0] , delim ) ; 
        parsed[0] = malloc(strlen(token) * sizeof(char));
        i_p++ ;
        strcpy(parsed[0], token);

        while( token != NULL){
            token = strtok(NULL, delim);
            if (token != NULL){
                i_p++;
                parsed[i_p-1] = malloc( strlen(token) * sizeof(char) + 1);
                strcpy(parsed[i_p-1] , token);
            }
        }	    
        ///////////////////
        // etape de parsing
        //////////////////
        int n_options = 4 ; 
        // get the options & the args 
        int is_an_option ; 
        for (int j = 1 ; j < i_p ; j++ ){
            is_an_option = 0 ; 
            for (int k = 0 ; k < n_options ; k++){
                if (strcmp(options[k], parsed[j]) == 0){
                    i_o++;
                    is_an_option = 1 ; 
                    opts[i_o-1] = malloc( (strlen(parsed[j]) + 1)* sizeof(char)  ) ;
                    strcpy(opts[i_o-1], parsed[j]); // copie de l'option 
                }
            }
            if (is_an_option == 0){
                i_a++;
                args[i_a-1] = malloc( ( strlen(parsed[j]) + 1 ) *sizeof(char) );
                strcpy(args[i_a-1], parsed[j]);
            }
            
        }
        char n[2] = "\0";
        for (int j = i_a ; j < 20 ; j++){
            args[j] = malloc( sizeof(char));            
            args[j] = (char *)NULL ; 

            //strcpy(args[i_a], n) ; // NULL a la fin
            //args[i_a][0] = "\0" ;
        }

        // structure for the thread
        struct input_thread  it ;
        it.inter = inter ; 
        it.size_args = i_a ; 
        it.size_opts = i_o ;
        it.opts = opts ; 
        it.args = args ; 

        for (int j = 0 ; j < i_a+1 ; j++){
	        waddstr(win , "\n vous avew saisi  les arguments :") ;   
            waddstr(win, "|") ;         
            waddstr(win , args[j]) ;waddstr(win, "|") ;       
            
        }
        new_main_line(win) ;        
        for (int j = 0 ; j < i_o ; j++){
            waddstr(win , "\n vous avew saisi les options  :") ;
            waddstr(win , opts[j]) ;          
        }




        pthread_t thread ; 
        if (strcmp(command[0], parsed[0]) == 0){
                // its an exec .....
            waddstr(win , "\n EXEC :") ; 
            new_main_line(win) ; 
            int thr = 1 ; 
            pthread_create(&thread, NULL, spawn_thread, &it);
            pthread_join(thread , NULL) ; 
            wrefresh(win);
        }else{
	    waddstr(win , "\n NOTHING") ; 
	    new_main_line(win) ; 
        }        

        ///////////////////
        // deallocation
        //////////////////

        for (int j = i_p-1 ; j >= 0 ; j--){
            free(parsed[j]);
        }



	    /*
	    if ( strcmp(strToken, command[0]) ){
	    	waddstr(win, "Lancement de l'execution") ; 
	    	new_main_line(win) ; 
	    	
	    	pid_t child_pid ; 
	    	int pid_status ; 
	    	child_pid = fork() ; 
	    	char *argprog[] = {"/home/sbstndbs/debugger/target/mon_programme", "95433", (char *)NULL} ; 
	    	char *arg[] = {"mon_programme", "9434543", (char *)NULL} ; 
	    	if (child_pid == 0){
	    		//execvp(arg[0], &arg[0]) ;
	    		execv(argprog[0], &argprog[0]) ;  
	    	}

	    	waddstr(win, "Programme lance") ; 
	    	new_main_line(win) ; 	    	
	    
	    }
	    	*/
	
	    

}

int keyboard_input(struct Interface *inter, WINDOW *win, vec_t *input){
	int key ; 
    while(( key = getch())){

	if (function_key(win, key)){
		break;
	}
	else if (enter_key(win, key)){
	
	     // donnees a envoyer au parseur : 
            char end = '\0' ; // signal de terminaison de la saisie
	    vec_push(input, (char *)&end) ; // le vecteur est rempli d caracteres a interpreter termin2 par null
	    // affichagele temps de debug	
	    //waddstr(win , "\n vous avew saisi : \n") ; 
	    //waddstr(win, (char *)(&input->data)[0] ) ; // affichage de la siason en guise de verif
	    //waddstr(win , "\n") ; 
	    parse(inter, win, input) ; 
	    vec_clear(input) ; // apres envoi au parseur, onvide le vec 
	    	    
	}
        else{
            mvaddstr(1,1,"notspecialkey") ; 
            waddch(win, key) ; 
            //wprintw(main_win, temp) ; 
            doupdate();
            wrefresh(win);
 
            vec_push(input, (char *)&key) ; 
 

        }
        box(win, 0,0) ; 
        wrefresh(win) ; 
     }	
	return key ;
}




void refresh_window_start(struct Interface *inter){
    box(inter->right_window[0], 0, 0);
    mvaddstr(1, 1, "show window start");
    refresh();
    mvwaddstr(inter->right_window[0], 2, 1, "start panel");
    box(inter->right_window[0], 0, 0);    
    wrefresh(inter->right_window[0]);         
}

void refresh_window_processes(struct Interface *inter, vec_t *vp ){
    box(inter->right_window[1], 0, 0) ;  
    int c ;
    mvaddstr(1, 1, "show window processes");

    mvwaddstr(inter->right_window[1], 2, 1, "process panel");
    wrefresh(inter->right_window[1]);  

    // affichage des processes
    wclear(inter->right_window[1]); 
    box(inter->right_window[1], 0, 0) ; 
    wrefresh(inter->right_window[1]) ; 

    char *entity_name[] = { "PID", "PPID", "GID", "status", "threads", "Memory", (char *)NULL, } ; 
    char *reduced_entity_name[] = { "PID", "PPID", "GID", "stat.", "th.", "Mem.", (char *)NULL, } ; 
    // first line : legendary line
    // other lines : show each processes of vec.
    int ncol, nrow ; 
    getmaxyx(inter->right_window[1], nrow, ncol) ; // get the specific window size
    int nentity_col = 6 ; 
    int width = ncol / nentity_col ; 
    int max_width = 10 ; 
    if (width > max_width){
        width = 10 ; 
    }
    // print the first title line 

 

    wattron(inter->right_window[1], COLOR_PAIR(1));
    char *symbol[1] = {" "} ; 
    for (int i = 0 ; i < ncol - 2 ; i++){
        mvwaddstr(inter->right_window[1], 1,i + 1, symbol[0]) ;
    }
    for (int i = 0 ; i < nentity_col; i++)
    {
        if (width > 7){
            mvwaddstr(inter->right_window[1], 1, 1 + width * i, entity_name[i]) ; 
        }
        else{
            mvwaddstr(inter->right_window[1], 1, 1 + width * i, reduced_entity_name[i]) ;            
        }
    }
    wattroff(inter->right_window[1], COLOR_PAIR(1));   
    wrefresh(inter->right_window[1]) ;


// we want to have an arborescence between processes : sort them. 
// 

    // print each process line
    char tmp[10] ; 
    for (int i = 0 ; i < vp->len ; i++){
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].pid) ; 
            mvwaddstr(inter->right_window[1], 2 + i, 1, tmp  ) ;   
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].ppid) ;               
            mvwaddstr(inter->right_window[1], 2 + i, 1 + width * 1, tmp  ) ;           
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].gid) ;               
            mvwaddstr(inter->right_window[1], 2 + i, 1 + width * 2, tmp  ) ;     
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].status) ;               
            mvwaddstr(inter->right_window[1], 2 + i, 1 + width * 3, tmp  ) ;     
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].num_threads) ;               
            mvwaddstr(inter->right_window[1], 2 + i, 1 + width * 4, tmp  ) ;     
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].memory) ;               
            mvwaddstr(inter->right_window[1], 2 + i, 1 + width * 5, tmp  ) ;                                                                                                      
    }
    box(inter->right_window[1], 0, 0);    
    wrefresh(inter->right_window[1]) ;
}

void refresh_window_memory(struct Interface *inter, struct user_regs_struct reg ){
    wclear(inter->right_window[2]);
    //mvwaddstr(inter->right_window[2], 2, 1, "memory panel");
    wrefresh(inter->right_window[2]);  

    box(inter->right_window[2], 0, 0) ;  
    update_panels();
    int c ; 
    mvaddstr(1, 1, "show window memory");
    refresh();
    //mvwaddstr(inter->right_window[2], 2, 1, "memory panel");
    box(inter->right_window[2], 0, 0); 
    wrefresh(inter->right_window[2]);    


     char tmp2[20] ; 
    sprintf(tmp2, "rax  0x%llx",reg.rax) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "rbx  0x%llx",reg.rbx) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "rcx  0x%llx",reg.rcx) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "rdx  0x%llx",reg.rdx) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "rdi  0x%llx",reg.rdi) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "rsi  0x%llx",reg.rsi) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "rbp  0x%llx",reg.rbp) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "rsp  0x%llx",reg.rsp) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "r8   0x%llx",reg.r8) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "r9   0x%llx",reg.r9) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "r10   0x%llx",reg.r10) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "r11  0x%llx",reg.r11) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "r12  0x%llx",reg.r12) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "r13  0x%llx",reg.r13) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "r14  0x%llx",reg.r14) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "r15  0x%llx",reg.r15) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;

    sprintf(tmp2, "rip  0x%llx",reg.rip) ; 
    waddstr(inter->right_window[2], tmp2  ) ;                                                  
    new_main_line(inter->right_window[2]) ;           

    box(inter->right_window[2], 0, 0); 
    wrefresh(inter->right_window[2]); 


}

void refresh_window_code(struct Interface *inter){
    wclear(inter->right_window[3]);
    mvaddstr(1, 1, "show window others");
    refresh();

    mvwaddstr(inter->right_window[3], 2, 1, "others panel");
    wrefresh(inter->right_window[3]);  

    // we want to prijnt a specific part of the code where the bug occurs, or something else
    // protocole : we get a file, and a line 
    // example : want to print line 54 in main.c --> we print from line 54-height/2 to 54+height/2
    int line_to_print = 10; 

    int nrow, ncol ; 
    getmaxyx(inter->right_window[3], nrow, ncol) ; // get the specific window size 
    nrow-- ;
    int midheight = (int)( nrow / 2 ) ; 
    int begin ; 
    if (midheight < line_to_print){
        // we can naively print the code
        begin = line_to_print - midheight ; 
    } 
    else{
        // we have to begin with the first line of code
        begin = 0 ; 
    }


    // open the specific file 
    FILE *fp ; 
    char *buff = malloc( ncol * sizeof(char)) ; 

    // length of the char line
    int numdigits = NumDigits(line_to_print + midheight) ; 
    char *chline = malloc( numdigits * sizeof(char));   


    fp = fopen("/home/sbstndbs/debugger/src/interface.c", "r") ;
    if (fp == NULL){
        mvwaddstr(inter->right_window[3], 1, 4, "cannot open the selected file...");
    }
    int iline = 0 ; 
    int nline = 0 ; 
    while ((nline < nrow - 1 ) && (fgets(buff, ncol, fp) != NULL) ){
        mvaddstr(1, 1, "begin writing the line file ");
        if (iline >= begin){

            // print the line
            sprintf(chline, "%d", iline + 1);
            //wattron(inter->right_window[3], COLOR_PAIR(1));  
            mvwaddstr(inter->right_window[3], 1 + nline, 1, chline) ;
            //wattroff(inter->right_window[3], COLOR_PAIR(1));  

            if(nline + begin == line_to_print){
                char *symbol[1] = {" "} ; 
                wattron(inter->right_window[3], COLOR_PAIR(1));                  
                for (int i = 0 ; i < ncol - 2 ; i++){
                    mvwaddstr(inter->right_window[3], 1 + nline, i + 2 + numdigits, symbol[0]) ;
                }              
                mvwaddstr(inter->right_window[3], 1 + nline, 2 + numdigits, buff) ; 
                wattroff(inter->right_window[3], COLOR_PAIR(1));                                
            }
            else{
            mvwaddstr(inter->right_window[3], 1 + nline, 2 + numdigits, buff) ; 
            }
            nline++;
        }     
        iline++;  
    }
    // close file
    fclose(fp) ;

    box(inter->right_window[3], 0, 0);
    wrefresh(inter->right_window[3]) ;  
}

void refresh_window_elf(struct Interface *inter){
    box(inter->right_window[4], 0, 0);
    mvaddstr(1, 1, "show window start");
    refresh();
    mvwaddstr(inter->right_window[4], 2, 1, "start panel");
    box(inter->right_window[4], 0, 0);    
    wrefresh(inter->right_window[4]);         
}


// this is the way the main window is drawn
int show_window_start(struct Interface *inter, WINDOW *win, WINDOW *main_win, vec_t *input){
    int c ;
    c = keyboard_input(inter, main_win, input) ; 
    mvaddstr(1, 1, "exit window start"); 
    refresh();
     return c ; 
}





int main(int argc, char **argv){
    printf("This is the Ncurses sbstndbs debugger ! \n");   

    //struct Interface inter ; 
    struct Interface inter =  start_interface() ; 
    // compute dimensions of interface
    struct All_window_size ws = compute_size_window() ; 

  
    initscr();
    cbreak();
    noecho(); 
    curs_set(0);

    start_color() ; 
    init_pair(1, COLOR_BLACK, COLOR_WHITE);    

    int n_choice = sizeof(choice_panel) / sizeof(choice_panel[0]) ;

    setup_selector(&inter) ; 
    setup_window(&inter, &ws) ; 
    draw_box(&inter) ; 
    setup_panel(&inter) ; 
    setup_title(&inter) ; 
    setup_menu(&inter, &ws) ; 

    keypad(stdscr, TRUE);
    scrollok(inter.main_window[0], TRUE) ; 

    refresh() ;
    update_panels() ;    

        //attron(COLOR_PAIR(1));
        //mvprintw( 0, 0, "%s", "voila");
        //attroff(COLOR_PAIR(1));

    
    vec_t* vp = generate_processes() ;  // to add processes. Need to be obtain from branch/features

    keypad(inter.main_window[0], TRUE) ; 

	new_main_line(inter.main_window[0]);


	vec_t *input = vec_new( sizeof(char) ) ; 
    struct user_regs_struct reg ; 
    int c ;
  

       
 
    show_specific_panel(&inter, 5, 1); 
    refresh_window_start(&inter) ;
    usleep(100000);

    show_specific_panel(&inter, 5, 2);       
    refresh_window_processes(&inter, vp) ;
    usleep(100000); 

    show_specific_panel(&inter, 5, 3);         
    refresh_window_memory(&inter, reg) ;  
    usleep(100000);      

    show_specific_panel(&inter, 5, 4); 
    refresh_window_code(&inter) ; 
    usleep(100000);   

    show_specific_panel(&inter, 5, 5); 
    refresh_window_elf(&inter) ; 
    usleep(100000);         

    show_specific_panel(&inter, 5, 1); 



    c = show_window_start(&inter, inter.right_window[0], inter.main_window[0], input); // c est la touche d'interraction pressee
    //TODO: utiliser l'execution conditionelle au debug (compil) des mvwaddnstr 
    // qui servent au debug uniquement
    mvwaddnstr(inter.right_window[0],1, 1,  "Bienvenue sur le menu d'execution\n", 35); // a titre de debug
    box(inter.right_window[0], 0, 0) ; 

    update_panels();
    doupdate();
    refresh();    

    loop_execution(&inter, vp, c, input) ; 

    doupdate();
    getch();

    free_item(inter.my_items[0]);
    free_menu(inter.my_menus);
    vec_drop(input);
    endwin();

}

