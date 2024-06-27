#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <regex>

using namespace std;
using namespace chrono;


class Validation {
public:
    Validation() {}

    bool isValidName(const string& s) {
        for (char ch : s) {
            if (!isalpha(ch)) return false; // Проверяем, что все символы являются буквами
        }
        return true;
    }
    void promptForName(string& name) {
        cout << "Enter customer name: ";
        cin >> name;
        while (!isValidName(name)) {
            cout << "Invalid input. Please enter a valid name (letters only): ";
            cin >> name;
        }
    }
    void promptForBalance(double& balance) {
        cout << "Enter balance: ";
        while (!(cin >> balance)) {
            cout << "Invalid input. Please enter a numeric value: ";
            cin.clear(); // сбросить флаг ошибки
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // удалить неверные вводы из входного буфера
        }
    }
    bool isValidDate(const string& date) {
        regex datePattern(R"(^\d{2}/\d{2}/\d{4}$)"); // Регулярное выражение для формата даты DD/MM/YYYY
        return regex_match(date, datePattern);
    }
    void promptForDate(string& date, const string& prompt) {
        cout << prompt;
        cin >> date;
        while (!isValidDate(date)) {
            cout << "Invalid date format. Please enter date in format dd/mm/yyyy: ";
            cin >> date;
        }
    }
    void promptForRoomNumber(int& roomNumber) {
        cout << "Enter room number: ";
        while (!(cin >> roomNumber) && roomNumber <= 100) {

            cout << "Invalid input. Please enter a valid room number (integer): ";
            cin.clear(); // сбросить флаг ошибки
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // удалить неверные вводы из входного буфера
        }
    }


};

class Customer {
private:
    static int id;
    int roomNumber = 0;
    int customerId;
    string name;
    string checkInDate;
    string checkOutDate;
public:
    Customer(string n, string cid, string cod)
        : customerId(id++), name(n), checkInDate(cid), checkOutDate(cod) {}

    void setRoom(int n) {
        roomNumber = n;
    }
    string getName() const {
        return name;
    }
    string getCheckInDate() const {
        return checkInDate;
    }
    string getCheckOutDate() const {
        return checkOutDate;
    }

    int getId() const {
        return customerId;
    }


    void displayCustomerInfo() const {
        cout << "Customer ID: " << customerId << endl;
        cout << "Name: " << name << endl;
        cout << "Check-in Date: " << checkInDate << endl;
        cout << "Check-out Date: " << checkOutDate << endl;
    }
};

int Customer::id = 1;

class Room {
private:
    static int capacity;
    int roomNumber;
    bool available;
    int debit = 0; //his balance
    int credit = 0; // outr bill


public:
    Customer* customer; // Using pointer for customer
    Room() : available(true), customer(nullptr) {
        roomNumber = capacity;
        capacity++;
    }


    int getNumber() const {
        return roomNumber;
    }
    void setAvailability(bool avail) { available = avail; }
    bool isAvailable() const { return available; }

    void assignCustomer(Customer* cust, double balance) {
        if (customer) {
            delete customer; // Clean up existing customer
        }
        customer = cust;
        setDebit(balance);
        customer->setRoom(roomNumber);

        setAvailability(false);
    }

    void setDebit(double amount) {
        this->debit = amount;
    }
    void setCredit(double amount) {
        this->credit = amount;
    }

    int getDebit() const {
        return debit;
    }
    void chargeBill(double amount) {

        ofstream file("bills.db", ios::app);
        if (file.is_open()) {
            file << "Room Number: " << roomNumber << "\n"
                << "Check-In Date: " << this->customer->getCheckInDate() << "\n"
                << "Check-Out Date: " << this->customer->getCheckOutDate() << "\n"
                << "Amount: $" << amount << "\n\n";
            file.close();
            this->credit += amount;
            this->debit -= amount;
            updateBalanceInFile(roomNumber, this->debit);

        }
        else {
            cerr << "Unable to open file bills.db";
        }
    }

    void updateBalanceInFile(int roomNumber, double newBalance) {
        ifstream inFile("customers.db");
        if (!inFile) {
            cerr << "Unable to open file: " << "customers.db" << endl;
            return;
        }

        vector<string> lines;
        string line;
        bool foundRoom = false;

        while (getline(inFile, line)) {
            if (line.find("Room number:") != string::npos) {
                int currentRoomNumber = stoi(line.substr(line.find(":") + 2));
                foundRoom = (currentRoomNumber == roomNumber);
            }

            if (foundRoom && line.find("Balance:") != string::npos) {
                line = "Balance: " + to_string(static_cast<int>(newBalance));
            }

            lines.push_back(line);

            if (line.empty()) {
                foundRoom = false;
            }
        }

        inFile.close();

        ofstream outFile("customers.db");
        if (!outFile) {
            cerr << "Unable to open file for writing: " << "customers.db" << endl;
            return;
        }
        for (const auto& l : lines) {
            outFile << l << endl;
        }
        outFile.close();
    }

    void addDebit() {
        cout << "Enter amont to debit: ";
        int amount;
        cin >> amount;

        this->debit += amount;
        cout << endl;
    }

