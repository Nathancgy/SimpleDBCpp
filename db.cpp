#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <filesystem>
namespace fs = std::filesystem;

// g++ -std=c++17 -o YourProgram test.cpp

using namespace std;

class Table {
    vector<string> columns;
    vector<map<string, string>> rows;

public:
    // Function to add a column
    void addColumn(const string& columnName) {
        columns.push_back(columnName);
    }

    // Function to add a row
    void addRow(const map<string, string>& rowData) {
        rows.push_back(rowData);
    }

    // Function to display the table
    void display() {
        if (columns.empty() || rows.empty()) {
            cout << "The table is empty." << endl;
            return;
        }

        // Display column names
        for (const auto& col : columns) {
            cout << col << "\t";
        }
        cout << endl;

        // Display rows
        for (const auto& row : rows) {
            for (const auto& col : columns) {
                auto it = row.find(col);
                if (it != row.end()) {
                    cout << it->second << "\t";
                } else {
                    cout << "N/A\t"; // Not available or missing data
                }
            }
            cout << endl;
        }
    }

    void deleteRow(const string& key, const string& value) {
        rows.erase(
            remove_if(rows.begin(), rows.end(),
                    [key, value](const auto& row) { return row.at(key) == value; }),
            rows.end());
    }

    // Function to update a row based on a key
    void updateRow(const string& key, const string& oldValue, const string& newValue) {
        for (auto& row : rows) {
            if (row[key] == oldValue) {
                row[key] = newValue;
            }
        }
    }
    const vector<string>& getColumns() const {
        return columns;
    }

    void saveToFile(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file for writing: " << filename << endl;
            return;
        }

        // Write column names
        for (const auto& col : columns) {
            file << col << ",";
        }
        file << "\n";

        // Write rows
        for (const auto& row : rows) {
            for (const auto& col : columns) {
                file << row.at(col) << ",";
            }
            file << "\n";
        }
        file.close();
    }

    // Function to load the table from a file
    void loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file for reading: " << filename << endl;
            return;
        }

        string line, cell;
        // Read column names
        if (getline(file, line)) {
            stringstream lineStream(line);
            while (getline(lineStream, cell, ',')) {
                columns.push_back(cell);
            }
        }

        // Read rows
        while (getline(file, line)) {
            stringstream lineStream(line);
            map<string, string> rowData;
            size_t colIndex = 0;
            while (getline(lineStream, cell, ',') && colIndex < columns.size()) {
                rowData[columns[colIndex]] = cell;
                colIndex++;
            }
            if (colIndex == columns.size()) { // Only add complete rows
                rows.push_back(rowData);
            }
        }
        file.close();
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

// Function to add a new table
void addTable() {
    ensureTablesDirectory();
    string tableName;
    cout << "Enter new table name: ";
    cin >> tableName;

    // Construct the file path for the new table
    fs::path tablePath = fs::path("tables") / (tableName + ".txt");

    // Create and immediately close the file to ensure it exists
    ofstream(tablePath).close();
    cout << "Table " << tableName << " added.\n";
}

// Function to open an existing table
void openTable(Table& table) {
    ensureTablesDirectory();
    string tableName;
    cout << "Enter table name to open: ";
    cin >> tableName;

    // Construct the file path for the table
    fs::path tablePath = fs::path("tables") / (tableName + ".txt");

    if (fs::exists(tablePath)) {
        table.loadFromFile(tablePath.string());
        manageTable(table); // Call manageTable to perform operations on the opened table
        table.saveToFile(tablePath.string());
    } else {
        cout << "Cannot find table " << tableName << ".\n";
    }
}

// Function to delete an existing table
void deleteTable() {
    ensureTablesDirectory();
    string tableName;
    cout << "Enter table name to delete: ";
    cin >> tableName;

    // Construct the file path for the table
    fs::path tablePath = fs::path("tables") / (tableName + ".txt");

    if (fs::exists(tablePath)) {
        fs::remove(tablePath);
        cout << "Table " << tableName << " deleted.\n";
    } else {
        cout << "Cannot find table " << tableName << ".\n";
    }
}

void manageTable(Table& table) {
    int choice;
    do {
        cout << "\nTable Management\n";
        cout << "1. Add Column\n";
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
                string columnName;
                cout << "Enter column name: ";
                cin >> columnName;
                table.addColumn(columnName);
                break;
            }
            case 2: {
                map<string, string> rowData;
                string value;
                for (const auto& col : table.getColumns()) {
                    cout << "Enter value for " << col << ": ";
                    cin >> value;
                    rowData[col] = value;
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
                return;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 6);
}

void addContact() {
    ofstream file;
    file.open("contacts.txt", ios::app); // Open in append mode

    if (!file) {
        cerr << "Error in opening file!" << endl;
        return;
    }

    string name, phone;
    cout << "Enter name: ";
    cin.ignore();  // To flush the newline character out of the buffer
    getline(cin, name);
    cout << "Enter phone number: ";
    getline(cin, phone);

    file << name << "," << phone << endl; // Write to file

    file.close();
    cout << "Contact added successfully!\n";
}

void viewContacts() {
    ifstream file;
    file.open("contacts.txt");

    if (!file) {
        cerr << "Error in opening file!" << endl;
        return;
    }

    string line;
    cout << "Contacts:\n";
    while (getline(file, line)) {
        size_t pos = line.find(',');
        string name = line.substr(0, pos);
        string phone = line.substr(pos + 1);

        cout << "Name: " << name << ", Phone: " << phone << endl;
    }

    file.close();
}
void deleteContact() {
    ifstream file;
    file.open("contacts.txt");

    if (!file) {
        cerr << "Error in opening file!" << endl;
        return;
    }

    string line, nameToDelete;
    vector<string> contacts;
    cout << "Enter the name of the contact to delete: ";
    cin.ignore(); // To flush the newline character out of the buffer
    getline(cin, nameToDelete);

    bool found = false;
    while (getline(file, line)) {
        if (line.substr(0, line.find(',')) != nameToDelete) {
            contacts.push_back(line);
            break;
        } else {
            found = true;
            break;
        }
    }

    file.close();

    if (found) {
        ofstream outFile;
        outFile.open("contacts.txt", ios::trunc); // Open in truncate mode

        for (const auto& contact : contacts) {
            outFile << contact << endl;
        }

        outFile.close();
        cout << "Contact deleted successfully!\n";
    } else {
        cout << "Contact not found!\n";
    }
}

int main() {
    int choice;
    Table table;
    ensureTablesDirectory();
    do {
        cout << "\nContact Management System\n";
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
            // ... [Existing cases]
            case 4:
                addTable();
                break;
            case 5:
                openTable(table);
                break;
            case 6:
                deleteTable();
                break;
            case 7:
                cout << "Exiting...\n";
                break;
            // ... [Rest of the switch cases]
        }
    } while (choice != 7);
    return 0;
}
