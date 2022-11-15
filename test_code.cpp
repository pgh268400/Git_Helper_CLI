
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <list>

using namespace std;

void load_git_list();

list<string> git_list;

int main()
{
    load_git_list(); // git_list에 저장된 git 디렉토리를 불러온다
}

void load_git_list()
{
    // git_info.txt에 저장된 내용을 파일에서 읽어온다.
    int fd = open("git_info.txt", O_RDONLY);
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

    // git_list 순회하며 화면에 출력
    for (auto it = git_list.begin(); it != git_list.end(); it++)
    {
        cout << *it << endl;
    }
}