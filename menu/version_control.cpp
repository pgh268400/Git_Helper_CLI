#include "../tools/menu_box.hpp"
#include "menu_header.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <list>
#include "../tools/git.hpp"

static void menu_handler(const char *name);

static vector<string> menus = {"Git Status", "Easy Commit", "Show Commit List", "Show Different (Unstaged)", "Show Different (Staged)", "Back", ""};
menu_box version_menu(menus, 50, 10, 4, 4, menu_handler);

extern git git_manager;

void show_version_menu()
{
    version_menu.show_menu("Version Control");
}

// C 에서 함수를 static 으로 선언하는것은 그 함수는 해당 소스파일에서만 사용가능하게 한다.
// 메뉴에서 엔터칠 시 호출되는 함수
static void menu_handler(const char *name)
{
    move(20, 0);
    clrtoeol();
    mvprintw(20, 0, "Item selected is : %s", name);

    // 선택한 메뉴에 따라서 다른 동작을 수행

    if (strcmp(name, menus[0].c_str()) == 0)
    {
    }
    else if (strcmp(name, menus[1].c_str()) == 0)
    {
    }
    else if (strcmp(name, menus[2].c_str()) == 0)
    {
    }
    else if (strcmp(name, menus[3].c_str()) == 0)
    {
    }
    else if (strcmp(name, menus[4].c_str()) == 0)
    {
    }
    else if (strcmp(name, "Back") == 0)
    {
        show_main_menu();
    }
}
