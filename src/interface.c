#define _GNU_SOURCE
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
#include <sys/personality.h>

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

static unw_addr_space_t as;
static struct UPT_info *ui;

//#include "deb.h"


char *choice_panel[] = {
    /**
     * @brief  Contient la liste des noms des panneaux de droite disponibles. Ces noms apparaissent dans
     * le selecteur du bas. 
     */
    "Help",
    "Processes",
    "Register",
    "Code",
    "Dwarf",
    "Memory",
    (char *)NULL,
};

char *panel_func[] = {
    /**
     * @brief Liste des textes additionels des selecteurs. Laisss vide car on ne 
     * souhaite pas utiliser d'autre texte que celui de choice_panel 
     */
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    (char *)NULL,
};

char *command[] = {
    /**
     * @brief Correspond aux commandes à entrer dans la zone de saisie. Voici les descriptifs :
     * exec : execution simple,
     * breakpoint : ajouter un breakpoint au sein du code.
     */
    "exec", 
    "breakpoint",
    "memory",
    "register",    
    (char *)NULL
};

char *options[] = {
    /**
     * @brief correspond aux arguments à entrer dans la zone de saisie. Voici les descriptifs ; 
     * -s : singlestep
     * -r : show reg
     * -f : get flags
     * -v : verbose
     */
    "-s",
    "-r",
    "-f",
    "-v",
    (char *)NULL
};


char *entity_name[] = {
    "PID",
    "PPID",
    "GID",
    "status",
    "threads",
    "Memory",
    (char *)NULL,
};

char *reduced_entity_name[] = {
    "PID",
    "PPID",
    "GID",
    "stat.",
    "th.",
    "Mem.",
    (char *)NULL,
};

char *memory_type_raw[] = {
    "adress",
    "raw",
    (char *)NULL,
};
char *memory_type_fp[] = {
    "fp64",
    "fp32_1",
    "fp32_2",
    (char *)NULL,
};
char *memory_type_int[] = {
    "uint64",
    "uint32_1", 
    "uint32_2",
    "int64",
    "int32_1",
    "int32_2",
    (char *)NULL,
};
char *memory_type_char[] = {
    "ASCII",
    "ui81",
    "ui82",
    "ui83",
    "ui84",
    "ui85",
    "ui86",
    "ui87",
    "ui88",     
    "i81",
    "i82",
    "i83",
    "i84",
    "i85",
    "i86",
    "i87",
    "i88",                                      
    (char *)NULL,
};
char *memory_type_int16[] = {
    "uint16_1",
    "uint16_2",
    "uint16_3",
    "uint16_4",    
    "int8_1",
    "int8_2",
    "int8_3",
    "int8_4",                                      
    (char *)NULL,
};
int width_type_raw[] = {15, 18} ; 
int width_type_fp[] = {12,  12, 12} ; 
int width_type_int[] = {21, 13, 13, 21, 13, 13} ; 
int width_type_char[] = {18, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5} ; 
int width_type_int16[] = {10, 10, 10, 10, 10, 10, 10, 10} ; 
int nentity_horizontal[] = {2, 3, 6, 17, 8} ; 
// raw, fp, int, char
struct process_t{
    /**
     * @brief stocke l'etat d'un processus en vue de l'afficher
     * state = 0 for killed , state = 1 for active, state = 2 for inactive
     * (this cnvention may change.)
     * num_thread is the thread amount shared in the process
     * memory is the mount of memory claimed by this process
     */
    int status;
    int pid;
    int ppid;
    int gid;
    int num_threads;
    int memory;
};

void init_process_t(struct process_t *process){
    process->pid = 0 ; 
    process->ppid = 0 ; 
    process->gid = 0 ; 
    process->memory = 0 ; 
    process->num_threads = 0 ; 
    process->status = 0 ; 
}

vec_t *generate_processes(void){
    /**
     * @brief permet de generer des processus en vue de tester son affichage dans la fenetre dediee
     * --> regadrer "void refresh_window_processes"
     */
    struct process_t p1 = {
        .pid = 2,
        .ppid = 1,
        .gid = 4399,
        .status = 1,
        .num_threads = 2,
        .memory = 329848};
    // second main process
    struct process_t p2 = {
        .pid = 3,
        .ppid = 1,
        .gid = 4399,
        .status = 1,
        .num_threads = 2,
        .memory = 329848};
    // processes attached to second main process
    struct process_t p3 = {
        .pid = 4,
        .ppid = 3,
        .gid = 765,
        .status = 1,
        .num_threads = 1,
        .memory = 4344};
    struct process_t p4 = {
        .pid = 5,
        .ppid = 3,
        .gid = 76,
        .status = 1,
        .num_threads = 1,
        .memory = 143};
    struct process_t p5 = {
        .pid = 6,
        .ppid = 3,
        .gid = 543,
        .status = 1,
        .num_threads = 1,
        .memory = 98};
    vec_t *vp = vec_new(sizeof(struct process_t));
    vec_push(vp, &p1);
    vec_push(vp, &p2);
    vec_push(vp, &p3);
    vec_push(vp, &p4);
    vec_push(vp, &p5);
    return vp;
}


struct Interface{
    /**
     * @brief Permet de stocker les entitees de l'interface NCurses :
     * - number_right_window stockele nombre de fenetres de droites selectionnables ;
     * - number_left_window stocke le nombre de fenetre de gauche 
     * - main_window correspond a la fenetre de gauche
     * - right_window correspond aux multiples fenetres de droites, 
     * - title_window correspond au texte du haut (exemple : NCurses debugger)
     * - selector_panel corespind au selecteur du bas contenant le nom de la fenetre
     *   de droite en cours d'affichage
     * - my_panels correspond au regroupement des fenetres au sein d'une structure panel. Les
     *      panels permettent de gerer la superposition de fenetres dans risquer de creer d'artefacts d'affichage
     * - cur_item permet d'indiquer quel est la case de selecteur affichee 
     */

    int number_right_window;
    int number_left_window;

    WINDOW *main_window[1];
    WINDOW *right_window[6];
    WINDOW *title_window[1];
    WINDOW *selector_window[1];
    PANEL *my_panels[1 + 6 + 2];
    ITEM **my_items;
    MENU *my_menus;
    ITEM *cur_item;

    // scrolling position ; 
    int scroll_position_memory ;  
    int horizontal_position_memory ;  
    int scroll_position_register ;  
    int horizontal_position_register ; 
    int horizontal_position_code ;     

};



struct maps_info{
    /**
     * @brief Structure of the proc/pid/maps to do easier analysis on this data
     * "start" & "stop" are the adress of the "name" data. "is_*" is from rwxp".
     */
    uint64_t *start ;
    uint64_t *stop ; 
    int *is_readable ; 
    int *is_writable ; 
    int *is_executable ; 
    int *is_shared ;
    int *is_private ; 
    char **line ;
    char **libs ; 
    uint64_t number_lines ;
    uint64_t number_libs ; 

};

struct regs_info{
    struct user_regs_struct *reg ; 
    struct user_fpregs_struct *fpreg ; 
};

struct memory_info{
    vec_t *adress ; 
    vec_t *value ; 
};


struct process_info{
    struct process_t *process_father ; 
    struct process_t *process_child ;    
    vec_t *vp ;  
};

struct code_info{
    char **paths ; 
    char **name ; 
    uint64_t *line ; 
    uint64_t count_func ; 
};

struct backtrace_info{
    unw_addr_space_t as ; 
    struct UPT_info *ui ; 
    char **names ; // list of functions from deepest 
    uint64_t size ;

};

struct Debugger{
    /**
     * @brief the ptrace is encapsuled in a pthread. Hence, we need to 
     * creat e an argument structure for that function.
     * There are :
     * - inter : interface structure
     * - args : arguments from the parser
     * - opts : options from the parser
     * - size_args and size_opts to determine the length of the arguments and optiosn from the parser
     * - have_breakpoint : true if the command is "breakpoint", false either
     *          -is_function : true if the breakpoint is associated to a function. False if it is 
     *              associated to an adress
     *          - breakpoint_function :  function associated to the breakpoint
     *          - breakpoint_adress : relative adress associated with the breakpoint
     */
    /**
     * @brief Structure to set booleans to each options which can be specified in the prompt
     */    
    char **args;
    char **opts;
    int size_args;
    int size_opts;
    int have_breakpoint;
    int is_function;
    char *breakpoint_function;
    char *breakpoint_adress;
    char *register_adress ; 
    char *register_content ; 
    char *memory_adress ; 
    char *memory_content ;
    int have_rmemory ; 
    int have_wmemory ; 
    int have_wregister ; 

    int singlestep;
    int get_reg;
    int get_all_sig;
    int verbose;    

};




struct Data{
    struct Interface *inter ; 
    struct Debugger *debug ; 
    struct maps_info *maps;
    struct regs_info *regs ; 
    struct process_info *procs ; 
    struct memory_info *mems ;
    struct code_info *codes ; 
    struct backtrace_info *backs ; 
    
    char *buff1 ; 
    char *buff2 ; 
    char *buff2_2 ; 
    char *buff16 ; 
    char *buff32 ; 
    char *buff32_2 ; 
    char *buff64 ; 
    char *buff64_2 ; 
    char *buff64_3 ; 
    char *buff128 ; 
    char *buff256 ; 
    char *buff512 ; 
    char *buff1024 ; 
    char *buff1024_2 ;

    char **dbuff64 ; 
    char **dbuff1024 ; 
};


struct Window_size{
    /**
     * @brief Permet de stocker les dimensions d'une fenetre
     * - x, y, sont les coordonnees du coin superieur gauche
     * - dx dy sont les hauteurs et les largeurs de la fenetre
     */
    int x;
    int y;
    int dx;
    int dy;
};

struct All_window_size{
    /**
     * @brief Permet de stocker les dimensions de toutes les fenetres de l'espace d'affichage.
     */
    struct Window_size main;
    struct Window_size right;
    struct Window_size title;
    struct Window_size selector;
};


struct Data start_data(void){ 
    struct Data data ; 
    //struct Interface inter ; 
    //struct Debugger debug ; 
    //data.inter = &inter ; 
    //data.debug = &debug ; 
    return data ; 
}


struct winsize compute_size_terminal(void){
    /**
     * @brief Permet de recuperer la taille du terminal d'apres le tty associée
     */
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return w;
}

struct All_window_size compute_size_window(void){
    /**
     * @brief Main function to determine for each NCurses window its size.
     * - first : get the terminal size
     * - second : compute each window size by giving horizontal and vertical ratios 
     *          between main_window and right_window
     * 
     * - ratios are from 0 to 1. vertical_ratio is unused for now but we can easily 
     *          add a third window to have 3 panels.
     */
    struct All_window_size size;
    struct winsize w = compute_size_terminal(); 

    float vertical_ratio = 0.4;
    float horizontal_ratio = 0.6;

    size.main.dx = (int)(1.0 * w.ws_row) - 3,  size.main.dy = (int)(vertical_ratio * w.ws_col);
    size.right.dx = (int)(1.0 * w.ws_row) - 3, size.right.dy = (int)((1.0 - vertical_ratio) * w.ws_col) - 3;
    size.title.dx = 1;                         size.title.dy = 29;
    size.selector.dx = 1;                      size.selector.dy = (int)(1.0 * w.ws_col) - 1;

    size.main.x = 2,                           size.main.y = 1;
    size.right.x = 2,                          size.right.y = size.main.dy + 3;
    size.title.x = 1;                          size.title.y = (int)(w.ws_col / 2 - size.title.dy / 2);
    size.selector.x = (int)(1.0 * w.ws_row) - size.selector.dx;
                                               size.selector.y = 1;
    return size;
};

int NumDigits(int n){
    /**
     * @brief this is a method to determine the lenght of the decimal representation of a nuumber
     * stolen from stackoverflow. This is for the code panel, to compute the lenght of the line code number - 
     * for printing it to the left.
     */
    int digits = 0;
    while (n)
    {
        n /= 10;
        ++digits;
    }
    return digits;
}

