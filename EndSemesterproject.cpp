#include "BankSystem.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

BankSystem::BankSystem() : lastAccountNumber(1000) {
    cout << "Initializing Bank System..." << endl;

    try {
        loadAccounts();

        if (accounts.empty()) {
            cout << "No accounts found. Starting with empty database..." << endl;
        }
        else {
            cout << "Bank System ready with " << accounts.size() << " accounts" << endl;
        }
    }
    catch (const exception& e) {
        cerr << "Error during initialization: " << e.what() << endl;
    }
}

BankSystem::~BankSystem() {
    cout << "Saving data before exit..." << endl;
    try {
        saveAccounts();
    }
    catch (const exception& e) {
        cerr << "Error saving accounts: " << e.what() << endl;
    }
}

int BankSystem::generateAccountNumber() {
    return ++lastAccountNumber;
}



int BankSystem::createAccount(const string& name, const string& phone,
    const string& email, const string& address,
    char type, unsigned long initialDeposit) {
    type = toupper(type);

    unsigned long minDeposit = (type == 'S') ? 500 : 1000;
    if (initialDeposit < minDeposit) {
        return -1;
    }

    int accNo = generateAccountNumber();
    Account newAccount(accNo, name, phone, email, address, type, initialDeposit);
    accounts.push_back(newAccount);
    saveAccounts();

    cout << "Account created: " << accNo << endl;

    return accNo;
}

Account* BankSystem::findAccount(int accountNumber) {
    for (size_t i = 0; i < accounts.size(); i++) {
        if (accounts[i].getAccountNumber() == accountNumber) {
            return &accounts[i];
        }
    }
    return nullptr;
}

bool BankSystem::modifyAccount(int accountNumber, const string& newName, char newType) {
    for (size_t i = 0; i < accounts.size(); i++) {
        if (accounts[i].getAccountNumber() == accountNumber) {
            accounts[i].setName(newName);
            accounts[i].setType(newType);
            saveAccounts();
            return true;
        }
    }
    return false;
}

bool BankSystem::deleteAccount(int accountNumber) {
    auto it = remove_if(accounts.begin(), accounts.end(),
        [accountNumber](const Account& acc) {
            return acc.getAccountNumber() == accountNumber;
        });

    if (it != accounts.end()) {
        accounts.erase(it, accounts.end());
        saveAccounts();
        return true;
    }

    return false;
}

vector<Account> BankSystem::getAllAccounts() const {
    return accounts;
}

bool BankSystem::deposit(int accountNumber, unsigned long amount) {
    for (size_t i = 0; i < accounts.size(); i++) {
        if (accounts[i].getAccountNumber() == accountNumber) {
            accounts[i].deposit(amount);
            saveAccounts();
            cout << "Deposited Rs." << amount << " to account " << accountNumber << endl;
            cout << "New balance: Rs." << accounts[i].getBalance() << endl;
            cout << "Transactions: " << accounts[i].getTransactions().size() << endl;
            return true;
        }
    }
    cerr << "Account " << accountNumber << " not found for deposit" << endl;
    return false;
}

bool BankSystem::withdraw(int accountNumber, unsigned long amount) {
    for (size_t i = 0; i < accounts.size(); i++) {
        if (accounts[i].getAccountNumber() == accountNumber) {
            if (accounts[i].withdraw(amount)) {
                saveAccounts();
                cout << "Withdrew Rs." << amount << " from account " << accountNumber << endl;
                cout << "New balance: Rs." << accounts[i].getBalance() << endl;
                cout << "Transactions: " << accounts[i].getTransactions().size() << endl;
                return true;
            }
            cerr << "Insufficient balance for withdrawal" << endl;
            return false;
        }
    }
    cerr << "Account " << accountNumber << " not found for withdrawal" << endl;
    return false;
}

void BankSystem::calculateAllInterests() {
    int count = 0;
    for (size_t i = 0; i < accounts.size(); i++) {
        if (accounts[i].getType() == 'S' || accounts[i].getType() == 's') {
            accounts[i].calculateInterest();
            count++;
        }
    }
    saveAccounts();
    cout << "Interest calculated for " << count << " savings accounts" << endl;
}

string BankSystem::getTransactionHistory(int accountNumber) {
    for (size_t i = 0; i < accounts.size(); i++) {
        if (accounts[i].getAccountNumber() == accountNumber) {
            string history = accounts[i].getTransactionHistory();
            cout << "Retrieved transaction history for account " << accountNumber << endl;
            cout << "Total transactions: " << accounts[i].getTransactions().size() << endl;
            return history;
        }
    }
    return "Account not found.";
}

