#include "./lib.hpp"

// ---------------------------------------------------------------------------
// Small UTF-8 helpers: we count terminal cells instead of bytes so that
// Cyrillic / other multibyte text is not cut in the middle of a character.
// ---------------------------------------------------------------------------

namespace change_script_lib {
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

    // Cut a string so that it occupies at most `width` terminal cells.
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

    // Remove ANSI escape sequences (colors, cursor movement, OSC titles) and
    // stray control characters, so the output stays readable inside ncurses.
    string stripAnsi(const string &in)
    {
        string out;
        out.reserve(in.size());
        for (size_t i = 0; i < in.size();) {
            unsigned char c = (unsigned char)in[i];
            if (c == 0x1b && i + 1 < in.size()) {
                char n = in[i + 1];
                if (n == '[') {                       // CSI ... final byte
                    i += 2;
                    while (i < in.size()) {
                        unsigned char f = (unsigned char)in[i++];
                        if (f >= 0x40 && f <= 0x7e) {
                            break;
                        }
                    }
                    continue;
                }
                if (n == ']') {                       // OSC ... BEL or ESC '\'
                    i += 2;
                    while (i < in.size()) {
                        if (in[i] == '\a') { 
                            ++i; 
                            break; 
                        }
                        
                        if (in[i] == 0x1b && i + 1 < in.size() && in[i + 1] == '\\') {
                            i += 2;
                            break;
                        }
                        ++i;
                    }
                    continue;
                }
                i += 2;                               // other two-byte escape
                continue;
            }
            if (c == '\t') {
                out.push_back('\t');
                ++i;
                continue;
            }
            if (c < 0x20 || c == 0x7f) {              // drop control chars
                ++i;
                continue;
            }
            out.push_back((char)c);
            ++i;
        }
        return out;
    }

