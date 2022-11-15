#pragma once

#include <ncurses.h>
#include <menu.h>
#include <string>
#include <vector>
#include <cstring>
// include for strcmp

using namespace std;

// Ncurses Menu 를 쉽게 재활용하기 위한 Wrapper Class
class menu_box
{
public:
    menu_box(vector<string> menu_list, int menu_width, int menu_height, int menu_starty, int menu_startx, void (*menu_handler)(const char *name));
    ~menu_box();
    void show_menu(string title);
    void set_active_git(string name);
    void release_menu_resource();

    // 현재 활성화된 git 디렉토리 경로
    // 전 객체가 공유해야 하므로 static 키워드로 설정
    // 또 static 은 외부에서 또 정의를 해줘야 한다지만,
    // C++ 17부터 inline 키워드를 주면 이런 문제없이 할 수 있다고 함 (뭐 이리 힘드노? ㅡㅡ)
    inline static string active_git_dir;

private:
    // Member Function
    void print_in_middle(int starty, int startx, int width, string str, chtype color);

    // Member Variable
    vector<string> menu_lst;                //메뉴 목록 문자열 동적 배열
    WINDOW *my_menu_win;                    //메뉴를 출력할 윈도우
    MENU *my_menu;                          //메뉴를 통째로 저장할 포인터
    ITEM **my_items;                        // 메뉴 요소들을 동적할당할 배열 (2차원 동적 할당된 배열의 포인터 - 더블 포인터)
    int n_choices;                          //메뉴 요소 갯수 (끝 NULL 포함)
    void (*menu_handler)(const char *name); //함수 포인터, 메뉴 핸들러 (콜백 함수)

    // 메뉴 좌표 & 사이즈
    int start_x;
    int start_y;

    int menu_width;
    int menu_height;
};