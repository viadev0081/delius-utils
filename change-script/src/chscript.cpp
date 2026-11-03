#if __has_include(<ncursesw/ncurses.h>)
    #include <ncursesw/ncurses.h>
#else
    #include <ncurses.h>
#endif

#include <algorithm>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "./lib/lib.hpp"

using namespace change_script_lib;

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    const char *homeEnv = getenv("HOME");
    string home = homeEnv ? homeEnv : ".";

    bool listMode = false;
    bool listHidden = false;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--list") {
            listMode = true;
        }
        else if (arg == "--all" || arg == "-a") {
            listHidden = true;
        }
        else if (arg == "--help" || arg == "-h") {
            printf("usage: %s [--list] [--all]\n", argv[0]);
            printf("  --list   print all discovered scripts and exit\n");
            printf("  --all    include hidden directories (also toggled with 'a')\n");
            return 0;
        }
    }
    if (listMode) {
        printList(home, listHidden);
        return 0;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    App a;
    a.home = home;

    // Show something while scanning (large home directories can take a moment).
    mvaddstr(0, 0, "Scanning for .sh / .bash / .zsh / .fish files ...");
    refresh();

    a.scripts = findScripts(home, a.includeHidden);

    layout(a);
    if (!a.menuWin || !a.outWin) {
        endwin();
        fprintf(stderr, "Terminal is too small (need at least 20x4).\n");
        return 1;
    }
    if (!a.scripts.empty())
        a.message = to_string(a.scripts.size()) + " script(s)";
    else
        a.message = "no scripts found";

    bool running = true;
    while (running) {
        drawAll(a);

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (a.focusOutput) {
                    scrollOutput(a, -1);
                }
                else {
                    moveSelection(a, -1);
                }
            break;

            case KEY_DOWN:
                if (a.focusOutput) {
                    scrollOutput(a, 1);
                }
                else {
                    moveSelection(a, 1);
                }
            break;

            case KEY_PPAGE:
                scrollOutput(a, -max(1, a.outH - 3));
            break;

            case KEY_NPAGE:
                scrollOutput(a, max(1, a.outH - 3));
            break;

            case KEY_HOME:
                if (a.focusOutput) {
                    a.outScroll = 0;
                }
                else {
                    a.selected = 0;
                }
            break;

            case KEY_END:
                if (a.focusOutput) {
                    a.outScroll = max(0, (int)a.wrapped.size() - (a.outH - 2));
                }
                else if (!a.scripts.empty()) {
                    a.selected = (int)a.scripts.size() - 1;
                }
            break;

            case '\t':
                a.focusOutput = !a.focusOutput;
            break;

            case '\n':

            case '\r':

            case KEY_ENTER:
                runSelected(a);
            break;

            case 'r':

            case 'R':
                rescan(a);
            break;

            case 'a':

            case 'A':
                a.includeHidden = !a.includeHidden;
                rescan(a);
            break;

            case 'q':

            case 'Q':
                running = false;
            break;

            case KEY_RESIZE:
                layout(a);
            break;

            default:
                break;
        }
    }

    closeWindows(a);
    endwin();
    return 0;
}