    // Wrap one logical line into pieces that fit `width` cells. Tabs are expanded
    // to 8-column stops and wide characters are kept intact.
    void wrapLine(
        const string &line, 
        int width, 
        vector<string> &out
    ) {
        if (width < 1) width = 1;
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

    // ---------------------------------------------------------------------------
    // Filesystem scanning
    // ---------------------------------------------------------------------------
    bool hasScriptExtension(const string &name)
    {
        auto endsWith = [&](const char *ext) {
            size_t n = strlen(ext);
            return name.size() >= n && name.compare(name.size() - n, n, ext) == 0;
        };
    
        return endsWith(".sh") || endsWith(".bash") || endsWith(".zsh") ||
              endsWith(".zhs") || endsWith(".fish");
    }

    bool skipDirectory(const fs::path &p, bool includeHidden)
    {
        string name = p.filename().string();
        static const char *junk[] = {
            ".git", ".hg", ".svn", "node_modules", "__pycache__",
            ".cache", ".npm", ".gradle", ".m2", "site-packages",
            "venv", ".venv", "target",
        };

        for (const char *j : junk) {
            if (name == j) {
                return true; 
            }
        }

        // Go's module cache and similar package stores contain thousands of
        // unrelated helper scripts.
        string full = p.generic_string();
        if (full.find("/go/pkg/mod") != string::npos) {
            return true;
        }
        if (full.find("/.var/app/") != string::npos) {
            return true;
        }

        if (!includeHidden && !name.empty() && name[0] == '.') {
            return true;
        }

        return false;
    }

    vector<Script> findScripts(
        const string &home, 
        bool includeHidden
    ) {
        vector<Script> found;
        error_code ec;

        fs::recursive_directory_iterator it(
            home, 
            fs::directory_options::skip_permission_denied, 
            ec
        );

        fs::recursive_directory_iterator end;
    
        if (ec) { 
            return found;
        }

        for (; it != end; it.increment(ec)) {
            if (ec) {
                ec.clear();
                continue;
            }

            const fs::path &p = it->path();
            string name = p.filename().string();

            error_code statEc;
            if (it->is_directory(statEc)) {
                if (skipDirectory(p, includeHidden)) {
                    it.disable_recursion_pending();
                }
                continue;
            }

            if (!it->is_regular_file(statEc)) {
                continue;
            }

            if (!includeHidden && !name.empty() && name[0] == '.') {
                continue;
            }

            if (!hasScriptExtension(name)) {
                continue;
            }

            Script s;
            s.path = p.string();
            error_code relEc;
            fs::path rel = fs::relative(p, home, relEc);
            
            if (!relEc && !rel.empty() && rel.native().compare(0, 2, "..") != 0) {
                s.label = "~/" + rel.string();
            }
            else {
                s.label = s.path;
            }

            found.push_back(std::move(s));
        }

        sort(found.begin(), found.end(), [](const Script &a, const Script &b) {
            return a.label < b.label;
        });
        
        return found;
    }

    // ---------------------------------------------------------------------------
    // Running a script
    // ---------------------------------------------------------------------------
    string shellQuote(const string &s)
    {
        string r = "'";
        for (char c : s) {
            if (c == '\'') {
                r += "'\\''";
            }

            else {
                r += c;
            }
        }

        r += "'";
        return r;
    }

    string lowerExtension(const string &path)
    {
        string ext = fs::path(path).extension().string();
        for (char &c : ext){
            c = (char)tolower((unsigned char)c);
        }

        return ext;
    }

    string buildExecCommand(const string &path)
    {
        // Prefer running the file directly when it is executable (shebang wins).
        if (access(path.c_str(), X_OK) == 0) {
            return shellQuote(path);
        }

        string ext = lowerExtension(path);
    
        if (ext == ".sh" || ext == ".bash") {
            return "bash " + shellQuote(path);
        }
    
        if (ext == ".zsh" || ext == ".zhs") {
            return "zsh " + shellQuote(path);
        }

        if (ext == ".fish") {
            return "fish " + shellQuote(path);
        }

        return "sh " + shellQuote(path);
    }

    // Read stdout+stderr of the command. Lines are cleaned and stored; the process
    // is always drained fully so pclose() cannot deadlock on a full pipe.
    void captureOutput(
        const string &cmd, 
        vector<string> &lines, 
        string &result
    ) {
        lines.clear();
        result = "";

        FILE * pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            lines.push_back("[error] failed to run command");
            result = "popen failed";
            return;
        }

        const size_t maxBytes = 4 * 1024 * 1024;
        const size_t maxLines = 50000;

        string pending;
        char buf[4096];
        size_t total = 0;
        bool truncated = false;
        bool stopStoring = false;

        auto pushLine = [&](string line) {
            size_t cr = line.rfind('\r');        // keep the last overwrite
            if (cr != string::npos) {
                line = line.substr(cr + 1);
            }
            lines.push_back(stripAnsi(line));
        };

        while (fgets(buf, sizeof(buf), pipe)) {
            if (stopStoring) {
                continue;
            }

            pending += buf;
            total += strlen(buf);

            size_t pos;
            while ((pos = pending.find('\n')) != string::npos) {
                string line = pending.substr(0, pos);
                pending.erase(0, pos + 1);
                pushLine(std::move(line));
                if (lines.size() >= maxLines) {
                    truncated = true;
                    stopStoring = true;
                    break;
                }
            }

            if (total > maxBytes) {
                truncated = true;
                stopStoring = true;
            }
        }

        if (!stopStoring && !pending.empty()) {
            pushLine(std::move(pending));
        }

        int status = pclose(pipe);

        if (WIFEXITED(status)) {
            result = "exit " + to_string(WEXITSTATUS(status));
        } 
        else if (WIFSIGNALED(status)) {
            result = "signal " + to_string(WTERMSIG(status));
        } 
        else {
            result = "finished";
        }

        if (truncated) {
            lines.push_back("[output truncated]");
            result += " (truncated)";
        }

        if (lines.empty()) {
            lines.push_back("[no output]");
        }
    }

    // ---------------------------------------------------------------------------
    // UI
    // ---------------------------------------------------------------------------
    void closeWindows(App &a)
    {
        if (a.menuWin) { 
            delwin(a.menuWin); 
            a.menuWin = nullptr; 
        }
        
        if (a.outWin) { 
            delwin(a.outWin);  
            a.outWin = nullptr; 
        }
    }