void setup_selector(struct Data *data){
    /**
     * @brief selector allocation. Called only one time
     * - n_choice is the number of right panels used
     */
    struct Interface *inter = data->inter ; 

    int n_choice = sizeof(choice_panel) / sizeof(choice_panel[0]);
    inter->my_items = (ITEM **)calloc(n_choice, sizeof(ITEM *));
    for (int i = 0; i < n_choice; i++){
        inter->my_items[i] = new_item(panel_func[i], choice_panel[i]);
    }
    inter->my_items[n_choice] = (ITEM *)NULL;
    inter->my_menus = new_menu((ITEM **)inter->my_items);
}

void setup_window(struct Data *data, struct All_window_size *ws){
    /**
     * @brief window allocator. Called only one time after the Compute_size_window function
     */
    struct Interface *inter = data->inter ; 
    inter->number_right_window = 6;

    for (int i = 0; i < 6; i++){
        inter->right_window[i] = newwin(ws->right.dx, ws->right.dy, ws->right.x, ws->right.y);
    }
    inter->main_window[0] = newwin(ws->main.dx, ws->main.dy, ws->main.x, ws->main.y);
    inter->title_window[0] = newwin(ws->title.dx, ws->title.dy, ws->title.x, ws->title.y);
    inter->selector_window[0] = newwin(ws->selector.dx, ws->selector.dy, ws->selector.x, ws->selector.y);
}

void setup_panel(struct Data *data){
    /**
     * @brief panel allocator. Called only one time.
     */
    struct Interface *inter = data->inter ; 
    inter->my_panels[0] = new_panel(inter->main_window[0]);
    for (int i = 0; i < 6; i++){
        inter->my_panels[i + 1] = new_panel(inter->right_window[i]);
    }
    inter->my_panels[7] = new_panel(inter->title_window[0]);
    inter->my_panels[8] = new_panel(inter->selector_window[0]);
}

void setup_title(struct Data *data){
    /**
     * @brief title allocator. Called only one time.
     */
    struct Interface *inter = data->inter ; 
    char title[] = "    NCurses debugger v0.4    ";
    wattron(inter->title_window[0], COLOR_PAIR(1));
    mvwprintw(inter->title_window[0], 0, 0, "%s", title);
    wattroff(inter->title_window[0], COLOR_PAIR(1));
}

void setup_menu(struct Data *data, struct All_window_size *ws){
    /**
     * @brief Menu allocator. Called only one time.
     */
    struct Interface *inter = data->inter ; 
    int n_choice = sizeof(choice_panel) / sizeof(choice_panel[0]);
    set_menu_win(inter->my_menus, inter->selector_window[0]);
    set_menu_sub(inter->my_menus, derwin(inter->selector_window[0], ws->selector.dx, ws->selector.dy, 0, 0));
    set_menu_format(inter->my_menus, 1, n_choice);
    post_menu(inter->my_menus);
    set_current_item(inter->my_menus, inter->my_items[0]);
}

void draw_box(struct Data *data){
    /**
     * @brief Draw the boxes of all panels. 
     * This is in fact useless because the boxes are called after each refresh_window calls...
     */
    struct Interface *inter = data->inter ;     
    box(inter->main_window[0], 0, 0);
    for (int i = 0; i < 6; i++){
        box(inter->right_window[i], 0, 0);
    }
}


void refresh_window_start(struct Data *data);
void refresh_window_processes(struct Data *data);
void refresh_window_register(struct Data *data);
void refresh_window_code(struct Data *data);
void refresh_window_dwarf(struct Data *data);
void refresh_window_memory(struct Data *data);
void refresh_window_tree(struct Data *data, unw_addr_space_t as, struct UPT_info *ui );
int keyboard_input(struct Data *data, WINDOW *win, vec_t *input);

void show_specific_panel(struct Data *data, int panel_amount, int panel_id){
    /**
     * @brief This function is called when the user want to select another window. Hence, 
     * the selected panel is showed and the others are hided, and the selector is set to another name to.
     * - Note that main_panel has no panel so there is an offset between my_items and the selector ID.
     */
    struct Interface *inter = data->inter ; 
    set_current_item(inter->my_menus, inter->my_items[panel_id - 1]); // main_window has no panel
    for (int i = 1; i < panel_id; i++)
    {
        hide_panel(inter->my_panels[i]);
    }
    for (int i = panel_id + 1; i < panel_amount + 1; i++)
    {
        hide_panel(inter->my_panels[i]);
    }
    show_panel(inter->my_panels[panel_id]);
    refresh();
    update_panels();
    doupdate();
}

void loop_execution(struct Data *data, vec_t *vp, int c, vec_t *input){
    /**
     * @brief This is where the input is interpreted. 
     * There are three cases :
     * - The key is standard : letters, numbers like a, r or 5. So the corresponding character is added to the prompt
     * - The key is a special key : F1, F2 etc. So the selected panel is shown
     * - The key is Enter : The text is send to the parser and a new line is started
     * - FUTURE : Add scroll / Pagedown Pageup to add scrolling support for windows.
     */

    struct Interface *inter = data->inter ; 
    WINDOW *mw = (WINDOW *)inter->main_window[0] ; 

    int panel_amount = 6;
    while (c != KEY_F(10)){
        //mvaddstr(1, 1, "while main");
        //refresh();
        switch (c){
        case KEY_F(1):
            show_specific_panel(data, panel_amount, 1); // be careful , main window in panel[0]
            update_panels();
            doupdate();
            wrefresh(inter->right_window[0]);
            c = keyboard_input(data, mw, input);
            break;
        // case 50:
        case KEY_F(2):
            show_specific_panel(data, panel_amount, 2); // be careful , main window in panel[0]
            update_panels();
            doupdate();
            wrefresh(inter->right_window[1]);
            c = keyboard_input(data, mw, input);
            break;
        // case 51:
        case KEY_F(3):
            show_specific_panel(data, panel_amount, 3); // be careful , main window in panel[0]
            update_panels();
            doupdate();
            wrefresh(inter->right_window[2]);
            c = keyboard_input(data, mw, input);
            break;
        // case 52:
        case KEY_F(4):
            show_specific_panel(data, panel_amount, 4); // be careful , main window in panel[0]
            update_panels();
            doupdate();
            wrefresh(inter->right_window[3]);
            c = keyboard_input(data, mw, input);
            break;
        // case 53:
        case KEY_F(5):
            show_specific_panel(data, panel_amount, 5); // be careful , main window in panel[0]
            update_panels();
            doupdate();
            wrefresh(inter->right_window[4]);
            c = keyboard_input(data, mw, input);
            break;
        case KEY_F(6):
            show_specific_panel(data, panel_amount, 6); // be careful , main window in panel[0]
            update_panels();
            doupdate();
            wrefresh(inter->right_window[5]);
            c = keyboard_input(data, mw, input);
            break;            
        }
        doupdate();
        refresh();
    }
}

void new_main_line(WINDOW *win){
    /**
     * @brief If the enter key is send, so the cursor is set to the next line with an additional space. 
     * This is because the first character is taken by tje box character.
     */
    wprintw(win, "\n");
    wprintw(win, " ");
    wrefresh(win);
}

int function_key(WINDOW *win, int key){
    /**
     * @brief If a special function is selected, so send true
     */
    if ((key == KEY_F(1)) || (key == KEY_F(2)) || (key == KEY_F(3)) || (key == KEY_F(4)) || (key == KEY_F(5)) || (key == KEY_F(6))){
        mvaddstr(1, 1, "specialkey");
        wrefresh(win);
        return 1;
    }
    else{
        return 0;
    }
}

int enter_key(WINDOW *win, int key){
    /**
     * @brief If the enter key (key == 10) is pressed, add a new line + send true.
     * 
     */
    if (key == 10){ 
        new_main_line(win);
        wrefresh(win);
        return 1;
    }
    else{
        return 0;
    }
}

int page_key(WINDOW *win, int key){
    /**
     * @brief if the PageUp or PageDown is pressed, scroll the page 
     *      (if implemented in this specific window) 
     */
    if ((key == KEY_PPAGE) || (key == KEY_NPAGE) || (key == KEY_HOME) || (key == KEY_END)){
        // scroll the current window :
        // need to implement function named "scroll_window(win, id, direction)"
        return 1 ; 
    }
    else{
        return 0 ; 
    }
}

int horizontal_key(WINDOW *win, int key){
    /**
     * @brief if the PageUp or PageDown is pressed, scroll the page 
     *      (if implemented in this specific window) 
     */
    if ((key == KEY_LEFT) || (key == KEY_RIGHT)){
        // scroll the current window :
        // need to implement function named "scroll_window(win, id, direction)"
        return 1 ; 
    }
    else{
        return 0 ; 
    }
}


void print_parsed(struct Data *data){
    /**
     * @brief print is main window the parsed text to control that the parser is 
     * working well and to be sure what is sended to the ptrace function
     */
    WINDOW *main_win = data->inter->main_window[0] ;\
    struct Debugger *d = data->debug ;  
    waddstr(main_win, "\n   Arguments list :  ");
    for (int j = 0; j < d->size_args + 1; j++){
        waddstr(main_win, d->args[j]);
        waddstr(main_win, "   ") ; 
    }
    waddstr(main_win, "\n   Options list :  ");
    for (int j = 0; j < d->size_opts; j++){
        waddstr(main_win, d->opts[j]);
        waddstr(main_win, "   ") ;         
    }
    waddstr(main_win, "\n\n ");
}

void get_pid(struct process_t *pid){
    /**
     * @brief Add to  specified process_t structure the pid, ppid and gid associated to the caller
     */
    pid->pid = (int)getpid();
    pid->ppid = (int)getppid();
    pid->gid = (int)getgid();
}

void print_pid(WINDOW *win, struct process_t *pid, int info){
    /**
     * @brief Show in a window in a standard mode the pid, ppid and gid of the 
     * specified structure
     */
    if (info == 0){
        waddstr(win, "Process fils : \n ");
    }
    else{
        waddstr(win, "Process père : \n ");
    }
    char tmp[10];
    sprintf(tmp, "%d", pid->pid);
    waddstr(win, "  PID : ");
    waddstr(win, tmp);
    new_main_line(win);
    sprintf(tmp, "%d", pid->ppid);
    waddstr(win, "  PPID : ");
    waddstr(win, tmp);
    new_main_line(win);
    sprintf(tmp, "%d", pid->gid);
    waddstr(win, "  GID : ");
    waddstr(win, tmp);
    new_main_line(win);
    refresh();
}

