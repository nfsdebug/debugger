
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <panel.h>
#include <menu.h>
#include <form.h>

char *choice_panel[] = { "Start", "Processes", "Memory", "Others", (char *)NULL, } ; 
char *panel_func[] = { " ", " ", " ", " ", (char *)NULL, } ; 


int show_window_start(WINDOW *start);
int show_window_processes(WINDOW *processes);
int show_window_memory(WINDOW *memory);
int show_window_others(WINDOW *others);


int main(int argc, char **argv){
    printf("This is the Ncurses sbstndbs debugger ! \n");   

    // entities creation : 
    // first : windows
    WINDOW *main_window[4] ; // there are 2 distinct windows
    WINDOW *right_window[1] ; 
    WINDOW *title_window[1] ; 
    WINDOW *selector_window[1] ;  
    PANEL *my_panels[7] ;
    // second : toggle management 
    ITEM **my_items ; 
    MENU *my_menus ; 
    ITEM *cur_item ; // selected item 




    // get the window size
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    printf("Size is %d row %d cols\n", w.ws_row, w.ws_col);



    int linesmain = (int)(1.0 * w.ws_row) - 3  ,  colsmain = (int)(0.8 * w.ws_col)  ; 
    int linesright = (int)(1.0 * w.ws_row) - 3  ,  colsright = (int)(0.2 * w.ws_col) - 2  ; 
    int linestitle = 1 ; 
    int colstitle = 30 ;

    int linesselector = 1 ; 
    int colsselector = (int)(1.0 * w.ws_col) - 1 ; 
    // size of windows
    //TODO : compute lines and cols from dimemsions by tty

    int xmain = 2 , ymain = 1 ; 
    // position of the first window
    int xright = 2 , yright = colsmain + 3 ; 
    // position of the second window
    int xtitle = 1 ; 
    int ytitle = (int)(w.ws_col/2 - colstitle/2) ; 

    int xselector = (int)(1.0 * w.ws_row) - linestitle  ;
    int yselector = 1 ; 



    initscr();
    cbreak();
    noecho(); 
    curs_set(0);
    //keypad(stdscr, TRUE) ; // toggle managmeent with keyboard
   
    int n_choice = sizeof(choice_panel) / sizeof(choice_panel[0]) ; 
    my_items = (ITEM **)calloc(n_choice  , sizeof(ITEM *)) ; 

    for (int i = 0 ; i < n_choice ; i++){
        my_items[i] = new_item(panel_func[i], choice_panel[i]);
    }
    my_items[n_choice] = (ITEM *)NULL ;

    my_menus= new_menu((ITEM **)my_items) ; 


    // window creation
    for (int i = 0 ; i < 4 ; i++){
        main_window[i] = newwin(linesmain, colsmain, xmain, ymain) ; 
    }
    right_window[0] = newwin(linesright, colsright, xright, yright) ; 
    title_window[0] = newwin(linestitle, colstitle, xtitle, ytitle) ; 
    selector_window[0] = newwin(linesselector, colsselector, xselector, yselector) ; 
    //keypad(selector_window[0], TRUE);

    // draw boxes
    for (int i = 0 ; i < 4 ; i++){
        box(main_window[i], 0, 0) ;     
    }   
    box(right_window[0], 0, 0) ; 
    

    // panel initialsation ( works well with window superpositions)

    for(int i = 0 ; i < 4 ; i++){
    my_panels[i] = new_panel(main_window[i]) ;
    }
    my_panels[4] = new_panel(right_window[0]) ;
    my_panels[5] = new_panel(title_window[0]) ;
    my_panels[6] = new_panel(selector_window[0]) ;


    // title 
    char title[] = "NCurses debugger v0.1" ;
    mvwprintw(title_window[0], 0, 0, "%s", title) ; 


    // assiciate 4th window with the menu selector
    set_menu_win(my_menus, selector_window[0]) ; 
    set_menu_sub(my_menus, derwin(selector_window[0], linesselector, colsselector, 0, 0));
    set_menu_format(my_menus, 1, n_choice );
    //set_menu_mark(my_menus, "*");
    //box(my_window_selector[0], 0, 0) ;




    refresh() ; 
    
    update_panels();    
    
    post_menu(my_menus) ; 
    wrefresh(selector_window[0]) ; 

    int c ;
    set_current_item(my_menus, my_items[0]);



    c = show_window_start(main_window[0]); // c est la touche d'interraction pressee
    mvwaddnstr(main_window[0],1, 1,  "Bienvenue sur le menu d'execution\n", 35);
    //leaveok(win, TRUE);
    //wprintw(win, "test de wprintw \n");
    box(main_window[0], 0, 0) ; 
    update_panels();
    doupdate();
    refresh();    
                    // au moment de la sotie de la fenetre
    while(c != KEY_F(10)){
        mvaddstr(1, 1, "while main"); 
        refresh();
        switch(c){
            case 49:   
            mvaddstr(1, 1, "F1 pressed ");  
                refresh();
                set_current_item(my_menus, my_items[0]);
                hide_panel(my_panels[1]);
                hide_panel(my_panels[2]);
                hide_panel(my_panels[3]);
                show_panel(my_panels[0]);    
                refresh();                
                update_panels();  
                doupdate();       
                c = show_window_start(main_window[0]) ; 
                break;
            case 50: 
            mvaddstr(1, 1, "F2 pressed ");    
                refresh(); 
                hide_panel(my_panels[0]);
                hide_panel(my_panels[2]);
                hide_panel(my_panels[3]);
                show_panel(my_panels[1]);        
                refresh();        
                update_panels();
                doupdate();                              
                set_current_item(my_menus, my_items[1]);
                c = show_window_processes(main_window[1]) ;
                break;
            case 51:
            mvaddstr(1, 1, "F3 pressed ");      
                refresh();
                hide_panel(my_panels[0]);
                hide_panel(my_panels[1]);
                hide_panel(my_panels[3]);
                show_panel(my_panels[2]);        
                refresh();        
                update_panels();  
                doupdate();                                     
                set_current_item(my_menus, my_items[2]);
                c =show_window_memory(main_window[2]) ;
                break;                
            case 52:  
                mvaddstr(1, 1, "F4 pressed ");   
                    refresh(); 
                set_current_item(my_menus, my_items[3]);
                c = show_window_others(main_window[3]) ;
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

    doupdate();
    getch();

    free_item(my_items[0]);
    free_menu(my_menus);
    endwin();

}



// thi is the way the main window is drawn
int show_window_start(WINDOW *win){
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
    while(( c = getch())){
        if( (c >= 49) & (c <= 52)){
            break;
        } 
           // waddch(win, c) ; 
            doupdate();
            refresh();
     }
    mvaddstr(1, 1, "exit window start"); 
    refresh();
     return c ; 
}

int show_window_processes(WINDOW *win){
    box(win, 0, 0) ;  
    update_panels();
    refresh();
    refresh();
    int c ;
    mvaddstr(1, 1, "show window processes");
    refresh();
    while(( c = getch())){
        if( (c >= 49) & (c <= 52)){
            break;
        } 
            //waddch(win, text[0]) ; 
            doupdate();
            refresh();
     }
    mvaddstr(1, 1, "exit window processes"); 
    refresh();
     return c ; 


}

int show_window_memory(WINDOW *win){
    box(win, 0, 0) ;  
    update_panels();
    int c ; 
    mvaddstr(1, 1, "show window memory");
    refresh();
    while(( c = getch())){
        if( (c >= 49) & (c <= 52)){
            break;
        } 
            //waddch(win, text[0]) ; 
            doupdate();
            refresh();
     }
    mvaddstr(1, 1, "exit window memory"); 
    refresh();
     return c ;    

}

int show_window_others(WINDOW *win){
    box(win, 0, 0) ;  
    update_panels();
    mvaddstr(1, 1, "show window others");
    refresh();
    int c ; 
    while(( c = getch())){
        if( (c >= 49) & (c <= 52)){
            break;
        } 
            //waddch(win, text[0]) ; 
            doupdate();
            refresh();
     }
    mvaddstr(1, 1, "exit window others"); 
    refresh();
     return c ; 

}

