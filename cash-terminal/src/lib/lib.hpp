#ifndef CASH_TERMINAL_LIB_HPP
#define CASH_TERMINAL_LIB_HPP

#if __has_include(<ncursesw/ncurses.h>)
    #include <ncursesw/ncurses.h>
#else
    #include <ncurses.h>
#endif

#include <algorithm>
#include <cctype>
#include <climits>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

enum ColorPair {
    CP_BAR = 1,
    CP_ACCENT,
    CP_MUTED,
    CP_WARN,
    CP_DANGER,
    CP_SELECT,
    CP_SELECT_ALT,
    CP_TOTAL,
    CP_BOX,
    CP_BOX_ACTIVE,
    CP_GOOD,
    CP_TITLE,
};

enum class Mode {
    Sale,
    Search,
    Payment,
    QuickItem,
    Barcode,
    Discount,
    Report,
    Receipt,
    Help,
    ConfirmQuit,
    ConfirmClear,
};

enum class PaymentMethod {
    Cash,
    Card,
};

struct Product {
    string barcode;
    string name;
    string category;
    long long price = 0;       // price in kopecks
};

struct CartLine {
    string name;
    string category;
    long long price = 0;       // unit price in kopecks
    int qty = 1;
    bool custom = false;
};

struct Sale {
    string id;
    string time;
    vector<CartLine> lines;
    long long subtotal = 0;
    long long discount = 0;
    long long total = 0;
    int discountPercent = 0;
    PaymentMethod method = PaymentMethod::Cash;
    long long tendered = 0;
    long long change = 0;
};

struct ShiftStats {
    int sales = 0;
    int items = 0;
    long long revenue = 0;
    long long cash = 0;
    long long card = 0;
    long long discounts = 0;
    string startedAt;
};

struct App {
    WINDOW *headerWin = nullptr;
    WINDOW *catalogWin = nullptr;
    WINDOW *cartWin = nullptr;
    WINDOW *modalWin = nullptr;

    int termH = 0, termW = 0;
    int catalogH = 0, catalogW = 0;
    int cartH = 0, cartW = 0;
    bool ready = false;

    string rootDir;
    string dataDir;
    string receiptsDir;
    string storeName = "ДЕЛЬТА МАРКЕТ";
    string cashier = "ADMIN";
    string shiftLabel;

    vector<Product> products;
    vector<string> categories;
    vector<CartLine> cart;
    vector<int> visible;
    int selected = 0;
    int catalogScroll = 0;
    int categoryIndex = 0;
    int cartSelected = 0;
    int cartScroll = 0;
    bool focusCart = false;

    string search;
    string message;
    int messagePair = CP_MUTED;
    string clockText;

    Mode mode = Mode::Sale;
    PaymentMethod paymentMethod = PaymentMethod::Cash;
    string input;
    string quickName;
    string quickPrice;
    bool quickNameActive = true;
    int discountPercent = 0;
    long long tendered = 0;
    long long changeDue = 0;
    string error;

    string modalTitle;
    string modalHint;
    int modalScroll = 0;
    vector<string> modalLines;
    Sale lastSale;
    string lastReceiptPath;
    ShiftStats shift;

    bool running = true;
};

namespace cash_terminal_lib {
    int utf8Len(unsigned char c);

    int charWidth(
        const string &s,
        size_t i,
        int len
    );

    int textWidth(const string &s);
    string fitText(const string &s, int width);
    string padRight(const string &s, int width);
    string padLeft(const string &s, int width);
    string twoCols(
        const string &left,
        const string &right,
        int width
    );

    string repeatText(
        const string &unit,
        int count
    );

    string codepointToUtf8(unsigned int cp);
    void popUtf8(string &s);
    string trim(const string &s);
    string lowerText(const string &s);

    bool containsText(
        const string &haystack,
        const string &needle
    );

    void wrapLine(
        const string &line,
        int width,
        vector<string> &out
    );

    string formatMoney(
        long long cents,
        bool withSymbol = true
    );

    bool parseMoneyInput(
        const string &raw,
        long long &cents
    );

