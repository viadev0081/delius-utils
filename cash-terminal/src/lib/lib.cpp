#include "./lib.hpp"

namespace cash_terminal_lib {
    int utf8Len(unsigned char c)
    {
        if (c < 0x80) {
            return 1;
        }

        if ((c >> 5) == 0x06) {
            return 2;
        }

        if ((c >> 4) == 0x0E) {
            return 3;
        }

        if ((c >> 3) == 0x1E) {
            return 4;
        }

        return 1;
    }

    int charWidth(
        const string &s,
        size_t i,
        int len
    ) {
        wchar_t wc = 0;
        mbstate_t st;
        memset(&st, 0, sizeof(st));
        size_t n = mbrtowc(&wc, s.c_str() + i, len, &st);

        if (n == (size_t)-1 || n == (size_t)-2) {
            return 1;
        }

        int w = wcwidth(wc);
        return w < 0 ? 1 : w;
    }

    int textWidth(const string &s)
    {
        int width = 0;
        for (size_t i = 0; i < s.size();) {
            int len = utf8Len((unsigned char)s[i]);

            if (i + len > s.size()) {
                len = 1;
            }

            width += charWidth(s, i, len);
            i += len;
        }
        return width;
    }

    string fitText(const string &s, int width)
    {
        if (width <= 0) {
            return "";
        }

        string out;
        int col = 0;
        for (size_t i = 0; i < s.size();) {
            int len = utf8Len((unsigned char)s[i]);

            if (i + len > s.size()) {
                len = 1;
            }

            int w = charWidth(s, i, len);

            if (col + w > width) {
                break;
            }

            out.append(s, i, len);
            col += w;
            i += len;
        }
        return out;
    }

    string padRight(const string &s, int width)
    {
        string t = fitText(s, width);
        int w = textWidth(t);
        if (w < width) {
            t.append(width - w, ' ');
        }
        return t;
    }

    string padLeft(const string &s, int width)
    {
        string t = fitText(s, width);
        int w = textWidth(t);
        if (w < width) {
            return string(width - w, ' ') + t;
        }
        return t;
    }

    string twoCols(
        const string &left,
        const string &right,
        int width
    ) {
        if (width <= 0) {
            return "";
        }

        string l = left;
        string r = fitText(right, width);
        int lw = textWidth(l);
        int rw = textWidth(r);
        int space = width - lw - rw;

        if (space < 0) {
            l = fitText(l, max(0, width - rw - 1));
            lw = textWidth(l);
            space = width - lw - rw;
        }

        if (space < 0) {
            space = 0;
        }

        return l + string(space, ' ') + r;
    }

    string repeatText(
        const string &unit,
        int count
    ) {
        string out;
        for (int i = 0; i < count; ++i) {
            out += unit;
        }
        return out;
    }

    string codepointToUtf8(unsigned int cp)
    {
        string out;

        if (cp <= 0x7F) {
            out.push_back((char)cp);
        }
        else if (cp <= 0x7FF) {
            out.push_back((char)(0xC0 | (cp >> 6)));
            out.push_back((char)(0x80 | (cp & 0x3F)));
        }
        else if (cp <= 0xFFFF) {
            out.push_back((char)(0xE0 | (cp >> 12)));
            out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back((char)(0x80 | (cp & 0x3F)));
        }
        else if (cp <= 0x10FFFF) {
            out.push_back((char)(0xF0 | (cp >> 18)));
            out.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back((char)(0x80 | (cp & 0x3F)));
        }

        return out;
    }

