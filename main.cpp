// Library Management System - Data Structures project (3rd Sem B.Tech CSE)
// Data structures used:
//   * Binary Search Tree  -> book catalogue (search/insert/delete by Book ID)
//   * Hash Table (chaining) -> member records
//   * Queue               -> waiting list for a book with no copies left
//   * Stack               -> undo of the last issue/return operations
//   * File handling       -> data persists between runs
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <algorithm>
using namespace std;

// ---------------------------------------------------------------- Book + BST
struct Book {
    int id;
    string title, author;
    int total, available;
};

struct BNode {
    Book b;
    BNode *left = nullptr, *right = nullptr;
    explicit BNode(const Book& x) : b(x) {}
};

class BookBST {
    BNode* root = nullptr;

    BNode* insert(BNode* n, const Book& b, bool& ok) {
        if (!n) { ok = true; return new BNode(b); }
        if (b.id < n->b.id) n->left = insert(n->left, b, ok);
        else if (b.id > n->b.id) n->right = insert(n->right, b, ok);
        else ok = false;  // duplicate id
        return n;
    }
    BNode* minNode(BNode* n) { while (n->left) n = n->left; return n; }
    BNode* remove(BNode* n, int id, bool& ok) {
        if (!n) return nullptr;
        if (id < n->b.id) n->left = remove(n->left, id, ok);
        else if (id > n->b.id) n->right = remove(n->right, id, ok);
        else {
            ok = true;
            if (!n->left)  { BNode* r = n->right; delete n; return r; }
            if (!n->right) { BNode* l = n->left;  delete n; return l; }
            BNode* s = minNode(n->right);          // inorder successor
            n->b = s->b;
            bool dummy;
            n->right = remove(n->right, s->b.id, dummy);
        }
        return n;
    }
    void inorder(BNode* n, vector<Book*>& out) {
        if (!n) return;
        inorder(n->left, out); out.push_back(&n->b); inorder(n->right, out);
    }
    void destroy(BNode* n) { if (n) { destroy(n->left); destroy(n->right); delete n; } }

public:
    ~BookBST() { destroy(root); }
    bool insert(const Book& b) { bool ok = false; root = insert(root, b, ok); return ok; }
    bool remove(int id) { bool ok = false; root = remove(root, id, ok); return ok; }
    Book* find(int id) {
        BNode* n = root;
        while (n) {
            if (id == n->b.id) return &n->b;
            n = (id < n->b.id) ? n->left : n->right;
        }
        return nullptr;
    }
    vector<Book*> all() { vector<Book*> v; inorder(root, v); return v; }
};

// ------------------------------------------------- Member + Hash table (chaining)
struct Member {
    int id;
    string name;
    vector<int> borrowed;
};

class MemberTable {
    static const int SIZE = 31;
    list<Member> table[SIZE];
    int hash(int id) const { return id % SIZE; }
public:
    bool add(const Member& m) {
        if (find(m.id)) return false;
        table[hash(m.id)].push_back(m);
        return true;
    }
    Member* find(int id) {
        for (auto& m : table[hash(id)]) if (m.id == id) return &m;
        return nullptr;
    }
    vector<Member*> all() {
        vector<Member*> v;
        for (auto& bucket : table) for (auto& m : bucket) v.push_back(&m);
        sort(v.begin(), v.end(), [](Member* a, Member* b) { return a->id < b->id; });
        return v;
    }
};

// ------------------------------------------------------------------- Library
enum class Op { ISSUE, RETURN };
struct Action { Op op; int memberId, bookId; };

class Library {
    BookBST books;
    MemberTable members;
    map<int, queue<int>> waitlist;   // bookId -> queue of memberIds
    stack<Action> undoStack;
    const string bookFile = "books.txt", memberFile = "members.txt";

    static void borrow(Member* m, Book* b) { m->borrowed.push_back(b->id); b->available--; }
    static void giveBack(Member* m, Book* b) {
        m->borrowed.erase(find(m->borrowed.begin(), m->borrowed.end(), b->id));
        b->available++;
    }

public:
    Library() { load(); }
    ~Library() { save(); }

    void addBook(int id, const string& t, const string& a, int copies) {
        if (copies <= 0) { cout << "Copies must be positive.\n"; return; }
        cout << (books.insert({id, t, a, copies, copies}) ? "Book added.\n" : "A book with this ID already exists.\n");
    }
    void removeBook(int id) {
        Book* b = books.find(id);
        if (b && b->available != b->total) { cout << "Cannot remove: copies are currently issued.\n"; return; }
        cout << (books.remove(id) ? "Book removed.\n" : "Book not found.\n");
    }
    void searchBook(int id) {
        Book* b = books.find(id);
        if (!b) { cout << "Book not found.\n"; return; }
        printBook(*b);
    }
    void listBooks() {
        auto v = books.all();
        if (v.empty()) { cout << "No books in catalogue.\n"; return; }
        cout << "\nID    Title                          Author               Avail/Total\n";
        cout << string(75, '-') << "\n";
        for (auto* b : v) printBook(*b);
    }
    void addMember(int id, const string& name) {
        cout << (members.add({id, name, {}}) ? "Member registered.\n" : "Member ID already exists.\n");
    }
    void listMembers() {
        auto v = members.all();
        if (v.empty()) { cout << "No members registered.\n"; return; }
        for (auto* m : v) {
            cout << m->id << "  " << m->name << "  | borrowed: ";
            if (m->borrowed.empty()) cout << "none";
            for (int id : m->borrowed) cout << id << " ";
            cout << "\n";
        }
    }