void get_maps(struct Data *data){
    /**
     * @brief get_maps is creating and filling the maps_info structure from the pid structure.
     * Hence, the function opens the following proc maps file and read on it. 
     *      We can place the old show_libraries function file parser on it.
     */
    struct Interface *inter = data->inter ; 
    WINDOW *win = (WINDOW *)inter->main_window[0] ; 
    struct maps_info *maps = data->maps ; 
    struct process_info *procs = data->procs ; 
    struct Debugger *debug = data->debug ; 
    struct process_t *process_child = procs->process_child ; 

    maps->start = malloc(sizeof(uint64_t)) ; 
    maps->stop = malloc(sizeof(uint64_t)) ; 
    maps->is_readable = malloc( sizeof(int) );
    maps->is_writable = malloc( sizeof(int) );
    maps->is_executable = malloc( sizeof(int) );
    maps->is_shared = malloc( sizeof(int) );
    maps->is_private = malloc( sizeof(int) );
    maps->line = malloc( sizeof(char *) ) ; 
    maps->libs = malloc( sizeof(char *) ) ;  
    maps->number_libs = 0 ; 
    maps->number_lines = 0 ;    

    // memory allocation for string manipulation
    // Data preallocated buffer
    char *buffpid = data->buff2 ; 
    char *buffpath  = data->buff64 ; 
    char *buffline = data->buff1024 ; 
    char *buffadress = data->buff64_2 ; 
    char *buffadress2 = data->buff64 ; 
    //char *buffchar = data->buff1;   
    //char *line = data->dbuff1024 ;   
    //char *buffpid = (char *)malloc(10 * sizeof(char));
    //char *buffpath = (char *)malloc(30 * sizeof(char));
    //char *buffline = (char *)malloc(1000 * sizeof(char));
    //char *buffadress = (char *)malloc(18 * sizeof(char));
    //char *buffadress2 = (char *)malloc(18 * sizeof(char));    
    char *buffchar = (char *)malloc(sizeof(char)) ; 
    char *line = malloc(1000 * sizeof(char *));

    // print the mapos localisation is the dedicated window
    sprintf(buffpid, "%d", process_child->pid);
    waddstr(win, "     --> Maps localisation :  ");
    sprintf(buffpath, "/proc/%s/maps", buffpid);
    waddstr(win, buffpath);
    waddstr(win, " \n");
    // open the maps file 


    //waddstr(win, "  lecture du fichier...\n ");

 
    FILE *fp = fopen(buffpath, "r");
    if (!fp){
        waddstr(win, "  XXX impossible d'afficher les librairies chargees XXX\n ");
    }
    else{

        
        //waddstr(win, "  --> Lecture du /proc/pid/maps ... \n ");
       // now we want to show loaded libraries We save line after line the maps file
        size_t line_buf_size = 0;
        maps->number_lines = 0;
        while (getline(&buffline, &line_buf_size, fp) > -1){
            maps->line[maps->number_lines] = malloc((line_buf_size + 1) * sizeof(char));
            strcpy(maps->line[maps->number_lines], buffline);
            maps->number_lines++;
            maps->line = realloc( maps->line , (maps->number_lines + 1) * sizeof(char *) ) ;             
        } 
        maps->line[maps->number_lines] = malloc((line_buf_size + 1) * sizeof(char));
        strcpy(maps->line[maps->number_lines], buffline);      
        maps->number_lines++;

      
        // we set the maps delimiter which is the space cgaracter
        const char delim[2] = " ";
        const char delim2[2] = "-";
        // we recognize a loaded library thanks to the slash character after a space
        char slash[2] = "/\0";
        char comp[2] = "\0\0";

        char *token;
        
 

        // for each lines of the proc maps
        for (uint64_t j = 0; j < maps->number_lines; j++){
            char *parsed_line[10];
            strcpy(line, maps->line[j]) ; 
            token = strtok((char *)line, delim);
            // parsed line is the line splited by the separator " ".
            // we are going to fullfill it
            parsed_line[0] = malloc(strlen(token) * sizeof(char));
            strcpy(parsed_line[0], token);
            uint64_t index = 0;


            // its the start-stop memory adress
            // we want to parse with the "-" separator 
            // proposition --> strncpy with 12 characters
            // BE CAREFUL : this is not portable !!!!!
            if ( strncmp(parsed_line[index]+12 , delim2, 1) != 0){
                break;
            } 
            strncpy(buffadress , parsed_line[index], 12) ;
            sprintf(buffadress2, "0x%s", buffadress) ;            
            maps->start[j] = strtoull(buffadress2, NULL, 0) ;    
            strncpy(buffadress , parsed_line[index]+13, 12) ; 
            sprintf(buffadress2, "0x%s", buffadress) ; 
            maps->stop[j] = strtoull(buffadress2, NULL, 0) ; 

            while (token != NULL){
                token = strtok(NULL, delim);
                if (token != NULL){
                    index++;
                    parsed_line[index] = malloc(strlen(token) * sizeof(char) + 1);
                    strcpy(parsed_line[index], token);   // copy data

                    strncpy(comp, parsed_line[index], 1);
                    
                    if (index == 1){
                        // its the rwxp information
                        strncpy(buffchar, parsed_line[index], 1) ;
                        if ( strcmp(buffchar, "r") == 1 ){
                            maps->is_readable[j] = 1 ; 
                        }
                        strncpy(buffchar, parsed_line[index]+1, 1) ; 
                        if ( strcmp(buffchar, "w") == 1 ){
                            maps->is_writable[j] = 1 ; 
                        }
                        strncpy(buffchar, parsed_line[index]+2, 1) ;
                        if ( strcmp(buffchar, "x") == 1 ){
                            maps->is_executable[j] = 1 ; 
                        }
                        strncpy(buffchar, parsed_line[index]+3, 1) ; 
                        if ( strcmp(buffchar, "p") == 1 ){
                            maps->is_private[j] = 1 ; 
                        }                                               
                    }
                    
                    // detection if its a library (begin with slash)
                    else if (strcmp(comp, slash) == 0){
                        // search if the loadd library already saved
                        uint64_t already_saved = 0;
                        // search if its an already finded library path
                        for (uint64_t k = 0; k < maps->number_libs ; k++){
                            if (strcmp(maps->libs[k], parsed_line[index]) == 0){
                                already_saved = 1;
                            }
                        }
                        // si le groupe commence par un backslash --> c'est une lib
                        
                        if (already_saved == 0){
                            // if (true){
                            maps->libs[maps->number_libs] = malloc( strlen(parsed_line[index]) * sizeof(char));
                            strcpy(maps->libs[maps->number_libs], parsed_line[index]);
                            maps->number_libs++;                                      
                            maps->libs = realloc( maps->libs, (maps->number_libs+1) *   sizeof(char *) ) ;
                        }
                        
                    }
                    
                }
            }
            // realloc ..
            maps->start = realloc(maps->start,  (j+2) *  sizeof(uint64_t)) ; 
            maps->stop = realloc( maps->stop, (j+2) *  sizeof(uint64_t)) ; 
            maps->is_readable = realloc( maps->is_readable, (j+2) *   sizeof(int) );
            maps->is_writable = realloc(maps->is_writable,  (j+2) *   sizeof(int) );
            maps->is_executable = realloc(maps->is_executable, (j+2) *   sizeof(int) );
            maps->is_shared = realloc(maps->is_shared, (j+2) *   sizeof(int) );
            maps->is_private = realloc(maps->is_private, (j+2) *   sizeof(int) );

        }
            
    }     
    //free(buffpid) ; 
    //free(buffpath);
    //free(buffadress);
    //free(buffadress2);
    free(buffchar);
    //free(buffline);
    
}

void get_codes(struct Data *data){
    WINDOW *win = data->inter->main_window[0] ;
    struct code_info *codes = data->codes ; 
    codes->count_func = count_func ; 
    codes->paths = malloc( count_func * sizeof(void*))  ; 
    codes->name = malloc( count_func * sizeof(void*))  ; 
    codes->line = malloc( sizeof(uint64_t) * count_func) ; 
    for (uint64_t i = 0 ; i < count_func ; i++){
        codes->paths[i] = malloc( strlen(func[i].path) ) ; 
        codes->name[i] = malloc( strlen(func[i].name) ) ; 
        strcpy(codes->paths[i] ,  func[i].path) ; 
        strcpy(codes->name[i] ,  func[i].name) ;
        codes->line[i] = func[i].line; 
    }
}

void get_backtrace(struct Data *data){
    struct backtrace_info *backs =  data->backs ; 
    backs->as = as ; 
    backs->ui = ui ; 
    unw_word_t ip, start_ip = 0, sp, off;
    int n = 0, ret;
    unw_cursor_t c;
    char buf[128];
    char buf2[150];
    size_t len;
    ret = unw_init_remote(&c, as, ui);
    char *buffspace[] = {" ", (char *)NULL};
    uint64_t size_names = 2 ; 
    uint64_t pass = 0 ; 
    backs->names = (char *)malloc( size_names * sizeof(char*)) ; 
    do{
        if (n == 0){
            start_ip = ip;
        }
        unw_get_proc_name(&c, data->buff64, 64, &off);
        sprintf(data->buff64_2, "%s", data->buff64);
        // copy to the files char**
        backs->names[pass] = malloc( strlen(data->buff64_2) ) ; 
        strcpy(backs->names[pass] , data->buff64_2) ; 
        backs->names = realloc(backs->names , ++size_names * sizeof(char *)) ; 
        ret = unw_step(&c);
        pass++;
        } while (ret > 0);
    backs->size = pass; 
    sprintf(data->buff64 , "     --> fetch %llu backtrace functions\n" , pass) ; 
    waddstr(data->inter->main_window[0] , data->buff64) ; 
    wrefresh(data->inter->main_window[0]);
    
}


void show_libraries_2(struct Data *data){
    struct Interface *inter = data->inter ;         
    struct maps_info *maps = data->maps ; 
    struct Debugger *debug = data->debug ; 
    WINDOW *win = (WINDOW *)inter->right_window[1] ; 

    if (debug->verbose == 1){
        waddstr(win, "\n\n\n  all lines : \n\n  ");
        for (int j = 0; j < maps->number_lines; j++){
            waddstr(win, maps->line[j]);
            waddstr(win, "    ");
        }
    }

    waddstr(win, "\n  Loaded libraries : \n\n  ");
    for (int j = 0; j < maps->number_libs; j++){
        waddstr(win, maps->libs[j]);
        waddstr(win, "  ");
    }
/*
    waddstr(win, "\n  start stop  : \n\n  ");
    char *buf = malloc(51) ; 
    for (int j = 0; j < maps.number_lines; j++){
        sprintf(buf, "%llx", maps.start[j]) ; 
        waddstr(win, buf);
        waddstr(win, "  ");
    }
    waddstr(win, "\n\n") ; 
    for (int j = 0; j < maps.number_lines; j++){
        sprintf(buf, "%llx\n", maps.stop[j]) ; 
        waddstr(win, buf);
        waddstr(win, "  ");
    }
    free(buf) ;
*/
    box(win, 0, 0);
    wrefresh(win);


}

void show_libraries(WINDOW *win, struct process_t *pid, int verbose){
    /**
     * @brief This function write the loaded libraries of a pid specified from process_t process. 
     * This parse the /proc/<pid>/maps file 
     * - we can select or not the verbose option. If verbnose is selected hence all the segments of the 
     *      maps is shown.
     */
    // allocate buffers for character manipulation
    char *buffpid = (char *)malloc(10 * sizeof(char));
    char *buffpath = (char *)malloc(30 * sizeof(char));
    char *buffline = (char *)malloc(1000 * sizeof(char));
    int size_line = 1000;
    int size_loaded_libs = 1000;
    char **line = malloc(size_line * sizeof(char *));
    char **loaded_libs = malloc(size_loaded_libs * sizeof(char *));
    // print the mapos localisation is the dedicated window
    sprintf(buffpid, "%d", pid->pid);
    waddstr(win, " \n\n  Maps localisation :  ");
    sprintf(buffpath, "/proc/%s/maps", buffpid);
    waddstr(win, buffpath);
    waddstr(win, " \n\n");
    // open the maps file 
    FILE *fp = fopen(buffpath, "r");
    if (!fp){
        waddstr(win, "impossible d'afficher les librairies chargees\n ");
    }
    else{
        // now we want to show loaded libraries We save line after line the maps file
        size_t line_buf_size = 0;
        int i = 0;
        while (getline(&buffline, &line_buf_size, fp) > -1){
            line[i] = malloc((line_buf_size + 1) * sizeof(char));
            strcpy(line[i], buffline);
            i++;
        }
        line[i] = malloc((line_buf_size + 1) * sizeof(char));
        strcpy(line[i], buffline);
        // we set the maps delimiter which is the space cgaracter
        const char delim[2] = " ";
        // we recognize a loaded library thanks to the slash character after a space
        char slash[2] = "/\0";
        char comp[2] = "\0\0";

        char *token;
        char *parsed_line[10];

        int number_loaded_lib = 0;
        waddstr(win, "   ");
        // for each line
        for (int j = 0; j < i; j++){
            // if verbose print the line
            if (verbose == 1){
                waddstr(win, line[j]);
                waddstr(win, "   ");
            }
            token = strtok((char *)line[j], delim);
            parsed_line[0] = malloc(strlen(token) * sizeof(char));
            strcpy(parsed_line[0], token);
            int index = 0;
            // for each information, see if its a library path
            while (token != NULL){
                token = strtok(NULL, delim);
                if (token != NULL){
                    // this while if need to be improved...
                    index++;
                    parsed_line[index] = malloc(strlen(token) * sizeof(char) + 1);
                    strcpy(parsed_line[index], token);
                    strncpy(comp, parsed_line[index], 1);
                    // if its a library path
                    if (strcmp(comp, slash) == 0){
                        // search if the loadd library already saved
                        int already_saved = 0;
                        // search if its an already finded library path
                        for (int k = 0; k < number_loaded_lib; k++){
                            if (strcmp(loaded_libs[k], parsed_line[index]) == 0){
                                already_saved = 1;
                            }
                        }
                        // si le groupe commence par un backslash --> c'est une lib
                        if (already_saved == 0){
                            // if (true){
                            loaded_libs[number_loaded_lib] = malloc((100 + 1) * sizeof(char));
                            strcpy(loaded_libs[number_loaded_lib], parsed_line[index]);
                            number_loaded_lib++;
                        }
                    }
                }
            }
        }
        // method to get the libs - little parser
        waddstr(win, "\n  Loaded libraries : \n\n  ");
        for (int j = 0; j < number_loaded_lib; j++){
            waddstr(win, loaded_libs[j]);
            waddstr(win, "  ");
        }
        box(win, 0, 0);
        wrefresh(win);
        fclose(fp);
        for (int j = 0; j < number_loaded_lib + 1; j++){
            free(loaded_libs[j]);
        }
        for (int j = 0; j < i + 1; j++){
            free(line[j]);
        }
    }
    // free all the previous mallpc
    free(buffpid);
    free(buffpath);
    free(buffline);
    free(loaded_libs);
    free(line);
}

