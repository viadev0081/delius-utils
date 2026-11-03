#if __has_include(<ncursesw/ncurses.h>)
    #include <ncursesw/ncurses.h>
#else
    #include <ncurses.h>
#endif

#include <clocale>
#include <cstdio>
#include <string>

#include "./lib/lib.hpp"

using namespace cash_terminal_lib;

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    string store = "NEGRITOS MARKET";
    string cashier = "ADMIN";

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printf("usage: %s [--store NAME] [--cashier NAME]\n", argv[0]);
            printf("  --store NAME     store name shown in the header and receipts\n");
            printf("  --cashier NAME   cashier name shown in the header and receipts\n");
            printf("  --help, -h       show this help and exit\n");
            return 0;
        }

        if (arg == "--store") {
            if (i + 1 >= argc) {
                fprintf(stderr, "missing value for --store\n");
                return 1;
            }
            store = argv[++i];
        }
        else if (arg == "--cashier") {
            if (i + 1 >= argc) {
                fprintf(stderr, "missing value for --cashier\n");
                return 1;
            }
            cashier = argv[++i];
        }
    }

    string root = appRoot(argv[0]);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(500);

    initColors();

    App a;
    a.storeName = store;
    a.cashier = cashier;
    initApp(a, root);
    layout(a);

    while (a.running) {
        updateClock(a);
        drawAll(a);

        wint_t wch = 0;
        int rc = get_wch(&wch);

        if (rc == ERR) {
            continue;
        }

        if (rc == KEY_CODE_YES) {
            handleKey(a, (int)wch, true);
        }
        else if (rc == OK) {
            handleKey(a, (int)wch, false);
        }
    }

    closeWindows(a);
    endwin();

    printf(
        "Смена завершена · чеков %d · выручка %s\n",
        a.shift.sales,
        formatMoney(a.shift.revenue).c_str()
    );

    return 0;
}
