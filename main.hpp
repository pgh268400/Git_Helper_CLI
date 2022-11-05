#pragma once

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

// 함수 원형 선언
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
void show_main_menu();
void release_menu_resource(WINDOW *win, MENU *my_menu, ITEM **my_items, int n_choices);

// 문법적으로만 본다면 선언 자체는 전혀 문제가 없으므로 경고는 무시해도 좋습니다.
// 그러나 선언의 의미를 따져볼 때 static 함수의 선언을 헤더에 넣는 것은 이치에 맞지
// 않기 때문에 똑똑한 컴파일러가 경고를 주는 것입니다.