void print_siginfo(struct Data *data, siginfo_t *signinf){
    /**
     * @brief Just a function to show the siginfo to the win 
     * 
     */

    WINDOW *main_win = data->inter->main_window[0] ; 
    struct regs_info *regs = data->regs ;
    char tmp2[100];
    char tmp3[100];
    char tmp4[100];
    sprintf(tmp2, "%s", strsignal(signinf->si_signo));
    sprintf(tmp3, "%d", signinf->si_signo);
    sprintf(tmp4, "%llx", regs->reg->rip);
    wattron(main_win, COLOR_PAIR(5)); 
    waddstr(main_win, "\n   Child stopped : ");
    waddstr(main_win, tmp2);
    waddstr(main_win, " (");
    waddstr(main_win, tmp3);
    waddstr(main_win, ") at adress 0x");
    waddstr(main_win, tmp4);
    wattroff(main_win, COLOR_PAIR(5)); 
    new_main_line(main_win);
    box(main_win, 0, 0) ; 
    wrefresh(main_win);
}



void config_debugger(struct Data *data){ 
    struct Debugger *d = data->debug ; 
    d->singlestep = 0;
    d->get_reg = 0;
    d->get_all_sig = 0;
    d->verbose = 0;
    for (int i = 0; i < d->size_opts; i++){
        if (strcmp(options[0], d->opts[i]) == 0){
            d->singlestep = 1;
        }
        if (strcmp(options[1], d->opts[i]) == 0){
            d->get_reg = 1;
        }
        if (strcmp(options[2], d->opts[i]) == 0){
            d->get_all_sig = 1;
        }
        if (strcmp(options[3], d->opts[i]) == 0){
            d->verbose = 1;
        }
    }
}

void get_memory(struct Data *data) ; 

static unw_addr_space_t as;
static struct UPT_info *ui;
void *spawn_thread(void *idata){
    /**
     * @brief This is the equivalent of the "main" function of a terminal based  ptracer. All the ptrace calls are made here. 
     * - input is from the input_thread structure (interface, args, options, ...)
     */
    struct Data *data = idata ; 
    struct Debugger *debug = data->debug ;
    struct Interface *inter = data->inter ; 
    struct process_info *procs = data->procs ; 
    WINDOW *main_win = inter->main_window[0];
    WINDOW *process_win = inter->right_window[1];

    struct process_t process_child;
    struct process_t process_father;
    init_process_t(&process_child) ; 
    init_process_t(&process_father) ; 
    data->procs->process_child = &process_child ; 
    data->procs->process_father = &process_father ;   

    struct user_regs_struct reg;
    struct user_fpregs_struct fpreg ; 
    data->regs->reg = &reg ; 
    data->regs->fpreg = &fpreg ; 
 
    print_parsed(data);
    config_debugger(data);

    as = unw_create_addr_space(&_UPT_accessors, 0);

    pid_t child_pid;
    int pid_status;
    child_pid = fork();

    if (child_pid == 0){
        // TRACEE 
        personality(ADDR_NO_RANDOMIZE);
        get_pid(data->procs->process_child);
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
            return printf("Error with ptrace, check the manual"), -2;
    }
    if (child_pid != 0){
        data->procs->process_child->pid = child_pid;
        get_pid(data->procs->process_father);

        vec_t *vp = vec_new(sizeof(struct process_t));
        vec_push(vp, procs->process_father);
        vec_push(vp, procs->process_child);
        procs->vp = vp ; 

        ui = _UPT_create(child_pid);
    }
    int is_executed = 1;
    if (child_pid == 0){
        personality(ADDR_NO_RANDOMIZE);
        execvp(debug->args[0], debug->args);
        is_executed = 0;
        _exit(-1);
        // is_executed set to -1 if execvp returned an error
    }
    if (is_executed == 0)
    {
        waddstr(main_win , "\n Issue : cannot trace this program") ;
    }
    if ((child_pid != 0) & (is_executed != 0))
    {
        int wait_status;
        // try to get the args without the ./
        // Note : don't work for smthing like 
        // ../src/test
        // solve : change at the level of get_dgb function
        char *path_for_dbg = malloc( sizeof(strlen(debug->args[0])) * sizeof(char)) ; 
            strcpy(path_for_dbg, debug->args[0] + 2);
        char *buff = malloc(128 * sizeof(char));        
        //sprintf(buff, "    modified path : %s\n",path_for_dbg);
        //waddstr(main_win, buff);
        wrefresh(main_win);
        //
        waddstr(main_win, "\n   Get dgb ...\n") ; 
        wrefresh(main_win) ;         
        get_dbg(path_for_dbg);
        waddstr(main_win, "   Done !\n") ; 
        wrefresh(main_win) ;                 
        free(path_for_dbg);
        if (count_func == 0){
            waddstr(main_win, "\n   Get elf ...\n") ; 
            wrefresh(main_win) ;             
            get_elf(debug->args[0]);
            waddstr(main_win, "   Done !\n") ; 
            wrefresh(main_win) ;             
        }
        long long prog_offset;
        char fname[128];

        char buff2[512];
        sprintf(fname, "/proc/%d/maps", child_pid);
        FILE *f = fopen(fname, "rb");
        fscanf(f, "%llx", &prog_offset);

        refresh_window_dwarf(data) ; 
        // Wait for the program (first execution)
        waitpid(child_pid, &wait_status, 0);


        char tmp[10];
        char tmp2[100];
        char tmp3[100];
        char tmp4[100];
        unsigned long long number_of_instructions = 0;
        int loop = 0;

        // add breakpoint
        if (debug->have_breakpoint)
        {
            if (debug->is_function == 0)
            {
                waddstr(main_win, "\n   Add address breakpoint ...\n") ; 
                sprintf(tmp2, "DEBUG:\tSetting a breakpoint on adress %s\n", debug->breakpoint_adress);
                waddstr(main_win, tmp2);
                Dwarf_Addr adr = strtoll(debug->breakpoint_adress, NULL, 16) + prog_offset;
                sprintf(tmp3, " Address avec offset : %llx\n", adr);
                waddstr(main_win, tmp3);
                // Add 3 to the adress

                u_int64_t w3 = (ptrace(PTRACE_PEEKDATA, child_pid, adr, 0) & ~0xff) | 0xcc;
                if (ptrace(PTRACE_POKEDATA, child_pid, adr, w3) < 0)
                {
                    sprintf(tmp2, "Wrong adress for the breakpoint");
                    waddstr(main_win, tmp2);
                }
                else{
                    waddstr(main_win, " Done !\n") ; 
                    wrefresh(main_win) ; 
                }
            }
            else
            {
                waddstr(main_win, "\n   Add function breakpoint ...\n") ; 
                for (int j = 0; j < count_func; j++)
                {
                    if (strcmp(debug->breakpoint_function, func[j].name) == 0)
                    {
                        sprintf(tmp3, " function name : %s\n", func[j].name);
                        waddstr(main_win, tmp3);
                        sprintf(tmp3, " offset : %llx\n", prog_offset);
                        waddstr(main_win, tmp3);
                        sprintf(tmp3, " lowpc : %llx\n", func[j].lowpc);
                        waddstr(main_win, tmp3);
                        Dwarf_Addr adr = func[j].lowpc + prog_offset;
                        // Add 3 to the adress
                        long long bef = (ptrace(PTRACE_PEEKDATA, child_pid, (void *)adr, 0) & ~0xff) | 0xcc;
                        sprintf(tmp3, " Address avec offset : %llx\n", adr);
                        waddstr(main_win, tmp3);
                        ptrace(PTRACE_POKEDATA, child_pid, (void *)adr, (void *)bef);
                    }
                }
                waddstr(main_win, "   Done !\n") ; 
            }
            ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
            waitpid(child_pid, &wait_status, 0);
        }
// add memory management
        if (debug->have_wmemory)
        {
            waddstr(main_win, "\n   Write in memory ...\n") ; 
            sprintf(tmp2, " DEBUG:\tWrite %s in memory on adress %s\n", debug->memory_content, debug->memory_adress);
            waddstr(main_win, tmp2);
            Dwarf_Addr adr = strtoll(debug->memory_adress, NULL, 16) + prog_offset;
            sprintf(tmp3, " Address with offset : %llx\n", adr);
            waddstr(main_win, tmp3);

            sprintf(tmp3, " Before : %llx\n", ptrace(PTRACE_PEEKDATA, child_pid, adr, 0));
            waddstr(main_win, tmp3);

            if (ptrace(PTRACE_POKEDATA, child_pid, adr, strtoll(debug->memory_content, NULL, 16)) < 0)
                waddstr(main_win, " Error with the adress you entered\n");
            else
                waddstr(main_win, " Content succesfully modified\n");
            sprintf(tmp3, " After : %llx\n", ptrace(PTRACE_PEEKDATA, child_pid, adr, 0));
            waddstr(main_win, tmp3);
        }

        // add write register
        if (debug->have_wregister)
        {
            sprintf(tmp2, "DEBUG:\tWrite %s on register %s\n", debug->register_content, debug->register_adress);
            waddstr(main_win, tmp2);

            set_register(debug->register_adress, child_pid, strtoll(debug->register_content, NULL, 16));
            waddstr(main_win, "Content succesfully modified\n");
        }
        wattron(main_win, COLOR_PAIR(6)); 
        waddstr(main_win, "\n   --- Start tracing ---\n") ;
        wattron(main_win, COLOR_PAIR(6)); 
        box(main_win, 0, 0); 
        wrefresh(main_win) ; 
        while (1)
        {

            if (WIFEXITED(wait_status) || WIFSIGNALED(wait_status))
                break;
            loop++;
            if (debug->singlestep == 1)
            {
                ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0);
                number_of_instructions++;
            }
            else
            {
                ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
            }
            waitpid(child_pid, &wait_status, 0);
            siginfo_t signinf;
            ptrace(PTRACE_GETSIGINFO, child_pid, NULL, &signinf);
            ptrace(PTRACE_GETREGS, child_pid, NULL, data->regs->reg);
            ptrace(PTRACE_GETFPREGS, child_pid, NULL, data->regs->fpreg) ; 

            if ((number_of_instructions % 10000 == 0) & debug->singlestep == 1)
            {
                refresh_window_register(data);
            }
            else
            {
                // refresh_window_register(i->inter);
            }
            if (WIFSTOPPED(wait_status))
            {
                if (signinf.si_signo != 5)
                {
                    print_siginfo(data, &signinf);
                    break;
                }
                else if ((signinf.si_signo == 5) && (debug->have_breakpoint))
                {
                    print_siginfo(data, &signinf);
                    break;
                }
            }
                    /////////
                    //get_backtrace(data) ;       
                    //refresh_window_tree(data, as, ui);
                    ////////
        }   
        waddstr(main_win, "\n   Get backtrace information ...\n") ; 
        wrefresh(main_win) ;         
        get_backtrace(data) ;       
        waddstr(main_win, "   Done !\n") ;  
        wrefresh(main_win) ;        

        //refresh_window_tree(data, as, ui);

        refresh_window_register(data);

        
        refresh_window_processes(data);

        waddstr(main_win, "\n   Get maps ...\n") ; 
        wrefresh(main_win) ;         
        get_maps(data) ; 
        waddstr(main_win, "   Done !\n") ;         
        get_codes(data) ; 
        refresh_window_code(data) ; 
        show_libraries_2(data) ;
        waddstr(main_win, "\n   Dump memory ...\n") ; 
        get_memory(data); 
        waddstr(main_win, "   Done !\n") ; 
        refresh_window_memory(data) ; 
        //show_libraries(process_win, &process_child, debug.verbose);
    
        free(buff);
    }

    pthread_exit(NULL);
}


