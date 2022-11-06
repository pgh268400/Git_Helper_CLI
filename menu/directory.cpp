#include "../tools/menu_box.hpp"
#include "full_menu.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

static void menu_handler(const char *name);

//끝에 NULL terminate 문자열은 String에서 " " 로 적으면 알아서 c_str이 NULL로 변환
static vector<string> menus = {"Show Managed Directory", "Active Working Directory", "Add Directory for Version Control", "Remove Directory from Version Control", "Back", ""};
menu_box directory_menu(menus, 50, 9, 4, 4, menu_handler);

void show_directory_menu()
{
    directory_menu.show_menu("Git Helper");
}

void run_git_init(string path)
{
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

    // Show Managed Dir
    if (strcmp(name, menus[0].c_str()) == 0)
    {
    }
    else if (strcmp(name, menus[1].c_str()) == 0)
    {
    }
    // Add Dir
    else if (strcmp(name, menus[2].c_str()) == 0)
    {
        erase();

        // char mesg[] = ">>> Enter your target path :  "; /* message to be appeared on the screen */
        char str[100] = {
            0,
        };
        int row, col;               /* to store the number of rows and *
                                     * the number of colums of the screen */
        initscr();                  /* start the curses mode */
        getmaxyx(stdscr, row, col); /* get the number of rows and columns */

        // move(10, 10);
        // printw("This is 10, 10");
        // 라고 쓰는것과 mvprintw(10, 10, "This is 10, 10"); 는 같음. move + printw = mvprintw

        // int mid_x = (col - strlen(mesg)) / 2;
        int mid_x = 1;
        int mid_y = row / 2;

        mvprintw(mid_y, mid_x, ">>> Enter your target path :  ");
        /* print the message at the center of the screen */
        getstr(str);

        erase();

        // 글자 전체 위치 위로 올리기
        mid_y -= 2;

        // 현재 커서위치 가져오기
        mvprintw(mid_y, mid_x, "Your Input Path is : %s", str);
        mid_y += 1; //한줄 띄우기
        mvprintw(mid_y, mid_x, "Run \"cd : %s\"...", str);
        mid_y += 1;
        mvprintw(mid_y, mid_x, "Run \"git init\"...");
        mid_y += 1;

        // exec() 계열 함수는 한번에 하나의 명령어만 실행 가능하므로
        // 실제로는 cd와 git init 을 동시에 실행시키지 않고 chdir() 함수를 이용해서
        // 현재 디렉토리를 변경한 후 git init 을 실행시킨다.

        int fd[2];
        // 부모 자식간 통신을 위한 익명 파이프 생성
        if (pipe(fd) == -1)
        {
            printf("pipe error");
            exit(0);
        }

        // fork를 통해 파이프를 나눠가짐
        pid_t pid;
        if ((pid = fork()) == -1)
        {
            printf("fork error");
            exit(0);
        }
        else if (pid == 0)
        {
            // child process
            close(fd[0]);
            // fd1의 파일 디스크립터가 명시한 STDOUT_FILENO의 파일 디스크립터로 복사된다.
            // 즉 fd[1]은 그대로이며, STDOUT_FILENO은 fd[1]이 가리키던 위치로 변경된다.
            // 앞으로 모니터 출력을 걸면 STDOUT의 내용이 모니터로 출력되는 것이 아니라 fd[1]로 출력된다.
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]); // fd[1]은 dup2() 함수로 STDOUT_FILENO로 복사되었으므로 따로 사용할 필요가 없다.

            // cd와 같은 효과를 내기 위해 chdir() 함수를 사용
            int status = chdir(str);
            if (status == -1)
            {
                printf(" chdir error : cannot access %s", str);
                fflush(stdout); //버퍼에 남아있는 내용을 모두 출력하도록 한다 (fflush == 출력버퍼 사용 전용), fflush 를 안해주면 바로 출력이 안됨
                exit(0);
            }

            // execlp()는 PATH에 등록된 모든 디렉토리에 있는 프로그램을 실행하므로 프로그램 이름만 입력해도 실행이 됩니다.
            execlp("git", "git", "init", NULL);
            exit(0);
            // 자식은 여기서 종료

            // exec 한 다음 exit를 호출해 주는 코드가 있던데, 어차피 exec하고 나면 해당 파일을 자신의 메모리에 덮어쓰기 때문에 실행되다 알아서 종료된다. exec 이후에 적은 코드는 실행 안되니까,
            // exec 오류일 경우만 exit등으로 예외처리만 해주면 된다.
        }
        else
        {
            // parent process
            char buf[4096] = {
                0,
            };
            close(fd[1]);
            int nbytes = read(fd[0], buf, sizeof(buf));

            mvprintw(mid_y, 0, "%.*s\n", nbytes, buf);
            mid_y += 1;

            string git_output(buf); // char array -> string 변환

            if (git_output.find("Initialized") != string::npos)
            {
                mvprintw(mid_y, 0, "Status : Successful initialized.");
                mid_y += 1;
                mvprintw(mid_y, 0, "Write information to a file...");

                // open() 으로 파일을 열고, write() 로 파일에 내용을 쓴다.
                int fd = open("git_info.txt", O_APPEND | O_CREAT | O_TRUNC, 0644); //파일 open
                write(fd, str, strlen(str));
                close(fd);
            }
            else if (git_output.find("Reinitialized") != string::npos)
            {
                mvprintw(mid_y, 0, "Status : Reinitialized existing Git repository.");
            }
            else
            {
                mvprintw(mid_y, 0, "Status : Unknown error. Program terminated.");
                getch();
                exit(0);
            }

            wait(NULL); //자식 프로세스가 종료될때까지 우선 대기  (자식 프로세스의 종료 status 는 저장할필요 없으므로 NULL)
        }

        mid_y += 1;
        mvprintw(mid_y, mid_x, "Press any key to continue...");

        // 화면 정중앙에 출력
        getch();
        show_directory_menu();
    }
    // Remove Dir
    else if (strcmp(name, menus[3].c_str()) == 0)
    {
    }
    // Back
    else if (strcmp(name, menus[4].c_str()) == 0)
    {
        show_main_menu();
    }
}
