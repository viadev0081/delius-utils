#if __has_include(<ncursesw/ncurses.h>)
    #include <ncursesw/ncurses.h>
#else
    #include <ncurses.h>
#endif

#include <locale.h>
#include <string>
#include <cstring>

const int HEIGHT = 34;
const int WIDTH = 136;
const int PAGE_COUNT = 3;

void drawPage(WINDOW * win, int page)
{
    int height, width;
    getmaxyx(win, height, width);
    std::string labels[PAGE_COUNT];
    int totalWidth = 0;
    int x = (width - totalWidth) / 2;
    int y = height - 2;

    werase(win);
    wattron(win, COLOR_PAIR(1));
    box(win, 0, 0);

    const char *title = " Viadev0081's Fastfetch Logos ";
    int titleX = (width - static_cast<int>(strlen(title))) / 2;
    mvwaddstr(win, 0, titleX, title);

    wattroff(win, COLOR_PAIR(1));

    if (page == 0) {
        wattron(win, COLOR_PAIR(1));

        init_pair(1, COLOR_CYAN, -1);

        mvwprintw(win, 1,  1, "Arch Logos: ", COLOR_CYAN);
        mvwprintw(win, 4,  1, "                       ▄                         ", COLOR_CYAN);
        mvwprintw(win, 5,  1, "                      ▟█▙                        ", COLOR_CYAN);
        mvwprintw(win, 6,  1, "                     ▟███▙                                          /\\                 ", COLOR_CYAN);
        mvwprintw(win, 7,  1, "                    ▟█████▙                                        /  \\                ", COLOR_CYAN);
        mvwprintw(win, 8,  1, "                   ▟███████▙                                      /    \\               ", COLOR_CYAN);
        mvwprintw(win, 9,  1, "                  ▂▔▀▜██████▙                                    /      \\              ", COLOR_CYAN);
        mvwprintw(win, 10, 1, "                 ▟██▅▂▝▜█████▙                                   \\       \\            ", COLOR_CYAN);
        mvwprintw(win, 11, 1, "                ▟█████████████▙                                __/        \\            ", COLOR_CYAN);
        mvwprintw(win, 12, 1, "               ▟███████████████▙                              /            \\           (BOLSHE NEMA LOGOTIPOV) ", COLOR_CYAN);
        mvwprintw(win, 13, 1, "              ▟█████████████████▙                            /     .--.     \\          ", COLOR_CYAN);
        mvwprintw(win, 14, 1, "             ▟███████████████████▙                          /     |    |    /           ", COLOR_CYAN);
        mvwprintw(win, 15, 1, "            ▟█████████▛▀▀▜████████▙                        /      |    |    \\__        ", COLOR_CYAN);
        mvwprintw(win, 16, 1, "           ▟████████▛      ▜███████▙                      /    . /       \\ .   \\      ", COLOR_CYAN);
        mvwprintw(win, 17, 1, "          ▟█████████        ████████▙                    /  _/              \\_  \\     ", COLOR_CYAN);
        mvwprintw(win, 18, 1, "         ▟██████████        █████▆▅▄▃▂                  /__/                  \\__\\    ", COLOR_CYAN);
        mvwprintw(win, 19, 1, "        ▟██████████▛        ▜█████████▙                                                 ", COLOR_CYAN);
        mvwprintw(win, 20, 1, "       ▟██████▀▀▀              ▀▀██████▙                                                ", COLOR_CYAN);
        mvwprintw(win, 21, 1, "      ▟███▀▘                       ▝▀███▙                                               ", COLOR_CYAN);
        mvwprintw(win, 22, 1, "     ▟▛▀                               ▀▜▙                                              ", COLOR_CYAN);
        mvwprintw(win, 25, 1, "                                                                                        ", COLOR_CYAN);
        wattroff(win, COLOR_PAIR(1));
    }
    else if (page == 1) {
        wattron(win, COLOR_PAIR(1));

        init_pair(1, COLOR_GREEN, -1);

        mvwprintw(win, 1, 1, "Void Logos: ", COLOR_GREEN);
        mvwprintw(win, 4,  1, "                  █████████████             ", COLOR_GREEN);
        mvwprintw(win, 5,  1, "              ████████████████████          ", COLOR_GREEN);
        mvwprintw(win, 6,  1, "               █████████████████████        ", COLOR_GREEN);
        mvwprintw(win, 7,  1, "                ███        ██████████       ", COLOR_GREEN);
        mvwprintw(win, 8,  1, "         ████                 ████████      ", COLOR_GREEN);
        mvwprintw(win, 9,  1, "        ██████                  ███████     ", COLOR_GREEN);
        mvwprintw(win, 10, 1, "       ███████      ██████       ███████    ", COLOR_GREEN);
        mvwprintw(win, 11, 1, "       ██████      ████████      ███████    ", COLOR_GREEN);
        mvwprintw(win, 12, 1, "       ██████     ██████████      ██████                   (BOLSHE NEMA LOGOTIPOV) ", COLOR_GREEN);
        mvwprintw(win, 13, 1, "       ██████     ██████████       ██████   ", COLOR_GREEN);
        mvwprintw(win, 14, 1, "       ███████     ████████       ██████    ", COLOR_GREEN);
        mvwprintw(win, 15, 1, "       ████████      ████         ██████    ", COLOR_GREEN);
        mvwprintw(win, 16, 1, "        ███████                   █████     ", COLOR_GREEN);
        mvwprintw(win, 17, 1, "         ████████                   ███     ", COLOR_GREEN);
        mvwprintw(win, 18, 1, "          ██████████       █████            ", COLOR_GREEN);
        mvwprintw(win, 19, 1, "            ██████████████████████          ", COLOR_GREEN);
        mvwprintw(win, 20, 1, "              ███████████████████           ", COLOR_GREEN);
        mvwprintw(win, 21, 1, "                  █████████                 ", COLOR_GREEN);

        wattroff(win, COLOR_PAIR(1));
    }
    else if (page == 2) {
        wattron(win, COLOR_PAIR(1));

        init_pair(1, COLOR_CYAN, -1);

        mvwprintw(win, 1,  1, "Artix Logos: ", COLOR_CYAN);
        mvwprintw(win, 4,   1, "                       ▄                    ", COLOR_CYAN);
        mvwprintw(win, 5,   1, "                      ▟█▙                   ", COLOR_CYAN);
        mvwprintw(win, 6,   1, "                     ▟███▙                  ", COLOR_CYAN);
        mvwprintw(win, 7,   1, "                    ▟█████▙                 ", COLOR_CYAN);
        mvwprintw(win, 8,   1, "                   ▟███████▙                ", COLOR_CYAN);
        mvwprintw(win, 9,   1, "                  ▟█████████▙               ", COLOR_CYAN);
        mvwprintw(win, 10,  1, "                 ▟███████████▙              ", COLOR_CYAN);
        mvwprintw(win, 11,  1, "                ▟█████████████▙             ", COLOR_CYAN);
        mvwprintw(win, 12,  1, "                   ▜███████████▙            ", COLOR_CYAN);
        mvwprintw(win, 13,  1, "              ▟█▙      ▜████████▙           ", COLOR_CYAN);
        mvwprintw(win, 14,  1, "             ▟██████▙      ▜█████▙                         (BOLSHE NEMA LOGOTIPOV) ", COLOR_CYAN);
        mvwprintw(win, 15,  1, "            ▟████████████▙   ▜████▙         ", COLOR_CYAN);
        mvwprintw(win, 16,  1, "           ▟███████████████▙    ▜██▙        ", COLOR_CYAN);
        mvwprintw(win, 17,  1, "          ▟██████████████████▙              ", COLOR_CYAN);
        mvwprintw(win, 18,  1, "         ▟███████████████████▀  ▂████▙      ", COLOR_CYAN);
        mvwprintw(win, 19,  1, "        ▟███████████████▀▀    ▂▂██████▙     ", COLOR_CYAN);
        mvwprintw(win, 20,  1, "       ▟███████████▀▀       ▄▄█████████▙    ", COLOR_CYAN);
        mvwprintw(win, 21,  1, "      ▟███████▀▀          ▀█████████████▙   ", COLOR_CYAN);
        mvwprintw(win, 22,  1, "     ▟████▀▀                  ▀▀█████████▙  ", COLOR_CYAN);
        mvwprintw(win, 23,  1, "    ▟██▀                          ▀▀██████▙ ", COLOR_CYAN);

        wattroff(win, COLOR_PAIR(1));
    }

    mvwprintw(win, y, 1, "Press 'q' to exit");

    for (int i = 0; i < PAGE_COUNT; ++i) {
        labels[i] = (i == page)
            ? "[" + std::to_string(i + 1) + "]"
            : " " + std::to_string(i + 1) + " ";

        totalWidth += static_cast<int>(labels[i].size());
        if (i > 0) {
            totalWidth += 2;
        }
    }

    for (int i = 0; i < PAGE_COUNT; ++i) {
        if (i > 0) {
            x += 2;
        }

        if (i == page) {
            wattron(win, A_REVERSE);
        }

        mvwprintw(win, y, x, "%s", labels[i].c_str());

        if (i == page) {
            wattroff(win, A_REVERSE);
        }

        x += static_cast<int>(labels[i].size());
    }

    wrefresh(win);
}

int main()
{
    setlocale(LC_ALL, "");

    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int starty = (LINES - HEIGHT) / 2;
    int startx = (COLS - WIDTH) / 2;

    WINDOW * win = newwin(HEIGHT, WIDTH, starty, startx);
    if (win == nullptr) {
        endwin();
        return 1;
    }

    wbkgdset(win, ' ' | COLOR_PAIR(0));
    keypad(win, TRUE);

    int currentPage = 0;
    drawPage(win, currentPage);

    bool running = true;

    while (running) {
        int key = wgetch(win);

        switch (key) {
            case KEY_LEFT:
                if (currentPage > 0) {
                    --currentPage;
                }
            break;

            case KEY_RIGHT:
                if (currentPage < PAGE_COUNT - 1) {
                    ++currentPage;
                }
            break;

            case KEY_RESIZE:
                resizeterm(0, 0);
                wresize(win, LINES, COLS);
                mvwin(win, 0, 0);
            break;

            case 'q':
            case 'Q':
                running = false;
            break;
        }

        if (running) {
            drawPage(win, currentPage);
        }
    }

    delwin(win);
    endwin();
    return 0;
}