void scroll_window(struct Data *data, int id, int direction ){
    struct Interface *inter = data->inter ;     
    WINDOW *main_win = (WINDOW *)inter->main_window[0] ; 

    if (id == 5){
        // it is the memory panel : we want to scroll it !!
        wrefresh(main_win) ; 
        inter->scroll_position_memory += direction*20 ; 
        refresh_window_memory(data) ; 
    }
    if (id == 2){
        // it is the memory panel : we want to scroll it !!
        wrefresh(main_win) ; 
        inter->scroll_position_register += direction/8 ; 
        refresh_window_register(data) ; 
    }

}

void horizontal_window(struct Data *data, int id, int direction ){
    struct Interface *inter = data->inter ;     
    WINDOW *main_win = (WINDOW *)inter->main_window[0] ; 

    if (id == 5){
        // it is the memory panel : we want to scroll it !!
        wrefresh(main_win) ; 
        inter->horizontal_position_memory += direction ; 
        refresh_window_memory(data) ; 
    }
    if (id == 2){
        // it is the memory panel : we want to scroll it !!
        wrefresh(main_win) ; 
        inter->horizontal_position_register += direction ; 
        refresh_window_register(data) ; 
    }    
    if (id == 3){
        // it is the memory panel : we want to scroll it !!
        wrefresh(main_win) ; 
        inter->horizontal_position_code += direction ; 
        refresh_window_code(data) ; 
    }       
}

void parse(struct Data *data, WINDOW *win, vec_t *input){
    struct Interface *inter = data->inter ; 
    struct Debugger *debug = data->debug ; 
    const char delim[2] = " ";

    /////////////////
    // etape de recuperation de l'input
    /////////////////

    char *token;
    char *parsed[20]; // we do not consider more than 20 args...
    char *opts[20];
    char *args[20];

    int i_p = 0; // nombre d'occurence
    int i_o = 0;
    int i_a = 0;

    token = strtok((char *)(&input->data)[0], delim);
    int is_valid = 1;
    if (token != NULL)
    {
        parsed[0] = malloc(strlen(token) * sizeof(char));
        i_p++;
        strcpy(parsed[0], token);
    }
    else
    {
        is_valid = 0;
    }

    while (token != NULL)
    {
        token = strtok(NULL, delim);
        if (token != NULL)
        {
            i_p++;
            parsed[i_p - 1] = malloc(strlen(token) * sizeof(char) + 1);
            strcpy(parsed[i_p - 1], token);
        }
    }

    ///////////////////
    // etape de parsing
    //////////////////

    int n_options = 4;
    // get the options & the args
    int is_an_option;
    for (int j = 1; j < i_p; j++)
    {
        is_an_option = 0;
        for (int k = 0; k < n_options; k++)
        {
            if (strcmp(options[k], parsed[j]) == 0)
            {
                i_o++;
                is_an_option = 1;
                opts[i_o - 1] = malloc((strlen(parsed[j]) + 1) * sizeof(char));
                strcpy(opts[i_o - 1], parsed[j]); // copie de l'option
            }
        }
        if (is_an_option == 0)
        {
            i_a++;
            args[i_a - 1] = malloc((strlen(parsed[j]) + 1) * sizeof(char));
            strcpy(args[i_a - 1], parsed[j]);
        }
    }
    char n[2] = "\0";
    for (int j = i_a; j < 20; j++)
    {
        args[j] = malloc(sizeof(char));
        args[j] = (char *)NULL;
    }

    // structure for the thread
    debug->have_breakpoint = 0;
    debug->have_rmemory = 0 ; 
    debug->have_wmemory = 0 ; 
    debug->have_wregister = 0 ; 
    debug->size_args = i_a;
    debug->size_opts = i_o;

    debug->opts = opts;
    debug->args = args;

    pthread_t thread;
    if (is_valid){
        /// for EXEC command ; simple execution
        if ((strcmp(command[0], parsed[0]) == 0) & (i_a > 0)){
            waddstr(win, "\n --> Start standard execution mode ... ");
            new_main_line(win);
            int thr = 1;
            debug->opts = opts;
            debug->args = args;
            pthread_create(&thread, NULL, spawn_thread, data);
            pthread_join(thread, NULL);
            wrefresh(win);
        }
        /// for BREAKPOINT command : define an explicit breakpoint
        if ((strcmp(command[1], parsed[0]) == 0) & (i_a > 1)){
            waddstr(win, "\n --> Start breakpoint execution mode ... ");
            new_main_line(win);
            int thr = 1;
            debug->have_breakpoint = 1;
            if (strcmp(parsed[1], "function") == 0){
                debug->breakpoint_function = malloc(strlen(parsed[2]) * sizeof(char));
                strcpy(debug->breakpoint_function, parsed[2]);
                debug->is_function = 1;
            }
            else if (strcmp(parsed[1], "adress") == 0){
                debug->breakpoint_adress = malloc(strlen(parsed[2]) * sizeof(char));
                strcpy(debug->breakpoint_adress, parsed[2]);
                debug->is_function = 0;
            }
            else{
                is_valid = 0;
            }
            if (is_valid == 1){
                for (int i = 0; i < i_a - 2; i++){
                    //(it.inter->main_window[0], &it) ;
                    strcpy(debug->args[i], debug->args[i + 2]);
                }
                debug->size_args -= 2;
                debug->args[i_a - 1] = (char *)NULL;
                debug->args[i_a - 1] = (char *)NULL;
                debug->args[i_a] = (char *)NULL;

                // print_parsed(it.inter->main_window[0], &it) ;

                pthread_create(&thread, NULL, spawn_thread, data);
                pthread_join(thread, NULL);
                wrefresh(win);
            }
        }

        if ((strcmp(command[2], parsed[0]) == 0) & (i_a > 1))
        {
            waddstr(win, "\n --> Start Memory execution mode ... ");
            new_main_line(win);
            int thr = 1;
            debug->have_wmemory = 1;
            debug->memory_adress = malloc(strlen(parsed[1]) * sizeof(char));
            strcpy(debug->memory_adress, parsed[1]);
            debug->memory_content = malloc(strlen(parsed[2]) * sizeof(char));
            strcpy(debug->memory_content, parsed[2]);

            if (is_valid == 1)
            {
                for (int i = 0; i < i_a - 2; i++)
                {
                    //(debug.inter->main_window[0], &debug) ;
                    strcpy(debug->args[i], debug->args[i + 2]);
                }
                debug->size_args -= 2;
                debug->args[i_a - 1] = (char *)NULL;
                debug->args[i_a - 1] = (char *)NULL;
                debug->args[i_a] = (char *)NULL;

                // print_parsed(it.inter->main_window[0], &it) ;

                pthread_create(&thread, NULL, spawn_thread, data);
                pthread_join(thread, NULL);
                wrefresh(win);
            }
        }
        if ((strcmp(command[3], parsed[0]) == 0) & (i_a > 1))
        {
            waddstr(win, "\n --> Start Register execution mode ... ");
            new_main_line(win);
            int thr = 1;
            debug->have_wregister = 1;
            debug->register_adress = malloc(strlen(parsed[1]) * sizeof(char));
            strcpy(debug->register_adress, parsed[1]);
            debug->register_content = malloc(strlen(parsed[2]) * sizeof(char));
            strcpy(debug->register_content, parsed[2]);
        }
        else
        {
            is_valid = 0;
        }
        if (is_valid == 1)
        {
            for (int i = 0; i < i_a - 2; i++)
            {
                //(it.inter->main_window[0], &it) ;
                strcpy(debug->args[i], debug->args[i + 2]);
            }
            debug->size_args -= 2;
            debug->args[i_a - 1] = (char *)NULL;
            debug->args[i_a - 1] = (char *)NULL;
            debug->args[i_a] = (char *)NULL;

            // print_parsed(it.inter->main_window[0], &it) ;

            pthread_create(&thread, NULL, spawn_thread, data);
            pthread_join(thread, NULL);
            wrefresh(win);
        }

    }




    if (is_valid == 0){
        wattron(win, COLOR_PAIR(5));
        waddstr(win, "\n    Please enter a valid command... \n");
        waddstr(win, "    Select Help panel for more information.\n");
        wattroff(win, COLOR_PAIR(5));        

    }

    ///////////////////
    // deallocation
    //////////////////

    for (int j = i_p - 1; j >= 0; j--){
        free(parsed[j]);
    }
    for (int j = i_o - 1; j >= 0; j--){
        free(opts[j]);
    }
    for (int j = i_a - 1; j >= 0; j--){
        free(args[j]);
    }
}

int keyboard_input(struct Data *data, WINDOW *win, vec_t *input){
    struct Interface *inter = data->inter ;     
    int key;
    while ((key = getch())){
        if (function_key(win, key)){
            break;
        }
        else if (enter_key(win, key)){

            // donnees a envoyer au parseur :
            char end = '\0';               // signal de terminaison de la saisie
            vec_push(input, (char *)&end); // le vecteur est rempli d caracteres a interpreter termin2 par null
            parse(data, win, input);
            vec_clear(input); // apres envoi au parseur, onvide le vec
        }
        else if(page_key(win, key)){
            // we need to interact with the rght window
            int direction ; 
            if (key == KEY_PPAGE){
                direction = -8 ; 
            }
            else if (key == KEY_NPAGE){
                direction = 8 ; 
            }
            else if (key == KEY_HOME){
                direction = -8 * 10 ; 
            }
            else if (key == KEY_END){
                direction = 8 * 10 ; 
            }                        
            //int id = 0 ;
            int id = item_index(current_item(data->inter->my_menus)) ;  
            //sprintf(data->buff16, "\n valeur de id : %i", id) ; 
            //waddstr(win, data->buff16) ; 
            scroll_window(data, id, direction) ; 
        }
        else if(horizontal_key(win, key)){
            int direction ; 
            if (key == KEY_LEFT){
                direction = -1 ; 
            }
            else if(key == KEY_RIGHT){
                direction = 1 ;
            }
            int id = item_index(current_item(data->inter->my_menus)) ; 
            horizontal_window(data, id, direction) ;
        }

        else{
            waddch(win, key);
            doupdate();
            wrefresh(win);
            vec_push(input, (char *)&key);
        }
        box(win, 0, 0);
        wrefresh(win);
    }
    return key;
}

void refresh_window_start(struct Data *data){
    struct Interface *inter = data->inter ;    
    // box(inter->right_window[0], 0, 0);
    // mvaddstr(1, 1, "show window start");
    // refresh();
    // mvwaddstr(inter->right_window[0], 2, 1, "start panel");
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    waddstr(inter->right_window[0], "\n\n");
    wattron(inter->title_window[0], COLOR_PAIR(1));
    waddstr(inter->right_window[0], "  COMMANDS  : \n\n");
    wattroff(inter->title_window[0], COLOR_PAIR(1));
    waddstr(inter->right_window[0], "     exec : Standard execution until the bug \n\n");
    waddstr(inter->right_window[0], "     breakpoint : Add a breakpoint to the code \n");
    waddstr(inter->right_window[0], "         breakpoint  function muFync\n");
    waddstr(inter->right_window[0], "         breakpoint  adress 0x00000000\n\n");
    wattron(inter->title_window[0], COLOR_PAIR(1));
    waddstr(inter->right_window[0], "  EXEMPLES : \n\n");
    wattroff(inter->title_window[0], COLOR_PAIR(1));
    waddstr(inter->right_window[0], "     exec  ./main arg1 arg2 opt1 \n");
    waddstr(inter->right_window[0], "     breakcpoint function myFunc ./main arg1 arg2 opt1 \n\n");
    wattron(inter->title_window[0], COLOR_PAIR(1));
    waddstr(inter->right_window[0], "  OPTIONS  : \n\n");
    wattroff(inter->title_window[0], COLOR_PAIR(1));
    waddstr(inter->right_window[0], "     -s  :  single step execution \n");
    waddstr(inter->right_window[0], "     -v  :  verbose \n\n");
    wattron(inter->title_window[0], COLOR_PAIR(1));
    waddstr(inter->right_window[0], "  KEYBOARD LAYOUT  : \n\n");
    wattroff(inter->title_window[0], COLOR_PAIR(1));
    waddstr(inter->right_window[0], "     F1 : Help \n");
    waddstr(inter->right_window[0], "     F2 : Processes \n");
    waddstr(inter->right_window[0], "     F3 : Register \n");
    waddstr(inter->right_window[0], "     F4 : Code \n");
    waddstr(inter->right_window[0], "     F5 : Dwarf \n");
    waddstr(inter->right_window[0], "     F6 : Memory \n");    
    waddstr(inter->right_window[0], "     Enter : Select current input");

    box(inter->right_window[0], 0, 0);
    wrefresh(inter->right_window[0]);
}

