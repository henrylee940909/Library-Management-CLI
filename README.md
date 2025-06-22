# Library‑Management‑CLI

A fully featured **C++17** console library‑management system, developed for NYCU **OOPDS** (HW2). The program provides multi‑role authentication, comprehensive book/loan workflows, advanced Boolean search, collaborative‑filter recommendations, rich console UI, and ASCII statistics.

---

## Features

| Category                        | Key Features                                                                                                                                                             |
| ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Account & Security**          | • Multi‑role login (Admin / Staff / Reader)<br>• Salted SHA‑256 password hashing<br>• First‑run admin setup & password change                                            |
| **Book Management**             | • Add / edit / delete books with full field validation (ISBN, language, pages, multi‑category)<br>• Pagination (20 items / page) & sorting by title, author, year, pages |
| **Advanced Search**             | • Boolean query parser (AND / OR / NOT, parentheses)<br>• Inverted index for fast multi‑field lookup                                                                     |
| **Loan Workflow**               | • Borrow / return with user, due date, copy tracking<br>• Automatic overdue detection & fine calculation                                                                 |
| **Statistics & Recommendation** | • ASCII bar / pie / line charts (top books, active users, category ratio, monthly trends)<br>• Hybrid recommendation: collaborative filtering + content similarity       |
| **User Experience**             | • Colourised ANSI console UI (titles, menus, progress bars, alerts)<br>• Robust error handling & sensible defaults                                                       |
| **Persistence**                 | • All data stored in JSON (`books.json`, `users.json`, `loans.json`)<br>• Auto‑save / load on exit & launch                                                              |

---

## Dependencies

> **Compiler:** g++ 10 (or clang++ 11) or newer, with `-std=c++17`<br>
> **External library:** single-header **nlohmann/json.hpp** — already included in *include/*, so no extra install is required.

---

## Build & Run

```bash
# Clone the repository
git clone <repository-url>
cd DS-HW2

# Create necessary directories (automatically created by Makefile)
mkdir -p obj bin data

# Build the project
make

# Run the program
make run

# Clean build artifacts
make clean
```

> First launch creates an **Admin** account if `data/users.json` is empty.

---

## Sample Data

The **data/** folder ships with ready‑made JSON datasets so the program boots with meaningful content.

| File           | Purpose                                             |
| -------------- | --------------------------------------------------- |
| **books.json** | 150 sample books across multiple categories         |
| **users.json** | 3 built‑in accounts: admin / staff / reader         |
| **loans.json** | Historical loan records used by statistics & charts |

> **Reset tip** — Delete `data/users.json` and restart to trigger first‑run *Admin* setup if you need new credentials.

---

## File Structure

```
Library-Management-CLI/
├── include/             # Header files
├── src/                 # Source files (.cpp)
├── data/                # Sample JSON datasets
├── docs/                # Project report (PDF)
│   └── report.pdf
├── Makefile             # Build script
└── README.md            # Project overview and usage
```

---

## Report

* [HW2 Project Report (PDF)](docs/report.pdf)

---

## Author

| Name (中文 / EN)  | Affiliation / ID   | Course              |
| --------------- | ------------------ | ------------------- |
| 李品翰 (Henry Lee) | NYCU CS, 113550193 | OOPDS (Spring 2025) |

---

*This repository contains academic homework code and documentation for learning purposes.*
