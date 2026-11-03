#ifndef CHANGE_SCRIPT_LIB_HPP
#define CHANGE_SCRIPT_LIB_HPP

#if __has_include(<ncursesw/ncurses.h>)
    #include <ncursesw/ncurses.h>
#else
    #include <ncurses.h>
#endif

#include <algorithm>
#include <cctype>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

struct Script {
    string path;
    string label;   // "~/relative/path"
};

struct App {
    WINDOW *menuWin = nullptr;
    WINDOW *outWin = nullptr;

    int termH = 0, termW = 0;
    int menuH = 0, menuW = 0;
    int outH = 0, outW = 0;

    string home;
    bool includeHidden = false;
    vector<Script> scripts;
    int selected = 0;
    int menuScroll = 0;

    vector<string> rawOut;
    vector<string> wrapped;
    int outWrapW = -1;
    int outScroll = 0;

    bool focusOutput = false;
    string message;
};

namespace change_script_lib {
    int utf8Len(unsigned char c);

    int charWidth(
        const string &s, 
        size_t i, 
        int len
    );

    int textWidth(const string &s);
    string fitText(const string &s, int width);
    string stripAnsi(const string &in);
    
    void wrapLine(
        const string &line, 
        int width, 
        vector<string> &out
    );

    bool hasScriptExtension(const string &name);
    bool skipDirectory(const fs::path &p, bool includeHidden);
 
    vector<Script> findScripts(
        const string &home, 
        bool includeHidden
    );

    string shellQuote(const string &s);
    string lowerExtension(const string &path);
    string buildExecCommand(const string &path);
    
    void captureOutput(
        const string &cmd, 
        vector<string> &lines, 
        string &result
    );

    void closeWindows(App &a);
    void layout(App &a);
    void rewrapOutput(App &a);
    void clampMenuScroll(App &a);
    void drawMenu(App &a);
    void drawOutput(App &a);
    void drawStatus(App &a);
    void drawAll(App &a);
    void scrollOutput(App &a, int delta);
    void moveSelection(App &a, int delta);
    void runSelected(App &a);
    void rescan(App &a);
    
    void printList(
        const string &home, 
        bool includeHidden
    );
}

#endif