    void layout(App &a)
    {
        int h = 0, w = 0;
        getmaxyx(stdscr, h, w);
        if (h < 4 || w < 20) {
            return;                          // keep the previous layout if too small
        }

        a.termH = h;
        a.termW = w;

        closeWindows(a);

        a.menuH = a.termH - 1;               // last row is the status bar
        a.menuW = a.termW * 2 / 5;
    
        if (a.menuW < 24) {
            a.menuW = min(24, a.termW / 2);
        }

        if (a.termW - a.menuW - 1 < 20) {
            a.menuW = max(10, a.termW - 21);
        }

        a.outW = a.termW - a.menuW - 1;
        a.outH = a.menuH;

        a.menuWin = newwin(a.menuH, a.menuW, 0, 0);
        a.outWin  = newwin(a.outH, a.outW, 0, a.menuW + 1);
    
        keypad(a.menuWin, TRUE);
        keypad(a.outWin, TRUE);

        a.outWrapW = -1;                     // force re-wrap on next draw
    }

    void rewrapOutput(App &a)
    {
        int innerW = a.outW - 2;
        if (innerW < 1) {
            innerW = 1;
        }

        a.wrapped.clear();
    
        for (const string &line : a.rawOut) {
            wrapLine(line, innerW, a.wrapped);
        }

        a.outWrapW = innerW;
    }

    void clampMenuScroll(App &a)
    {
        int listH = a.menuH - 2;
        int count = (int)a.scripts.size();
    
        if (listH < 1) {
            listH = 1;
        }

        if (a.selected < 0) {
            a.selected = 0;
        }

        if (a.selected >= count) {
            a.selected = count > 0 ? count - 1 : 0;
        }

        int maxScroll = max(0, count - listH);
    
        if (a.selected < a.menuScroll) {
            a.menuScroll = a.selected;
        }

        if (a.selected >= a.menuScroll + listH) {
            a.menuScroll = a.selected - listH + 1;
        }

        if (a.menuScroll < 0) {
            a.menuScroll = 0;
        }

        if (a.menuScroll > maxScroll) {
            a.menuScroll = maxScroll;
        }
    }

    void drawMenu(App &a)
    {
        werase(a.menuWin);

        if (!a.focusOutput) {
            wattron(a.menuWin, A_BOLD);
        }

        box(a.menuWin, 0, 0);
    
        if (!a.focusOutput) {
            wattroff(a.menuWin, A_BOLD);
        }

        string title = " Scripts (" + to_string(a.scripts.size()) + ") ";
        mvwaddstr(a.menuWin, 0, 2, fitText(title, a.menuW - 4).c_str());

        int listH = a.menuH - 2;
        int innerW = a.menuW - 2;
    
        clampMenuScroll(a);

        for (int row = 0; row < listH; ++row) {
            int idx = a.menuScroll + row;
            if (idx >= (int)a.scripts.size()) {
                break;
            }

            bool sel = (idx == a.selected);
            if (sel) {
                wattron(a.menuWin, A_REVERSE);
            }

            mvwhline(a.menuWin, 1 + row, 1, ' ', innerW);
            string text = " " + a.scripts[idx].label;
            mvwaddstr(
                a.menuWin, 
                1 + row, 1,
                fitText(text, innerW).c_str()
            );

            if (sel) {
                wattroff(a.menuWin, A_REVERSE);
            }
        }

        wrefresh(a.menuWin);
    }

    void drawOutput(App &a)
    {   
        werase(a.outWin);

        if (a.focusOutput) {
            wattron(a.outWin, A_BOLD);
        }

        box(a.outWin, 0, 0);

        if (a.focusOutput) {
            wattroff(a.outWin, A_BOLD);
        }

        string title = " Output ";
        if (!a.scripts.empty()) {
            title += "- " + a.scripts[a.selected].label;
        }

        mvwaddstr(a.outWin, 0, 2, fitText(title, a.outW - 4).c_str());

        if (a.outWrapW != a.outW - 2) {
            rewrapOutput(a);
        }

        int innerH = a.outH - 2;
        int innerW = a.outW - 2;
        if (innerH < 1) {
            innerH = 1;
        }

        int maxScroll = max(0, (int)a.wrapped.size() - innerH);
    
        if (a.outScroll > maxScroll) {
            a.outScroll = maxScroll;
        }

        if (a.outScroll < 0) {
            a.outScroll = 0;
        }

        if (a.wrapped.empty()) {
            mvwaddstr(
                a.outWin, 1, 2,
                fitText("No output yet. Press Enter to run the selected script.",innerW - 1).c_str()
            );
        } 
        else {
            for (int row = 0; row < innerH; ++row) {
                int idx = a.outScroll + row;
            
                if (idx >= (int)a.wrapped.size()) {
                    break;
                }

                mvwaddstr(
                    a.outWin, 
                    1 + row, 1,
                    fitText(a.wrapped[idx], innerW).c_str()
                );
            }
        }

        wrefresh(a.outWin);
    }

