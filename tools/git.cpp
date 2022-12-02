#include "git.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <curses.h>
#include <sys/wait.h>

git::git(string git_info_dir, string git_active_dir) : git_info_dir(git_info_dir), git_active_dir(git_active_dir)
{
    // 1. git_list 멤버 변수에 데이터 담기.
    // git_info에 저장된 내용을 파일에서 읽어온다.
    int fd = open(git_info_dir.c_str(), O_RDONLY);
    if (fd == -1)
    {
        perror("open() error!");
        exit(-1);
    }

    // 파일에서 읽어온 내용을 저장할 버퍼
    char buf[1024] = {
        0,
    };

    int len = 0;

    // 파일에서 읽어온 내용을 buf에 저장하고, 읽어온 길이를 len에 저장
    // 기본적으로 read를 계속 호출할때마다 파일의 offset이 증가하므로, 0을 반환할때까지 While을 돌리면서 버퍼에 저장하고 읽으면
    // 모든 파일의 내용을 읽을 수 있다. (일반적인 스트림처럼 순차적으로 읽을 수 있음.)
    // 참고 : 파일의 끝에 도달하면 0을 리턴
    while ((len = read(fd, buf, sizeof(buf))) > 0)
    {
        // buf에 저장된 내용을 string으로 변환
        string str(buf, len);

        // string에서 개행문자를 기준으로 분리
        size_t pos = 0;
        string token;
        while ((pos = str.find('\n')) != string::npos)
        {
            token = str.substr(0, pos);
            git_list.push_back(token); // 분리된 내용을 git_list에 저장
            str.erase(0, pos + 1);     // 0부터 pos 인덱스까지 문자열 삭제 (끝에 pos+1 은 포함안됨.)
        }
    }

    // 2. git_active 에 데이터 담기
    fd = open(git_active_dir.c_str(), O_RDONLY);
    if (fd == -1)
    {
        perror("open() error!");
        exit(-1);
    }

    while ((len = read(fd, buf, sizeof(buf))) > 0)
    {
        // buf에 저장된 내용을 git_active 에 저장
        git_active = string(buf, len);
    }
}

// 멤버 함수 getter
// 성능을 위해 const 레퍼런스 타입으로 (원본 수정 불가)
// getter 함수이므로 끝에 const 키워드까지
const string &git::get_active_dir() const
{
    return git_active;
}

const list<string> &git::get_git_list() const
{
    return git_list;
}

// Git 디렉토리를 추가 / 삭제할때마다 매번 git_info 파일을 자동 업데이트 해준다.
// 당연하지만 Git List도 같이 업데이트 됨.

// 프로그램이 관리할 Git 디렉토리를 추가 (멤버 변수 수정, 파일 수정)
void git::add_git_list(string git_dir)
{
    // git_list에 값 추가
    git_list.push_back(git_dir);

    // git_list 파일을 우선 삭제
    unlink(git_dir.c_str());

    // 현재 git_list 내용을 파일에 저장
    // git_info.txt 열어서 저장 (없으면 새로 만들고 & 빈 상태로 만듬(O_TRUNC) & 권한 0644)
    int fd = open(git_info_dir.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }

    // git_list 반복자로 반복하면서 파일에 저장
    // write 를 반복호출할때마다 offset 이 증가하면서 파일의 끝에 계속 추가됨.
    for (auto it = git_list.begin(); it != git_list.end(); it++)
    {
        // git_list에 저장된 내용을 파일에 저장
        write(fd, it->c_str(), it->length());
        write(fd, "\n", 1); // write시 널문자는 고려하지 않는듯? (널문자는 쓸필요 없으니깐)
    }
}