void refresh_window_processes(struct Data *data){
    struct Interface *inter = data->inter ;     
    struct Debugger *debug = data->debug ;
    struct process_info *procs = data->procs ; 
    vec_t *vp = procs->vp ;  
    
    WINDOW *w = (WINDOW *)inter->right_window[1];

    // box(inter->right_window[1], 0, 0) ;
    int c;
    // mvaddstr(1, 1, "show window processes");

    // mvwaddstr(inter->right_window[1], 2, 1, "process panel");
    // wrefresh(inter->right_window[1]);

    // affichage des processes
    wclear(w);
    // box(inter->right_window[1], 0, 0) ;
    // wrefresh(inter->right_window[1]) ;


    // first line : legendary line
    // other lines : show each processes of vec.
    int ncol, nrow;
    getmaxyx(w, nrow, ncol); // get the specific window size
    int nentity_col = 6;
    int width = ncol / nentity_col;
    int max_width = 10;
    if (width > max_width){
        width = 10;
    }
    // print the first title line

    wattron(w, COLOR_PAIR(1));
    char *symbol[1] = {" "};
    for (int i = 0; i < ncol - 2; i++){
        mvwaddstr(w, 1, i + 1, symbol[0]);
    }
    for (int i = 0; i < nentity_col; i++){
        if (width > 7){
            mvwaddstr(w, 1, 1 + width * i, entity_name[i]);
        }
        else{
            mvwaddstr(w, 1, 1 + width * i, reduced_entity_name[i]);
        }
    }
    wattroff(w, COLOR_PAIR(1));
    box(w, 0, 0);
    wrefresh(w);

    // we want to have an arborescence between processes : sort them.
    //

    // print each process line
    char tmp[10];
    for (int i = 0; i < vp->len; i++){
        sprintf(tmp, "%d", ((struct process_t *)vp->data)[i].pid);
        mvwaddstr(w, 2 + i, 1, tmp);
        sprintf(tmp, "%d", ((struct process_t *)vp->data)[i].ppid);
        mvwaddstr(w, 2 + i, 1 + width * 1, tmp);
        sprintf(tmp, "%d", ((struct process_t *)vp->data)[i].gid);
        mvwaddstr(w, 2 + i, 1 + width * 2, tmp);
        sprintf(tmp, "%d", ((struct process_t *)vp->data)[i].status);
        mvwaddstr(w, 2 + i, 1 + width * 3, tmp);
        sprintf(tmp, "%d", ((struct process_t *)vp->data)[i].num_threads);
        mvwaddstr(w, 2 + i, 1 + width * 4, tmp);
        sprintf(tmp, "%d", ((struct process_t *)vp->data)[i].memory);
        mvwaddstr(w, 2 + i, 1 + width * 5, tmp);
    }
    waddstr(w, "\n - not all features have been implemented yet - ");
    box(w, 0, 0);
    wrefresh(w);
}

void refresh_window_register(struct Data *data){
    struct Interface *inter = data->inter ;     
    WINDOW *w = (WINDOW *)inter->right_window[2] ; 
    struct regs_info *regs = data->regs ; 
    wclear(inter->right_window[2]);
    //update_panels();
    //waddstr(w, "\n\n  Register Visualisation \n\n ");
    sprintf(data->buff64, " rax      0x%llx", regs->reg->rax);
    mvwaddstr(w, 2, 1, data->buff64);

    sprintf(data->buff64, " rbx      0x%llx", regs->reg->rbx);
    mvwaddstr(w, 2, 30, data->buff64);

    sprintf(data->buff64, " rcx      0x%llx", regs->reg->rcx);
    mvwaddstr(w, 3, 1, data->buff64);

    sprintf(data->buff64, " rdx      0x%llx", regs->reg->rdx);
    mvwaddstr(w, 3, 30, data->buff64);

    sprintf(data->buff64, " rdi      0x%llx", regs->reg->rdi);
    mvwaddstr(w, 4, 1, data->buff64);

    sprintf(data->buff64, " rsi      0x%llx", regs->reg->rsi);
    mvwaddstr(w, 4, 30, data->buff64);

    sprintf(data->buff64, " rbp      0x%llx", regs->reg->rbp);
    mvwaddstr(w, 5, 1, data->buff64);

    sprintf(data->buff64, " rsp      0x%llx", regs->reg->rsp);
    mvwaddstr(w, 5, 30, data->buff64);

    sprintf(data->buff64, " r8       0x%llx", regs->reg->r8);
    mvwaddstr(w, 6, 1, data->buff64);

    sprintf(data->buff64, " r9       0x%llx", regs->reg->r9);
    mvwaddstr(w, 6, 30, data->buff64);

    sprintf(data->buff64, " r10      0x%llx", regs->reg->r10);
    mvwaddstr(w, 7, 1, data->buff64);

    sprintf(data->buff64, " r11      0x%llx", regs->reg->r11);
    mvwaddstr(w, 7, 30, data->buff64);
    sprintf(data->buff64, " r12      0x%llx", regs->reg->r12);
    mvwaddstr(w, 8, 1, data->buff64);

    sprintf(data->buff64, " r13      0x%llx", regs->reg->r13);
    mvwaddstr(w, 8, 30, data->buff64);

    sprintf(data->buff64, " r14      0x%llx", regs->reg->r14);
    mvwaddstr(w, 9, 1, data->buff64);

    sprintf(data->buff64, " r15      0x%llx", regs->reg->r15);
    mvwaddstr(w, 9, 30, data->buff64);

    sprintf(data->buff64, " rip      0x%llx", regs->reg->rip);
    mvwaddstr(w, 10, 1, data->buff64);

    sprintf(data->buff64, " rdx      0x%llx", regs->reg->rdx);
    mvwaddstr(w, 10, 30, data->buff64);

    sprintf(data->buff64, " eflags   0x%llx", regs->reg->eflags);
    mvwaddstr(w, 11, 1, data->buff64);

    sprintf(data->buff64, " cs       0x%llx", regs->reg->cs);
    mvwaddstr(w, 11, 30, data->buff64);

    sprintf(data->buff64, " orig_rax 0x%llx", regs->reg->orig_rax);
    mvwaddstr(w, 12, 1, data->buff64);

    sprintf(data->buff64, " fs_b     0x%llx", regs->reg->fs_base);
    mvwaddstr(w, 12, 30, data->buff64);

    sprintf(data->buff64, " gs_b     0x%llx", regs->reg->gs_base);
    mvwaddstr(w, 13, 1, data->buff64);

    sprintf(data->buff64, " fs_a     0x%llx", regs->reg->fs);
    mvwaddstr(w, 13, 30, data->buff64);

    sprintf(data->buff64, " gs_a     0x%llx", regs->reg->gs);
    mvwaddstr(w, 14, 1, data->buff64);

    sprintf(data->buff64, " ss       0x%llx", regs->reg->ss);
    mvwaddstr(w, 14, 30, data->buff64);

    sprintf(data->buff64, " ds       0x%llx", regs->reg->ds);
    mvwaddstr(w, 15, 1, data->buff64);

    sprintf(data->buff64, " es       0x%llx", regs->reg->es);
    mvwaddstr(w, 15, 30, data->buff64);

    // we want to print the right amount of xmms to fill the screen
    //    but without enfore the automatic window scrolling
    double fp_64 ; 
    float fp_32_1 ; 
    float fp_32_2 ; 
    uint64_t uint_64 ; 
    uint32_t uint_32_1 ; 
    uint32_t uint_32_2 ; 
    int64_t int_64 ; 
    int32_t int_32_1 ; 
    int32_t int_32_2 ; 
    char *char_8 = malloc( 8 ) ; 
    uint8_t *uint8 = malloc( 8 * sizeof(uint8_t)) ; 
    int8_t *int8 = malloc( 8 * sizeof(int8_t)) ; 
    uint16_t *uint16 = malloc( 4 * sizeof(uint16_t)) ;     
    int16_t *int16 = malloc( 4 * sizeof(int16_t)) ;      
    
    int ncol, nrow;
    getmaxyx(w, nrow, ncol);  
    int dx_of_xmm = nrow - 20 ; 
    int debut = data->inter->scroll_position_register ;  
    int horizontal = inter->horizontal_position_register ;     
    if (debut > 32){
        debut = 0 ; 
    }
    if (debut < 0){
        debut = 0 ; 
    }
    if (debut + dx_of_xmm > 32){
        dx_of_xmm = 32 - debut ; 
    }
    if ((horizontal < 0) | (horizontal > 4)){
        horizontal = 0 ; 
    }      
    char *symbol[1] = {" "} ;
    wattron(w, COLOR_PAIR(1)); 
    for (int i = 0; i < ncol - 2; i++){
        mvwaddstr(w, 17, i + 1, symbol[0]);
    }
    int nentity = nentity_horizontal[horizontal]  ; 
    int current_position = 1 ; 
    if(horizontal == 0){
        mvwaddstr(w, 17, current_position, memory_type_raw[0]) ;
        current_position += width_type_raw[0];
        mvwaddstr(w, 17, current_position, memory_type_raw[1]) ;
        current_position += width_type_raw[1];  
    }
    else if(horizontal == 1){
        for (int i = 0 ; i < nentity ; i++){
            mvwaddstr(w, 17, current_position, memory_type_fp[i]) ;
            current_position += width_type_fp[i];
        }        
    }    
    else if(horizontal == 2){
        for (int i = 0 ; i < nentity ; i++){
            mvwaddstr(w, 17, current_position, memory_type_int[i]) ;
            current_position += width_type_int[i];
        }        
    } 
    else if(horizontal == 3){
        for (int i = 0 ; i < nentity ; i++){
            mvwaddstr(w, 17, current_position, memory_type_char[i]) ;
            current_position += width_type_char[i];
        }        
    }        
    else if(horizontal == 4){
        for (int i = 0 ; i < nentity ; i++){
            mvwaddstr(w, 17, current_position, memory_type_int16[i]) ;
            current_position += width_type_int16[i];
        }        
    }   
    wattroff(w, COLOR_PAIR(1)); 

    // xmm register
    waddstr(w, " ") ; 
    //for (int i = debut ; i < debut + dx_of_xmm ; i++){
    //    sprintf(data->buff64, "xmm%i", i);
    //    mvwaddstr(w, 18 + i - debut, 1, data->buff64) ; 
    //    sprintf(data->buff64, "%016x" , regs->fpreg->xmm_space[i]) ; 
    //    mvwaddstr(w, 18 + i - debut , 10, data->buff64) ; 
    //}
    for (unsigned long long i = debut ; i < debut + dx_of_xmm ; i++){
        current_position = 1 ;  
        if (horizontal == 0){
            sprintf(data->buff64, "xmm%i", i);
            mvwaddstr(w, 18 + i - debut, 1, data->buff64) ; 
            sprintf(data->buff64, "%016x" , regs->fpreg->xmm_space[i]) ; 
            mvwaddstr(w, 18 + i - debut , 10, data->buff64) ;
        }
        else if (horizontal == 1){
            // fp values
            fp_64 =   (double)regs->fpreg->xmm_space[i] ; 
            fp_32_1 = (float)regs->fpreg->xmm_space[2*i] ; 
            fp_32_2 = (float)regs->fpreg->xmm_space[2*i + 1] ;  
            sprintf(data->buff128, "%.04e\n",fp_64);
            mvwaddstr(w, 18 + i - debut, current_position, data->buff128) ; 
            current_position += width_type_fp[0] ; 
            sprintf(data->buff128, "%.04e\n",fp_32_1);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
            current_position += width_type_fp[1] ; 
            sprintf(data->buff128, "%.04e\n",fp_32_2);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
        }
        else if(horizontal == 2){
            // intger values
            uint_64 = (uint64_t)regs->fpreg->xmm_space[i] ; 
            uint_32_1 = (uint32_t*)regs->fpreg->xmm_space[2 * i] ; 
            uint_32_2 = (uint32_t*)regs->fpreg->xmm_space[2 * i + 4] ;  
            int_64 = (int64_t)regs->fpreg->xmm_space[i] ; 
            int_32_1 = (int32_t*)regs->fpreg->xmm_space[2 * i] ; 
            int_32_2 = (int32_t*)regs->fpreg->xmm_space[2 * i + 4] ;             
            sprintf(data->buff128, "%lu\n",uint_64);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
            current_position += width_type_int[0] ; 
            sprintf(data->buff128, "%u\n",uint_32_1);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
            current_position += width_type_int[1] ; 
            sprintf(data->buff128, "%u\n",uint_32_2);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
            current_position += width_type_int[2] ;         
            sprintf(data->buff128, "%ld\n",int_64);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
            current_position += width_type_int[3] ; 
            sprintf(data->buff128, "%d\n",int_32_1);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
            current_position += width_type_int[4] ; 
            sprintf(data->buff128, "%d\n",int_32_2);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
            current_position += width_type_int[5] ;              
        }
        else if(horizontal == 3){
            for (int j = 0 ; j < 8 ; j++){
                char_8[j] = (char*)regs->fpreg->xmm_space[8 * i + j] ; 
                uint8[j] = (uint8_t*)regs->fpreg->xmm_space[8 * i + j] ; 
                int8[j] = (int8_t*)regs->fpreg->xmm_space[8 * i + j] ; 
            }          
            sprintf(data->buff128, "%s\n",char_8);
            mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
            current_position += width_type_char[0] ;
            for (int j = 0 ; j < 8 ; j++){
                sprintf(data->buff128, "%u\n",uint8[j]);
                mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
                current_position += width_type_char[j+1] ;
            } 
            for (int j = 0 ; j < 8 ; j++){
                sprintf(data->buff128, "%i\n",int8[j]);
                mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
                current_position += width_type_char[j+8+1] ;
            }             
        }
        else if(horizontal == 4){
            for (int j = 0 ; j < 4 ; j++){
                uint16[j] = (uint16_t*)regs->fpreg->xmm_space[4 * i + j] ; 
                int16[j] = (int16_t*)regs->fpreg->xmm_space[4 * i + j] ; 
            }            
            for (int j = 0 ; j < 4 ; j++){
                sprintf(data->buff128, "%u\n",uint16[j]);
                mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
                current_position += width_type_int16[j+1] ;
            }       
            for (int j = 0 ; j < 4 ; j++){
                sprintf(data->buff128, "%u\n",int16[j]);
                mvwaddstr(w, 18 + i - debut,  current_position, data->buff128) ; 
                current_position += width_type_int16[j+4+1] ;
            }                   
        }        

    }    

    free(char_8);
    free(uint8) ; 
    free(int8) ; 
    free(uint16) ; 
    free(int16) ; 
    box(w, 0, 0);
    wrefresh(w);
}

