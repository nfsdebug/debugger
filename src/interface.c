
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#include <panel.h>
#include <menu.h>
#include <form.h>
#include "../ext/vec/src/vec.h"



char *choice_panel[] = { "Start", "Processes", "Memory", "Others", (char *)NULL, } ; 
char *panel_func[] = { " ", " ", " ", " ", (char *)NULL, } ; 
char *command[] = { "exe", "value", "adress", "reg",   (char *)NULL} ;



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




// start_interface
struct Interface{
    // first : windows
    WINDOW *main_window[1] ;
    WINDOW *right_window[4] ; 
    WINDOW *title_window[1] ; 
    WINDOW *selector_window[1] ;  
    PANEL *my_panels[7] ;
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
    size.title.dy = 23 ;
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



void setup_selector(struct Interface *inter){
    int n_choice = sizeof(choice_panel) / sizeof(choice_panel[0]) ; 
    inter->my_items = (ITEM **)calloc(n_choice  , sizeof(ITEM *)) ; 

    for (int i = 0 ; i < n_choice ; i++){
        inter->my_items[i] = new_item(panel_func[i], choice_panel[i]);
    }
    inter->my_items[n_choice] = (ITEM *)NULL ;

    inter->my_menus= new_menu((ITEM **)inter->my_items) ;
}

void setup_window(struct Interface *inter, struct All_window_size *ws){
    for (int i = 0 ; i < 4 ; i++){
        inter->right_window[i] = newwin(ws->right.dx, ws->right.dy, ws->right.x, ws->right.y) ; 
    }
    inter->main_window[0] = newwin(ws->main.dx, ws->main.dy, ws->main.x, ws->main.y) ; 
    inter->title_window[0] = newwin(ws->title.dx, ws->title.dy, ws->title.x, ws->title.y) ; 
    inter->selector_window[0] = newwin(ws->selector.dx, ws->selector.dy, ws->selector.x, ws->selector.y) ;    
}

void draw_box(struct Interface *inter){
     box(inter->main_window[0], 0, 0) ;     
    for (int i = 0 ; i < 4 ; i++){
        box(inter->right_window[i], 0, 0) ;     
    }   
   
}

void setup_panel(struct Interface *inter){
    inter->my_panels[0] = new_panel(inter->main_window[0]) ;    
     for(int i = 0 ; i < 4 ; i++){
        inter->my_panels[i+1] = new_panel(inter->right_window[i]) ;
    }
    inter->my_panels[5] = new_panel(inter->title_window[0]) ;
    inter->my_panels[6] = new_panel(inter->selector_window[0]) ;   
}

void setup_title(struct Interface *inter){
    char title[] = "NCurses debugger v0.1" ;
    mvwprintw(inter->title_window[0], 0, 0, "%s", title) ;     
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
int show_window_start(WINDOW *start, WINDOW *main, vec_t *input);
int show_window_processes(WINDOW *processes, WINDOW *main, vec_t *vp, vec_t *input);
int show_window_memory(WINDOW *memory, WINDOW *main, vec_t *input);
int show_window_others(WINDOW *others, WINDOW *main, vec_t *input);

void loop_execution(struct Interface *inter, vec_t *vp, int c, vec_t *input){
    while(c != KEY_F(10)){
        mvaddstr(1, 1, "while main"); 
        refresh();
        switch(c){
            //case 49:   
            case KEY_F(1):
            mvaddstr(1, 1, "F1 pressed ");  
                refresh();
                set_current_item(inter->my_menus, inter->my_items[0]);
                hide_panel(inter->my_panels[2]);
                hide_panel(inter->my_panels[3]);
                hide_panel(inter->my_panels[4]);
                show_panel(inter->my_panels[1]);    
                refresh();                
                update_panels();  
                doupdate();       
                c = show_window_start(inter->right_window[0], inter->main_window[0], input) ; 
                break;
            //case 50:
            case KEY_F(2):             
            mvaddstr(1, 1, "F2 pressed ");    
                refresh(); 
                hide_panel(inter->my_panels[1]);
                hide_panel(inter->my_panels[3]);
                hide_panel(inter->my_panels[4]);
                show_panel(inter->my_panels[2]);        
                refresh();        
                update_panels();
                doupdate();                              
                set_current_item(inter->my_menus, inter->my_items[1]);
                c = show_window_processes(inter->right_window[1], inter->main_window[0], vp, input) ;
                break;
            //case 51:
            case KEY_F(3):            
            mvaddstr(1, 1, "F3 pressed ");      
                refresh();
                hide_panel(inter->my_panels[1]);
                hide_panel(inter->my_panels[2]);
                hide_panel(inter->my_panels[4]);
                show_panel(inter->my_panels[3]);        
                refresh();        
                update_panels();  
                doupdate();                                     
                set_current_item(inter->my_menus, inter->my_items[2]);
                c =show_window_memory(inter->right_window[2], inter->main_window[0], input) ;
                break;                
            //case 52:  
            case KEY_F(4):            
                mvaddstr(1, 1, "F4 pressed ");   
                    refresh(); 
                set_current_item(inter->my_menus, inter->my_items[3]);
                c = show_window_others(inter->right_window[3], inter->main_window[0], input) ;
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
        if ((key == KEY_F(1)) || (key == KEY_F(2)) || (key == KEY_F(3)) || (key == KEY_F(4))){
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


void parse(WINDOW *win, vec_t *input){
	    waddstr(win , "\n vous avew saisi :") ; 
	    new_main_line(win) ;  
	    waddstr(win, (char *)(&input->data)[0] ) ; // affichage de la siason en guise de verif
	    new_main_line(win) ;  
	    // split method from internet hint
	    const char *sep = " " ; 
	    char * strToken = strtok((char *)(&input->data)[0] , sep ) ; 
	    if ( strcmp(strToken, command[0]) ){
	    	waddstr(win, "Lancement de l'execution") ; 
	    	new_main_line(win) ; 
	    }
	    while (strToken != (char *)NULL){
	    	//waddstr(win,strToken) ;
	    	//new_main_line(win);  
	    	strToken = strtok (NULL, sep) ; 
	    }			
	    

}

int keyboard_input(WINDOW *win, vec_t *input){
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
	    parse(win, input) ; 
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

// this is the way the main window is drawn
int show_window_start(WINDOW *win, WINDOW *main_win, vec_t *input){
    keypad(win, FALSE);
    box(win, 0, 0) ; 
    //attron(A_BLINK); 

    // ecrire les caracteres saisis autre que les touches fonctions. SI les touches fonctions sont saisies : on sort du while 
    // et il faudrat retourner un code d'erreur 
    int c ;

    int id;
    char text[1];
    mvaddstr(1, 1, "show window start");
    refresh();
    mvwaddstr(win, 2, 1, "sart panel");
    wrefresh(win);   

// test de scroll
    scrollok(main_win, TRUE) ; 
    int i = 0 ; 
    /*
    while( i < 30){   

        wprintw(main_win, "this is another line\n") ; 
        wrefresh(main_win) ; 
        i++;
    }

    while( i > 0){   

        wprintw(main_win, "this is another loop line\n") ; 
        wrefresh(main_win) ; 
        i--;
    }
    */
 
    //int espace = 32 ;
    //waddch(main_win, espace) ;  
    wrefresh(main_win) ; 


	// allocation of the buffer input with libvec
	//char *single = (char *)malloc( sizeof(char) );
	/* some test
	char *input = (char *)malloc( 4 * sizeof(char) ) ; 
	vec_t *input2 = vec_with_capacity( 100 , sizeof(char)) ; 
	
	int size_input = 2 ; 
	for (int i = 0 ; i < 1 ; i+=3){
		input[i] = 'a' ; 
		input[i+1] = 'b' ; 	
		input[i+2] = 'c' ;
		input[i+3] = '\0' ;
		((char*)input2->data)[0] = 'd' ; 
		((char*)input2->data)[1] = 'e' ; 
		((char*)input2->data)[2] = 'f' ; 
		((char*)input2->data)[3] = '\0' ; 		
		
	} 	
	*/
	//waddstr(main_win , (const char *)&input[0]) ; 
	//waddstr(main_win , (char *)(&input2->data)[0] ); 	
	//free(input) ; 
	//waddstr(main_win , &input[1]) ; 	
	wrefresh(main_win) ; 


    c = keyboard_input(main_win, input) ; 
    
    

    

    mvaddstr(1, 1, "exit window start"); 
    refresh();
     return c ; 
}


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

int show_window_processes(WINDOW *win, WINDOW *main_win, vec_t *vp , vec_t *input){
    box(win, 0, 0) ;  
    update_panels();
    refresh();
    refresh();
    int c ;
    mvaddstr(1, 1, "show window processes");
    refresh();
    mvwaddstr(win, 2, 1, "processes panel");
    wrefresh(win);


    // affichage des processes
    wclear(win); 
    box(win, 0, 0) ; 
    wrefresh(win) ; 

    char *entity_name[] = { "PID", "PPID", "GID", "status", "threads", "Memory", (char *)NULL, } ; 
    char *reduced_entity_name[] = { "PID", "PPID", "GID", "stat.", "th.", "Mem.", (char *)NULL, } ; 
    // first line : legendary line
    // other lines : show each processes of vec.
    int ncol, nrow ; 
    getmaxyx(win, nrow, ncol) ; // get the specific window size
    int nentity_col = 6 ; 
    int width = ncol / nentity_col ; 
    int max_width = 10 ; 
    if (width > max_width){
        width = 10 ; 
    }
    // print the first title line 

 

    wattron(win, COLOR_PAIR(1));
    char *symbol[1] = {" "} ; 
    for (int i = 0 ; i < ncol - 2 ; i++){
        mvwaddstr(win, 1,i + 1, symbol[0]) ;
    }
    for (int i = 0 ; i < nentity_col; i++)
    {
        if (width > 7){
            mvwaddstr(win, 1, 1 + width * i, entity_name[i]) ; 
        }
        else{
            mvwaddstr(win, 1, 1 + width * i, reduced_entity_name[i]) ;            
        }
    }
    wattroff(win, COLOR_PAIR(1));
    wrefresh(win) ;


// we want to have an arborescence between processes : sort them. 
// 

    // print each process line
    char tmp[10] ; 
    for (int i = 0 ; i < vp->len ; i++){
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].pid) ; 
            mvwaddstr(win, 2 + i, 1, tmp  ) ;   
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].ppid) ;               
            mvwaddstr(win, 2 + i, 1 + width * 1, tmp  ) ;           
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].gid) ;               
            mvwaddstr(win, 2 + i, 1 + width * 2, tmp  ) ;     
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].status) ;               
            mvwaddstr(win, 2 + i, 1 + width * 3, tmp  ) ;     
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].num_threads) ;               
            mvwaddstr(win, 2 + i, 1 + width * 4, tmp  ) ;     
            sprintf(tmp, "%d", ((struct process_t*)vp->data)[i].memory) ;               
            mvwaddstr(win, 2 + i, 1 + width * 5, tmp  ) ;                                                                                                      
    }
    wrefresh(win) ;
    



    //show_process_list(vp) ; 
