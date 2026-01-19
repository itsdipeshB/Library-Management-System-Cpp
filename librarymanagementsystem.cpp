#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <limits>
#include <algorithm>

using namespace std;

// "day number" to keep dates simple (days since epoch)
static int todayDay() {
    return (int)(time(nullptr) / 86400);
}

static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, b - a + 1);
}

static vector<string> split(const string& s, char delim) {
    vector<string> parts;
    string item;
    stringstream ss(s);
    while (getline(ss, item, delim)) parts.push_back(item);
    return parts;
}

// Avoid breaking file format if user types '|'
static string safeField(string s) {
    replace(s.begin(), s.end(), '|', '/');
    return s;
}

struct Book {
    int id=0, total=0, avail=0;
    string title, author;
};

struct Member {
    int id=0;
    string name;
};

struct Issue {
    int rid=0, bookId=0, memberId=0;
    int issueDay=0, dueDay=0;
    int returnedDay=-1; // -1 if not returned
};

class LMS {
    vector<Book> books;
    vector<Member> members;
    vector<Issue> issues;

    int nextBookId=1, nextMemberId=1, nextRecId=1;
    const int LOAN_DAYS = 14;

    const string BF="books.txt", MF="members.txt", IF="issues.txt";

public:
    void run() {
        load();
        while (true) {
            cout << "\n=== LIBRARY MANAGEMENT SYSTEM ===\n"
                 << "1) Books\n2) Members\n3) Issue Book\n4) Return Book\n5) View Issues\n0) Save & Exit\nChoose: ";
            int c = readInt();
            if (c==1) booksMenu();
            else if (c==2) membersMenu();
            else if (c==3) issueBook();
            else if (c==4) returnBook();
            else if (c==5) viewIssues();
            else if (c==0) { save(); cout << "Saved.\n"; return; }
            else cout << "Invalid.\n";
        }
    }

private:
    // ---------- Safe input ----------
    int readInt() {
        while (true) {
            string s;
            if (!getline(cin, s)) return 0;
            s = trim(s);
            if (s.empty()) { cout << "Enter a number: "; continue; }
            try {
                size_t idx = 0;
                int v = stoi(s, &idx);
                if (idx != s.size()) throw runtime_error("bad");
                return v;
            } catch (...) {
                cout << "Invalid number. Try again: ";
            }
        }
    }

    string readLine(const string& prompt) {
        cout << prompt;
        string s;
        getline(cin, s);
        return trim(s);
    }

    // ---------- File I/O ----------
    void load() {
        loadBooks(); loadMembers(); loadIssues();
        for (auto &b: books) nextBookId = max(nextBookId, b.id + 1);
        for (auto &m: members) nextMemberId = max(nextMemberId, m.id + 1);
        for (auto &r: issues) nextRecId = max(nextRecId, r.rid + 1);
    }

    void save() {
        saveBooks(); saveMembers(); saveIssues();
    }

