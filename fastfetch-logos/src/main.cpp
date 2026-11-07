#if __has_include(<ncursesw/ncurses.h>)
    #include <ncursesw/ncurses.h>
#else
    #include <ncurses.h>
#endif

#include <locale.h>
#include <string>
#include <cstring>
#include <cstdio>

const int HEIGHT = 34;
const int WIDTH = 136;
const int PAGE_COUNT = 4;

enum ColorPair {
    PAIR_FRAME  = 1,   // рамка и заголовок окна — белый
    PAIR_GARUDA = 2,   // логотип GARUDA — красный
    PAIR_MINT   = 3,   // логотип MINT — зелёный
    PAIR_ARCH   = 4,   // страница Arch — циан
    PAIR_VOID   = 5,   // страница Void — зелёный
    PAIR_ARTIX  = 6    // страница Artix — циан
};

static void printPair(
    WINDOW * win,
    int y,
    int x,
    const char * text,
    int pair
) {
    wattron(win, COLOR_PAIR(pair));
    mvwaddstr(win, y, x, text);
    wattroff(win, COLOR_PAIR(pair));
}

void drawPage(WINDOW * win, int page)
{
    int height, width;
    getmaxyx(win, height, width);
    std::string labels[PAGE_COUNT];
    int totalWidth = 0;

    werase(win);

    // Рамка и заголовок всегда белые (PAIR_FRAME).
    wattron(win, COLOR_PAIR(PAIR_FRAME));
    box(win, 0, 0);

    const char *title = " Viadev0081's Fastfetch Logos ";
    int titleX = (width - static_cast<int>(strlen(title))) / 2;
    mvwaddstr(win, 0, titleX, title);
    wattroff(win, COLOR_PAIR(PAIR_FRAME));

    if (page == 0) {
        printPair(win, 1, 1, "Arch Logos: ", PAIR_ARCH);
        printPair(win,  4, 1, "                       ▄                         ", PAIR_ARCH);
        printPair(win,  5, 1, "                      ▟█▙                        ", PAIR_ARCH);
        printPair(win,  6, 1, "                     ▟███▙                                          /\\                 ", PAIR_ARCH);
        printPair(win,  7, 1, "                    ▟█████▙                                        /  \\                ", PAIR_ARCH);
        printPair(win,  8, 1, "                   ▟███████▙                                      /    \\               ", PAIR_ARCH);
        printPair(win,  9, 1, "                  ▂▔▀▜██████▙                                    /      \\              ", PAIR_ARCH);
        printPair(win, 10, 1, "                 ▟██▅▂▝▜█████▙                                   \\       \\            ", PAIR_ARCH);
        printPair(win, 11, 1, "                ▟█████████████▙                                __/        \\            ", PAIR_ARCH);
        printPair(win, 12, 1, "               ▟███████████████▙                              /            \\           (BOLSHE NEMA LOGOTIPOV) ", PAIR_ARCH);
        printPair(win, 13, 1, "              ▟█████████████████▙                            /     .--.     \\          ", PAIR_ARCH);
        printPair(win, 14, 1, "             ▟███████████████████▙                          /     |    |    /           ", PAIR_ARCH);
        printPair(win, 15, 1, "            ▟█████████▛▀▀▜████████▙                        /      |    |    \\__        ", PAIR_ARCH);
        printPair(win, 16, 1, "           ▟████████▛      ▜███████▙                      /    . /       \\ .   \\      ", PAIR_ARCH);
        printPair(win, 17, 1, "          ▟█████████        ████████▙                    /  _/              \\_  \\     ", PAIR_ARCH);
        printPair(win, 18, 1, "         ▟██████████        █████▆▅▄▃▂                  /__/                  \\__\\    ", PAIR_ARCH);
        printPair(win, 19, 1, "        ▟██████████▛        ▜█████████▙                                                 ", PAIR_ARCH);
        printPair(win, 20, 1, "       ▟██████▀▀▀              ▀▀██████▙                                                ", PAIR_ARCH);
        printPair(win, 21, 1, "      ▟███▀▘                       ▝▀███▙                                               ", PAIR_ARCH);
        printPair(win, 22, 1, "     ▟▛▀                               ▀▜▙                                              ", PAIR_ARCH);
        printPair(win, 25, 1, "                                                                                        ", PAIR_ARCH);
    }
    else if (page == 1) {
        printPair(win, 1, 1, "Void Logos: ", PAIR_VOID);
        printPair(win,  4, 1, "                  █████████████             ", PAIR_VOID);
        printPair(win,  5, 1, "              ████████████████████          ", PAIR_VOID);
        printPair(win,  6, 1, "               █████████████████████        ", PAIR_VOID);
        printPair(win,  7, 1, "                ███        ██████████       ", PAIR_VOID);
        printPair(win,  8, 1, "         ████                 ████████      ", PAIR_VOID);
        printPair(win,  9, 1, "        ██████                  ███████     ", PAIR_VOID);
        printPair(win, 10, 1, "       ███████      ██████       ███████    ", PAIR_VOID);
        printPair(win, 11, 1, "       ██████      ████████      ███████    ", PAIR_VOID);
        printPair(win, 12, 1, "       ██████     ██████████      ██████                   (BOLSHE NEMA LOGOTIPOV) ", PAIR_VOID);
        printPair(win, 13, 1, "       ██████     ██████████       ██████   ", PAIR_VOID);
        printPair(win, 14, 1, "       ███████     ████████       ██████    ", PAIR_VOID);
        printPair(win, 15, 1, "       ████████      ████         ██████    ", PAIR_VOID);
        printPair(win, 16, 1, "        ███████                   █████     ", PAIR_VOID);
        printPair(win, 17, 1, "         ████████                   ███     ", PAIR_VOID);
        printPair(win, 18, 1, "          ██████████       █████            ", PAIR_VOID);
        printPair(win, 19, 1, "            ██████████████████████          ", PAIR_VOID);
        printPair(win, 20, 1, "              ███████████████████           ", PAIR_VOID);
        printPair(win, 21, 1, "                  █████████                 ", PAIR_VOID);
    }
    else if (page == 2) {
        printPair(win, 1, 1, "Artix Logos: ", PAIR_ARTIX);
        printPair(win,  4, 1, "                       ▄                                          ▄", PAIR_ARTIX);
        printPair(win,  5, 1, "                      ▟█▙                                        ▟█▙", PAIR_ARTIX);
        printPair(win,  6, 1, "                     ▟███▙                                      ▟███▙", PAIR_ARTIX);
        printPair(win,  7, 1, "                    ▟█████▙                                    ▟█████▙", PAIR_ARTIX);
        printPair(win,  8, 1, "                   ▟███████▙                                  ▟███████▙", PAIR_ARTIX);
        printPair(win,  9, 1, "                  ▟█████████▙                                ▟█████████▙", PAIR_ARTIX);
        printPair(win, 10, 1, "                 ▟███████████▙                              ▟███████████▙", PAIR_ARTIX);
        printPair(win, 11, 1, "                ▟█████████████▙                            ▐█████████████▙", PAIR_ARTIX);
        printPair(win, 12, 1, "                   ▜███████████▙                             ▀▀███████████▙", PAIR_ARTIX);
        printPair(win, 13, 1, "              ▟█▙      ▜████████▙                        ▄▄▂     ▀▀████████▙", PAIR_ARTIX);
        printPair(win, 14, 1, "             ▟██████▙      ▜█████▙                      ▟████▆▅▄▂    ▀▀█████▙", PAIR_ARTIX);
        printPair(win, 15, 1, "            ▟████████████▙   ▜████▙                    ▟██████████▆▅▄▂   ▀▀██▙", PAIR_ARTIX);
        printPair(win, 16, 1, "           ▟███████████████▙    ▜██▙                  ▟████████████████▆▅▄▂  ▀", PAIR_ARTIX);
        printPair(win, 17, 1, "          ▟██████████████████▙                       ▟█████████████████████▀ ", PAIR_ARTIX);
        printPair(win, 18, 1, "         ▟███████████████████▀  ▂████▙              ▟███████████████████▀▀    ▂▄▄", PAIR_ARTIX);
        printPair(win, 19, 1, "        ▟███████████████▀▀    ▂▂██████▙            ▟███████████████▀▀     ▂▄▆████▙", PAIR_ARTIX);
        printPair(win, 20, 1, "       ▟███████████▀▀       ▄▄█████████▙          ▟███████████▀▀         █████████▙", PAIR_ARTIX);
        printPair(win, 21, 1, "      ▟███████▀▀          ▀█████████████▙        ▟███████▀▀                ▀▀██████▙", PAIR_ARTIX);
        printPair(win, 22, 1, "     ▟████▀▀                  ▀▀█████████▙      ▟███▀▀                         ▀▀███▙", PAIR_ARTIX);
        printPair(win, 23, 1, "    ▟██▀                          ▀▀██████▙    ▟▛▀                                 ▀▜▙", PAIR_ARTIX);
    }
    else if (page == 3) {
        printPair(win, 1, 1, "Other logos: ", PAIR_FRAME);

        printPair(win,  4, 1, "         ████████████████████████    ", PAIR_GARUDA);
        printPair(win,  5, 1, "        ██                      ██   ", PAIR_GARUDA);
        printPair(win,  6, 1, "       ██           ██           ██  ", PAIR_GARUDA);
        printPair(win,  7, 1, "      ██           ██             ██ ", PAIR_GARUDA);
        printPair(win,  8, 1, "     ██           ██              ██ ", PAIR_GARUDA);
        printPair(win,  9, 1, "    ██            ████████████████   ", PAIR_GARUDA);
        printPair(win, 10, 1, "   ██                                ", PAIR_GARUDA);
        printPair(win, 11, 1, "   ██                                ", PAIR_GARUDA);
        printPair(win, 12, 1, "   ██             ███████████████    ", PAIR_GARUDA);
        printPair(win, 13, 1, "   ██                          ██    ", PAIR_GARUDA);
        printPair(win, 14, 1, "    ██                        ██     ", PAIR_GARUDA);
        printPair(win, 15, 1, "     ██                     ██       ", PAIR_GARUDA);
        printPair(win, 16, 1, "      ██████████████████████         ", PAIR_GARUDA);
        printPair(win, 17, 1, "               GARUDA                ", PAIR_GARUDA);

        printPair(win, 19, 1, "      █████████████████████      ", PAIR_MINT);
        printPair(win, 20, 1, "    ██                     ██    ", PAIR_MINT);
        printPair(win, 21, 1, "  ██    ██  ███████████      ██  ", PAIR_MINT);
        printPair(win, 22, 1, "  ██    ██  ██   ██    ██    ██  ", PAIR_MINT);
        printPair(win, 23, 1, "  ██    ██  ██   ██    ██    ██  ", PAIR_MINT);
        printPair(win, 24, 1, "  ██    ██  ██   ██    ██    ██  ", PAIR_MINT);
        printPair(win, 25, 1, "  ██    ██  ██   ██    ██    ██  ", PAIR_MINT);
        printPair(win, 26, 1, "  ██    ██  ██   ██    ██    ██  ", PAIR_MINT);
        printPair(win, 27, 1, "  ██    ██             ██    ██  ", PAIR_MINT);
        printPair(win, 28, 1, "  ██     ██████████████      ██  ", PAIR_MINT);
        printPair(win, 29, 1, "    ██                    ██     ", PAIR_MINT);
        printPair(win, 30, 1, "      ████████████████████       ", PAIR_MINT);
        printPair(win, 31, 1, "               MINT              ", PAIR_MINT);
    }

    mvwaddstr(win, height - 2, 1, "Press 'q' to exit");

    for (int i = 0; i < PAGE_COUNT; ++i) {
        labels[i] = (i == page)
            ? "[" + std::to_string(i + 1) + "]"
            : " " + std::to_string(i + 1) + " ";

        totalWidth += static_cast<int>(labels[i].size());
        if (i > 0) {
            totalWidth += 2;
        }
    }

    int x = (width - totalWidth) / 2;
    int y = height - 2;

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

    if (!has_colors()) {
        endwin();
        std::fprintf(stderr, "Терминал не поддерживает цвет\n");
        return 1;
    }

    start_color();
    use_default_colors();

    init_pair(PAIR_FRAME,  COLOR_WHITE, -1);
    init_pair(PAIR_GARUDA, COLOR_RED,   -1);
    init_pair(PAIR_MINT,   COLOR_GREEN, -1);
    init_pair(PAIR_ARCH,   COLOR_CYAN,  -1);
    init_pair(PAIR_VOID,   COLOR_GREEN, -1);
    init_pair(PAIR_ARTIX,  COLOR_CYAN,  -1);

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

            case KEY_RESIZE: {
                int newy = (LINES - HEIGHT) / 2;
                int newx = (COLS - WIDTH) / 2;
                wresize(win, HEIGHT, WIDTH);
                mvwin(win, newy, newx);
            }
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
