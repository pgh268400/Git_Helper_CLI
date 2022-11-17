class ncurses_typewriter
{
public:
    ncurses_typewriter(int starty, int startx, int width, int height);
    ~ncurses_typewriter();
    void show_typewriter();
    void release_typewriter_resource();
    // string get_typing_string();
};