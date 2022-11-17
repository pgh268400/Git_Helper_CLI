#include "git.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

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

// 프로그램이 관리할 Git 디렉토리를 추가
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

// 관리중인 Git 디렉토리에서 삭제
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
}

// 관리중인 Git 디렉토리를 변경
void git::set_active_dir(string git_dir)
{
    git_active = git_dir;
}