// 관리중인 Git 디렉토리에서 삭제 (멤버 변수 수정, 파일 수정)
void git::remove_git_list(string git_dir)
{
    // git_list에 값 삭제
    git_list.remove(git_dir);

    // git_list 파일을 우선 삭제
    unlink(git_dir.c_str());

    // 현재 git_list 내용을 파일에 저장
    // git_info.txt 열어서 저장 (없으면 새로 만들고 & 빈 상태로 만듬(O_TRUNC) & 권한 0644)
    int fd = open(git_info_dir.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }

    // git_list 반복자로 반복하면서 파일에 저장
    // write 를 반복호출할때마다 offset 이 증가하면서 파일의 끝에 계속 추가됨.
    for (auto it = git_list.begin(); it != git_list.end(); it++)
    {
        // git_list에 저장된 내용을 파일에 저장
        write(fd, it->c_str(), it->length());
        write(fd, "\n", 1); // write시 널문자는 고려하지 않는듯? (널문자는 쓸필요 없으니깐)
    }

    // 만약 삭제한 디렉토리가 현재 활성화된 디렉토리라면 활성화된 디렉토리를 비활성화 시킴.
    if (git_active == git_dir)
    {
        set_active_dir("");
    }
}

void git::git_status()
{
    clear();
    int mid_y = LINES / 2;
    int mid_x = COLS / 2;

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
        int status = chdir(git_active.c_str());
        if (status == -1)
        {
            printf(" chdir error : cannot access %s", git_active.c_str());
            fflush(stdout); //버퍼에 남아있는 내용을 모두 출력하도록 한다 (fflush == 출력버퍼 사용 전용), fflush 를 안해주면 바로 출력이 안됨
            exit(0);
        }

        // execlp()는 PATH에 등록된 모든 디렉토리에 있는 프로그램을 실행하므로 프로그램 이름만 입력해도 실행이 됩니다.
        execlp("git", "git", "status", NULL);
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

        // if (git_output.find("Initialized") != string::npos)
        // {
        //     mvprintw(mid_y, 0, "Status : Successful initialized.");
        //     mid_y += 1;
        //     mvprintw(mid_y, 0, "Write information to a file...");

        //     git_manager.add_git_list(str);
        // }
        // else if (git_output.find("Reinitialized") != string::npos)
        // {
        //     mvprintw(mid_y, 0, "Status : Reinitialized existing Git repository.");
        // }
        // else
        // {
        //     mvprintw(mid_y, 0, "Status : Unknown error. Program terminated.");
        //     getch();
        //     exit(0);
        // }

        wait(NULL); //자식 프로세스가 종료될때까지 우선 대기  (자식 프로세스의 종료 status 는 저장할필요 없으므로 NULL)
    }

    mid_y += 5;
    mvprintw(mid_y, 0, "Press any key to continue...");
    getch();
}

void git::set_easy_commit()
{
    erase();

    char commit_name[100] = {
        0,
    };
    int row, col;
    initscr();
    getmaxyx(stdscr, row, col);

    int mid_x = 1;
    int mid_y = row / 2;

    mvprintw(mid_y, mid_x, ">>> Enter your commit name :  ");
    getstr(commit_name);

    // 글자 전체 위치 위로 올리기
    mid_y -= 2;

    // 현재 커서위치 가져오기
    mvprintw(mid_y, mid_x, "Your Commit name is : %s", commit_name);
    mid_y += 1; //한줄 띄우기
    mvprintw(mid_y, mid_x, "Run git commit -am \"%s\"...", commit_name);
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
        int status = chdir(git_active.c_str());
        if (status == -1)
        {
            printf(" chdir error : cannot access %s", git_active.c_str());
            fflush(stdout); //버퍼에 남아있는 내용을 모두 출력하도록 한다 (fflush == 출력버퍼 사용 전용), fflush 를 안해주면 바로 출력이 안됨
            exit(0);
        }

        string commit_name_str = "\"" + string(commit_name) + "\"";

        // 새로운 자식 프로세스를 fork() 후 git add . 명령어를 실행
        pid_t pid;
        if ((pid = fork()) == -1)
        {
            printf("fork error");
            exit(0);
        }
        else if (pid == 0)
        {
            // child process
            // git add . 명령어를 실행
            chdir(git_active.c_str());
            execlp("git", "git", "add", ".", NULL);
            exit(0);
        }
        else
        {
            // parent process
            wait(NULL); //자식 프로세스가 종료될때까지 우선 대기  (자식 프로세스의 종료 status 는 저장할필요 없으므로 NULL)
        }

        // execlp()는 PATH에 등록된 모든 디렉토리에 있는 프로그램을 실행하므로 프로그램 이름만 입력해도 실행이 됩니다.
        execlp("git", "git", "commit", "-am", commit_name_str.c_str(), NULL);
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
        wait(NULL); //자식 프로세스가 종료될때까지 우선 대기  (자식 프로세스의 종료 status 는 저장할필요 없으므로 NULL)
    }

    mid_y += 7;
    mvprintw(mid_y, mid_x, "Press any key to continue...");
    getch();
}

