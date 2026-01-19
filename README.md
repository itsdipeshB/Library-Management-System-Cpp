# Library Management System (C++)

A simple, console-based **Library Management System** written in **C++ (C++17)**.  
It supports basic features commonly required for a college library project:

- Manage **Books** (Add / List / Search)
- Manage **Members** (Add / List / Search)
- **Issue** books to members (with due date)
- **Return** books
- View all **Issue Records**
- **File-based storage** (data persists after closing the program)

---

## Features

### Books
- Add new books (title, author, total copies)
- List all books
- Search books by keyword (title/author)
- Tracks availability automatically (Available/Total)

### Members
- Add new members
- List all members
- Search members by keyword (name)

### Issue / Return
- Issue a book if copies are available
- Due date is automatically set to **14 days**
- Return a book using the issue **Record ID**
- Shows late status (late days) based on due day

### Data Persistence
The program automatically saves/loads data using 3 text files:
- `books.txt`
- `members.txt`
- `issues.txt`

---

## Project Structure