    void popUtf8(string &s)
    {
        if (s.empty()) {
            return;
        }

        size_t i = s.size() - 1;
        while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) {
            --i;
        }
        s.erase(i);
    }

    string trim(const string &s)
    {
        size_t begin = 0;
        size_t end = s.size();

        while (begin < end && isspace((unsigned char)s[begin])) {
            ++begin;
        }

        while (end > begin && isspace((unsigned char)s[end - 1])) {
            --end;
        }

        return s.substr(begin, end - begin);
    }

    string lowerText(const string &s)
    {
        string out;
        mbstate_t inState;
        memset(&inState, 0, sizeof(inState));

        size_t i = 0;
        while (i < s.size()) {
            wchar_t wc = 0;
            size_t n = mbrtowc(
                &wc,
                s.c_str() + i,
                s.size() - i,
                &inState
            );

            if (n == (size_t)-1 || n == (size_t)-2 || n == 0) {
                out.push_back(s[i]);
                ++i;
                memset(&inState, 0, sizeof(inState));
                continue;
            }

            wchar_t lower = (wchar_t)towlower((wint_t)wc);
            char buf[MB_LEN_MAX];
            mbstate_t outState;
            memset(&outState, 0, sizeof(outState));
            size_t m = wcrtomb(buf, lower, &outState);

            if (m == (size_t)-1) {
                out.append(s, i, n);
            }
            else {
                out.append(buf, m);
            }

            i += n;
        }

        return out;
    }

    bool containsText(
        const string &haystack,
        const string &needle
    ) {
        if (needle.empty()) {
            return true;
        }

        return lowerText(haystack).find(lowerText(needle)) != string::npos;
    }

    void wrapLine(
        const string &line,
        int width,
        vector<string> &out
    ) {
        if (width < 1) {
            width = 1;
        }

        string cur;
        int col = 0;

        auto flush = [&]() {
            out.push_back(cur);
            cur.clear();
            col = 0;
        };

        for (size_t i = 0; i < line.size();) {
            unsigned char c = (unsigned char)line[i];

            if (c == '\t') {
                int spaces = 8 - (col % 8);
                while (spaces-- > 0) {
                    if (col >= width) {
                        flush();
                    }
                    cur.push_back(' ');
                    ++col;
                }
                ++i;
                continue;
            }

            int len = utf8Len(c);

            if (i + len > line.size()) {
                len = 1;
            }

            int w = charWidth(line, i, len);

            if (w < 1) {
                w = 1;
            }

            if (col > 0 && col + w > width) {
                flush();
            }

            cur.append(line, i, len);
            col += w;
            i += len;
        }

        out.push_back(cur);
    }

    // -----------------------------------------------------------------------
    // Money and time helpers
    // -----------------------------------------------------------------------
    string formatMoney(
        long long cents,
        bool withSymbol
    ) {
        bool negative = cents < 0;
        unsigned long long value = (unsigned long long)(
            negative ? -cents : cents
        );

        unsigned long long rubles = value / 100;
        int kopecks = (int)(value % 100);

        string digits = to_string(rubles);
        string grouped;

        for (size_t i = 0; i < digits.size(); ++i) {
            if (i > 0 && (digits.size() - i) % 3 == 0) {
                grouped.push_back(' ');
            }
            grouped.push_back(digits[i]);
        }

        char tail[16];
        snprintf(tail, sizeof(tail), ",%02d", kopecks);
        grouped += tail;

        if (withSymbol) {
            grouped += " ₽";
        }

        if (negative) {
            grouped = "-" + grouped;
        }

        return grouped;
    }

    bool parseMoneyInput(
        const string &raw,
        long long &cents
    ) {
        string s = trim(raw);
        if (s.empty()) {
            return false;
        }

        string whole;
        string fraction;
        bool hasDot = false;

        for (char c : s) {
            if (isdigit((unsigned char)c)) {
                if (!hasDot) {
                    whole.push_back(c);
                }
                else if (fraction.size() < 2) {
                    fraction.push_back(c);
                }
            }
            else if (c == ',' || c == '.') {
                if (hasDot) {
                    return false;
                }
                hasDot = true;
            }
            else if (c == ' ' || c == '\t') {
                continue;
            }
            else {
                return false;
            }
        }

        if (whole.empty() && fraction.empty()) {
            return false;
        }

        if (whole.size() > 12) {
            return false;
        }

        long long rubles = 0;
        for (char c : whole) {
            rubles = rubles * 10 + (c - '0');
        }

        int kopecks = 0;
        if (!fraction.empty()) {
            kopecks = (fraction[0] - '0') * 10;
        }
        if (fraction.size() > 1) {
            kopecks += fraction[1] - '0';
        }

        cents = rubles * 100 + kopecks;
        return true;
    }

    static string formatTime(const char *format)
    {
        time_t now = time(nullptr);
        struct tm local;
        localtime_r(&now, &local);

        char buf[64];
        strftime(buf, sizeof(buf), format, &local);
        return string(buf);
    }

    string nowClock()
    {
        return formatTime("%H:%M:%S");
    }

    string nowDate()
    {
        return formatTime("%d.%m.%Y");
    }

    string dateTimeText()
    {
        return formatTime("%d.%m.%Y %H:%M:%S");
    }

    string receiptStamp()
    {
        return formatTime("%Y%m%d-%H%M%S");
    }

    // -----------------------------------------------------------------------
    // Storage and catalog
    // -----------------------------------------------------------------------
    string appRoot(const char *argv0)
    {
        const char *envHome = getenv("CASH_TERMINAL_HOME");
        if (envHome && *envHome) {
            return string(envHome);
        }

        error_code ec;
        fs::path exe = fs::canonical("/proc/self/exe", ec);

        if (!ec && !exe.empty()) {
            fs::path dir = exe.parent_path();
            if (dir.filename() == "bin") {
                return dir.parent_path().string();
            }
            return dir.string();
        }

        fs::path argPath(argv0 ? argv0 : ".");
        fs::path absolute = fs::absolute(argPath, ec);

        if (!ec) {
            fs::path dir = absolute.parent_path();
            if (dir.filename() == "bin") {
                return dir.parent_path().string();
            }
            return dir.string();
        }

        return fs::current_path(ec).string();
    }

    void initColors()
    {
        if (!has_colors()) {
            return;
        }

        start_color();
        use_default_colors();

        init_pair(CP_BAR, COLOR_WHITE, COLOR_BLUE);
        init_pair(CP_ACCENT, COLOR_CYAN, -1);
        init_pair(CP_MUTED, COLOR_WHITE, -1);
        init_pair(CP_WARN, COLOR_YELLOW, -1);
        init_pair(CP_DANGER, COLOR_RED, -1);
        init_pair(CP_SELECT, COLOR_BLACK, COLOR_CYAN);
        init_pair(CP_SELECT_ALT, COLOR_BLACK, COLOR_GREEN);
        init_pair(CP_TOTAL, COLOR_GREEN, -1);
        init_pair(CP_BOX, COLOR_BLUE, -1);
        init_pair(CP_BOX_ACTIVE, COLOR_CYAN, -1);
        init_pair(CP_GOOD, COLOR_GREEN, -1);
        init_pair(CP_TITLE, COLOR_YELLOW, -1);
    }

    void ensureDirs(App &a)
    {
        error_code ec;
        fs::create_directories(a.dataDir, ec);
        ec.clear();
        fs::create_directories(a.receiptsDir, ec);
    }

    vector<Product> defaultProducts()
    {
        return {
            {"4600000000011", "Кофе американо", "Напитки", 15000},
            {"4600000000028", "Капучино", "Напитки", 22000},
            {"4600000000035", "Чай чёрный", "Напитки", 12000},
            {"4600000000042", "Кола 0.5", "Напитки", 18000},
            {"4600000000059", "Вода питьевая 0.5", "Напитки", 9000},
            {"4600000000066", "Круассан с маслом", "Выпечка", 16000},
            {"4600000000073", "Синнабон", "Выпечка", 21000},
            {"4600000000080", "Пирожок с яблоком", "Выпечка", 9000},
            {"4600000000097", "Чипсы картофельные", "Снеки", 14000},
            {"4600000000103", "Шоколад молочный", "Снеки", 13000},
            {"4600000000110", "Печенье овсяное", "Снеки", 11000},
            {"4600000000127", "Пакет-майка", "Прочее", 1000},
            {"4600000000134", "Салфетки", "Прочее", 2000},
        };
    }

    vector<Product> loadProducts(const string &path)
    {
        vector<Product> products;
        ifstream file(path);

        if (!file.is_open()) {
            return products;
        }

        string line;
        while (getline(file, line)) {
            line = trim(line);

            if (line.empty() || line[0] == '#') {
                continue;
            }

            vector<string> parts;
            size_t start = 0;

            while (true) {
                size_t tab = line.find('\t', start);
                if (tab == string::npos) {
                    parts.push_back(line.substr(start));
                    break;
                }

                parts.push_back(line.substr(start, tab - start));
                start = tab + 1;
            }

            if (parts.size() < 4) {
                continue;
            }

            Product p;
            p.barcode = trim(parts[0]);
            p.name = trim(parts[1]);
            p.category = trim(parts[2]);

            if (p.name.empty()) {
                continue;
            }

            try {
                p.price = stoll(trim(parts[3]));
            }
            catch (...) {
                continue;
            }

            if (p.price < 0) {
                continue;
            }

            products.push_back(std::move(p));
        }

        sortProducts(products);
        return products;
    }

    void saveProducts(
        const string &path,
        const vector<Product> &products
    ) {
        error_code ec;
        fs::path parent = fs::path(path).parent_path();

        if (!parent.empty()) {
            fs::create_directories(parent, ec);
        }

        ofstream file(path, ios::trunc);
        if (!file.is_open()) {
            return;
        }

        file << "# barcode\tname\tcategory\tprice_cents\n";

        for (const Product &p : products) {
            file << p.barcode << '\t'
                 << p.name << '\t'
                 << p.category << '\t'
                 << p.price << '\n';
        }
    }

    void sortProducts(vector<Product> &products)
    {
        sort(products.begin(), products.end(), [](const Product &a, const Product &b) {
            string ac = lowerText(a.category);
            string bc = lowerText(b.category);

            if (ac != bc) {
                return ac < bc;
            }

            return lowerText(a.name) < lowerText(b.name);
        });
    }

    void initApp(
        App &a,
        const string &rootDir
    ) {
        a.rootDir = rootDir;
        a.dataDir = rootDir + "/data";
        a.receiptsDir = rootDir + "/receipts";
        a.shift.startedAt = nowClock();
        a.shiftLabel = "СМЕНА " + nowDate();

        ensureDirs(a);

        string catalogPath = a.dataDir + "/products.tsv";
        a.products = loadProducts(catalogPath);

        if (a.products.empty()) {
            a.products = defaultProducts();
            sortProducts(a.products);
            saveProducts(catalogPath, a.products);
        }

        rebuildCategories(a);
        rebuildVisible(a);

        setMessage(a, "Добро пожаловать · выберите товар", CP_MUTED);
    }

    void rebuildCategories(App &a)
    {
        a.categories.clear();
        a.categories.push_back("Все товары");

        for (const Product &p : a.products) {
            bool found = false;

            for (const string &category : a.categories) {
                if (category == p.category) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                a.categories.push_back(p.category);
            }
        }

        sort(a.categories.begin() + 1, a.categories.end(), [](const string &a, const string &b) {
            return lowerText(a) < lowerText(b);
        });

        if (a.categoryIndex < 0 || a.categoryIndex >= (int)a.categories.size()) {
            a.categoryIndex = 0;
        }
    }

    void rebuildVisible(App &a)
    {
        a.visible.clear();

        string query = trim(a.search);
        string selectedCategory;

        if (a.categoryIndex > 0 && a.categoryIndex < (int)a.categories.size()) {
            selectedCategory = a.categories[a.categoryIndex];
        }

        for (size_t i = 0; i < a.products.size(); ++i) {
            const Product &p = a.products[i];

            if (!selectedCategory.empty() && p.category != selectedCategory) {
                continue;
            }

            if (!query.empty() &&
                !containsText(p.name, query) &&
                !containsText(p.category, query) &&
                !containsText(p.barcode, query)) {
                continue;
            }

            a.visible.push_back((int)i);
        }

        if (a.selected < 0) {
            a.selected = 0;
        }

        if (a.selected >= (int)a.visible.size()) {
            a.selected = a.visible.empty() ? 0 : (int)a.visible.size() - 1;
        }
    }

    int currentProductIndex(App &a)
    {
        if (a.selected < 0 || a.selected >= (int)a.visible.size()) {
            return -1;
        }

        return a.visible[a.selected];
    }

    static void pushCartLine(
        App &a,
        const CartLine &line
    ) {
        for (size_t i = 0; i < a.cart.size(); ++i) {
            CartLine &existing = a.cart[i];

            if (existing.name == line.name &&
                existing.category == line.category &&
                existing.price == line.price) {
                if (existing.qty < 99) {
                    ++existing.qty;
                }
                a.cartSelected = (int)i;
                return;
            }
        }

        a.cart.push_back(line);
        a.cartSelected = (int)a.cart.size() - 1;
    }

    void addProductToCart(
        App &a,
        const Product &p
    ) {
        CartLine line;
        line.name = p.name;
        line.category = p.category;
        line.price = p.price;
        line.qty = 1;
        line.custom = false;
        pushCartLine(a, line);
    }

    void addSelected(App &a)
    {
        int index = currentProductIndex(a);

        if (index < 0) {
            setMessage(a, "Товар не выбран", CP_WARN);
            return;
        }

        const Product &p = a.products[index];
        addProductToCart(a, p);
        setMessage(a, "Добавлено: " + p.name, CP_GOOD);
    }

    void changeCartQty(App &a, int delta)
    {
        if (a.cart.empty()) {
            setMessage(a, "Чек пуст", CP_MUTED);
            return;
        }

        if (a.cartSelected < 0 || a.cartSelected >= (int)a.cart.size()) {
            a.cartSelected = 0;
        }

        CartLine &line = a.cart[a.cartSelected];

        if (delta < 0 && line.qty <= 1) {
            setMessage(a, "Минимум 1 · Del — убрать позицию", CP_WARN);
            return;
        }

        line.qty += delta;

        if (line.qty < 1) {
            line.qty = 1;
        }

        if (line.qty > 99) {
            line.qty = 99;
            setMessage(a, "Максимум 99 шт.", CP_WARN);
            return;
        }

        setMessage(a, line.name + " · " + to_string(line.qty) + " шт.", CP_MUTED);
    }

    void removeCartLine(App &a)
    {
        if (a.cart.empty()) {
            return;
        }

        if (a.cartSelected < 0 || a.cartSelected >= (int)a.cart.size()) {
            a.cartSelected = 0;
        }

        a.cart.erase(a.cart.begin() + a.cartSelected);

        if (a.cartSelected >= (int)a.cart.size()) {
            a.cartSelected = a.cart.empty() ? 0 : (int)a.cart.size() - 1;
        }

        setMessage(a, "Позиция удалена", CP_MUTED);
    }

    void clearCart(App &a)
    {
        a.cart.clear();
        a.cartSelected = 0;
        a.cartScroll = 0;
        a.discountPercent = 0;
        setMessage(a, "Чек очищен", CP_MUTED);
    }

    int cartItems(App &a)
    {
        int items = 0;
        for (const CartLine &line : a.cart) {
            items += line.qty;
        }
        return items;
    }

    long long cartSubtotal(App &a)
    {
        long long subtotal = 0;
        for (const CartLine &line : a.cart) {
            subtotal += line.price * line.qty;
        }
        return subtotal;
    }

    long long cartDiscount(App &a)
    {
        long long subtotal = cartSubtotal(a);

        if (a.discountPercent <= 0) {
            return 0;
        }

        return (subtotal * a.discountPercent + 50) / 100;
    }

    long long cartTotal(App &a)
    {
        long long total = cartSubtotal(a) - cartDiscount(a);
        return total < 0 ? 0 : total;
    }

    // -----------------------------------------------------------------------
    // Screen layout
    // -----------------------------------------------------------------------
    void closeModal(App &a)
    {
        if (a.modalWin) {
            delwin(a.modalWin);
            a.modalWin = nullptr;
        }
    }

    void closeWindows(App &a)
    {
        if (a.headerWin) {
            delwin(a.headerWin);
            a.headerWin = nullptr;
        }

        if (a.catalogWin) {
            delwin(a.catalogWin);
            a.catalogWin = nullptr;
        }

        if (a.cartWin) {
            delwin(a.cartWin);
            a.cartWin = nullptr;
        }

        closeModal(a);
    }

    bool isModalMode(Mode mode)
    {
        return mode != Mode::Sale && mode != Mode::Search;
    }

    void layout(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(stdscr, h, w);

        a.termH = h;
        a.termW = w;
        a.ready = false;

        closeWindows(a);
        erase();

        if (h < 24 || w < 80) {
            refresh();
            return;
        }

        const int headerH = 4;
        const int footerH = 2;
        const int gap = 1;
        const int contentY = headerH;
        const int contentH = h - headerH - footerH;

        int catalogW = max(38, w * 54 / 100);

        if (w - catalogW - gap < 34) {
            catalogW = w - gap - 34;
        }

        if (catalogW < 30) {
            catalogW = w / 2;
        }

        int cartW = w - catalogW - gap;

        if (cartW < 24) {
            catalogW = max(30, w - gap - 24);
            cartW = w - catalogW - gap;
        }

        a.catalogW = catalogW;
        a.catalogH = contentH;
        a.cartW = cartW;
        a.cartH = contentH;

        a.headerWin = newwin(headerH, w, 0, 0);
        a.catalogWin = newwin(contentH, catalogW, contentY, 0);
        a.cartWin = newwin(contentH, cartW, contentY, catalogW + gap);

        if (!a.headerWin || !a.catalogWin || !a.cartWin) {
            closeWindows(a);
            refresh();
            return;
        }

        a.ready = true;
    }

    void clampCatalog(App &a)
    {
        int listH = a.catalogH - 4;

        if (listH < 1) {
            listH = 1;
        }

        int total = (int)a.visible.size();

        if (a.selected < 0) {
            a.selected = 0;
        }

        if (a.selected >= total) {
            a.selected = total > 0 ? total - 1 : 0;
        }

        int maxScroll = max(0, total - listH);

        if (a.selected < a.catalogScroll) {
            a.catalogScroll = a.selected;
        }

        if (a.selected >= a.catalogScroll + listH) {
            a.catalogScroll = a.selected - listH + 1;
        }

        if (a.catalogScroll < 0) {
            a.catalogScroll = 0;
        }

        if (a.catalogScroll > maxScroll) {
            a.catalogScroll = maxScroll;
        }
    }

    void clampCart(App &a)
    {
        int sepY = a.cartH - 6;
        int listH = sepY - 1;

        if (listH < 1) {
            listH = 1;
        }

        int total = (int)a.cart.size();

        if (a.cartSelected < 0) {
            a.cartSelected = 0;
        }

        if (a.cartSelected >= total) {
            a.cartSelected = total > 0 ? total - 1 : 0;
        }

        int maxScroll = max(0, total - listH);

        if (a.cartSelected < a.cartScroll) {
            a.cartScroll = a.cartSelected;
        }

        if (a.cartSelected >= a.cartScroll + listH) {
            a.cartScroll = a.cartSelected - listH + 1;
        }

        if (a.cartScroll < 0) {
            a.cartScroll = 0;
        }

        if (a.cartScroll > maxScroll) {
            a.cartScroll = maxScroll;
        }
    }

    void moveCatalog(App &a, int delta)
    {
        if (a.visible.empty()) {
            return;
        }

        a.selected += delta;
        clampCatalog(a);
    }

    void moveCart(App &a, int delta)
    {
        if (a.cart.empty()) {
            return;
        }

        a.cartSelected += delta;
        clampCart(a);
    }

    void cycleCategory(App &a, int delta)
    {
        if (a.categories.empty()) {
            return;
        }

        int count = (int)a.categories.size();
        a.categoryIndex = (a.categoryIndex + delta + count) % count;
        a.selected = 0;
        a.catalogScroll = 0;
        rebuildVisible(a);

        setMessage(
            a,
            "Категория: " + a.categories[a.categoryIndex],
            CP_MUTED
        );
    }

    // -----------------------------------------------------------------------
    // Drawing primitives
    // -----------------------------------------------------------------------
    void drawFrame(
        WINDOW *win,
        int h,
        int w,
        const string &title,
        int borderPair,
        bool active,
        bool doubleLine,
        bool centerTitle
    ) {
        if (!win || h < 2 || w < 2) {
            return;
        }

        string horizontal = doubleLine ? "═" : "─";
        string vertical = doubleLine ? "║" : "│";
        string topLeft = doubleLine ? "╔" : "╭";
        string topRight = doubleLine ? "╗" : "╮";
        string bottomLeft = doubleLine ? "╚" : "╰";
        string bottomRight = doubleLine ? "╝" : "╯";

        string top = topLeft + repeatText(horizontal, w - 2) + topRight;
        string bottom = bottomLeft + repeatText(horizontal, w - 2) + bottomRight;

        wattron(win, COLOR_PAIR(borderPair));
        if (active) {
            wattron(win, A_BOLD);
        }

        mvwaddstr(win, 0, 0, top.c_str());
        mvwaddstr(win, h - 1, 0, bottom.c_str());

        for (int y = 1; y < h - 1; ++y) {
            mvwaddstr(win, y, 0, vertical.c_str());
            mvwaddstr(win, y, w - 1, vertical.c_str());
        }

        if (active) {
            wattroff(win, A_BOLD);
        }
        wattroff(win, COLOR_PAIR(borderPair));

        if (!title.empty() && w > 4) {
            string text = fitText(" " + title + " ", w - 4);
            int x = centerTitle
                ? max(2, (w - textWidth(text)) / 2)
                : 2;

            wattron(win, COLOR_PAIR(active ? CP_TITLE : CP_MUTED));
            if (active) {
                wattron(win, A_BOLD);
            }

            mvwaddstr(win, 0, x, text.c_str());

            if (active) {
                wattroff(win, A_BOLD);
            }
            wattroff(win, COLOR_PAIR(active ? CP_TITLE : CP_MUTED));
        }
    }

    void drawFrameHint(
        WINDOW *win,
        int h,
        int w,
        const string &hint,
        int pair
    ) {
        if (!win || h < 2 || hint.empty() || w < 8) {
            return;
        }

        string text = fitText(" " + hint + " ", w - 4);
        int x = max(2, (w - textWidth(text)) / 2);

        wattron(win, COLOR_PAIR(pair));
        mvwaddstr(win, h - 1, x, text.c_str());
        wattroff(win, COLOR_PAIR(pair));
    }

    void drawScrollbar(
        WINDOW *win,
        int x,
        int y,
        int h,
        int total,
        int scroll,
        int visible,
        int pair
    ) {
        if (!win || h <= 0 || total <= visible) {
            return;
        }

        int thumb = max(1, (h * visible) / total);
        if (thumb > h) {
            thumb = h;
        }

        int maxScroll = total - visible;
        int pos = 0;

        if (maxScroll > 0) {
            pos = (scroll * (h - thumb)) / maxScroll;
        }

        for (int i = 0; i < h; ++i) {
            bool onThumb = i >= pos && i < pos + thumb;

            if (onThumb) {
                wattron(win, COLOR_PAIR(pair) | A_BOLD);
                mvwaddstr(win, y + i, x, "█");
                wattroff(win, COLOR_PAIR(pair) | A_BOLD);
            }
            else {
                wattron(win, COLOR_PAIR(CP_BOX));
                mvwaddstr(win, y + i, x, "░");
                wattroff(win, COLOR_PAIR(CP_BOX));
            }
        }
    }

    void drawValueRow(
        WINDOW *win,
        int y,
        int x,
        int width,
        const string &label,
        const string &value,
        int pair,
        bool bold
    ) {
        string row = twoCols(label, value, width);

        wattron(win, COLOR_PAIR(pair));
        if (bold) {
            wattron(win, A_BOLD);
        }

        mvwaddstr(win, y, x, row.c_str());

        if (bold) {
            wattroff(win, A_BOLD);
        }
        wattroff(win, COLOR_PAIR(pair));
    }

    void drawInputField(
        WINDOW *win,
        int y,
        int x,
        int width,
        const string &text,
        bool active
    ) {
        if (width < 2) {
            return;
        }

        string shown = " " + fitText(text, max(0, width - 3));

        if (active) {
            shown += "█";
            wattron(win, COLOR_PAIR(CP_SELECT) | A_BOLD);
            mvwhline(win, y, x, ' ', width);
            mvwaddstr(win, y, x, shown.c_str());
            wattroff(win, COLOR_PAIR(CP_SELECT) | A_BOLD);
        }
        else {
            wattron(win, COLOR_PAIR(CP_MUTED) | A_UNDERLINE);
            mvwhline(win, y, x, ' ', width);
            mvwaddstr(win, y, x, shown.c_str());
            wattroff(win, COLOR_PAIR(CP_MUTED) | A_UNDERLINE);
        }
    }

    // -----------------------------------------------------------------------
    // Main panes
    // -----------------------------------------------------------------------
    void drawHeader(App &a)
    {
        if (!a.headerWin) {
            return;
        }

        werase(a.headerWin);

        int h = 0, w = 0;
        getmaxyx(a.headerWin, h, w);

        drawFrame(
            a.headerWin,
            h,
            w,
            "",
            CP_BOX_ACTIVE,
            true,
            false,
            false
        );

        string right = "Кассир: " + a.cashier +
                       "  ·  " + nowDate() +
                       "  " + a.clockText;
        right = fitText(right, max(10, w - 24));

        int rightWidth = textWidth(right);
        int rightX = max(2, w - rightWidth - 2);

        wattron(a.headerWin, COLOR_PAIR(CP_GOOD) | A_BOLD);
        mvwaddstr(a.headerWin, 1, 2, "●");
        wattroff(a.headerWin, COLOR_PAIR(CP_GOOD) | A_BOLD);

        string name = fitText(a.storeName, max(1, rightX - 6));
        wattron(a.headerWin, COLOR_PAIR(CP_TITLE) | A_BOLD);
        mvwaddstr(a.headerWin, 1, 4, name.c_str());
        wattroff(a.headerWin, COLOR_PAIR(CP_TITLE) | A_BOLD);

        wattron(a.headerWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        mvwaddstr(a.headerWin, 1, rightX, right.c_str());
        wattroff(a.headerWin, COLOR_PAIR(CP_MUTED) | A_DIM);

        string shift = "Смена открыта " + a.shift.startedAt +
                       "  ·  чеков " + to_string(a.shift.sales) +
                       "  ·  выручка " + formatMoney(a.shift.revenue);
        shift = fitText(shift, w - 4);

        wattron(a.headerWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        mvwaddstr(a.headerWin, 2, 2, shift.c_str());
        wattroff(a.headerWin, COLOR_PAIR(CP_MUTED) | A_DIM);

        wrefresh(a.headerWin);
    }

    void drawCatalog(App &a)
    {
        if (!a.catalogWin) {
            return;
        }

        werase(a.catalogWin);

        int h = 0, w = 0;
        getmaxyx(a.catalogWin, h, w);

        bool active = !a.focusCart;

        string category = a.categories.empty()
            ? "Все товары"
            : a.categories[a.categoryIndex];
        string title = "КАТАЛОГ · " + category +
                       " · " + to_string(a.visible.size());

        drawFrame(
            a.catalogWin,
            h,
            w,
            title,
            active ? CP_BOX_ACTIVE : CP_BOX,
            active,
            false,
            false
        );

        int innerW = w - 2;
        int listTop = 3;
        int listH = h - 4;

        if (listH < 1) {
            listH = 1;
        }

        if (a.mode == Mode::Search) {
            string query = " ПОИСК: " + a.search + "█";
            wattron(a.catalogWin, COLOR_PAIR(CP_WARN) | A_BOLD);
            mvwaddstr(
                a.catalogWin,
                1,
                1,
                fitText(query, innerW - 1).c_str()
            );
            wattroff(a.catalogWin, COLOR_PAIR(CP_WARN) | A_BOLD);

            string hint = "Enter — добавить · Esc — отмена";
            int hintW = textWidth(hint);

            if (hintW + 16 < innerW) {
                wattron(a.catalogWin, COLOR_PAIR(CP_MUTED) | A_DIM);
                mvwaddstr(
                    a.catalogWin,
                    1,
                    1 + innerW - hintW - 1,
                    hint.c_str()
                );
                wattroff(a.catalogWin, COLOR_PAIR(CP_MUTED) | A_DIM);
            }
        }
        else {
            string cat = "‹ " + category + " ›";
            wattron(a.catalogWin, COLOR_PAIR(CP_ACCENT) | A_BOLD);
            mvwaddstr(
                a.catalogWin,
                1,
                2,
                fitText(cat, innerW - 2).c_str()
            );
            wattroff(a.catalogWin, COLOR_PAIR(CP_ACCENT) | A_BOLD);

            string keys = "[ / ]";
            int keysX = 2 + textWidth(cat) + 2;
            string count = to_string(a.visible.size()) + " поз.";
            int countX = 1 + innerW - textWidth(count) - 1;

            if (keysX + textWidth(keys) < countX) {
                wattron(a.catalogWin, COLOR_PAIR(CP_MUTED) | A_DIM);
                mvwaddstr(a.catalogWin, 1, keysX, keys.c_str());
                wattroff(a.catalogWin, COLOR_PAIR(CP_MUTED) | A_DIM);
            }

            wattron(a.catalogWin, COLOR_PAIR(CP_MUTED) | A_DIM);
            mvwaddstr(a.catalogWin, 1, countX, count.c_str());
            wattroff(a.catalogWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        }

        wattron(a.catalogWin, COLOR_PAIR(CP_BOX));
        mvwaddstr(
            a.catalogWin,
            2,
            1,
            repeatText("─", innerW).c_str()
        );
        wattroff(a.catalogWin, COLOR_PAIR(CP_BOX));

        clampCatalog(a);

        int rowW = innerW - 1;

        if (a.visible.empty()) {
            wattron(a.catalogWin, COLOR_PAIR(CP_MUTED) | A_DIM);
            mvwaddstr(a.catalogWin, listTop + 1, 2, "Ничего не найдено");
            if (listH > 3) {
                mvwaddstr(
                    a.catalogWin,
                    listTop + 3,
                    2,
                    "Измените запрос или категорию"
                );
            }
            wattroff(a.catalogWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        }
        else {
            for (int row = 0; row < listH; ++row) {
                int index = a.catalogScroll + row;

                if (index >= (int)a.visible.size()) {
                    break;
                }

                const Product &p = a.products[a.visible[index]];
                string price = formatMoney(p.price);
                string name = " " + p.name;
                bool selected = index == a.selected;

                if (selected) {
                    string line = twoCols(name, price, rowW);
                    wattron(
                        a.catalogWin,
                        COLOR_PAIR(CP_SELECT) | A_BOLD
                    );
                    mvwhline(a.catalogWin, listTop + row, 1, ' ', rowW);
                    mvwaddstr(
                        a.catalogWin,
                        listTop + row,
                        1,
                        fitText(line, rowW).c_str()
                    );
                    wattroff(
                        a.catalogWin,
                        COLOR_PAIR(CP_SELECT) | A_BOLD
                    );
                }
                else {
                    int priceW = textWidth(price);
                    string clippedName = fitText(
                        name,
                        max(1, rowW - priceW - 1)
                    );

                    mvwhline(a.catalogWin, listTop + row, 1, ' ', rowW);
                    mvwaddstr(
                        a.catalogWin,
                        listTop + row,
                        1,
                        clippedName.c_str()
                    );

                    wattron(
                        a.catalogWin,
                        COLOR_PAIR(CP_MUTED) | A_DIM
                    );
                    mvwaddstr(
                        a.catalogWin,
                        listTop + row,
                        1 + rowW - priceW,
                        price.c_str()
                    );
                    wattroff(
                        a.catalogWin,
                        COLOR_PAIR(CP_MUTED) | A_DIM
                    );
                }
            }
        }

        drawScrollbar(
            a.catalogWin,
            w - 2,
            listTop,
            listH,
            (int)a.visible.size(),
            a.catalogScroll,
            listH,
            CP_BOX_ACTIVE
        );

        wrefresh(a.catalogWin);
    }

    void drawCart(App &a)
    {
        if (!a.cartWin) {
            return;
        }

        werase(a.cartWin);

        int h = 0, w = 0;
        getmaxyx(a.cartWin, h, w);

        bool active = a.focusCart;
        string title = "ЧЕК · " + to_string(cartItems(a)) + " шт";

        drawFrame(
            a.cartWin,
            h,
            w,
            title,
            active ? CP_BOX_ACTIVE : CP_BOX,
            active,
            false,
            false
        );

        int innerW = w - 2;
        int sepY = h - 6;
        int listH = sepY - 1;

        if (listH < 1) {
            listH = 1;
        }

        clampCart(a);

        int rowW = innerW - 1;

        if (a.cart.empty()) {
            wattron(a.cartWin, COLOR_PAIR(CP_MUTED) | A_DIM);
            mvwaddstr(a.cartWin, 2, 2, "Чек пуст");
            mvwaddstr(
                a.cartWin,
                4,
                2,
                "Enter — добавить товар"
            );
            mvwaddstr(
                a.cartWin,
                5,
                2,
                "F2 — быстрый товар"
            );
            wattroff(a.cartWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        }
        else {
            for (int row = 0; row < listH; ++row) {
                int index = a.cartScroll + row;

                if (index >= (int)a.cart.size()) {
                    break;
                }

                const CartLine &line = a.cart[index];
                string price = formatMoney(line.price * line.qty);
                bool selected = index == a.cartSelected;

                if (selected) {
                    string left = " " + to_string(line.qty) +
                                  "× " + line.name;
                    string text = twoCols(left, price, rowW);

                    wattron(
                        a.cartWin,
                        COLOR_PAIR(CP_SELECT_ALT) | A_BOLD
                    );
                    mvwhline(a.cartWin, 1 + row, 1, ' ', rowW);
                    mvwaddstr(
                        a.cartWin,
                        1 + row,
                        1,
                        fitText(text, rowW).c_str()
                    );
                    wattroff(
                        a.cartWin,
                        COLOR_PAIR(CP_SELECT_ALT) | A_BOLD
                    );
                }
                else {
                    string qty = to_string(line.qty) + "×";
                    int priceW = textWidth(price);
                    int qtyW = textWidth(qty);
                    string name = fitText(
                        " " + line.name,
                        max(1, rowW - qtyW - priceW - 2)
                    );

                    mvwhline(a.cartWin, 1 + row, 1, ' ', rowW);

                    wattron(
                        a.cartWin,
                        COLOR_PAIR(CP_ACCENT) | A_BOLD
                    );
                    mvwaddstr(
                        a.cartWin,
                        1 + row,
                        1,
                        (" " + qty).c_str()
                    );
                    wattroff(
                        a.cartWin,
                        COLOR_PAIR(CP_ACCENT) | A_BOLD
                    );

                    mvwaddstr(
                        a.cartWin,
                        1 + row,
                        1 + qtyW + 1,
                        name.c_str()
                    );

                    wattron(
                        a.cartWin,
                        COLOR_PAIR(CP_MUTED) | A_DIM
                    );
                    mvwaddstr(
                        a.cartWin,
                        1 + row,
                        1 + rowW - priceW,
                        price.c_str()
                    );
                    wattroff(
                        a.cartWin,
                        COLOR_PAIR(CP_MUTED) | A_DIM
                    );
                }
            }
        }

        drawScrollbar(
            a.cartWin,
            w - 2,
            1,
            listH,
            (int)a.cart.size(),
            a.cartScroll,
            listH,
            CP_BOX_ACTIVE
        );

        long long subtotal = cartSubtotal(a);
        long long discount = cartDiscount(a);
        long long total = cartTotal(a);

        wattron(a.cartWin, COLOR_PAIR(CP_BOX));
        mvwaddstr(
            a.cartWin,
            sepY,
            1,
            repeatText("─", innerW).c_str()
        );
        wattroff(a.cartWin, COLOR_PAIR(CP_BOX));

        drawValueRow(
            a.cartWin,
            sepY + 1,
            1,
            innerW,
            " Сумма",
            formatMoney(subtotal) + " ",
            CP_MUTED,
            false
        );

        string discountLabel = " Скидка";
        if (a.discountPercent > 0) {
            discountLabel += " " + to_string(a.discountPercent) + "%";
        }

        drawValueRow(
            a.cartWin,
            sepY + 2,
            1,
            innerW,
            discountLabel,
            (discount > 0 ? "-" : "") + formatMoney(discount) + " ",
            discount > 0 ? CP_WARN : CP_MUTED,
            false
        );

        drawValueRow(
            a.cartWin,
            sepY + 3,
            1,
            innerW,
            " К ОПЛАТЕ",
            formatMoney(total) + " ",
            CP_TOTAL,
            true
        );

        string footer = twoCols(
            " " + to_string(a.cart.size()) +
            " поз. · " + to_string(cartItems(a)) + " шт.",
            "F9 — оплата ",
            innerW
        );

        wattron(a.cartWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        mvwaddstr(a.cartWin, sepY + 4, 1, footer.c_str());
        wattroff(a.cartWin, COLOR_PAIR(CP_MUTED) | A_DIM);

        wrefresh(a.cartWin);
    }

    // -----------------------------------------------------------------------
    // Status line and modal dialogs
    // -----------------------------------------------------------------------
    static string statusHints(App &a)
    {
        bool wide = a.termW >= 112;

        switch (a.mode) {
            case Mode::Search:
                return "Печатайте запрос  ·  ↑↓ выбор  ·  Enter — добавить  ·  Esc — отмена";

            case Mode::Payment:
                return "Tab / ←→ — способ  ·  Enter — оплатить  ·  Esc — отмена";

            case Mode::QuickItem:
                return "Tab — поле  ·  Enter — добавить  ·  Esc — отмена";

            case Mode::Barcode:
                return "Введите штрихкод  ·  Enter — найти  ·  Esc — отмена";

            case Mode::Discount:
                return "0–90%  ·  Enter — применить  ·  Esc — отмена";

            case Mode::Report:
            case Mode::Receipt:
            case Mode::Help:
                return "↑↓ / PgUp/PgDn — прокрутка  ·  Esc — закрыть";

            case Mode::ConfirmQuit:
            case Mode::ConfirmClear:
                return "Enter / y — да  ·  Esc / n — нет";

            case Mode::Sale:
            default:
                break;
        }

        if (a.focusCart) {
            if (wide) {
                return "↑↓ выбор  ·  +/− кол-во  ·  Del — убрать  ·  Tab — каталог  ·  F6 — скидка  ·  F7 — очистить  ·  F9 — оплата";
            }
            return "↑↓ выбор  ·  +/− кол-во  ·  Del — убрать  ·  Tab — каталог  ·  F9 — оплата  ·  F1 — помощь";
        }

        if (wide) {
            return "↑↓ выбор  ·  Enter — добавить  ·  / — поиск  ·  [ ] — категория  ·  Tab — чек  ·  F2 товар  ·  F3 код  ·  F4 смена  ·  F9 — оплата";
        }
        return "↑↓ выбор  ·  Enter +  ·  / поиск  ·  Tab чек  ·  F9 оплата  ·  F1 помощь";
    }

    void drawStatus(App &a)
    {
        int barY = a.termH - 2;
        int infoY = a.termH - 1;

        wattron(stdscr, COLOR_PAIR(CP_BAR) | A_BOLD);
        mvwhline(stdscr, barY, 0, ' ', a.termW);

        string hints = statusHints(a);
        string right = (a.mode == Mode::Sale || a.mode == Mode::Search)
            ? "F10 выход "
            : "Esc закрыть ";

        int rightW = textWidth(right);
        int leftW = max(0, a.termW - rightW - 1);

        mvaddstr(barY, 0, fitText(" " + hints, leftW).c_str());
        mvaddstr(barY, a.termW - rightW, right.c_str());
        wattroff(stdscr, COLOR_PAIR(CP_BAR) | A_BOLD);

        move(infoY, 0);
        clrtoeol();

        string cartInfo = a.cart.empty()
            ? "Чек пуст "
            : "Чек: " + to_string(cartItems(a)) +
              " шт  ·  " + formatMoney(cartTotal(a)) + " ";

        int cartW = textWidth(cartInfo);
        int messageW = max(0, a.termW - cartW - 1);

        wattron(stdscr, COLOR_PAIR(a.messagePair));
        if (a.messagePair == CP_MUTED) {
            wattron(stdscr, A_DIM);
        }

        mvaddstr(infoY, 0, fitText(" " + a.message, messageW).c_str());

        if (a.messagePair == CP_MUTED) {
            wattroff(stdscr, A_DIM);
        }
        wattroff(stdscr, COLOR_PAIR(a.messagePair));

        wattron(stdscr, COLOR_PAIR(CP_ACCENT) | A_BOLD);
        mvaddstr(infoY, a.termW - cartW, cartInfo.c_str());
        wattroff(stdscr, COLOR_PAIR(CP_ACCENT) | A_BOLD);

        refresh();
    }

    static void drawChip(
        WINDOW *win,
        int y,
        int x,
        const string &label,
        bool selected
    ) {
        string text = selected
            ? "[ " + label + " ]"
            : "  " + label + "  ";

        if (selected) {
            wattron(win, COLOR_PAIR(CP_SELECT) | A_BOLD);
        }
        else {
            wattron(win, COLOR_PAIR(CP_MUTED) | A_DIM);
        }

        mvwaddstr(win, y, x, text.c_str());

        if (selected) {
            wattroff(win, COLOR_PAIR(CP_SELECT) | A_BOLD);
        }
        else {
            wattroff(win, COLOR_PAIR(CP_MUTED) | A_DIM);
        }
    }

    static void drawPaymentModal(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);

        werase(a.modalWin);

        drawFrame(
            a.modalWin,
            h,
            w,
            a.modalTitle,
            CP_BOX_ACTIVE,
            true,
            true,
            true
        );

        int x = 2;
        int width = w - 4;
        long long total = cartTotal(a);
        long long parsedValue = 0;
        bool parsed = parseMoneyInput(a.input, parsedValue);

        if (a.paymentMethod == PaymentMethod::Card) {
            parsedValue = total;
            parsed = true;
        }

        a.tendered = parsed ? parsedValue : 0;
        a.changeDue = a.tendered - total;

        drawValueRow(
            a.modalWin,
            2,
            x,
            width,
            "К ОПЛАТЕ",
            formatMoney(total),
            CP_TOTAL,
            true
        );

        wattron(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        mvwaddstr(a.modalWin, 4, x, "Способ оплаты:");
        wattroff(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);

        drawChip(
            a.modalWin,
            5,
            x,
            "НАЛИЧНЫЕ",
            a.paymentMethod == PaymentMethod::Cash
        );

        drawChip(
            a.modalWin,
            5,
            x + 16,
            "КАРТА",
            a.paymentMethod == PaymentMethod::Card
        );

        string received;
        if (a.paymentMethod == PaymentMethod::Card) {
            received = "—";
        }
        else if (a.input.empty()) {
            received = "0,00 ₽█";
        }
        else {
            received = formatMoney(a.tendered) + "█";
        }

        drawValueRow(
            a.modalWin,
            7,
            x,
            width,
            "ПОЛУЧЕНО",
            received,
            CP_ACCENT,
            false
        );

        string change;
        int changePair = CP_MUTED;

        if (a.paymentMethod == PaymentMethod::Card) {
            change = "0,00 ₽";
            changePair = CP_TOTAL;
        }
        else if (!parsed || a.input.empty()) {
            change = "—";
        }
        else if (a.tendered < total) {
            change = "не хватает " + formatMoney(total - a.tendered);
            changePair = CP_DANGER;
        }
        else {
            change = formatMoney(a.changeDue);
            changePair = CP_GOOD;
        }

        drawValueRow(
            a.modalWin,
            8,
            x,
            width,
            "СДАЧА",
            change,
            changePair,
            true
        );

        if (!a.error.empty()) {
            wattron(a.modalWin, COLOR_PAIR(CP_DANGER) | A_BOLD);
            mvwaddstr(
                a.modalWin,
                10,
                x,
                fitText(a.error, width).c_str()
            );
            wattroff(a.modalWin, COLOR_PAIR(CP_DANGER) | A_BOLD);
        }
        else {
            string note = a.paymentMethod == PaymentMethod::Card
                ? "Нажмите Enter для оплаты картой"
                : "Введите сумму наличных";

            wattron(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);
            mvwaddstr(
                a.modalWin,
                10,
                x,
                fitText(note, width).c_str()
            );
            wattroff(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        }

        drawFrameHint(
            a.modalWin,
            h,
            w,
            a.modalHint,
            CP_MUTED
        );

        wrefresh(a.modalWin);
    }

    static void drawQuickItemModal(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);

        werase(a.modalWin);

        drawFrame(
            a.modalWin,
            h,
            w,
            a.modalTitle,
            CP_BOX_ACTIVE,
            true,
            true,
            true
        );

        int x = 2;
        int width = w - 4;

        wattron(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        mvwaddstr(a.modalWin, 2, x, "НАЗВАНИЕ ТОВАРА:");
        mvwaddstr(a.modalWin, 5, x, "ЦЕНА, ₽:");
        wattroff(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);

        drawInputField(
            a.modalWin,
            3,
            x,
            width,
            a.quickName,
            a.quickNameActive
        );

        drawInputField(
            a.modalWin,
            6,
            x,
            width,
            a.quickPrice,
            !a.quickNameActive
        );

        if (!a.error.empty()) {
            wattron(a.modalWin, COLOR_PAIR(CP_DANGER) | A_BOLD);
            mvwaddstr(
                a.modalWin,
                8,
                x,
                fitText(a.error, width).c_str()
            );
            wattroff(a.modalWin, COLOR_PAIR(CP_DANGER) | A_BOLD);
        }
        else {
            wattron(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);
            mvwaddstr(
                a.modalWin,
                8,
                x,
                fitText("Пример: Пакет-майка · 10,00", width).c_str()
            );
            wattroff(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        }

        drawFrameHint(
            a.modalWin,
            h,
            w,
            a.modalHint,
            CP_MUTED
        );

        wrefresh(a.modalWin);
    }

    static void drawBarcodeModal(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);

        werase(a.modalWin);

        drawFrame(
            a.modalWin,
            h,
            w,
            a.modalTitle,
            CP_BOX_ACTIVE,
            true,
            true,
            true
        );

        int x = 2;
        int width = w - 4;

        wattron(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        mvwaddstr(a.modalWin, 2, x, "ШТРИХКОД:");
        wattroff(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);

        drawInputField(
            a.modalWin,
            3,
            x,
            width,
            a.input,
            true
        );

        if (!a.error.empty()) {
            wattron(a.modalWin, COLOR_PAIR(CP_DANGER) | A_BOLD);
            mvwaddstr(
                a.modalWin,
                5,
                x,
                fitText(a.error, width).c_str()
            );
            wattroff(a.modalWin, COLOR_PAIR(CP_DANGER) | A_BOLD);
        }

        drawFrameHint(
            a.modalWin,
            h,
            w,
            a.modalHint,
            CP_MUTED
        );

        wrefresh(a.modalWin);
    }

    static void drawDiscountModal(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);

        werase(a.modalWin);

        drawFrame(
            a.modalWin,
            h,
            w,
            a.modalTitle,
            CP_BOX_ACTIVE,
            true,
            true,
            true
        );

        int x = 2;
        int width = w - 4;

        wattron(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);
        mvwaddstr(a.modalWin, 2, x, "СКИДКА НА ЧЕК, %:");
        wattroff(a.modalWin, COLOR_PAIR(CP_MUTED) | A_DIM);

        drawInputField(
            a.modalWin,
            3,
            x,
            12,
            a.input,
            true
        );

        int percent = 0;
        if (!a.input.empty()) {
            percent = atoi(a.input.c_str());
        }

        int previewPercent = max(0, min(99, percent));
        long long subtotal = cartSubtotal(a);
        long long discount = (subtotal * previewPercent + 50) / 100;
        long long total = max(0LL, subtotal - discount);

        drawValueRow(
            a.modalWin,
            5,
            x,
            width,
            "Сумма скидки",
            (discount > 0 ? "-" : "") + formatMoney(discount),
            CP_WARN,
            false
        );

        drawValueRow(
            a.modalWin,
            6,
            x,
            width,
            "К оплате",
            formatMoney(total),
            CP_TOTAL,
            true
        );

        if (!a.error.empty()) {
            wattron(a.modalWin, COLOR_PAIR(CP_DANGER) | A_BOLD);
            mvwaddstr(
                a.modalWin,
                8,
                x,
                fitText(a.error, width).c_str()
            );
            wattroff(a.modalWin, COLOR_PAIR(CP_DANGER) | A_BOLD);
        }

        drawFrameHint(
            a.modalWin,
            h,
            w,
            a.modalHint,
            CP_MUTED
        );

        wrefresh(a.modalWin);
    }

    static void drawConfirmModal(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);

        werase(a.modalWin);

        drawFrame(
            a.modalWin,
            h,
            w,
            a.modalTitle,
            CP_WARN,
            true,
            true,
            true
        );

        int innerW = w - 4;
        int y = 2;

        for (const string &line : a.modalLines) {
            if (y >= h - 2) {
                break;
            }

            string text = fitText(line, innerW);
            int x = max(2, (w - textWidth(text)) / 2);

            wattron(a.modalWin, COLOR_PAIR(CP_MUTED));
            mvwaddstr(a.modalWin, y, x, text.c_str());
            wattroff(a.modalWin, COLOR_PAIR(CP_MUTED));

            ++y;
        }

        drawFrameHint(
            a.modalWin,
            h,
            w,
            a.modalHint,
            CP_MUTED
        );

        wrefresh(a.modalWin);
    }

    static void drawReceiptModal(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);

        werase(a.modalWin);

        drawFrame(
            a.modalWin,
            h,
            w,
            a.modalTitle,
            CP_BOX_ACTIVE,
            true,
            true,
            true
        );

        vector<string> wrapped = wrapModalLines(a);
        int innerH = max(1, h - 2);
        int innerW = max(1, w - 3);
        int maxScroll = max(0, (int)wrapped.size() - innerH);

        if (a.modalScroll < 0) {
            a.modalScroll = 0;
        }

        if (a.modalScroll > maxScroll) {
            a.modalScroll = maxScroll;
        }

        for (int row = 0; row < innerH; ++row) {
            int index = a.modalScroll + row;

            if (index >= (int)wrapped.size()) {
                break;
            }

            string text = fitText(wrapped[index], innerW);
            int padding = max(0, (innerW - textWidth(text)) / 2);

            mvwaddstr(
                a.modalWin,
                1 + row,
                1 + padding,
                text.c_str()
            );
        }

        drawScrollbar(
            a.modalWin,
            w - 2,
            1,
            innerH,
            (int)wrapped.size(),
            a.modalScroll,
            innerH,
            CP_BOX_ACTIVE
        );

        drawFrameHint(
            a.modalWin,
            h,
            w,
            a.modalHint,
            CP_MUTED
        );

        wrefresh(a.modalWin);
    }

    static void drawTextModal(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);

        werase(a.modalWin);

        drawFrame(
            a.modalWin,
            h,
            w,
            a.modalTitle,
            CP_BOX_ACTIVE,
            true,
            true,
            true
        );

        vector<string> wrapped = wrapModalLines(a);
        int innerH = max(1, h - 2);
        int innerW = max(1, w - 3);
        int maxScroll = max(0, (int)wrapped.size() - innerH);

        if (a.modalScroll < 0) {
            a.modalScroll = 0;
        }

        if (a.modalScroll > maxScroll) {
            a.modalScroll = maxScroll;
        }

        for (int row = 0; row < innerH; ++row) {
            int index = a.modalScroll + row;

            if (index >= (int)wrapped.size()) {
                break;
            }

            mvwaddstr(
                a.modalWin,
                1 + row,
                1,
                fitText(wrapped[index], innerW).c_str()
            );
        }

        drawScrollbar(
            a.modalWin,
            w - 2,
            1,
            innerH,
            (int)wrapped.size(),
            a.modalScroll,
            innerH,
            CP_BOX_ACTIVE
        );

        drawFrameHint(
            a.modalWin,
            h,
            w,
            a.modalHint,
            CP_MUTED
        );

        wrefresh(a.modalWin);
    }

    void drawModal(App &a)
    {
        if (!a.modalWin) {
            return;
        }

        switch (a.mode) {
            case Mode::Payment:
                drawPaymentModal(a);
                break;

            case Mode::QuickItem:
                drawQuickItemModal(a);
                break;

            case Mode::Barcode:
                drawBarcodeModal(a);
                break;

            case Mode::Discount:
                drawDiscountModal(a);
                break;

            case Mode::Receipt:
                drawReceiptModal(a);
                break;

            case Mode::Report:
            case Mode::Help:
                drawTextModal(a);
                break;

            case Mode::ConfirmQuit:
            case Mode::ConfirmClear:
                drawConfirmModal(a);
                break;

            default:
                break;
        }
    }

    void updateClock(App &a)
    {
        a.clockText = nowClock();
    }

    void drawAll(App &a)
    {
        if (!a.ready) {
            erase();

            int h = 0, w = 0;
            getmaxyx(stdscr, h, w);

            string first = "Терминал слишком мал";
            string second = "Нужно минимум 80×24, сейчас " +
                            to_string(w) + "×" + to_string(h);
            string third = "Увеличьте окно · F10 — выход";

            int x1 = max(0, (w - textWidth(first)) / 2);
            int x2 = max(0, (w - textWidth(second)) / 2);
            int x3 = max(0, (w - textWidth(third)) / 2);
            int y = max(0, h / 2 - 1);

            wattron(stdscr, COLOR_PAIR(CP_DANGER) | A_BOLD);
            mvaddstr(y, x1, fitText(first, w).c_str());
            wattroff(stdscr, COLOR_PAIR(CP_DANGER) | A_BOLD);

            wattron(stdscr, COLOR_PAIR(CP_MUTED) | A_DIM);
            mvaddstr(y + 2, x2, fitText(second, w).c_str());
            mvaddstr(y + 4, x3, fitText(third, w).c_str());
            wattroff(stdscr, COLOR_PAIR(CP_MUTED) | A_DIM);

            refresh();
            return;
        }

        drawHeader(a);
        drawCatalog(a);
        drawCart(a);
        drawStatus(a);
        drawModal(a);
    }

    void setMessage(
        App &a,
        const string &text,
        int pair
    ) {
        a.message = text;
        a.messagePair = pair;
    }

    vector<string> wrapModalLines(App &a)
    {
        vector<string> wrapped;

        if (!a.modalWin) {
            return wrapped;
        }

        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);
        (void)h;
        int innerW = max(1, w - 3);

        for (const string &line : a.modalLines) {
            wrapLine(line, innerW, wrapped);
        }

        if (wrapped.empty()) {
            wrapped.push_back("");
        }

        return wrapped;
    }

    int maxModalScroll(App &a)
    {
        if (!a.modalWin) {
            return 0;
        }

        int h = 0, w = 0;
        getmaxyx(a.modalWin, h, w);
        vector<string> wrapped = wrapModalLines(a);
        return max(0, (int)wrapped.size() - max(1, h - 2));
    }

    void scrollModal(App &a, int delta)
    {
        a.modalScroll += delta;
        int maxScroll = maxModalScroll(a);

        if (a.modalScroll < 0) {
            a.modalScroll = 0;
        }

        if (a.modalScroll > maxScroll) {
            a.modalScroll = maxScroll;
        }
    }

    void openModal(App &a, Mode mode)
    {
        closeModal(a);

        int termH = 0, termW = 0;
        getmaxyx(stdscr, termH, termW);

        int mh = 12;
        int mw = 54;

        switch (mode) {
            case Mode::Payment:
                mh = 15;
                mw = 60;
                break;

            case Mode::QuickItem:
                mh = 13;
                mw = 60;
                break;

            case Mode::Barcode:
                mh = 10;
                mw = 50;
                break;

            case Mode::Discount:
                mh = 11;
                mw = 56;
                break;

            case Mode::Report:
                mh = min(24, termH - 4);
                mw = min(76, termW - 6);
                break;

            case Mode::Receipt:
                mh = min(26, termH - 4);
                mw = min(60, termW - 6);
                break;

            case Mode::Help:
                mh = min(24, termH - 4);
                mw = min(74, termW - 6);
                break;

            case Mode::ConfirmQuit:
            case Mode::ConfirmClear:
                mh = 10;
                mw = 56;
                break;

            default:
                break;
        }

        mh = min(mh, max(8, termH - 2));
        mw = min(mw, max(40, termW - 2));

        int y = max(0, (termH - mh) / 2);
        int x = max(0, (termW - mw) / 2);

        a.modalWin = newwin(mh, mw, y, x);
        a.mode = mode;
    }

    void startSearch(App &a)
    {
        a.mode = Mode::Search;
        a.focusCart = false;
        a.search.clear();
        a.selected = 0;
        a.catalogScroll = 0;
        rebuildVisible(a);
        setMessage(a, "Введите запрос · Esc — отмена", CP_MUTED);
    }

    void finishSearch(App &a)
    {
        a.mode = Mode::Sale;
    }

    void startPayment(App &a)
    {
        if (a.cart.empty()) {
            setMessage(a, "Чек пуст · добавьте товар", CP_WARN);
            return;
        }

        a.paymentMethod = PaymentMethod::Cash;
        a.input.clear();
        a.error.clear();
        a.tendered = 0;
        a.changeDue = 0;
        a.modalTitle = "ОПЛАТА";
        a.modalHint = "Tab — способ · Enter — оплатить · Esc — отмена";
        a.modalScroll = 0;
        openModal(a, Mode::Payment);
    }

    void startQuickItem(App &a)
    {
        a.quickName.clear();
        a.quickPrice.clear();
        a.quickNameActive = true;
        a.error.clear();
        a.modalTitle = "БЫСТРЫЙ ТОВАР";
        a.modalHint = "Tab — поле · Enter — добавить · Esc — отмена";
        a.modalScroll = 0;
        openModal(a, Mode::QuickItem);
    }

    void startBarcode(App &a)
    {
        a.input.clear();
        a.error.clear();
        a.modalTitle = "ПОИСК ПО ШТРИХКОДУ";
        a.modalHint = "Enter — найти · Esc — отмена";
        a.modalScroll = 0;
        openModal(a, Mode::Barcode);
    }

    void startDiscount(App &a)
    {
        a.input = a.discountPercent > 0
            ? to_string(a.discountPercent)
            : "";
        a.error.clear();
        a.modalTitle = "СКИДКА НА ЧЕК";
        a.modalHint = "0–90% · Enter — применить · Esc — отмена";
        a.modalScroll = 0;
        openModal(a, Mode::Discount);
    }

    void openHelp(App &a)
    {
        a.modalTitle = "ПОМОЩЬ";
        a.modalHint = "↑↓ / PgUp/PgDn — прокрутка · Esc — закрыть";
        a.modalScroll = 0;
        a.modalLines = {
            "КАТАЛОГ",
            "  /              поиск по названию, коду или категории",
            "  [ / ]          предыдущая / следующая категория",
            "  ↑ ↓ PgUp PgDn  навигация по списку",
            "  Enter          добавить выбранный товар",
            "  Tab            перейти к чеку",
            "",
            "ЧЕК",
            "  ↑ ↓            выбрать позицию",
            "  + / -          изменить количество",
            "  Delete         удалить позицию",
            "  F7             очистить чек",
            "  F6             скидка на чек",
            "  F9             оплата",
            "",
            "ДОПОЛНИТЕЛЬНО",
            "  F2             быстрый товар вне каталога",
            "  F3             поиск по штрихкоду",
            "  цифры          сразу открыть ввод штрихкода",
            "  F4             отчёт по смене",
            "  F5             последний чек",
            "  F1             эта справка",
            "  F10 / q        выход",
            "",
            "ОПЛАТА",
            "  Tab / ←→       наличные или карта",
            "  Enter          подтвердить",
            "  Esc            отмена",
            "",
            "ЧЕК СОХРАНЯЕТСЯ В ПАПКУ receipts/",
            "ЖУРНАЛ ПРОДАЖ — data/sales.tsv",
            "КАТАЛОГ — data/products.tsv",
        };

        openModal(a, Mode::Help);
    }

    void openReport(App &a)
    {
        const int width = 46;

        auto row = [&](const string &label, const string &value) {
            a.modalLines.push_back(twoCols(label, value, width));
        };

        a.modalLines.clear();
        a.modalLines.push_back(a.shiftLabel);
        a.modalLines.push_back(a.storeName);
        a.modalLines.push_back("----------------------------------------------");
        row("Начало смены", a.shift.startedAt);
        row("Текущее время", nowClock());
        a.modalLines.push_back("");
        row("Чеков", to_string(a.shift.sales));
        row("Позиций продано", to_string(a.shift.items));
        a.modalLines.push_back("");
        row("Выручка", formatMoney(a.shift.revenue));
        row("  наличные", formatMoney(a.shift.cash));
        row("  карта", formatMoney(a.shift.card));
        row("Скидки", formatMoney(a.shift.discounts));

        long long average = a.shift.sales > 0
            ? a.shift.revenue / a.shift.sales
            : 0;
        row("Средний чек", formatMoney(average));
        a.modalLines.push_back("");
        row("Сейчас в чеке", formatMoney(cartTotal(a)));
        row("Позиций в чеке", to_string(cartItems(a)));

        a.modalTitle = "ОТЧЁТ ПО СМЕНЕ";
        a.modalHint = "↑↓ / PgUp/PgDn — прокрутка · Esc — закрыть";
        a.modalScroll = 0;
        openModal(a, Mode::Report);
    }

    void openLastReceipt(App &a)
    {
        if (a.lastSale.lines.empty()) {
            setMessage(a, "Пока нет пробитых чеков", CP_MUTED);
            return;
        }

        openReceipt(a, a.lastSale);
    }

    void openReceipt(App &a, const Sale &sale)
    {
        a.modalLines.clear();
        receiptText(a, sale, a.modalLines);
        a.modalTitle = "ЧЕК · " + sale.id;
        a.modalHint = "↑↓ / PgUp/PgDn — прокрутка · Esc — закрыть";
        a.modalScroll = 0;
        openModal(a, Mode::Receipt);
    }

    void openConfirm(
        App &a,
        Mode mode,
        const string &title,
        const vector<string> &lines
    ) {
        a.modalTitle = title;
        a.modalHint = "Enter / y — да · Esc / n — нет";
        a.modalLines = lines;
        a.modalScroll = 0;
        openModal(a, mode);
    }

    void applyDiscount(App &a)
    {
        int percent = 0;

        if (!a.input.empty()) {
            percent = atoi(a.input.c_str());
        }

        if (percent < 0 || percent > 90) {
            a.error = "Скидка должна быть от 0 до 90%";
            return;
        }

        a.discountPercent = percent;
        closeModal(a);
        a.mode = Mode::Sale;

        if (percent == 0) {
            setMessage(a, "Скидка снята", CP_MUTED);
        }
        else {
            setMessage(
                a,
                "Скидка " + to_string(percent) + "% · " +
                formatMoney(cartDiscount(a)),
                CP_GOOD
            );
        }
    }

    void addQuickItem(App &a)
    {
        string name = trim(a.quickName);
        long long price = 0;

        if (name.empty()) {
            a.error = "Введите название товара";
            return;
        }

        if (!parseMoneyInput(a.quickPrice, price) || price <= 0) {
            a.error = "Введите цену больше нуля";
            return;
        }

        CartLine line;
        line.name = name;
        line.category = "Быстрый товар";
        line.price = price;
        line.qty = 1;
        line.custom = true;
        pushCartLine(a, line);

        closeModal(a);
        a.mode = Mode::Sale;
        a.focusCart = true;
        a.quickName.clear();
        a.quickPrice.clear();
        setMessage(a, "Добавлено: " + name, CP_GOOD);
    }

    void findBarcode(App &a)
    {
        string code = trim(a.input);

        if (code.empty()) {
            a.error = "Введите штрихкод";
            return;
        }

        for (const Product &p : a.products) {
            if (p.barcode == code) {
                addProductToCart(a, p);
                closeModal(a);
                a.mode = Mode::Sale;
                setMessage(a, "Добавлено: " + p.name, CP_GOOD);
                return;
            }
        }

        a.error = "Товар не найден";
    }

    void requestQuit(App &a)
    {
        if (a.cart.empty()) {
            a.running = false;
            return;
        }

        openConfirm(
            a,
            Mode::ConfirmQuit,
            "ВЫХОД ИЗ КАССЫ",
            {
                "В чеке остались товары.",
                "Завершить смену и выйти?",
                "",
                "Текущий чек будет потерян.",
            }
        );
    }

    void finishPayment(App &a)
    {
        long long total = cartTotal(a);
        long long tendered = total;

        if (a.paymentMethod == PaymentMethod::Cash) {
            if (!parseMoneyInput(a.input, tendered)) {
                a.error = "Введите сумму наличных";
                return;
            }

            if (tendered < total) {
                a.error = "Недостаточно средств";
                return;
            }
        }

        Sale sale;
        sale.id = nextReceiptId(a);
        sale.time = dateTimeText();
        sale.lines = a.cart;
        sale.subtotal = cartSubtotal(a);
        sale.discount = cartDiscount(a);
        sale.total = total;
        sale.discountPercent = a.discountPercent;
        sale.method = a.paymentMethod;
        sale.tendered = tendered;
        sale.change = a.paymentMethod == PaymentMethod::Cash
            ? tendered - total
            : 0;

        vector<string> lines;
        receiptText(a, sale, lines);
        bool saved = saveReceipt(a, sale, lines);
        logSale(a, sale);

        int soldItems = 0;
        for (const CartLine &line : sale.lines) {
            soldItems += line.qty;
        }

        a.shift.sales += 1;
        a.shift.items += soldItems;
        a.shift.revenue += sale.total;
        a.shift.discounts += sale.discount;

        if (sale.method == PaymentMethod::Cash) {
            a.shift.cash += sale.total;
        }
        else {
            a.shift.card += sale.total;
        }

        a.lastSale = sale;
        a.lastReceiptPath = a.receiptsDir + "/" + sale.id + ".txt";

        a.cart.clear();
        a.cartSelected = 0;
        a.cartScroll = 0;
        a.discountPercent = 0;
        a.focusCart = false;
        a.tendered = 0;
        a.changeDue = 0;
        a.input.clear();
        a.error.clear();

        closeModal(a);
        a.mode = Mode::Receipt;
        a.modalLines = lines;
        a.modalTitle = "ЧЕК · " + sale.id;
        a.modalHint = "↑↓ / PgUp/PgDn — прокрутка · Esc — закрыть";
        a.modalScroll = 0;
        openModal(a, Mode::Receipt);

        string method = sale.method == PaymentMethod::Cash
            ? "наличные"
            : "карта";

        string text = "Оплата принята · " + method;

        if (sale.method == PaymentMethod::Cash) {
            text += " · сдача " + formatMoney(sale.change);
        }

        if (!saved) {
            text += " · чек не сохранён";
        }

        setMessage(a, text, saved ? CP_GOOD : CP_WARN);
    }

    string receiptText(
        App &a,
        const Sale &sale,
        vector<string> &lines
    ) {
        const int width = 44;

        lines.clear();

        auto center = [&](const string &text) {
            string value = fitText(text, width);
            int padding = max(0, (width - textWidth(value)) / 2);
            lines.push_back(string(padding, ' ') + value);
        };

        auto row = [&](const string &left, const string &right) {
            lines.push_back(twoCols(left, right, width));
        };

        string separator(width, '-');

        center("КАССОВЫЙ ЧЕК");
        center(a.storeName);
        center("ул. Терминальная, 1");
        center("тел. +7 (900) 000-00-00");
        lines.push_back(separator);
        lines.push_back("Чек:    " + sale.id);
        lines.push_back("Дата:   " + sale.time);
        lines.push_back("Кассир: " + a.cashier);
        lines.push_back(separator);

        int number = 1;
        for (const CartLine &line : sale.lines) {
            vector<string> parts;
            wrapLine(
                to_string(number) + ". " + line.name,
                width - 2,
                parts
            );

            for (const string &part : parts) {
                lines.push_back(" " + fitText(part, width - 1));
            }

            row(
                "   " + to_string(line.qty) + " x " +
                formatMoney(line.price, false),
                formatMoney(line.price * line.qty, false)
            );

            ++number;
        }

        lines.push_back(separator);
        row("ИТОГО:", formatMoney(sale.subtotal, false));

        if (sale.discountPercent > 0) {
            row(
                "СКИДКА " + to_string(sale.discountPercent) + "%:",
                "-" + formatMoney(sale.discount, false)
            );
        }

        row("К ОПЛАТЕ:", formatMoney(sale.total, false));
        row(
            sale.method == PaymentMethod::Cash
                ? "НАЛИЧНЫЕ:"
                : "КАРТА:",
            formatMoney(sale.tendered, false)
        );

        if (sale.method == PaymentMethod::Cash) {
            row("СДАЧА:", formatMoney(sale.change, false));
        }

        lines.push_back(separator);
        center("НДС 20% включён в стоимость");
        center("СПАСИБО ЗА ПОКУПКУ!");
        center("ЖДЁМ ВАС СНОВА");

        string text;
        for (const string &line : lines) {
            text += line;
            text += '\n';
        }

        return text;
    }

    string nextReceiptId(App &a)
    {
        string base = receiptStamp();
        string id = base;
        int suffix = 2;

        while (fs::exists(a.receiptsDir + "/" + id + ".txt")) {
            id = base + "-" + to_string(suffix);
            ++suffix;
        }

        return id;
    }

    bool saveReceipt(
        App &a,
        const Sale &sale,
        const vector<string> &lines
    ) {
        error_code ec;
        fs::create_directories(a.receiptsDir, ec);

        string path = a.receiptsDir + "/" + sale.id + ".txt";
        ofstream file(path, ios::trunc);

        if (!file.is_open()) {
            return false;
        }

        for (const string &line : lines) {
            file << line << '\n';
        }

        return file.good();
    }

    bool logSale(App &a, const Sale &sale)
    {
        error_code ec;
        fs::create_directories(a.dataDir, ec);

        string path = a.dataDir + "/sales.tsv";
        bool writeHeader = true;

        if (fs::exists(path)) {
            uintmax_t size = fs::file_size(path, ec);
            if (!ec && size > 0) {
                writeHeader = false;
            }
        }

        ofstream file(path, ios::app);
        if (!file.is_open()) {
            return false;
        }

        if (writeHeader) {
            file << "# time\treceipt\tmethod\titems\tsubtotal\t"
                 << "discount_percent\tdiscount\ttotal\ttendered\tchange\n";
        }

        int items = 0;
        for (const CartLine &line : sale.lines) {
            items += line.qty;
        }

        string method = sale.method == PaymentMethod::Cash
            ? "cash"
            : "card";

        file << sale.time << '\t'
             << sale.id << '\t'
             << method << '\t'
             << items << '\t'
             << sale.subtotal << '\t'
             << sale.discountPercent << '\t'
             << sale.discount << '\t'
             << sale.total << '\t'
             << sale.tendered << '\t'
             << sale.change << '\n';

        return file.good();
    }

    bool isPrintableCodepoint(int codepoint)
    {
        return codepoint >= 32 &&
               codepoint != 127 &&
               codepoint <= 0x10FFFF;
    }

    static bool isLetterCodepoint(int codepoint)
    {
        if (codepoint < 32 || codepoint > 0x10FFFF) {
            return false;
        }

        return iswalpha((wint_t)codepoint) != 0;
    }

    void handleKey(
        App &a,
        int key,
        bool isCode
    ) {
        if (key == KEY_RESIZE) {
            Mode oldMode = a.mode;
            layout(a);

            if (a.ready && isModalMode(oldMode)) {
                openModal(a, oldMode);
            }

            return;
        }

        if (!a.ready) {
            bool quit = (!isCode && (key == 'q' || key == 'Q')) ||
                        (isCode && key == KEY_F(10));

            if (quit) {
                a.running = false;
            }

            return;
        }

        switch (a.mode) {
            case Mode::Sale:
                handleSaleKey(a, key, isCode);
                break;

            case Mode::Search:
                handleSearchKey(a, key, isCode);
                break;

            case Mode::Payment:
                handlePaymentKey(a, key, isCode);
                break;

            case Mode::QuickItem:
                handleQuickItemKey(a, key, isCode);
                break;

            case Mode::Barcode:
                handleBarcodeKey(a, key, isCode);
                break;

            case Mode::Discount:
                handleDiscountKey(a, key, isCode);
                break;

            case Mode::Report:
            case Mode::Receipt:
            case Mode::Help:
                handleTextModalKey(a, key);
                break;

            case Mode::ConfirmQuit:
            case Mode::ConfirmClear:
                handleConfirmKey(a, key, isCode);
                break;
        }
    }

    void handleSaleKey(
        App &a,
        int key,
        bool isCode
    ) {
        if (!isCode && key >= '0' && key <= '9') {
            startBarcode(a);
            a.input.push_back((char)key);
            return;
        }

        switch (key) {
            case 27:
                if (a.focusCart) {
                    a.focusCart = false;
                    setMessage(a, "Фокус: каталог", CP_MUTED);
                }
                break;

            case '\t':
                a.focusCart = !a.focusCart;
                setMessage(
                    a,
                    a.focusCart ? "Фокус: чек" : "Фокус: каталог",
                    CP_MUTED
                );
                break;

            case '/':
                startSearch(a);
                break;

            case '[':
                cycleCategory(a, -1);
                break;

            case ']':
                cycleCategory(a, 1);
                break;

            case '+':
            case '=':
                if (a.focusCart) {
                    changeCartQty(a, 1);
                }
                break;

            case '-':
                if (a.focusCart) {
                    changeCartQty(a, -1);
                }
                break;

            case KEY_UP:
                if (a.focusCart) {
                    moveCart(a, -1);
                }
                else {
                    moveCatalog(a, -1);
                }
                break;

            case KEY_DOWN:
                if (a.focusCart) {
                    moveCart(a, 1);
                }
                else {
                    moveCatalog(a, 1);
                }
                break;

            case KEY_PPAGE:
                if (a.focusCart) {
                    moveCart(a, -max(1, a.cartH - 9));
                }
                else {
                    moveCatalog(a, -max(1, a.catalogH - 6));
                }
                break;

            case KEY_NPAGE:
                if (a.focusCart) {
                    moveCart(a, max(1, a.cartH - 9));
                }
                else {
                    moveCatalog(a, max(1, a.catalogH - 6));
                }
                break;

            case KEY_HOME:
                if (a.focusCart) {
                    a.cartSelected = 0;
                    clampCart(a);
                }
                else {
                    a.selected = 0;
                    clampCatalog(a);
                }
                break;

            case KEY_END:
                if (a.focusCart) {
                    a.cartSelected = a.cart.empty()
                        ? 0
                        : (int)a.cart.size() - 1;
                    clampCart(a);
                }
                else {
                    a.selected = a.visible.empty()
                        ? 0
                        : (int)a.visible.size() - 1;
                    clampCatalog(a);
                }
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                if (!a.focusCart) {
                    addSelected(a);
                }
                break;

            case KEY_DC:
            case KEY_BACKSPACE:
                if (a.focusCart) {
                    removeCartLine(a);
                }
                break;

            case KEY_F(1):
                openHelp(a);
                break;

            case KEY_F(2):
                startQuickItem(a);
                break;

            case KEY_F(3):
                startBarcode(a);
                break;

            case KEY_F(4):
                openReport(a);
                break;

            case KEY_F(5):
                openLastReceipt(a);
                break;

            case KEY_F(6):
                startDiscount(a);
                break;

            case KEY_F(7):
                if (a.cart.empty()) {
                    setMessage(a, "Чек уже пуст", CP_MUTED);
                }
                else {
                    openConfirm(
                        a,
                        Mode::ConfirmClear,
                        "ОЧИСТИТЬ ЧЕК",
                        {
                            "Удалить все позиции из чека?",
                            "Текущая скидка также будет снята.",
                        }
                    );
                }
                break;

            case KEY_F(9):
                startPayment(a);
                break;

            case KEY_F(10):
            case 'q':
            case 'Q':
                requestQuit(a);
                break;

            default:
                if (!isCode && isLetterCodepoint(key)) {
                    startSearch(a);
                    a.search += codepointToUtf8((unsigned int)key);
                    rebuildVisible(a);
                    a.selected = 0;
                    a.catalogScroll = 0;
                }
                break;
        }
    }

    void handleSearchKey(
        App &a,
        int key,
        bool isCode
    ) {
        if (!isCode && isPrintableCodepoint(key)) {
            a.search += codepointToUtf8((unsigned int)key);
            rebuildVisible(a);
            a.selected = 0;
            a.catalogScroll = 0;
            return;
        }

        switch (key) {
            case 27:
                a.search.clear();
                rebuildVisible(a);
                a.selected = 0;
                a.catalogScroll = 0;
                finishSearch(a);
                setMessage(a, "Поиск отменён", CP_MUTED);
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                popUtf8(a.search);
                rebuildVisible(a);
                a.selected = 0;
                a.catalogScroll = 0;
                break;

            case KEY_UP:
                moveCatalog(a, -1);
                break;

            case KEY_DOWN:
                moveCatalog(a, 1);
                break;

            case KEY_PPAGE:
                moveCatalog(a, -max(1, a.catalogH - 6));
                break;

            case KEY_NPAGE:
                moveCatalog(a, max(1, a.catalogH - 6));
                break;

            case KEY_HOME:
                a.selected = 0;
                clampCatalog(a);
                break;

            case KEY_END:
                a.selected = a.visible.empty()
                    ? 0
                    : (int)a.visible.size() - 1;
                clampCatalog(a);
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                if (a.visible.empty()) {
                    setMessage(a, "Ничего не найдено", CP_WARN);
                }
                else {
                    addSelected(a);
                    a.search.clear();
                    rebuildVisible(a);
                    a.selected = 0;
                    a.catalogScroll = 0;
                    finishSearch(a);
                }
                break;

            default:
                break;
        }
    }

    void handlePaymentKey(
        App &a,
        int key,
        bool isCode
    ) {
        if (!isCode && a.paymentMethod == PaymentMethod::Cash) {
            bool digit = key >= '0' && key <= '9';
            bool dot = key == ',' || key == '.';

            if (digit && a.input.size() < 12) {
                a.input.push_back((char)key);
                a.error.clear();
                return;
            }

            if (dot &&
                a.input.size() < 12 &&
                a.input.find(',') == string::npos &&
                a.input.find('.') == string::npos) {
                a.input.push_back(',');
                a.error.clear();
                return;
            }
        }

        switch (key) {
            case 27:
                closeModal(a);
                a.mode = Mode::Sale;
                a.error.clear();
                a.input.clear();
                setMessage(a, "Оплата отменена", CP_MUTED);
                break;

            case '\t':
            case KEY_LEFT:
            case KEY_RIGHT:
                a.paymentMethod =
                    a.paymentMethod == PaymentMethod::Cash
                        ? PaymentMethod::Card
                        : PaymentMethod::Cash;
                a.error.clear();
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                popUtf8(a.input);
                a.error.clear();
                break;

            case KEY_DC:
                a.input.clear();
                a.error.clear();
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                finishPayment(a);
                break;

            default:
                break;
        }
    }

    void handleQuickItemKey(
        App &a,
        int key,
        bool isCode
    ) {
        if (!isCode && isPrintableCodepoint(key)) {
            if (a.quickNameActive) {
                if (a.quickName.size() < 120) {
                    a.quickName += codepointToUtf8((unsigned int)key);
                    a.error.clear();
                }
            }
            else {
                bool digit = key >= '0' && key <= '9';
                bool dot = key == ',' || key == '.';

                if (digit && a.quickPrice.size() < 12) {
                    a.quickPrice.push_back((char)key);
                    a.error.clear();
                }
                else if (dot &&
                         a.quickPrice.size() < 12 &&
                         a.quickPrice.find(',') == string::npos &&
                         a.quickPrice.find('.') == string::npos) {
                    a.quickPrice.push_back(',');
                    a.error.clear();
                }
            }
            return;
        }

        switch (key) {
            case 27:
                closeModal(a);
                a.mode = Mode::Sale;
                a.error.clear();
                setMessage(a, "Быстрый товар отменён", CP_MUTED);
                break;

            case '\t':
            case KEY_BTAB:
                a.quickNameActive = !a.quickNameActive;
                a.error.clear();
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                if (a.quickNameActive) {
                    popUtf8(a.quickName);
                }
                else {
                    popUtf8(a.quickPrice);
                }
                a.error.clear();
                break;

            case KEY_DC:
                if (a.quickNameActive) {
                    a.quickName.clear();
                }
                else {
                    a.quickPrice.clear();
                }
                a.error.clear();
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                addQuickItem(a);
                break;

            default:
                break;
        }
    }

    void handleBarcodeKey(
        App &a,
        int key,
        bool isCode
    ) {
        if (!isCode &&
            isPrintableCodepoint(key) &&
            key != ' ' &&
            a.input.size() < 32) {
            a.input += codepointToUtf8((unsigned int)key);
            a.error.clear();
            return;
        }

        switch (key) {
            case 27:
                closeModal(a);
                a.mode = Mode::Sale;
                a.error.clear();
                setMessage(a, "Ввод штрихкода отменён", CP_MUTED);
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                popUtf8(a.input);
                a.error.clear();
                break;

            case KEY_DC:
                a.input.clear();
                a.error.clear();
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                findBarcode(a);
                break;

            default:
                break;
        }
    }

    void handleDiscountKey(
        App &a,
        int key,
        bool isCode
    ) {
        if (!isCode && key >= '0' && key <= '9' && a.input.size() < 2) {
            a.input.push_back((char)key);
            a.error.clear();
            return;
        }

        switch (key) {
            case 27:
                closeModal(a);
                a.mode = Mode::Sale;
                a.error.clear();
                setMessage(a, "Скидка не изменена", CP_MUTED);
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                popUtf8(a.input);
                a.error.clear();
                break;

            case KEY_DC:
                a.input.clear();
                a.error.clear();
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                applyDiscount(a);
                break;

            default:
                break;
        }
    }

    void handleTextModalKey(App &a, int key)
    {
        int h = 0, w = 0;

        if (a.modalWin) {
            getmaxyx(a.modalWin, h, w);
        }

        int page = max(1, h - 4);

        switch (key) {
            case 27:
            case 'q':
            case 'Q':
            case '\n':
            case '\r':
            case KEY_ENTER:
                closeModal(a);
                a.mode = Mode::Sale;
                break;

            case KEY_UP:
                scrollModal(a, -1);
                break;

            case KEY_DOWN:
                scrollModal(a, 1);
                break;

            case KEY_PPAGE:
                scrollModal(a, -page);
                break;

            case KEY_NPAGE:
                scrollModal(a, page);
                break;

            case KEY_HOME:
                a.modalScroll = 0;
                break;

            case KEY_END:
                a.modalScroll = maxModalScroll(a);
                break;

            default:
                break;
        }
    }

    void handleConfirmKey(
        App &a,
        int key,
        bool isCode
    ) {
        bool confirm = false;
        bool cancel = false;

        if (key == 27) {
            cancel = true;
        }
        else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
            confirm = true;
        }
        else if (!isCode && (key == 'y' || key == 'Y')) {
            confirm = true;
        }
        else if (!isCode && (key == 'n' || key == 'N')) {
            cancel = true;
        }

        if (confirm) {
            Mode oldMode = a.mode;

            if (oldMode == Mode::ConfirmQuit) {
                closeModal(a);
                a.running = false;
            }
            else if (oldMode == Mode::ConfirmClear) {
                clearCart(a);
                closeModal(a);
                a.mode = Mode::Sale;
            }
            return;
        }

        if (cancel) {
            closeModal(a);
            a.mode = Mode::Sale;
            setMessage(a, "Отменено", CP_MUTED);
        }
    }
}