void BankSystem::saveAccounts() {
    try {
        ofstream file(filename);

        if (!file.is_open()) {
            cerr << "Error: Could not open file for saving: " << filename << endl;
            return;
        }

        file << lastAccountNumber << "\n";
        file << accounts.size() << "\n";

        for (size_t i = 0; i < accounts.size(); i++) {
            const Account& acc = accounts[i];

            file << acc.getAccountNumber() << "|"
                << acc.getName() << "|"
                << acc.getPhone() << "|"
                << acc.getEmail() << "|"
                << acc.getAddress() << "|"
                << acc.getType() << "|"
                << acc.getBalance() << "|"
                << acc.getCreationDate() << "\n";

            auto transactions = acc.getTransactions();
            file << transactions.size() << "\n";

            for (size_t j = 0; j < transactions.size(); j++) {
                const Transaction& trans = transactions[j];
                file << trans.timestamp << "|"
                    << trans.type << "|"
                    << trans.amount << "|"
                    << trans.balanceAfter << "\n";
            }
        }

        file.close();
        cout << "Saved " << accounts.size() << " accounts to database" << endl;
    }
    catch (const exception& e) {
        cerr << "Error saving database: " << e.what() << endl;
    }
}

void BankSystem::loadAccounts() {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "No existing database found." << endl;
        return;
    }

    try {
        accounts.clear();

        file >> lastAccountNumber;
        int count;
        file >> count;
        file.ignore();

        cout << "Loading " << count << " accounts from database..." << endl;

        for (int i = 0; i < count; i++) {
            try {
                string line;
                if (!getline(file, line)) {
                    break;
                }

                stringstream ss(line);
                string token;
                vector<string> fields;

                while (getline(ss, token, '|')) {
                    fields.push_back(token);
                }

                if (fields.size() >= 7) {
                    int accNo = stoi(fields[0]);
                    string name = fields[1];
                    string phone = fields[2];
                    string email = fields[3];
                    string address = fields[4];
                    char type = fields[5][0];
                    unsigned long balance = stoul(fields[6]);

                    Account acc(accNo, name, phone, email, address, type, 0);

                    if (fields.size() >= 8) {
                        try {
                            time_t creationDate = stol(fields[7]);
                            acc.setCreationDate(creationDate);
                            acc.setLastInterestDate(creationDate);
                        }
                        catch (...) {}
                    }

                    acc.clearTransactions();

                    string transCountLine;
                    bool transLoaded = false;

                    if (getline(file, transCountLine)) {
                        try {
                            int transCount = stoi(transCountLine);
                            int successfullyLoaded = 0;

                            for (int j = 0; j < transCount; j++) {
                                string transLine;
                                if (getline(file, transLine)) {
                                    try {
                                        stringstream tss(transLine);
                                        vector<string> transFields;
                                        string ttoken;

                                        while (getline(tss, ttoken, '|')) {
                                            transFields.push_back(ttoken);
                                        }

                                        if (transFields.size() >= 4) {
                                            time_t timestamp = stol(transFields[0]);
                                            string transType = transFields[1];
                                            unsigned long amount = stoul(transFields[2]);
                                            unsigned long balAfter = stoul(transFields[3]);

                                            acc.addTransactionDirect(timestamp, transType, amount, balAfter);
                                            successfullyLoaded++;
                                        }
                                    }
                                    catch (...) {}
                                }
                            }

                            if (successfullyLoaded > 0) {
                                transLoaded = true;
                            }

                        }
                        catch (...) {}
                    }

                    if (!transLoaded && balance > 0) {
                        acc.addTransaction("OPENING", balance);
                    }

                    acc.setBalance(balance);
                    accounts.push_back(acc);

                }
                else {
                    string dummy;
                    if (getline(file, dummy)) {
                        try {
                            int transCount = stoi(dummy);
                            for (int j = 0; j < transCount; j++) {
                                getline(file, dummy);
                            }
                        }
                        catch (...) {}
                    }
                }

            }
            catch (...) {
                string dummy;
                if (getline(file, dummy)) {
                    try {
                        int transCount = stoi(dummy);
                        for (int j = 0; j < transCount; j++) {
                            getline(file, dummy);
                        }
                    }
                    catch (...) {}
                }
                continue;
            }
        }

        file.close();
        cout << "Successfully loaded " << accounts.size() << " accounts!" << endl;

    }
    catch (const exception& e) {
        cerr << "Error loading database: " << e.what() << endl;
        file.close();
    }
}
