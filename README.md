# Git Helper
![image](https://user-images.githubusercontent.com/31213158/205695309-a019fe63-3aae-404e-bd9b-014aac5bc022.png)

- 자료구조 & 시스템소프트웨어 실습 프로젝트
- Git을 CLI 환경 (터미널 환경) 에서 UI를 제공하여 쉽게 사용할 수 있는것을 목표로 제작
- Ncurses 라이브러리와 C++에서 리눅스 시스템 콜 호출하여 구현
  - Ncurses 라이브러리의 menu 활용 (전체 코드를 menubox 라는 Class로 래핑하여 메뉴 객체로 재사용함)

## 컴파일
해당 프로젝트는 WSL Ubuntu 18.04 환경에서 g++ 7.5.0 버전 컴파일러로 제작 됐습니다.  
컴파일을 원하면 해당 프로젝트 폴더 안에서 아래 명령어를 입력해주세요.
```
./run.sh && ./main
```

## 참고할만한 내용
NCURSES 한글 출력  
https://bakyeono.net/post/2015-05-12-ncurses-korean-utf-8.html
