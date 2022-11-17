#include <string>
#include <list>

using namespace std;

class git
{
public:
    git(string git_info_dir, string git_active_dir);

    const string &get_active_dir() const;
    void set_active_dir(string git_dir);

    const list<string> &get_git_list() const;
    void add_git_list(string git_dir);
    void remove_git_list(string git_dir);

private:
    // C++ STL에서 제공하는 연결 리스트(list)를 활용, 현재 프로그램에서 관리하는 git 디렉토리를 저장
    list<string> git_list;

    //실제로 프로그램에서 활성화된 git 디렉토리
    string git_active;

    // Git 디렉토리를 저장하는 파일 경로
    string git_info_dir;

    // Git 활성화된 디렉토리를 저장하는 파일 경로
    string git_active_dir;
};