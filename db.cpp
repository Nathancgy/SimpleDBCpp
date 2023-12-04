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
    
    const string& getName() const {
        return tableName;
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

    void refreshColumns() {
        columns.clear();
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

};

void manageTable(Table&, sqlite3*);

void ensureTablesDirectory() {
    const fs::path tablesDir{"tables"};
    if (!fs::exists(tablesDir)) {
        fs::create_directory(tablesDir);
    }
}

bool tableExists(sqlite3 *db, const string& tableName) {
    string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    bool exists = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return exists;
}

void addTable(sqlite3 *db) {
    string tableName, columnName, sql;
    vector<string> columns;

    cout << "Enter new table name: ";
    cin >> tableName;

    cout << "Enter column names (type 'done' to finish): \n";
    while (true) {
        cout << "Column" << columns.size() + 1 << ": ";
        cin >> columnName;

        if (columnName == "done") {
            break;
        }

        // Assuming all columns are of type TEXT for simplicity
        // Modify this part if you need different data types
        columns.push_back(columnName + " TEXT");
    }

    if (columns.empty()) {
        cout << "No columns specified. Table creation aborted.\n";
        return;
    }

    sql = "CREATE TABLE IF NOT EXISTS " + tableName + " (";
    for (const auto& col : columns) {
        sql += col + ", ";
    }
    sql.pop_back(); // Remove last space
    sql.pop_back(); // Remove last comma
    sql += ");";

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

    if (!tableExists(db, tableName)) {
        cout << "Table " << tableName << " does not exist." << endl;
        return;
    }

    table = Table(tableName, db);
    cout << "Opened table " << tableName << endl;
    manageTable(table, db);
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

void addColumnToTable(sqlite3 *db, Table& table, const string& tableName) {
    string newColumnName;

    cout << "Enter the name of the new column: ";
    cin >> newColumnName;

    // Assuming the new column is of type TEXT for simplicity
    string sql = "ALTER TABLE " + tableName + " ADD COLUMN " + newColumnName + " TEXT;";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg) != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Column " << newColumnName << " added to table " << table.getName() << " successfully.\n";
        table.refreshColumns(); // Refresh the columns after adding a new one

        sql = "SELECT id FROM " + table.getName();
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                string newValue;
                cout << "Enter value for " << newColumnName << " for Row " << id << ": ";
                cin >> newValue; // Assuming a single word input. For multi-word input use getline(cin, newValue);

                // Update the row with the new value for the new column
                string updateSql = "UPDATE " + table.getName() + " SET " + newColumnName + " = ? WHERE id = ?";
                sqlite3_stmt *updateStmt;
                if (sqlite3_prepare_v2(db, updateSql.c_str(), -1, &updateStmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(updateStmt, 1, newValue.c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(updateStmt, 2, id);
                    if (sqlite3_step(updateStmt) != SQLITE_DONE) {
                        cerr << "SQL error in updating row: " << sqlite3_errmsg(db) << endl;
                    }
                    sqlite3_finalize(updateStmt);
                } else {
                    cerr << "SQL error in prepare: " << sqlite3_errmsg(db) << endl;
                }
            }
            sqlite3_finalize(stmt);
        } else {
            cerr << "SQL error in SELECT ID: " << sqlite3_errmsg(db) << endl;
        }
    }
}

void manageTable(Table& table, sqlite3* db) {
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
                addColumnToTable(db, table, table.getName());
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

void contactOperations(sqlite3 *db) {
    int contactChoice;
    do {
        cout << "\nContact Operations\n";
        cout << "1. Add Contact\n";
        cout << "2. View Contacts\n";
        cout << "3. Delete Contact\n";
        cout << "4. Return to Main Menu\n";
        cout << "Enter your choice: ";
        cin >> contactChoice;

        switch (contactChoice) {
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
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (contactChoice != 4);
}

int main() {
    sqlite3 *db;
    if (sqlite3_open("database.db", &db)) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }
    createContactsTable(db); 
    Table table("", db);
    int mainChoice, tableChoice;

    ensureTablesDirectory();
    do {
        cout << "\nMain Menu\n";
        cout << "1. Contact Operations\n";
        cout << "2. Table Operations\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        cin >> mainChoice;

        if (cin.fail()) {
            cin.clear(); 
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (mainChoice) {
            case 1:
                contactOperations(db);
                break;
            case 2:
                do {
                    cout << "\nTable Operations\n";
                    cout << "1. Add Table\n";
                    cout << "2. Open Table\n";
                    cout << "3. Delete Table\n";
                    cout << "4. Return to Main Menu\n";
                    cout << "Enter your choice: ";
                    cin >> tableChoice;

                    switch (tableChoice) {
                        case 1:
                            addTable(db);
                            break;
                        case 2:
                            openTable(db, table);
                            break;
                        case 3:
                            deleteTable(db);
                            break;
                        case 4:
                            break;
                        default:
                            cout << "Invalid choice. Please try again.\n";
                    }
                } while (tableChoice != 4);
                break;
            case 3:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (mainChoice != 3);

    sqlite3_close(db);
    return 0;
}