    string nowClock();
    string nowDate();
    string dateTimeText();
    string receiptStamp();

    string appRoot(const char *argv0);
    void initColors();
    void ensureDirs(App &a);
    vector<Product> defaultProducts();
    vector<Product> loadProducts(const string &path);

    void saveProducts(
        const string &path,
        const vector<Product> &products
    );

    void initApp(
        App &a,
        const string &rootDir
    );

    void sortProducts(vector<Product> &products);
    void rebuildCategories(App &a);
    void rebuildVisible(App &a);
    int currentProductIndex(App &a);

    void addProductToCart(
        App &a,
        const Product &p
    );

    void addSelected(App &a);
    void changeCartQty(App &a, int delta);
    void removeCartLine(App &a);
    void clearCart(App &a);
    int cartItems(App &a);
    long long cartSubtotal(App &a);
    long long cartDiscount(App &a);
    long long cartTotal(App &a);

    void closeWindows(App &a);
    void closeModal(App &a);
    bool isModalMode(Mode mode);
    void layout(App &a);
    void clampCatalog(App &a);
    void clampCart(App &a);
    void moveCatalog(App &a, int delta);
    void moveCart(App &a, int delta);
    void cycleCategory(App &a, int delta);

    void drawFrame(
        WINDOW *win,
        int h,
        int w,
        const string &title,
        int borderPair,
        bool active,
        bool doubleLine = false,
        bool centerTitle = false
    );

    void drawFrameHint(
        WINDOW *win,
        int h,
        int w,
        const string &hint,
        int pair
    );

    void drawScrollbar(
        WINDOW *win,
        int x,
        int y,
        int h,
        int total,
        int scroll,
        int visible,
        int pair
    );

    void drawValueRow(
        WINDOW *win,
        int y,
        int x,
        int width,
        const string &label,
        const string &value,
        int pair,
        bool bold
    );

    void drawInputField(
        WINDOW *win,
        int y,
        int x,
        int width,
        const string &text,
        bool active
    );

    void drawHeader(App &a);
    void drawCatalog(App &a);
    void drawCart(App &a);
    void drawStatus(App &a);
    void drawModal(App &a);
    void drawAll(App &a);
    void updateClock(App &a);

    void startSearch(App &a);
    void finishSearch(App &a);
    void startPayment(App &a);
    void startQuickItem(App &a);
    void startBarcode(App &a);
    void startDiscount(App &a);
    void openHelp(App &a);
    void openReport(App &a);
    void openLastReceipt(App &a);
    void openReceipt(App &a, const Sale &sale);
    void openConfirm(
        App &a,
        Mode mode,
        const string &title,
        const vector<string> &lines
    );

    void openModal(App &a, Mode mode);
    void scrollModal(App &a, int delta);
    vector<string> wrapModalLines(App &a);
    int maxModalScroll(App &a);

    void applyDiscount(App &a);
    void addQuickItem(App &a);
    void findBarcode(App &a);
    void finishPayment(App &a);
    void requestQuit(App &a);
    void setMessage(
        App &a,
        const string &text,
        int pair
    );

    string receiptText(
        App &a,
        const Sale &sale,
        vector<string> &lines
    );

    string nextReceiptId(App &a);
    bool saveReceipt(
        App &a,
        const Sale &sale,
        const vector<string> &lines
    );

    bool logSale(App &a, const Sale &sale);

    bool isPrintableCodepoint(int codepoint);
    void handleKey(
        App &a,
        int key,
        bool isCode
    );

    void handleSaleKey(
        App &a,
        int key,
        bool isCode
    );

    void handleSearchKey(
        App &a,
        int key,
        bool isCode
    );

    void handlePaymentKey(
        App &a,
        int key,
        bool isCode
    );

    void handleQuickItemKey(
        App &a,
        int key,
        bool isCode
    );

    void handleBarcodeKey(
        App &a,
        int key,
        bool isCode
    );

    void handleDiscountKey(
        App &a,
        int key,
        bool isCode
    );

    void handleTextModalKey(App &a, int key);
    void handleConfirmKey(
        App &a,
        int key,
        bool isCode
    );
}

#endif