    void loadBooks() {
        books.clear();
        ifstream in(BF);
        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;

            auto p = split(line, '|');
            if (p.size() < 5) continue; // ignore bad lines

            Book b;
            try {
                b.id = stoi(p[0]);
                b.total = stoi(p[1]);
                b.avail = stoi(p[2]);
            } catch (...) { continue; }

            b.title = p[3];
            b.author = p[4];

            if (b.total < 0) b.total = 0;
            if (b.avail < 0) b.avail = 0;
            if (b.avail > b.total) b.avail = b.total;

            books.push_back(b);
        }
    }

    void saveBooks() {
        ofstream out(BF, ios::trunc);
        for (auto &b: books) {
            out << b.id << "|" << b.total << "|" << b.avail << "|"
                << safeField(b.title) << "|" << safeField(b.author) << "\n";
        }
    }

    void loadMembers() {
        members.clear();
        ifstream in(MF);
        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;

            auto p = split(line, '|');
            if (p.size() < 2) continue;

            Member m;
            try { m.id = stoi(p[0]); } catch (...) { continue; }
            m.name = p[1];
            members.push_back(m);
        }
    }

    void saveMembers() {
        ofstream out(MF, ios::trunc);
        for (auto &m: members) {
            out << m.id << "|" << safeField(m.name) << "\n";
        }
    }

    void loadIssues() {
        issues.clear();
        ifstream in(IF);
        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;

            auto p = split(line, '|');
            if (p.size() < 6) continue;

            Issue r;
            try {
                r.rid = stoi(p[0]);
                r.bookId = stoi(p[1]);
                r.memberId = stoi(p[2]);
                r.issueDay = stoi(p[3]);
                r.dueDay = stoi(p[4]);
                r.returnedDay = stoi(p[5]);
            } catch (...) { continue; }

            issues.push_back(r);
        }
    }

    void saveIssues() {
        ofstream out(IF, ios::trunc);
        for (auto &r: issues) {
            out << r.rid << "|" << r.bookId << "|" << r.memberId << "|"
                << r.issueDay << "|" << r.dueDay << "|" << r.returnedDay << "\n";
        }
    }

    // ---------- Find ----------
    Book* findBook(int id) {
        for (auto &b: books) if (b.id == id) return &b;
        return nullptr;
    }
    Member* findMember(int id) {
        for (auto &m: members) if (m.id == id) return &m;
        return nullptr;
    }
    Issue* findIssue(int rid) {
        for (auto &r: issues) if (r.rid == rid) return &r;
        return nullptr;
    }

    // ---------- Books ----------
    void booksMenu() {
        while (true) {
            cout << "\n--- BOOKS ---\n1) Add\n2) List\n3) Search\n0) Back\nChoose: ";
            int c = readInt();
            if (c==1) addBook();
            else if (c==2) listBooks();
            else if (c==3) searchBooks();
            else if (c==0) return;
            else cout << "Invalid.\n";
        }
    }

    void addBook() {
        Book b;
        b.id = nextBookId++;
        b.title = readLine("Title: ");
        b.author = readLine("Author: ");
        cout << "Total copies: ";
        b.total = readInt();
        if (b.total < 0) b.total = 0;
        b.avail = b.total;
        books.push_back(b);
        cout << "Added Book ID: " << b.id << "\n";
    }

    void listBooks() {
        if (books.empty()) { cout << "No books.\n"; return; }
        cout << "ID | Avail/Total | Title - Author\n";
        for (auto &b: books)
            cout << b.id << " | " << b.avail << "/" << b.total << " | " << b.title << " - " << b.author << "\n";
    }

    void searchBooks() {
        string q = readLine("Keyword: ");
        if (q.empty()) return;
        bool any=false;
        for (auto &b: books) {
            if ((b.title + " " + b.author).find(q) != string::npos) {
                any=true;
                cout << "ID " << b.id << " | " << b.avail << "/" << b.total
                     << " | " << b.title << " - " << b.author << "\n";
            }
        }
        if(!any) cout << "No matches.\n";
    }

    // ---------- Members ----------
    void membersMenu() {
        while (true) {
            cout << "\n--- MEMBERS ---\n1) Add\n2) List\n3) Search\n0) Back\nChoose: ";
            int c = readInt();
            if (c==1) addMember();
            else if (c==2) listMembers();
            else if (c==3) searchMembers();
            else if (c==0) return;
            else cout << "Invalid.\n";
        }
    }

    void addMember() {
        Member m;
        m.id = nextMemberId++;
        m.name = readLine("Name: ");
        members.push_back(m);
        cout << "Added Member ID: " << m.id << "\n";
    }

    void listMembers() {
        if (members.empty()) { cout << "No members.\n"; return; }
        cout << "ID | Name\n";
        for (auto &m: members) cout << m.id << " | " << m.name << "\n";
    }

    void searchMembers() {
        string q = readLine("Keyword: ");
        if (q.empty()) return;
        bool any=false;
        for (auto &m: members) {
            if (m.name.find(q) != string::npos) {
                any=true;
                cout << "ID " << m.id << " | " << m.name << "\n";
            }
        }
        if(!any) cout << "No matches.\n";
    }

    // ---------- Issue / Return ----------
    void issueBook() {
        cout << "Book ID: ";
        int bid = readInt();
        cout << "Member ID: ";
        int mid = readInt();

        Book* b = findBook(bid);
        Member* m = findMember(mid);

        if (!b) { cout << "Book not found.\n"; return; }
        if (!m) { cout << "Member not found.\n"; return; }
        if (b->avail <= 0) { cout << "No copies available.\n"; return; }

        // prevent duplicate active issue of same book to same member
        for (auto &x: issues) {
            if (x.bookId == bid && x.memberId == mid && x.returnedDay == -1) {
                cout << "This member already has this book (not returned).\n";
                return;
            }
        }

        Issue r;
        r.rid = nextRecId++;
        r.bookId = bid;
        r.memberId = mid;
        r.issueDay = todayDay();
        r.dueDay = r.issueDay + LOAN_DAYS;
        r.returnedDay = -1;

        issues.push_back(r);
        b->avail--;
        cout << "Issued. Record ID: " << r.rid << " | Due in " << LOAN_DAYS << " days.\n";
    }

    void returnBook() {
        cout << "Record ID: ";
        int rid = readInt();

        Issue* r = findIssue(rid);
        if (!r) { cout << "Record not found.\n"; return; }
        if (r->returnedDay != -1) { cout << "Already returned.\n"; return; }

        Book* b = findBook(r->bookId);
        if (!b) { cout << "Book missing (data issue).\n"; return; }

        r->returnedDay = todayDay();
        b->avail++;

        int late = r->returnedDay - r->dueDay;
        if (late > 0) cout << "Returned. Late by " << late << " day(s).\n";
        else cout << "Returned on time.\n";
    }

    void viewIssues() {
        if (issues.empty()) { cout << "No issue records.\n"; return; }
        cout << "RID | BookID | MemberID | DueInDays | Status\n";
        int t = todayDay();
        for (auto &r: issues) {
            int dueIn = r.dueDay - t;
            cout << r.rid << " | " << r.bookId << " | " << r.memberId
                 << " | " << dueIn << " | " << (r.returnedDay == -1 ? "ISSUED" : "RETURNED") << "\n";
        }
    }
};

int main() {
    LMS sys;
    sys.run();
    return 0;
}