    void drawStatus(App &a)
    {
        string left = a.focusOutput
            ? " PgUp/PgDn: scroll  Tab: pane  Enter: run  q: quit "
            : " Enter: run  Tab: pane  r: rescan  a: hidden  q: quit ";
    
        string right;
    
        if (!a.wrapped.empty()) {
            int innerH = max(1, a.outH - 2);
            int first = a.outScroll + 1;
            int last = min<int>(a.outScroll + innerH, (int)a.wrapped.size());
            right += "out " + to_string(first) + "-" + to_string(last) +
                    "/" + to_string(a.wrapped.size()) + "  ";
        }
        
        if (!a.message.empty()) {
            right += a.message + " ";
        }

        int rightCols = textWidth(right);
    
        if (rightCols > a.termW) {
            right = fitText(right, a.termW);
            rightCols = textWidth(right);
        }

        int leftCols = max(0, a.termW - rightCols);

        move(a.termH - 1, 0);
        clrtoeol();
        attron(A_REVERSE);
        mvaddstr(a.termH - 1, 0, fitText(left, leftCols).c_str());
    
        if (rightCols > 0) {
            mvaddstr(a.termH - 1, a.termW - rightCols, right.c_str());
        }        

        attroff(A_REVERSE);
        refresh();
    }

    void drawAll(App &a)
    {
        drawMenu(a);
        drawOutput(a);
        drawStatus(a);
    }

    void scrollOutput(App &a, int delta)
    {
        int innerH = max(1, a.outH - 2);
        int maxScroll = max(0, (int)a.wrapped.size() - innerH);
        a.outScroll += delta;
    
        if (a.outScroll < 0) {
            a.outScroll = 0;
        }

        if (a.outScroll > maxScroll) {
            a.outScroll = maxScroll;
        }
    }

    void moveSelection(App &a, int delta)
    {
        if (a.scripts.empty()) {
            return;
        }

        a.selected += delta;

        if (a.selected < 0) {
            a.selected = 0;
        }

        if (a.selected >= (int)a.scripts.size()) {
            a.selected = (int)a.scripts.size() - 1;
        }
    }

    void runSelected(App &a)
    {
        if (a.scripts.empty()) {
            a.message = "no scripts found";
            return;
        }

        const Script &s = a.scripts[a.selected];
        fs::path p(s.path);
        string dir = p.parent_path().string();
        if (dir.empty()) {
            dir = a.home;
        }

        // stdin is detached so a script cannot steal keys from the picker or
        // block forever waiting for input while we are capturing its output.
        string cmd = "cd " + shellQuote(dir) + " && " +
                    buildExecCommand(s.path) + " 2>&1 < /dev/null";

        // Give immediate feedback before the blocking popen() call.
        a.rawOut = {"Running " + s.label + " ..."};
        a.wrapped.clear();
        a.outWrapW = -1;
        a.outScroll = 0;
        a.focusOutput = true;
        drawAll(a);

        captureOutput(cmd, a.rawOut, a.message);
        a.outWrapW = -1;
        a.outScroll = 0;
    }

    void rescan(App &a)
    {
        string keep;
        if (!a.scripts.empty()) {
            keep = a.scripts[a.selected].path;
        }

        a.scripts = findScripts(a.home, a.includeHidden);
        a.selected = 0;
        a.menuScroll = 0;
        
        for (size_t i = 0; i < a.scripts.size(); ++i) {
            if (a.scripts[i].path == keep) {
                a.selected = (int)i;
                break;
            }
        }

        a.message = "found " + to_string(a.scripts.size()) + " script(s)" +
                (a.includeHidden ? " (hidden shown)" : "");
    }

    void printList(
        const string &home, 
        bool includeHidden
    ) {
        vector<Script> scripts = findScripts(home, includeHidden);
        for (const Script &s : scripts) {
            printf("%s\n", s.path.c_str());
        }
    }

}