    void issue(int memberId, int bookId) {
        Member* m = members.find(memberId);
        Book* b = books.find(bookId);
        if (!m || !b) { cout << "Invalid member or book ID.\n"; return; }
        if (find(m->borrowed.begin(), m->borrowed.end(), bookId) != m->borrowed.end()) {
            cout << "Member already has this book.\n"; return;
        }
        if (m->borrowed.size() >= 3) { cout << "Limit reached: max 3 books per member.\n"; return; }
        if (b->available > 0) {
            borrow(m, b);
            undoStack.push({Op::ISSUE, memberId, bookId});
            cout << "Issued '" << b->title << "' to " << m->name << ".\n";
        } else {
            waitlist[bookId].push(memberId);
            cout << "No copies available. " << m->name << " added to the waiting list (position "
                 << waitlist[bookId].size() << ").\n";
        }
    }

    void returnBook(int memberId, int bookId) {
        Member* m = members.find(memberId);
        Book* b = books.find(bookId);
        if (!m || !b) { cout << "Invalid member or book ID.\n"; return; }
        if (find(m->borrowed.begin(), m->borrowed.end(), bookId) == m->borrowed.end()) {
            cout << "This member has not borrowed that book.\n"; return;
        }
        giveBack(m, b);
        undoStack.push({Op::RETURN, memberId, bookId});
        cout << m->name << " returned '" << b->title << "'.\n";
        // serve the next person in the waiting list, if any
        auto& q = waitlist[bookId];
        while (!q.empty()) {
            Member* next = members.find(q.front()); q.pop();
            if (next && next->borrowed.size() < 3) {
                borrow(next, b);
                cout << "Auto-issued to " << next->name << " from the waiting list.\n";
                break;
            }
        }
    }

    void undo() {
        if (undoStack.empty()) { cout << "Nothing to undo.\n"; return; }
        Action a = undoStack.top(); undoStack.pop();
        Member* m = members.find(a.memberId);
        Book* b = books.find(a.bookId);
        if (!m || !b) { cout << "Cannot undo: record missing.\n"; return; }
        if (a.op == Op::ISSUE) {
            giveBack(m, b);
            cout << "Undid issue of '" << b->title << "' to " << m->name << ".\n";
        } else if (b->available > 0) {
            borrow(m, b);
            cout << "Undid return of '" << b->title << "' by " << m->name << ".\n";
        } else {
            cout << "Cannot undo return: no copy free (it was re-issued).\n";
        }
    }

private:
    static void printBook(const Book& b) {
        cout << b.id << "  " << b.title << "  |  " << b.author << "  |  "
             << b.available << "/" << b.total << "\n";
    }
    void save() {
        ofstream bf(bookFile), mf(memberFile);
        for (auto* b : books.all())
            bf << b->id << '|' << b->title << '|' << b->author << '|' << b->total << '|' << b->available << '\n';
        for (auto* m : members.all()) {
            mf << m->id << '|' << m->name << '|';
            for (int id : m->borrowed) mf << id << ',';
            mf << '\n';
        }
    }
    void load() {
        ifstream bf(bookFile), mf(memberFile);
        string line;
        while (getline(bf, line)) {
            stringstream ss(line); string id, t, a, tot, av;
            getline(ss, id, '|'); getline(ss, t, '|'); getline(ss, a, '|');
            getline(ss, tot, '|'); getline(ss, av, '|');
            if (id.empty()) continue;
            books.insert({stoi(id), t, a, stoi(tot), stoi(av)});
        }
        while (getline(mf, line)) {
            stringstream ss(line); string id, name, list_;
            getline(ss, id, '|'); getline(ss, name, '|'); getline(ss, list_);
            if (id.empty()) continue;
            Member m{stoi(id), name, {}};
            stringstream ls(list_); string tok;
            while (getline(ls, tok, ',')) if (!tok.empty()) m.borrowed.push_back(stoi(tok));
            members.add(m);
        }
    }
};

// ---------------------------------------------------------------- CLI helpers
int readInt(const string& prompt) {
    string s;
    while (true) {
        cout << prompt; getline(cin, s);
        if (!cin) exit(0);
        try { return stoi(s); } catch (...) { cout << "Please enter a number.\n"; }
    }
}
string readLine(const string& prompt) {
    string s; cout << prompt; getline(cin, s); return s;
}

int main() {
    Library lib;
    while (true) {
        cout << "\n===== LIBRARY MANAGEMENT SYSTEM =====\n"
                " 1. Add book           2. Remove book\n"
                " 3. Search book        4. List all books\n"
                " 5. Register member    6. List members\n"
                " 7. Issue book         8. Return book\n"
                " 9. Undo last action   0. Save & exit\n";
        int ch = readInt("Choice: ");
        switch (ch) {
            case 1: { int id = readInt("Book ID: "); string t = readLine("Title: "), a = readLine("Author: ");
                      lib.addBook(id, t, a, readInt("Copies: ")); break; }
            case 2: lib.removeBook(readInt("Book ID: ")); break;
            case 3: lib.searchBook(readInt("Book ID: ")); break;
            case 4: lib.listBooks(); break;
            case 5: { int id = readInt("Member ID: "); lib.addMember(id, readLine("Name: ")); break; }
            case 6: lib.listMembers(); break;
            case 7: { int m = readInt("Member ID: "); lib.issue(m, readInt("Book ID: ")); break; }
            case 8: { int m = readInt("Member ID: "); lib.returnBook(m, readInt("Book ID: ")); break; }
            case 9: lib.undo(); break;
            case 0: cout << "Data saved. Goodbye!\n"; return 0;
            default: cout << "Invalid choice.\n";
        }
    }
}