void refresh_window_tree(struct Data *data, unw_addr_space_t as, struct UPT_info *ui ){
    struct Interface *inter = data->inter ;     
    WINDOW *w = (WINDOW *)inter->main_window[0] ; 
    //wclear(w);
    unw_word_t ip, start_ip = 0, sp, off;
    int n = 0, ret;
    unw_cursor_t c;
    char buf[128];
    char buf2[150];
    size_t len;
    ret = unw_init_remote(&c, as, ui);
    char *buffspace[] = {" ", (char *)NULL};
    do{
        if (n == 0)
            start_ip = ip;
        //buf[0] = '\0';
        unw_get_proc_name(&c, buf, sizeof(buf), &off);
        sprintf(buf2, "%s\n", buf);
        waddstr(w,buf2) ;
        ret = unw_step(&c);
        waddstr(w, "");
        } while (ret > 0);        
    box(w, 0, 0) ; 
    wrefresh(w); 
}

void refresh_window_code(struct Data *data){

    struct Interface *inter = data->inter ;     
    WINDOW *w = (WINDOW *)inter->right_window[3] ; 
    struct code_info *codes = data->codes ; 
    struct backtrace_info *backs = data->backs ; 

    int selected_code = inter->horizontal_position_code ; 
    if (selected_code < 0){
        selected_code = 0 ; 
    }

    wclear(w);

    // first, we need to find the path code of the backtraced function. backtraced function is from undwind 
    // and path from dwarf. Hence, we do bad strcmp to find the path. This is CRAPPY ! we need a bijective relation
    // between them. 


    int finded_id = -1 ; 
    uint64_t index_dwarf = -1 ; 
    uint64_t index_unwind = -1 ; 
    int proposition  ;

    for (uint64_t i = 0 ; i < backs->size ; i++){
        proposition = 0 ; 
        for (uint64_t j = 0 ; j < codes->count_func ; j++){
            if ( strcmp( backs->names[i], codes->name[j] ) == 0 ){
                if (proposition == 0){
                    waddstr(w, "trouvé !!!") ; 
                    index_dwarf = j ; 
                    index_unwind = i ; 
                    //find_index = i ;                       
                }
                proposition = 1 ;  
            }
        }
        if (proposition == 1){
            finded_id++;
            if (finded_id == selected_code){
                break ; 
            }
        }
        
    }
    if ((index_dwarf < 0) | (index_unwind < 0)){
        waddstr(w, "impossible d'afficher le code...\n") ; 
        //waddstr(w, index_dwarf) ; 
        //waddstr(w, index_unwind) ; 
    }
    usleep(1000000);
    //data->buff64 = backs->names[]

    //data->buff64 = backs->names[selected_code] ; 
    // find the correspnding path from code_info
    /*
    int find_index = -1 ; 
    for (uint64_t i = 0 ; i < codes->count_func ; i++){
        sprintf(data->buff128 , "|%s|" , data->buff64 ) ; 
        waddstr(w, data->buff128) ; 
        waddstr(w, " ") ; 
        sprintf(data->buff128 , "|%s|" ,codes->name[i] ) ; 
        waddstr(w ,data->buff128  ) ; 
        waddstr(w, " \n ") ; 
        if ( strcmp( data->buff64, codes->name[i] ) == 0 ){
            waddstr(w, "trouvé !!!") ; 
            find_index = i ; 
            break ; 
        }
    }
    if (find_index == -1){
        return ; 
    }
    else{
        wclear(w) ; 
    }*/
    // the corresponding path is :
    data->buff128 = codes->paths[index_dwarf] ; 
    uint64_t line_to_print = codes->line[index_dwarf] - 1 ; 



    // we want to prijnt a specific part of the code where the bug occurs, or something else
    // protocole : we get a file, and a line
    // example : want to print line 54 in main.c --> we print from line 54-height/2 to 54+height/2
    //uint64_t line_to_print = codes->line[selected_code];
    int nrow, ncol;
    getmaxyx(w, nrow, ncol); // get the specific window size
    nrow--;
    int midheight = (int)(nrow / 2);
    int begin;
    if (midheight < line_to_print)
    {
        // we can naively print the code
        begin = line_to_print - midheight;
    }
    else
    {
        // we have to begin with the first line of code
        begin = 0;
    }

    // open the specific file
    FILE *fp;
    char *buff = malloc(ncol * sizeof(char));

    // length of the char line
    int numdigits = NumDigits(line_to_print + midheight);
    char *chline = malloc(numdigits * sizeof(char));

    waddstr(w,codes->paths[index_dwarf]);
    refresh() ; 
    fp = fopen(codes->paths[index_dwarf], "r");
    if (fp == NULL)
    {
        mvwaddstr(w, 1, 4, "cannot open the selected file...");
    }
    else{
        int iline = 0;
        int nline = 0;
        while ((nline < nrow - 1) && (fgets(buff, ncol, fp) != NULL))
        {
            mvaddstr(1, 1, "begin writing the line file ");
            if (iline >= begin)
            {

                // print the line
                sprintf(chline, "%d", iline + 1);
                // wattron(inter->right_window[3], COLOR_PAIR(1));
                mvwaddstr(w, 1 + nline, 1, chline);
                // wattroff(inter->right_window[3], COLOR_PAIR(1));

                if (nline + begin == line_to_print)
                {
                    char *symbol[1] = {" "};
                    wattron(w, COLOR_PAIR(1));
                    for (int i = 0; i < ncol - 2; i++)
                    {
                        mvwaddstr(w, 1 + nline, i + 2 + numdigits, symbol[0]);
                    }
                    mvwaddstr(w, 1 + nline, 2 + numdigits, buff);
                    wattroff(w, COLOR_PAIR(1));
                }
                else
                {
                    mvwaddstr(w, 1 + nline, 2 + numdigits, buff);
                }
                nline++;
            }
            iline++;
        }
    
        // close file
        fclose(fp);
    }

    box(w, 0, 0);
    wrefresh(w);
}

void refresh_window_dwarf(struct Data *data){
    struct Interface *inter = data->inter ;     
    WINDOW *w = (WINDOW *)inter->right_window[4] ; 
    waddstr(w, "DWARD Analysis : \n");
    sprintf(data->buff128, "  Function count : %d\n", count_func);
    waddstr(w, data->buff128);

    for (int ii = 0; ii < count_func; ii++)
    {
        sprintf(data->buff128, "    Function : %s %s at line %lld\n", func[ii].name, func[ii].path, func[ii].line);
        waddstr(w, data->buff128);
    }

    for (int ii = 0; ii < count_var; ii++)
    {
        sprintf(data->buff128, "    Variable  : %s dans func : ", var[ii].name);
        waddstr(w, data->buff128);
        sprintf(data->buff128, "%s\n", var[ii].funcname);
        waddstr(w, data->buff128);
    }
    box(w, 0, 0);
    wrefresh(w);
}
void get_memory(struct Data *data){
    struct Interface *inter = data->inter ;     
    struct maps_info *maps  = data->maps ; 
    struct memory_info *mems = data->mems; 
    //waddstr(inter->main_window[0] , "\n  Dump memory ...\n")  ;
    //wrefresh(inter->main_window[0]) ;

    //if (mems->adress->data != NULL){
    //    vec_clear(mems->adress) ; 
    //}
    //if (mems->value->data != NULL){
    //    vec_clear(mems->value) ; 
    //}    

    pid_t child_pid = data->procs->process_child->pid ; 
    uint64_t position_chunk = 0 ; 
    uint64_t chunk = 0 ; 
    uint64_t diff = maps->stop[chunk] - maps->start[chunk] ;  
    uint64_t adress ; 
    uint64_t value ; 
    uint64_t length ;
    //waddstr(inter->main_window[0] , "\n  begin for ...\n")  ;
    //wrefresh(inter->main_window[0]) ;

    for (uint64_t i = 0 ; i < maps->number_lines  ; i++){
        length = maps->stop[i] - maps->start[i] ; 
        for (uint64_t j = 0 ; j < length ; j++){

            // for every adress / data. push in the correspojding vectors
            adress = maps->start[i] + j ; 
            value = ptrace(PTRACE_PEEKDATA, child_pid, adress , 0) ; 
            vec_push(mems->adress , &adress) ;             
            vec_push(mems->value , &value ) ; 
        }
    }
    sprintf(data->buff64 , "     --> fetch %lu chunks !\n", maps->number_lines)  ;
    waddstr(inter->main_window[0], data->buff64) ; 
    box(inter->main_window[0], 0, 0) ; 
    wrefresh(inter->main_window[0]);
}


