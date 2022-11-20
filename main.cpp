
#include "tools/menu_box.hpp"
#include "menu/menu_header.hpp"
#include <list>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "tools/git.hpp"

using namespace std;
static void menu_handler(const char *name);
void load_git_list();

//끝에 NULL terminate 문자열은 String에서 "" 로 적으면 알아서 c_str이 NULL로 변환
static vector<string> menus = {"Manage Directory", "Version Control", "About Git", "Exit", ""};
menu_box main_menu(menus, 50, 8, 4, 4, menu_handler);

string git_info_dir = "./settings/git_info.txt";
string git_active_dir = "./settings/git_active.txt";

// 현재 프로그램에서 GIT에 관련된 파일 & 정보를 관리하는 객체
git git_manager(git_info_dir, git_active_dir);

int main()
{
    load_git_list();  // git_list에 저장된 git 디렉토리를 불러온다
    show_main_menu(); //메인 메뉴 화면을 출력한다 (콘솔)
}

void load_git_list()
{
    // menu_box의 active_git_dir 변수 참조
    menu_box::active_git_dir = git_manager.get_active_dir();
}

void show_main_menu()
{
    main_menu.show_menu("Git Helper");
}

void about_git()
{
    erase();

    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_WHITE, COLOR_BLUE);
    init_pair(4, COLOR_WHITE, COLOR_MAGENTA);

    attron(COLOR_PAIR(4));
    mvprintw(0, 0, "<About Git> Git is a free and open source distributed version control system designed to handle everything from small to very large projects with speed and efficiency.");
    mvprintw(3, 0, "Git is easy to learn and has a tiny footprint with lightning fast performance. It outclasses SCM tools like Subversion, CVS, Perforce, and ClearCase with features like cheap local branching, convenient staging areas, and multiple workflows.");
    attroff(COLOR_PAIR(4));

    mvprintw(6, 0, "Since this program is a program to use Git somewhat easily, you need to have some understanding of Git.");
    mvprintw(7, 0, "If you want to learn more about Git, please visit the following site and download book.");

    attron(COLOR_PAIR(3));
    mvprintw(9, 0, "https://git-scm.com/book/en/v2");
    mvprintw(11, 0, "Book name : Pro Git");
    mvprintw(13, 0, "Author : Scott Chacon, Ben Straub");
    attroff(COLOR_PAIR(3));

    mvprintw(15, 0, "This is a free book with everything about Git managed by Git. It can be downloaded as a PDF, and of course, Korean is also supported. If you want to improve your understanding of Git, I highly recommend reading it.");

    attron(COLOR_PAIR(2));
    mvprintw(18, 0, "Press any key to return to the main menu...");
    attroff(COLOR_PAIR(2));

    getch(); //엔터 입력 대기 (바로 종료되지 않게 하기)
    show_main_menu();
    refresh();
}
// C 에서 함수를 static 으로 선언하는것은 그 함수는 해당 소스파일에서만 사용가능하게 한다.
// 메뉴에서 엔터칠 시 호출되는 함수
static void menu_handler(const char *name)
{
    // 메뉴에서 선택한 항목의 이름을 출력

    // 윈도우 커서를 옮긴다, move(int y, int x);
    move(20, 0);
    // clrtoeol() , wclrtoeol()  : 현재 커서 위치에서 라인의 끝까지 내용을 지움 (이 함수는 커서를 업데이트 하지 않는다.)
    clrtoeol();
    mvprintw(20, 0, "Item selected is : %s", name);

    // 선택한 메뉴에 따라서 다른 동작을 수행
    // 1. Manage Directory
    if (strcmp(name, menus[0].c_str()) == 0)
    {
        show_directory_menu();
    }
    // 2. Version Control
    else if (strcmp(name, menus[1].c_str()) == 0)
    {
        show_version_menu();
    }
    // 3. About Git
    else if (strcmp(name, menus[2].c_str()) == 0)
    {
        about_git();
    }
    // 4. Exit
    else if (strcmp(name, menus[3].c_str()) == 0)
    {
        endwin();
        exit(0);
    }
}
