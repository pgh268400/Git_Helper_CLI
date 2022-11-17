
#include "../tools/menu_box.hpp"
#include "menu_header.hpp"
#include <list>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
static void menu_handler(const char *name);

extern list<string> git_list;
extern string git_active;

void show_active_menu()
{
    // git_list의 내용을 메뉴로 만들어서 동적으로 출력
    // 전역변수로 관리하면 스택 영역을 자주 사용하지 않아도 될탠데
    // 동적으로 읽어야 하기 때문에 지역변수로 매번 만들어서 출력해줘야함 ㅠㅠ

    //끝에 NULL terminate 문자열은 String에서 "" 로 적으면 알아서 c_str이 NULL로 변환
    // list<T> -> vector<T>로 변환
    vector<string> menus(git_list.begin(), git_list.end());
    menus.push_back("Back");
    menus.push_back("");
    menu_box active_menu(menus, 50, menus.size() + 3, 4, 4, menu_handler);

    active_menu.show_menu("Active Working Directory");
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

    // name == Back
    if (strcmp(name, "Back") == 0)
    {
        // 메뉴를 닫는다.
        show_directory_menu();
    }
    else
    {
        // git_active에 선택한 디렉토리를 저장
        git_active = name;

        // menu_box의 active_git_dir 변수 참조
        menu_box::active_git_dir = git_active;

        // git_active 파일을 열어서 git_active 변수 내용을 저장
        int fd = open("./settings/git_active.txt", O_WRONLY);
        write(fd, git_active.c_str(), git_active.length());
        // 메뉴를 닫는다.
        show_directory_menu();
    }
}