    void releaseRoom() {
        if (debit >= 0) {
            delete customer; // Clean up customer
            customer = nullptr; // Reset to nullptr
            setAvailability(true);
            setDebit(0);
            setCredit(0);
        }
        else {
            while (debit < 0) {
                cout << "Additional debit needed for successful check out: " << -debit << endl;
                addDebit();
            }
        }
    }

    const Customer* getCustomer() const { return customer; }
};

int Room::capacity = 1;



class Hotel {
private:
    string name;
    string description;


public:
    static vector<Room> rooms;

    static void getUpdate(int i, Room r) {
        if (i >= 0) {
            rooms[i] = r;
        }
    }

    static void addRoom(Room room) {
        rooms.push_back(room);
    }

    static vector<Room> getRooms() {
        return rooms;
    }
};

vector<Room> Hotel::rooms;

class Reception {
private:
    vector<Room> availableRooms;
    vector<Room> bookedRooms;
    vector<Customer> customers;
    Hotel hotel;
public:
    void checkInCustomer(Room& room, const string& name, double balance, const string& checkInDate, const string& checkOutDate) {
        Customer* customer = new Customer(name, checkInDate, checkOutDate);
        room.assignCustomer(customer, balance);
        Hotel::rooms[room.getNumber() - 1] = room;
        vector <Room> availableRooms = this->hotel.getRooms();
        bookedRooms.push_back(room);
        auto it = find_if(availableRooms.begin(), availableRooms.end(), [&](const Room& r) { return r.getNumber() == room.getNumber(); });
        if (it != availableRooms.end()) {
            availableRooms.erase(it);
        }
    }
};

class HotelSystem {
private:
    Hotel hotel;
    vector<Room> availableRooms;
    Reception reception;
    Customer* customer;
    Validation validation;
public:
    HotelSystem() {
        for (int i = 1; i < 11; i++) {
            Room room;
            this->hotel.addRoom(room);
        }
        loadCustomersFromFile();
    }
    void displayHotelDescription() {
        int booked = 0;
        int avaliable = 100;
        for (const auto& room : Hotel::rooms) {

            if (!room.isAvailable()) {
                booked++;
            }
        }
        cout << "---------------------------------------------" << endl;
        cout << " Hotel Description: " << endl;
        cout << " Hotel Name: Kazakhsiky Rusich " << endl;
        cout << " Hotel Capacity: " << avaliable << endl;
        cout << " Rooms Available: " << avaliable - booked << endl;
        cout << "---------------------------------------------" << endl;
    }

    void checkInCustomer() {
        string name, checkInDate, checkOutDate;
        double balance;
        int roomNumber;

        validation.promptForName(name);
        validation.promptForBalance(balance);
        validation.promptForDate(checkInDate, "Enter Check In Date: ");
        validation.promptForDate(checkOutDate, "Enter Check Out Date: ");
        validation.promptForRoomNumber(roomNumber);

        for (auto& room : Hotel::rooms) {

            if (room.getNumber() == roomNumber) {


                reception.checkInCustomer(room, name, balance, checkInDate, checkOutDate);
                cout << "Customer checked-in successfully!" << endl;
                break;
            }
        }
        saveToDatabase();

    }
    void removeRecordByRoomNumber(const string& filename, int roomNumber) {
        ifstream inFile(filename);
        if (!inFile) {
            cerr << "Unable to open file: " << filename << endl;
            return;
        }

        vector<string> lines;
        string line;
        bool skip = false;

        while (getline(inFile, line)) {
            if (line.find("Room number:") != string::npos) {
                int currentRoomNumber = stoi(line.substr(line.find(":") + 2));
                // Проверяем, совпадает ли номер комнаты
                skip = (currentRoomNumber == roomNumber);
            }
            if (!skip) {
                lines.push_back(line);
            }
            // Если достигнут конец записи, сбрасываем skip
            if (line.empty() && skip) {
                skip = false;
            }
        }

        inFile.close();

        // Перезаписываем файл без удаленной записи
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "Unable to open file for writing: " << filename << endl;
            return;
        }

        for (const auto& l : lines) {
            outFile << l << endl;
            if (l.empty()) {
                outFile << endl; // Добавляем пустую строку между записями
            }
        }

