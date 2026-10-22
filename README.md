# Library Management System (C++)

**Course:** Data Structures | **Program:** B.Tech CSE, 3rd Semester

A menu-driven console application that manages a library's books, members, and
issue/return operations. The project demonstrates how core data structures are
applied to a real problem.

## Data structures used

| Feature | Data structure | Why |
|---|---|---|
| Book catalogue | Binary Search Tree | Fast search/insert/delete by Book ID; inorder traversal gives sorted listing |
| Member records | Hash table (separate chaining) | O(1) average lookup by Member ID |
| Waiting list | Queue (FIFO) | First member to wait gets the book first |
| Undo | Stack (LIFO) | Reverses the most recent issue/return |
| Persistence | File I/O | Data is saved to `books.txt` and `members.txt` |

## Features
- Add, remove, search, and list books
- Register members and list them with borrowed books
- Issue / return books (max 3 per member)
- Automatic waiting list; returned book is auto-issued to the next person
- Undo last action
- Data saved on exit and reloaded at start

## Build and run
```
make        # compile
./library   # run
make clean  # remove binary and data files
```
Requires `g++` with C++17 support.

## Complexity
| Operation | Time |
|---|---|
| Book search/insert/delete | O(h), O(log n) average |
| Member lookup | O(1) average |
| Issue / return | O(log n) |
| Undo | O(1) |

## Possible extensions
Balanced tree (AVL), due dates and fines, GUI front end, database storage.