void git::get_easy_commit()
{
    clear();
    int mid_y = LINES / 2;
    int mid_x = COLS / 2;

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
        int status = chdir(git_active.c_str());
        if (status == -1)
        {
            printf(" chdir error : cannot access %s", git_active.c_str());
            fflush(stdout); //버퍼에 남아있는 내용을 모두 출력하도록 한다 (fflush == 출력버퍼 사용 전용), fflush 를 안해주면 바로 출력이 안됨
            exit(0);
        }

        // execlp()는 PATH에 등록된 모든 디렉토리에 있는 프로그램을 실행하므로 프로그램 이름만 입력해도 실행이 됩니다.
        execlp("git", "git", "log", "--pretty=oneline", NULL);
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
        wait(NULL); //자식 프로세스가 종료될때까지 우선 대기  (자식 프로세스의 종료 status 는 저장할필요 없으므로 NULL)
    }

    mid_y += 5;
    mvprintw(mid_y, 0, "Press any key to continue...");
    getch();
}

void git::show_diff_unstaged()
{
    clear();
    int mid_y = LINES / 2;
    int mid_x = COLS / 2;

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
        int status = chdir(git_active.c_str());
        if (status == -1)
        {
            printf(" chdir error : cannot access %s", git_active.c_str());
            fflush(stdout); //버퍼에 남아있는 내용을 모두 출력하도록 한다 (fflush == 출력버퍼 사용 전용), fflush 를 안해주면 바로 출력이 안됨
            exit(0);
        }

        // execlp()는 PATH에 등록된 모든 디렉토리에 있는 프로그램을 실행하므로 프로그램 이름만 입력해도 실행이 됩니다.
        execlp("git", "git", "diff", NULL);
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
        wait(NULL); //자식 프로세스가 종료될때까지 우선 대기  (자식 프로세스의 종료 status 는 저장할필요 없으므로 NULL)
    }

    mid_y += 5;
    mvprintw(mid_y, 0, "Press any key to continue...");
    getch();
}

void git::show_diff_staged()
{
    clear();
    int mid_y = LINES / 2;
    int mid_x = COLS / 2;

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
        int status = chdir(git_active.c_str());
        if (status == -1)
        {
            printf(" chdir error : cannot access %s", git_active.c_str());
            fflush(stdout); //버퍼에 남아있는 내용을 모두 출력하도록 한다 (fflush == 출력버퍼 사용 전용), fflush 를 안해주면 바로 출력이 안됨
            exit(0);
        }

        // execlp()는 PATH에 등록된 모든 디렉토리에 있는 프로그램을 실행하므로 프로그램 이름만 입력해도 실행이 됩니다.
        execlp("git", "git", "diff", "--cached", NULL);
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
        wait(NULL); //자식 프로세스가 종료될때까지 우선 대기  (자식 프로세스의 종료 status 는 저장할필요 없으므로 NULL)
    }

    mid_y += 5;
    mvprintw(mid_y, 0, "Press any key to continue...");
    getch();
}

// 관리중인 Git 디렉토리를 변경 (멤버 변수 수정, 파일 수정)
void git::set_active_dir(string git_dir)
{
    git_active = git_dir;
    unlink(git_active_dir.c_str());

    // git_active.txt 파일에 git_active 내용 저장
    int fd = open(git_active_dir.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }
    write(fd, git_active.c_str(), git_active.length());
}
