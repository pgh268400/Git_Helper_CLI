#include <string.h>
#include "menu_box.hpp"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

menu_box::menu_box(vector<string> menu_list, int menu_width, int menu_height, int menu_starty, int menu_startx, void (*handler)(const char *name))
{
    /* ncurses 초기화 */
    initscr(); //필수. curses를 위한 전체 화면을 초기화
    start_color();
    cbreak(); // carriage return을 기다리지 않고 타이핑된 즉시 키보드의 문자를 읽어들이도록 설정
              // https://kwangcheolchae.wordpress.com/2012/12/04/%EC%BA%90%EB%A6%AC%EC%A7%80-%EB%A6%AC%ED%84%B4%EC%9D%B4%EB%9E%80/
              // 캐리지 리턴(Carriage Return) 은 현재 위치를 나타내는 커서 를 맨 앞으로 이동시킨다는 뜻이고,
              // 라인피드 (Line Feed) 는 개행문자로, 커서의 위치를 아랫줄로 이동시킨다는 뜻입니다.
              // 윈도우에서는 이 두 동작을 합처 Enter 동작을 하는것입니다..
              // "CR+LF 로 커서를 앞으로 보낸후 줄을 한줄 바꾼다 라고 이해하면 됨.

    // curs_set(0); //터미널 커서 숨기기

    // noecho();                             //화면에 입력된 문자를 echoing 하는 것을 종료, 아마 batch 파일의 echo off 와 동일한 기능을 하는듯, 이를 사용하면 유저 입력을 해도 메아리 치듯이 유저의 입력이 보이지 않으므로 주의
    keypad(stdscr, TRUE);                 //입력 시 키보드 특수 키의 입력을 가능하게 설정하는 함수 (방향키, esc 등이 해당)
    init_pair(1, COLOR_RED, COLOR_BLACK); // ncurse 에서 사용 가능한 폰트 색상은 8가지 로 제한됩니다. 다만, 이들 컬러를 이용하여 폰트 전경색, 배경색 조합하여 다양한 표현을 할 수는 있습니다.
                                          // init_pair 함수는 이 색상 테이블을 만들때 사용 하는 함수입니다. 일종의 폰트 색상 팔레트로 보시면 정확 할 듯 싶군요.
                                          // 하나만 예를 들어 보자면, init_pair(1, COLOR_RED, COLOR_BLACK); 의 경우 팔레트 넘버 '1'로서 폰트의 전경색(Fore ground color)은 적색으로 그리고 배경색(Back ground color)은 검정색으로 선언 하고 있습니다.
                                          // 만들 수 있는 팔레트의 최대(MAX) 갯수는 해 보지 않아서 모르겠습니다

    // 값 초기화
    menu_lst = menu_list;
    menu_width = menu_width;
    menu_height = menu_height;
    start_y = menu_starty;
    start_x = menu_startx;
    menu_handler = handler;

    /* 아이템 동적할당, 대입 */
    n_choices = menu_lst.size();
    my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
    for (int i = 0; i < n_choices; i++)
        my_items[i] = new_item(menu_lst[i].c_str(), "");

    /* 메뉴 생성 */
    my_menu = new_menu((ITEM **)my_items);

    // 메뉴 박스 좌표는 터미널 화면 중심에 위치
    int menu_x = (COLS - menu_width) / 2;
    int menu_y = (LINES - menu_height) / 2;

    my_menu_win = newwin(menu_height, menu_width, menu_y, menu_x);
    keypad(my_menu_win, TRUE); //메뉴에서 키패드 활성화

    /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);

    // derwin == subwin : 새로운 윈도우를 생성하는 함수
    // WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);

    // 서브메뉴 윈도우 각각 생성

    int menu_cnt = menu_list.size() - 1; //메뉴 요소 갯수 (끝에 NULL 구분자는 갯수에서 제거)
    int sub_nlines = menu_cnt;
    int sub_ncols = 49;

    set_menu_sub(my_menu, derwin(my_menu_win, sub_nlines, sub_ncols, 3, 1));

    /* Set menu mark to the string " * " */
    set_menu_mark(my_menu, " * ");
}

void menu_box::show_menu(string title)
{
    erase();
    /* 윈도우의 테두리와 제목 출력 */

    box(my_menu_win, 0, 0);

    int title_y = 1;
    int title_x = 0;
    int title_width = 50;

    print_in_middle(title_y, title_x, title_width, title, COLOR_PAIR(1));
    mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
    mvwhline(my_menu_win, 2, 1, ACS_HLINE, title_width);
    mvwaddch(my_menu_win, 2, title_width - 1, ACS_RTEE);
    mvprintw(LINES - 2, 0, " Esc to exit");

    // 반드시 어떤 출력을 수행하였으면 refresh()를 수행해주도록 해야 합니다.
    // 만약 어떤 특정 창에 대해서 refresh()를 수행하고자 하는 경우에는 wrefresh(WINDOW *)를 실행해주시면 됩니다.
    refresh();

    /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);

    // 종료 키(ESC)가 아닌 경우만 계속 입력 받아서 처리
    int c;
    while ((c = wgetch(my_menu_win)) != 27)
    {
        switch (c)
        {
        case KEY_DOWN:
            menu_driver(my_menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(my_menu, REQ_UP_ITEM);
            break;
        // 엔터키 입력 받아서 화면에 내용 출력
        case 10: /* Enter */
            menu_handler(item_name(current_item(my_menu)));
            pos_menu_cursor(my_menu);
            break;
        }

        // 반드시 어떤 출력을 수행하였으면 refresh()를 수행해주도록 해야 합니다.
        // 만약 어떤 특정 창에 대해서 refresh()를 수행하고자 하는 경우에는 wrefresh(WINDOW *)를 실행해주시면 됩니다.
        wrefresh(my_menu_win);
        refresh();
    }

    /* 메뉴에 사용한 메모리를 전부 해제한다. */
    // 해제의 경우 어짜피 소멸자에서 이루어지므로 두번 delete 하지 않도록 한다.
    // release_menu_resource();
}

menu_box::~menu_box()
{
    release_menu_resource();
}

void menu_box::release_menu_resource()
{
    unpost_menu(my_menu);
    free_menu(my_menu);
    for (int i = 0; i < n_choices; i++)
        free_item(my_items[i]);
    endwin();            // curses모드를 종료한다. curses모드를 사용하고 종료할 때 반드시 써줘야한다.
                         // 만약 endwin() 을 하지 않고 프로그램이 종료됐을 경우 텍스트 모드의 이상을
                         // 가져 올수 있다.
    delwin(my_menu_win); // 윈도우도 삭제해준다.
}

void menu_box::print_in_middle(int starty, int startx, int width, string str, chtype color)
{
    int length, x, y;
    float temp;

    if (my_menu_win == NULL)
        my_menu_win = stdscr;
    getyx(my_menu_win, y, x);
    if (startx != 0)
        x = startx;
    if (starty != 0)
        y = starty;
    if (width == 0)
        width = 80;

    length = str.length();
    temp = (width - length) / 2;
    x = startx + (int)temp;
    wattron(my_menu_win, color);
    mvwprintw(my_menu_win, y, x, "%s", str.c_str());
    wattroff(my_menu_win, color);
    refresh();
}
