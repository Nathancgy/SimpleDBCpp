#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <filesystem>
#include <sqlite3.h>
namespace fs = std::filesystem;

// g++ -std=c++17 -o YourProgram test.cpp

using namespace std;

class Table {
    string tableName;
    sqlite3 *db;
    vector<string> columns;

public:
    Table(const string& name, sqlite3* database) : tableName(name), db(database) {
        if (tableName.empty()) {
            cerr << "Table name is empty." << endl;
            return;
        }
        string sql = "PRAGMA table_info(" + tableName + ");";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string colName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)); // Column name is in the second column
                columns.push_back(colName);
            }
            sqlite3_finalize(stmt);
        } else {
            cerr << "SQL error in table_info: " << sqlite3_errmsg(db) << endl;
        }
    }
    // Function to add a column
    void addColumn(const string& columnName) {
        columns.push_back(columnName);
    }

    // Function to add a row
    void addRow(const map<string, string>& rowData) {
        if (columns.empty()) {
            cerr << "Error: No columns defined for the table." << endl;
            return;
        }

        string sql = "INSERT INTO " + tableName + " (";
        for (const auto& col : columns) {
            if (col != "id") { // Skip the 'id' column
                sql += col + ", ";
            }
        }
        if (sql.back() == ' ') {
            sql.pop_back(); // Remove last space
            sql.pop_back(); // Remove last comma
        }
        sql += ") VALUES (";
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i] != "id") {
                sql += "?, ";
            }
        }
        if (sql.back() == ' ') {
            sql.pop_back(); // Remove last space
            sql.pop_back(); // Remove last comma
        }
        sql += ");";

        cout << "Debug SQL: " << sql << endl;

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
            return;
        }

        int bindIndex = 1;
        for (const auto& col : columns) {
            if (col != "id") {
                sqlite3_bind_text(stmt, bindIndex++, rowData.at(col).c_str(), -1, SQLITE_TRANSIENT);
            }
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "SQL error in step: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);
    }

    // Function to display the table
    void display() {
        string sql = "SELECT * FROM " + tableName;
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
            return;
        }

        // Display column names
        for (const auto& col : columns) {
            cout << col << "\t";
        }
        cout << endl;

        // Display rows
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
                cout << string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i))) << "\t";
            }
            cout << endl;
        }

        sqlite3_finalize(stmt);
    }

    void deleteRow(const string& key, const string& value) {
        string sql = "DELETE FROM " + tableName + " WHERE " + key + " = ?";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "SQL error in step: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);
    }
    // Function to update a row based on a key
    void updateRow(const string& key, const string& oldValue, const string& newValue) {
        string sql = "UPDATE " + tableName + " SET " + key + " = ? WHERE " + key + " = ?";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, newValue.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, oldValue.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "SQL error in step: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);
    }
    const vector<string>& getColumns() const {
        return columns;
    }

    // More functions (like deleteRow, updateRow, etc.) can be added here
};

void manageTable(Table& table);

void ensureTablesDirectory() {
    const fs::path tablesDir{"tables"};
    if (!fs::exists(tablesDir)) {
        fs::create_directory(tablesDir);
    }
}

void addTable(sqlite3 *db) {
    string tableName;
    cout << "Enter new table name: ";
    cin >> tableName;

    string sql = "CREATE TABLE IF NOT EXISTS " + tableName + " (id INTEGER PRIMARY KEY AUTOINCREMENT, event TEXT, date TEXT);";
 // Define your columns here
    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg) != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Table " << tableName << " created successfully.\n";
    }
}

// Function to open an existing table
void openTable(sqlite3 *db, Table& table) {
    string tableName;
    cout << "Enter table name to open: ";
    cin >> tableName;

    // Here, simply prepare the Table object with the given table name and database connection
    table = Table(tableName, db);
    manageTable(table);
}

// Function to delete an existing table
void deleteTable(sqlite3 *db) {
    string tableName;
    cout << "Enter table name to delete: ";
    cin >> tableName;

    string sql = "DROP TABLE IF EXISTS " + tableName;
    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg) != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Table " << tableName << " deleted successfully.\n";
    }
}