vec_t *input2 = vec_with_capacity( 100 , sizeof(char)) ; 
    c = keyboard_input(main_win, input2) ; 
    mvaddstr(1, 1, "exit window processes"); 
    refresh();
     return c ; 


}

int show_window_memory(WINDOW *win, WINDOW *main_win, vec_t *input){
    box(win, 0, 0) ;  
    update_panels();
    int c ; 
    mvaddstr(1, 1, "show window memory");
    refresh();
    mvwaddstr(win, 2, 1, "memory panel");
    wrefresh(win);    
    
vec_t *input2 = vec_with_capacity( 100 , sizeof(char)) ;     
    c = keyboard_input(main_win, input2) ; 
    mvaddstr(1, 1, "exit window memory"); 
    refresh();
     return c ;    

}

int show_window_others(WINDOW *win, WINDOW *main_win, vec_t *input){
    box(win, 0, 0) ;  
    update_panels();
    mvaddstr(1, 1, "show window others");
    refresh();
    //mvwaddstr(win, 2, 1, "others panel");
    wrefresh(win);

    // we want to prijnt a specific part of the code where the bug occurs, or something else
    // protocole : we get a file, and a line 
    // example : want to print line 54 in main.c --> we print from line 54-height/2 to 54+height/2
    int line_to_print = 10; 

    int nrow, ncol ; 
    getmaxyx(win, nrow, ncol) ; // get the specific window size 
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
        mvwaddstr(win, 1, 4, "cannot open the selected file...");
    }
    int iline = 0 ; 
    int nline = 0 ; 
    while ((nline < nrow - 1 ) && (fgets(buff, ncol, fp) != NULL) ){
        mvaddstr(1, 1, "begin writing the line file ");
        if (iline >= begin){

            // print the line
            sprintf(chline, "%d", iline + 1);
            //wattron(win, COLOR_PAIR(1));  
            mvwaddstr(win, 1 + nline, 1, chline) ;
            //wattroff(win, COLOR_PAIR(1));  

            if(nline + begin == line_to_print){
                char *symbol[1] = {" "} ; 
                wattron(win, COLOR_PAIR(1));                  
                for (int i = 0 ; i < ncol - 2 ; i++){
                    mvwaddstr(win, 1 + nline, i + 2 + numdigits, symbol[0]) ;
                }              
                mvwaddstr(win, 1 + nline, 2 + numdigits, buff) ; 
                wattroff(win, COLOR_PAIR(1));                                
            }
            else{
            mvwaddstr(win, 1 + nline, 2 + numdigits, buff) ; 
            }
            nline++;
        }     
        iline++;  
    }
    // close file
    fclose(fp) ;

    box(win, 0, 0);
    wrefresh(win) ;  

    int c ; 
    
vec_t *input2 = vec_with_capacity( 100 , sizeof(char)) ;     
    c = keyboard_input(main_win, input2) ; 
    mvaddstr(1, 1, "exit window others"); 
    refresh();
     return c ; 

}

void show_process_list(vec_t vp){

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


    refresh() ;
    update_panels() ;    

        attron(COLOR_PAIR(1));
        mvprintw( 0, 0, "%s", "voila");
        attroff(COLOR_PAIR(1));

    
    vec_t* vp = generate_processes() ;  // to add processes. Need to be obtain from branch/features

    keypad(inter.main_window[0], TRUE) ; 

	new_main_line(inter.main_window[0]);


	vec_t *input = vec_new( sizeof(char) ) ; 

    int c ;
    c = show_window_start(inter.right_window[0], inter.main_window[0], input); // c est la touche d'interraction pressee
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

