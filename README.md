# SimpleDBCpp 📖

A C++ application for managing contacts and custom tables, leveraging SQLite for efficient data storage and retrieval.

## How to Run 🚀

1. **Compile the Program**:
   Make sure SQLite is installed and linked. Compile with the following command:
   
   ```bash
   g++ -std=c++17 -o YourProgram db.cpp -lsqlite3
   ```
2. **Execute the Binary**:
   ```bash
   ./YourProgram
   ```

## System Requirements 📚
- C++17 Compiler (e.g., GCC, Clang)
- SQLite3
- Standard C++ Libraries

## Features Breakdown 📋
1. **Add Contact**: Prompt for name and phone number, then store in the `contacts` table.
2. **View Contacts**: Display all stored contacts.
3. **Delete Contact**: Remove a contact based on the provided name.
4. **Add Table**: Create a new custom table with specified columns.
5. **Open Table**: Interact with an existing table to add, view, or delete rows.
6. **Delete Table**: Remove an existing table and its data from the database.
7. **Exit**: Close the application.

## Table Management 🛠️

- Create and manage custom tables dynamically.
- Add, update, and delete records with ease.
- View table contents in a neatly formatted console output.

## Demonstration 👀
<img src="https://github.com/Nathancgy/SimpleDBCpp/blob/main/img/demo1.png?raw=true" alt="alt text" width="30%">
<img src="https://github.com/Nathancgy/SimpleDBCpp/blob/main/img/demo2.png?raw=true" alt="alt text" width="30%">
<img src="https://github.com/Nathancgy/SimpleDBCpp/blob/main/img/demo3.png?raw=true" alt="alt text" width="30%">
<img src="https://github.com/Nathancgy/SimpleDBCpp/blob/main/img/demo4.png?raw=true" alt="alt text" width="30%">
<img src="https://github.com/Nathancgy/SimpleDBCpp/blob/main/img/demo5.png?raw=true" alt="alt text" width="30%">

---

## Contributions 👏
Feel free to fork, modify, and send pull requests to contribute to this project!

## License 📜
This project is open-sourced under the [MIT License](LICENSE).

## Contact 📫
For queries and suggestions, feel free to open an issue.

Happy Coding! 💻