void refresh_window_memory(struct Data *data){
    WINDOW *win = data->inter->right_window[5] ; 
    wclear(win) ; 
    struct memory_info *mems = data->mems ; 
    unsigned long long offset = data->inter->scroll_position_memory ; 
    unsigned long long horizontal = data->inter->horizontal_position_memory ; 
    unsigned long long adress ; 
    unsigned long long value ; 
    double fp_64 ; 
    float fp_32_1 ; 
    float fp_32_2 ; 
    uint64_t uint_64 ; 
    uint32_t uint_32_1 ; 
    uint32_t uint_32_2 ; 
    int64_t int_64 ; 
    int32_t int_32_1 ; 
    int32_t int_32_2 ; 
    char *char_8 = malloc( 8 ) ; 
    uint8_t *uint8 = malloc( 8 * sizeof(uint8_t)) ; 
    int8_t *int8 = malloc( 8 * sizeof(int8_t)) ; 
    uint16_t *uint16 = malloc( 4 * sizeof(uint16_t)) ;     
    int16_t *int16 = malloc( 4 * sizeof(int16_t)) ;  

    struct All_window_size ws = compute_size_window() ; 
    int length = ws.right.dx - 3 ; 
    // test if length is > than the data to display
    if (offset < 0){
        offset = 0 ; 
    }      
    if (length > (mems->value->len - offset)  ){
        length = mems->value->len - offset ; 
    }
    if (horizontal < 0){
        horizontal = 0 ; 
    }   

    // show the white line with memory types
    wattron(win, COLOR_PAIR(1));     
    int ncol, nrow;
    getmaxyx(win, nrow, ncol);    
    char *symbol[1] = {" "} ;
    for (int i = 0; i < ncol - 2; i++){
        mvwaddstr(win, 1, i + 1, symbol[0]);
    }
    int nentity = nentity_horizontal[horizontal]  ; 
    int current_position = 1 ; 
    if(horizontal == 0){
        mvwaddstr(win, 1, current_position, memory_type_raw[0]) ;
        current_position += width_type_raw[0];
        mvwaddstr(win, 1, current_position, memory_type_raw[1]) ;
        current_position += width_type_raw[1];  
    }
    else if(horizontal == 1){
        for (int i = 0 ; i < nentity ; i++){
            mvwaddstr(win, 1, current_position, memory_type_fp[i]) ;
            current_position += width_type_fp[i];
        }        
    }    
    else if(horizontal == 2){
        for (int i = 0 ; i < nentity ; i++){
            mvwaddstr(win, 1, current_position, memory_type_int[i]) ;
            current_position += width_type_int[i];
        }        
    } 
    else if(horizontal == 3){
        for (int i = 0 ; i < nentity ; i++){
            mvwaddstr(win, 1, current_position, memory_type_char[i]) ;
            current_position += width_type_char[i];
        }        
    }        
    else if(horizontal == 4){
        for (int i = 0 ; i < nentity ; i++){
            mvwaddstr(win, 1, current_position, memory_type_int16[i]) ;
            current_position += width_type_int16[i];
        }        
    }      


    wattroff(win, COLOR_PAIR(1));
    waddstr(win, "\n") ; 

    ////////
    ////////
    for (unsigned long long i = 0 ; i < length ; i++){
        unsigned long long position = offset + 8 * i  ; // 64 bits 
        if (position > mems->value->len){
            mvwaddstr(win, 1,1, " Please scroll up !!!\n " ) ; 
            break;
        }
        adress = ((unsigned long long*)mems->adress->data)[position];
        value = ((unsigned long long*)mems->value->data)[position] ; 

        current_position = 1 ;  

        if (horizontal == 0){
            sprintf(data->buff128,  "0x%llx %016llx",  adress, value) ; 
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            //current_position += width_type[0] + width_type[1] ; 
        }
        else if (horizontal == 1){
            // fp values
            fp_64 = (double)value ; 
            fp_32_1 = ((float*)mems->value->data)[2 * position] ; 
            fp_32_2 = ((float*)mems->value->data)[2 * position + 4] ;  
            sprintf(data->buff128, "%.04e\n",fp_64);
            mvwaddstr(win, i + 2, current_position, data->buff128) ; 
            current_position += width_type_fp[0] ; 
            sprintf(data->buff128, "%.04e\n",fp_32_1);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            current_position += width_type_fp[1] ; 
            sprintf(data->buff128, "%.04e\n",fp_32_2);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
        }
        else if(horizontal == 2){
            // intger values
            uint_64 = (uint64_t)value ; 
            uint_32_1 = ((uint32_t*)mems->value->data)[2 * position] ; 
            uint_32_2 = ((uint32_t*)mems->value->data)[2 * position + 4] ;  
            int_64 = (int64_t)value ; 
            int_32_1 = ((int32_t*)mems->value->data)[2 * position] ; 
            int_32_2 = ((int32_t*)mems->value->data)[2 * position + 4] ;             
            sprintf(data->buff128, "%lu\n",uint_64);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            current_position += width_type_int[0] ; 
            sprintf(data->buff128, "%u\n",uint_32_1);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            current_position += width_type_int[1] ; 
            sprintf(data->buff128, "%u\n",uint_32_2);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            current_position += width_type_int[2] ;         
            sprintf(data->buff128, "%ld\n",int_64);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            current_position += width_type_int[3] ; 
            sprintf(data->buff128, "%d\n",int_32_1);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            current_position += width_type_int[4] ; 
            sprintf(data->buff128, "%d\n",int_32_2);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            current_position += width_type_int[5] ;              
        }
        else if(horizontal == 3){
            for (int j = 0 ; j < 8 ; j++){
                char_8[j] = ((char*)mems->value->data)[8 * position + j] ; 
                uint8[j] = ((uint8_t*)mems->value->data)[8 * position + j] ; 
                int8[j] = ((int8_t*)mems->value->data)[8 * position + j] ; 
            }          
            //for (int j = 0 ; j < 8 ; j++){
            //    strncpy(data->buff2 , char_8[j], 1) ; 
            //    //sprintf(data->buff16, "%s", char_8[j]);
            //    mvwaddstr(win, i + 2,  current_position + j, data->buff2) ;
            //}
            sprintf(data->buff128, "%s\n",char_8);
            mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
            current_position += width_type_char[0] ;
            for (int j = 0 ; j < 8 ; j++){
                sprintf(data->buff128, "%u\n",uint8[j]);
                mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
                current_position += width_type_char[j+1] ;
            } 
            for (int j = 0 ; j < 8 ; j++){
                sprintf(data->buff128, "%i\n",int8[j]);
                mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
                current_position += width_type_char[j+8+1] ;
            }             
        }
        else if(horizontal == 4){
            for (int j = 0 ; j < 4 ; j++){
                uint16[j] = ((uint16_t*)mems->value->data)[4 * position + j] ; 
                int16[j] = ((int16_t*)mems->value->data)[4 * position + j] ; 
            }            
            for (int j = 0 ; j < 4 ; j++){
                sprintf(data->buff128, "%u\n",uint16[j]);
                mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
                current_position += width_type_int16[j+1] ;
            }       
            for (int j = 0 ; j < 4 ; j++){
                sprintf(data->buff128, "%u\n",int16[j]);
                mvwaddstr(win, i + 2,  current_position, data->buff128) ; 
                current_position += width_type_int16[j+4+1] ;
            }                   
        }        

    }
    free(char_8);
    free(uint8) ; 
    free(int8) ; 
    free(uint16) ; 
    free(int16) ; 
    box(win,0,0) ; 
    wrefresh(win) ; 
}


// this is the way the main window is drawn
int show_window_start(struct Data *data, WINDOW *win, WINDOW *main_win, vec_t *input){
    struct Interface *inter = data->inter ;     
    refresh_window_start(data);
    int c;
    c = keyboard_input(data, main_win, input);
    // mvaddstr(1, 1, "exit window start");
    refresh();
    return c;
}

int main(int argc, char **argv){
    printf("This is the Ncurses sbstndbs debugger ! \n");

    struct Data data  ; 
    struct Interface inter ;
    inter.scroll_position_memory  = 0 ;
    inter.horizontal_position_memory = 0 ; 
    inter.scroll_position_register  = 0 ;
    inter.horizontal_position_register = 0 ;   
    inter.horizontal_position_code = 0 ;         
    struct Debugger debug;
    struct maps_info maps ; 
    struct regs_info regs ; 
    struct process_info procs ;
    struct memory_info mems ;  
    struct code_info codes; 
    struct backtrace_info backs ; 

    mems.value = vec_new( sizeof(unsigned long long) ) ; 
    mems.adress = vec_new( sizeof(unsigned long long) ) ; 

    data.buff1 = (char *)malloc(1) ;     
    data.buff2 = (char *)malloc(2) ; 
    data.buff2_2 = (char *)malloc(2) ;    
    data.buff16 = (char *)malloc(16) ; 
    data.buff32 = (char *)malloc(32) ;   
    data.buff32_2 = (char *)malloc(32) ;       
    data.buff64 = (char *)malloc(64) ; 
    data.buff64_2 = (char *)malloc(64) ; 
    data.buff64_3 = (char *)malloc(64) ;        
    data.buff128 = (char *)malloc(128) ;         
    data.buff512 = (char *)malloc(512) ; 
    data.buff1024 = (char *)malloc(1024) ;     
    data.buff1024_2 = (char *)malloc(1024) ;      
    data.dbuff64 = (char**)malloc(64 * sizeof(char *)) ;     
    data.dbuff1024 = (char**)malloc(1024 * sizeof(char *)) ;   


    data.inter = &inter ; 
    data.debug = &debug ; 
    data.maps = &maps ; 
    data.regs = &regs ; 
    data.procs = &procs ; 
    data.mems = &mems ; 
    data.codes = &codes ; 
    data.backs = &backs ; 


     
    // compute dimensions of interface
    struct All_window_size ws = compute_size_window();

    initscr();
    //newterm();
    cbreak();
    noecho();
    curs_set(0);

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_BLACK, COLOR_RED);    
    init_pair(4, COLOR_RED, COLOR_WHITE);
    init_pair(5, COLOR_RED, COLOR_BLACK);    
    init_pair(6, COLOR_GREEN, COLOR_BLACK);  

    int n_choice = sizeof(choice_panel) / sizeof(choice_panel[0]);



    setup_selector(&data);
    setup_window(&data, &ws);
    draw_box(&data);
    setup_panel(&data);
    setup_title(&data);
    setup_menu(&data, &ws);

    keypad(stdscr, TRUE);
    scrollok(inter.main_window[0], TRUE);
    for (int i = 0; i < 5; i++)
    {
        scrollok(inter.right_window[i], TRUE);
    }

    refresh();
    update_panels();



    vec_t *vp = generate_processes(); // to add processes. Need to be obtain from branch/features

    keypad(inter.main_window[0], TRUE);

    new_main_line(inter.main_window[0]);

    vec_t *input = vec_new(sizeof(char));
    struct user_regs_struct reg;
    int c;


    box(inter.main_window[0], 0, 0);
    wrefresh(inter.main_window[0]);

    c = show_window_start(&data, inter.right_window[0], inter.main_window[0], input); // c est la touche d'interraction pressee
    // TODO: utiliser l'execution conditionelle au debug (compil) des mvwaddnstr
    //  qui servent au debug uniquement

    update_panels();
    doupdate();
     refresh();

    // here is where the interface & the debugger is working
    loop_execution(&data, vp, c, input);
    ////

    doupdate();
    getch();
    free_item(inter.my_items[0]);
    free_menu(inter.my_menus);
    vec_drop(input);
    
    endwin();
    
    free(data.buff1) ; 
    free(data.buff2) ; 
    free(data.buff2_2) ;     
    free(data.buff16) ;
    free(data.buff32) ; 
    free(data.buff32_2) ;     
    free(data.buff64) ; 
    free(data.buff64_2) ; 
    free(data.buff64_3) ;         
    free(data.buff128) ; 
    free(data.buff256) ; 
    free(data.buff512) ; 
    free(data.buff1024) ;     
    free(data.buff1024_2) ;       
    free(data.dbuff64) ; 
    free(data.dbuff1024) ;     
}