        outFile.close();

    }

    void checkOutCustomer() {
        int roomNumber;
        validation.promptForRoomNumber(roomNumber);

        for (auto& room : Hotel::rooms) {

            if (room.getNumber() == roomNumber) {
                room.releaseRoom();
                removeRecordByRoomNumber("customers.db", roomNumber);
                Hotel::rooms[room.getNumber() - 1] = room;
                cout << "Customer checked-out successfully!" << endl;
                break;
            }
        }
        saveToDatabase();
    }

    void viewAvailableRooms() {
        cout << "Available Rooms:" << endl;
        for (const auto& room : Hotel::rooms) {
            if (room.isAvailable()) {
                cout << "Room " << room.getNumber() << endl;
            }
        }
    }

    void viewBookedRooms() {
        cout << "Booked Rooms:" << endl;
        for (const auto& room : Hotel::rooms) {
            if (!room.isAvailable()) {
                cout << "Room " << room.getNumber() << endl;
            }
        }
    }


    void saveToDatabase() {
        {
            ofstream file("rooms.db");
            if (!file) {
                cerr << "Error opening file rooms.db" << endl;
                return;
            }

            file << "Room Status :" << endl;

            for (const auto& room : hotel.getRooms()) {
                file << room.getNumber() << ": " << (room.isAvailable() ? "available" : "booked") << endl;


            }


            file.close();
        }
        {
            ofstream file("customers.db");
            if (!file) {
                cerr << "Error opening file customers.db" << endl;
                return;
            }

            file << "Customers :" << endl;

            for (const auto& room : hotel.getRooms()) {
                const Customer* cust = room.getCustomer();
                if (cust) {
                    file << "Room number: " << room.getNumber() << endl
                        << "Customer ID: " << cust->getId() << endl
                        << "Name: " << cust->getName() << endl
                        << "Check-in Date: " << cust->getCheckInDate() << endl
                        << "Check-out Date: " << cust->getCheckOutDate() << endl
                        << "Balance: " << fixed << setprecision(2) << room.getDebit() << endl << endl;
                }

            }

            file.close();
        }
        {
            ofstream file("bookings.db");
            if (!file) {
                cerr << "Error opening file bookings.db" << endl;
                return;
            }

            file << "Bookings Status :" << endl;

            for (const auto& room : hotel.getRooms()) {
                const Customer* cust = room.getCustomer();
                if (cust) {
                    file << "Room Number: " << room.getNumber() << endl
                        << "Status: " << (room.isAvailable() ? "available" : "booked") << endl
                        << "Customer Name: " << cust->getName() << endl
                        << "Check-In Date: " << cust->getCheckInDate() << endl
                        << "Check-Out Date " << cust->getCheckOutDate() << endl;
                }

            }


        }
    }

    void ShowBillsFromDatabase() {

        {
            string bdata;
            ifstream bookFile("bills.db");
            while (getline(bookFile, bdata))
                cout << bdata << endl;
            cout << "---------------------------------------------" << endl;
        }

    }

    void loadCustomersFromFile() {
        ifstream file("customers.db");
        if (!file) {
            cerr << "Unable to open file: customers.db" << endl;
            return;
        }

        string line;
        int roomnumber = 0;
        string name;
        double balance = 0;
        string checkInDate;
        string checkOutDate;

        while (getline(file, line)) {
            if (line.find("Room number:") != string::npos) {
                roomnumber = stoi(line.substr(line.find(":") + 2));
            }
            else if (line.find("Name:") != string::npos) {
                name = line.substr(line.find(":") + 2);
            }
            else if (line.find("Check-in Date:") != string::npos) {
                checkInDate = line.substr(line.find(":") + 2);
            }
            else if (line.find("Check-out Date:") != string::npos) {
                checkOutDate = line.substr(line.find(":") + 2);
            }
            else if (line.find("Balance:") != string::npos) {
                balance = stoi(line.substr(line.find(":") + 2));
            }
            for (auto& room : Hotel::rooms) {

                if (room.getNumber() == roomnumber) {


                    reception.checkInCustomer(room, name, balance, checkInDate, checkOutDate);

                    break;
                }
            }
        }

        if (customer) {  // Save the last customer to database or vector if needed
            customer->displayCustomerInfo();  // For demonstration, display the loaded customer
            delete customer;  // Assuming we are done with this customer
        }

        file.close();
    }

    void chargeCustomer() {
        cout << "Enter the room number: ";
        int roomNumber;
        validation.promptForRoomNumber(roomNumber);

        double amount;
        validation.promptForBalance(amount);
        for (auto& room : Hotel::rooms) {

            if (room.getNumber() == roomNumber) {
                room.chargeBill(amount);


                Hotel::rooms[room.getNumber() - 1] = room;

                break;
            }
        }
        saveToDatabase();

    }

    void menu() {
        int choice;
        do {
            cout << "Hotel Management System Menu" << endl;
            cout << "1. Display Hotel Description" << endl;
            cout << "2. Check-in Customer" << endl;
            cout << "3. Check-out Customer" << endl;
            cout << "4. View Available Rooms" << endl;
            cout << "5. View Booked Rooms" << endl;
            cout << "6. Charge Customer" << endl;
            cout << "7. Show Bill Logs(Only For High Level Administartion)" << endl;
            cout << "8. Exit" << endl;
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
            case 1:
                displayHotelDescription();
                break;
            case 2:
                checkInCustomer();
                break;
            case 3:
                checkOutCustomer();
                break;
            case 4:
                viewAvailableRooms();
                break;
            case 5:
                viewBookedRooms();
                break;
            case 6:
                chargeCustomer();
                break;
            case 7:
                ShowBillsFromDatabase(); //show bills logs
                break;
            case 8:
                cout << "Exiting program. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
            }
        } while (choice != 8);
    }
};

int main() {
    HotelSystem hotelSystem;
    hotelSystem.menu();
    return 0;
}