void manageTable(Table& table) {
    int choice;
    do {
        cout << "\nTable Management\n";
        cout << "1. Add Column\n";  // Note: SQLite does not support adding columns to existing tables easily
        cout << "2. Add Row\n";
        cout << "3. Delete Row\n";
        cout << "4. Update Row\n";
        cout << "5. Display Table\n";
        cout << "6. Return to Main Menu\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear(); // Clear error flags
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore the rest of the line
            continue; // Skip the rest of the loop iteration
        }

        switch (choice) {
            case 1: {
                // SQLite does not support adding columns to existing tables easily.
                // If you need this functionality, consider designing your database schema to be more flexible
                // or recreate the table with the new column.
                cout << "Adding columns is not supported in this version.\n";
                break;
            }
            case 2: {
                map<string, string> rowData;
                string value;
                for (const auto& col : table.getColumns()) {
                    if (col != "id") {
                        cout << "Enter value for " << col << ": ";
                        cin >> value;
                        rowData[col] = value;
                    }
                }
                table.addRow(rowData);
                break;
            }
            case 3: {
                string key, value;
                cout << "Enter key to delete row: ";
                cin >> key;
                cout << "Enter value of key to delete row: ";
                cin >> value;
                table.deleteRow(key, value);
                break;
            }
            case 4: {
                string key, oldValue, newValue;
                cout << "Enter key to update row: ";
                cin >> key;
                cout << "Enter old value of key: ";
                cin >> oldValue;
                cout << "Enter new value of key: ";
                cin >> newValue;
                table.updateRow(key, oldValue, newValue);
                break;
            }
            case 5:
                table.display();
                break;
            case 6:
                return; // Exit the manageTable function
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 6);
}


void addContact(sqlite3 *db) {
    string name, phone;
    cout << "Enter name: ";
    cin.ignore();  // To flush the newline character out of the buffer
    getline(cin, name);
    cout << "Enter phone number: ";
    getline(cin, phone);

    string sql = "INSERT INTO contacts (name, phone) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, phone.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "SQL error in step: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
    cout << "Contact added successfully!\n";
}


void viewContacts(sqlite3 *db) {
    string sql = "SELECT name, phone FROM contacts;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << "Contacts:\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        cout << "Name: " << name << ", Phone: " << phone << endl;
    }

    sqlite3_finalize(stmt);
}

void deleteContact(sqlite3 *db) {
    string nameToDelete;
    cout << "Enter the name of the contact to delete: ";
    cin.ignore(); // To flush the newline character out of the buffer
    getline(cin, nameToDelete);

    string sql = "DELETE FROM contacts WHERE name = ?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, nameToDelete.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "SQL error in step: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Contact deleted successfully!\n";
    }

    sqlite3_finalize(stmt);
}

void createContactsTable(sqlite3* db) {
    string sql = "CREATE TABLE IF NOT EXISTS contacts (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, phone TEXT);";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg) != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Contacts table created successfully.\n";
    }
}

int main() {
    sqlite3 *db;
    if (sqlite3_open("database.db", &db)) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }
    createContactsTable(db); 
    Table table("", db);
    int choice;

    ensureTablesDirectory();
    do {
        cout << "\nDirections\n";
        cout << "1. Add Contact\n";
        cout << "2. View Contacts\n";
        cout << "3. Delete Contact\n";
        cout << "4. Add Table\n";
        cout << "5. Open Table\n";
        cout << "6. Delete Table\n";
        cout << "7. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        if (cin.fail()) {
            cin.clear(); // Clears any error flags.
            // Ignore any input in the buffer up to the newline character or a large number of characters.
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            cout << "Invalid input. Please enter a number.\n";
            continue; // Skip the rest of the loop iteration.
        }

        switch (choice) {
            case 1:
                addContact(db);
                break;
            case 2:
                viewContacts(db);
                break;
            case 3:
                deleteContact(db);
                break;
            case 4:
                addTable(db);
                break;
            case 5:
                openTable(db, table);
                break;
            case 6:
                deleteTable(db);
                break;
            case 7:
                cout << "Exiting...\n";
                break;
            // ... [Rest of the switch cases]
        }
    } while (choice != 7);
    sqlite3_close(db);
    return 0;
}
