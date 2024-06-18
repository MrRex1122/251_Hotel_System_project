#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace std;

class Customer {
private:
    int id;
    string name;
    double balance;
    string checkInDate;
    string checkOutDate;
public:
    Customer() : id(0), name(""), balance(0.0), checkInDate(""), checkOutDate("") {}
    Customer(string n, double b, string cid, string cod) : name(n), balance(b), checkInDate(cid), checkOutDate(cod) { id++; }

    void setName(string name) { this->name = name; }
    string getName() const { return name; }

    void setCheckInDate(string date) { checkInDate = date; }
    string getCheckInDate() const { return checkInDate; }

    void setCheckOutDate(string date) { checkOutDate = date; }
    string getCheckOutDate() const { return checkOutDate; }

    double getBalance() const { return balance; }
    void updateBalance(double amount) { balance += amount; }
};

class Room {
private:
    static int number;
    bool available;
    Customer customer;
public:
    Room() { number++; }

    int getNumber() const { return number; }
    void setAvailability(bool available) { this->available = available; }
    bool isAvailable() const { return available; }

    void assignCustomer(const Customer& customer) { this->customer = customer; available = false; }
    void releaseRoom() { available = true; customer = Customer(); }

    Customer getCustomer() const { return customer; }
};

int Room::number = 0;

class Hotel {
private:
    string name;
    string description;
    vector<Room> rooms;
public:
    void setName(string name) { this->name = name; }
    string getName() const { return name; }

    void setDescription(string description) { this->description = description; }
    string getDescription() const { return description; }

    void addRoom(Room room) { rooms.push_back(room); }
    vector<Room> getRooms() const { return rooms; }
};

class Accounting {
private:
    string customerName;
    string checkInDate;
    string checkOutDate;
    double amount;
public:
    Accounting() : customerName(""), checkInDate(""), checkOutDate(""), amount(0.0) {}

    void generateBill(string customerName, string checkInDate, string checkOutDate, double amount) {
        this->customerName = customerName;
        this->checkInDate = checkInDate;
        this->checkOutDate = checkOutDate;
        this->amount = amount;
    }

    double getAmount() const { return amount; }
};

class Reception {
private:
    vector<Room> availableRooms;
    vector<Room> bookedRooms;
    vector<Accounting> bills;
    vector<Customer> customers;
public:
    void loadRoomsAndBookings(const string& filename) {
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                istringstream iss(line);
                int roomNumber;
                string status, customerName, dateIn, dateOut;
                iss >> roomNumber >> status >> customerName >> dateIn >> dateOut;

                Room room;
                room.assignCustomer(Customer(customerName, 0, dateIn, dateOut));
                room.setAvailability(status == "available");
                if (status == "available") {
                    availableRooms.push_back(room);
                }
                else {
                    bookedRooms.push_back(room);
                }
            }
            file.close();
        }
        else {
            cerr << "Unable to open file: " << filename << endl;
        }
    }

    void loadCustomersAndBills(const string& filename) {
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                istringstream iss(line);
                int id, room;
                string name, surname;
                double balance, billAmount;
                iss >> id >> name >> surname >> room >> balance >> billAmount;

                Customer customer(name, balance, "", "");
                customers.push_back(customer);

                Accounting bill;
                bill.generateBill(name, "", "", billAmount);
                bills.push_back(bill);
            }
            file.close();
        }
        else {
            cerr << "Unable to open file: " << filename << endl;
        }
    }

    void bookCustomer(Room& room, const string& name, double balance, const string& checkInDate, const string& checkOutDate) {
        Customer customer(name, balance, checkInDate, checkOutDate);
        room.assignCustomer(customer);
        bookedRooms.push_back(room);

        auto it = find_if(availableRooms.begin(), availableRooms.end(), [&](const Room& r) { return r.getNumber() == room.getNumber(); });
        if (it != availableRooms.end()) {
            availableRooms.erase(it);
        }

        Accounting bill;
        bill.generateBill(name, checkInDate, checkOutDate, balance);
        bills.push_back(bill);
    }

    void checkInCustomer(Room& room, const string& name, double balance, const string& checkInDate, const string& checkOutDate) {
        bookCustomer(room, name, balance, checkInDate, checkOutDate);
    }

    void checkOutCustomer(Room& room) {
        auto it = find_if(bookedRooms.begin(), bookedRooms.end(), [&](const Room& r) { return r.getNumber() == room.getNumber(); });
        if (it != bookedRooms.end()) {
            room.releaseRoom();
            availableRooms.push_back(room);
            bookedRooms.erase(it);
        }
    }

    vector<Room> getAvailableRooms() const { return availableRooms; }
    vector<Room> getBookedRooms() const { return bookedRooms; }

    void chargeCustomer(Customer& customer) {
        auto it = find_if(bills.begin(), bills.end(), [&](const Accounting& bill) { return bill.getAmount() == customer.getBalance(); });
        if (it != bills.end()) {
            double amount = it->getAmount();
            customer.updateBalance(-amount);
        }
    }

    void saveRoomsAndBookings(const string& filename) {
        ofstream file(filename);
        if (file.is_open()) {
            for (const auto& room : availableRooms) {
                file << room.getNumber() << " available " << " " << " " << endl;
            }
            for (const auto& room : bookedRooms) {
                file << room.getNumber() << " booked " << room.getCustomer().getName() << " " << room.getCustomer().getCheckInDate() << " " << room.getCustomer().getCheckOutDate() << endl;
            }
            file.close();
        }
        else {
            cerr << "Unable to open file: " << filename << endl;
        }
    }

    void saveCustomersAndBills(const string& filename) {
        ofstream file(filename);
        if (file.is_open()) {
            for (const auto& customer : customers) {
                file << customer.getName() << " " << customer.getCheckInDate() << " " << customer.getCheckOutDate() << " " << customer.getBalance() << endl;
            }
            for (const auto& bill : bills) {
                file << bill.getAmount() << endl;
            }
            file.close();
        }
        else {
            cerr << "Unable to open file: " << filename << endl;
        }
    }
};

class HotelSystem {
private:
    Hotel hotel;
    Reception reception;
    Accounting accounting;
    vector<Room> availableRooms;
public:
    HotelSystem() {
        readInitialStateFromFile();
    }

    void readInitialStateFromFile() {
        // Read initial state from db and initialize the system
        reception.loadRoomsAndBookings("rooms.db");
        reception.loadCustomersAndBills("customers.db");
    }

    void displayHotelDescription() {
        // Display hotel description
        cout << "Hotel Name: " << hotel.getName() << endl;
        cout << "Description: " << hotel.getDescription() << endl;
    }

    void checkInCustomer() {
        string name;
        double balance;
        string checkInDate, checkOutDate;
        int roomNumber;

        cout << "Enter customer name: ";
        cin >> name;
        cout << "Enter balance: ";
        cin >> balance;
        cout << "Enter check-in date: ";
        cin >> checkInDate;
        cout << "Enter check-out date: ";
        cin >> checkOutDate;
        cout << "Enter room number: ";
        cin >> roomNumber;

        Room room;
        room.assignCustomer(Customer(name, balance, checkInDate, checkOutDate));
        reception.checkInCustomer(room, name, balance, checkInDate, checkOutDate);
    }

    void checkOutCustomer() {
        int roomNumber;
        cout << "Enter room number: ";
        cin >> roomNumber;

        Room room;
        reception.checkOutCustomer(room);
    }

    void viewAvailableRooms() {
        vector<Room> rooms = reception.getAvailableRooms();
        cout << "Available Rooms: " << endl;
        for (const auto& room : rooms) {
            cout << "Room Number: " << room.getNumber() << endl;
        }
    }

    void viewBookedRooms() {
        vector<Room> rooms = reception.getBookedRooms();
        cout << "Booked Rooms: " << endl;
        for (const auto& room : rooms) {
            cout << "Room Number: " << room.getNumber() << endl;
            cout << "Customer Name: " << room.getCustomer().getName() << endl;
            cout << "Check-in Date: " << room.getCustomer().getCheckInDate() << endl;
            cout << "Check-out Date: " << room.getCustomer().getCheckOutDate() << endl;
        }
    }

    void viewBillingSystem() {
        // Display billing system information
    }

    void saveToDatabase() {
        reception.saveRoomsAndBookings("rooms.db");
        reception.saveCustomersAndBills("customers.db");
    }

    void otherFeatures() {
        cout << "Ты молодец!" << endl;
    }
};

void menu(HotelSystem& hotelSystem) {
    int choice;
    do {
        cout << "Hotel Management System Menu" << endl;
        cout << "1. Display Hotel Description" << endl;
        cout << "2. Check-in Customer" << endl;
        cout << "3. Check-out Customer" << endl;
        cout << "4. View Available Rooms" << endl;
        cout << "5. View Booked Rooms" << endl;
        cout << "6. View Billing System" << endl;
        cout << "7. Save to Database" << endl;
        cout << "8. Inshalla не обосремся" << endl;
        cout << "9. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            hotelSystem.displayHotelDescription();
            break;
        case 2:
            hotelSystem.checkInCustomer();
            break;
        case 3:
            hotelSystem.checkOutCustomer();
            break;
        case 4:
            hotelSystem.viewAvailableRooms();
            break;
        case 5:
            hotelSystem.viewBookedRooms();
            break;
        case 6:
            hotelSystem.viewBillingSystem();
            break;
        case 7:
            hotelSystem.saveToDatabase();
            break;
        case 8:
            hotelSystem.otherFeatures();
            break;
        case 9:
            cout << "Exiting program. Goodbye!" << endl;
            break;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 9);
}

void testLoadRoomsAndBookings() {
    Reception reception;
    reception.loadRoomsAndBookings("rooms.db");
    vector<Room> availableRooms = reception.getAvailableRooms();
    vector<Room> bookedRooms = reception.getBookedRooms();

    // Проверка, что комнаты загружены корректно
    cout << "Available Rooms: " << availableRooms.size() << endl;
    cout << "Booked Rooms: " << bookedRooms.size() << endl;
}

void testLoadCustomersAndBills() {
    Reception reception;
    reception.loadCustomersAndBills("customers.db");

    // Проверка, что клиенты и счета загружены корректно
    cout << "Customers Loaded: " << reception.getAvailableRooms().size() + reception.getBookedRooms().size() << endl;
}

void testSaveRoomsAndBookings() {
    Reception reception;
    reception.loadRoomsAndBookings("rooms.db");

    // Измените что-то в данных и сохраните
    reception.saveRoomsAndBookings("rooms_test_save.db");

    // Проверьте, что данные сохранены корректно
}

void testSaveCustomersAndBills() {
    Reception reception;
    reception.loadCustomersAndBills("customers.db");

    // Измените что-то в данных и сохраните
    reception.saveCustomersAndBills("customers_test_save.db");

    // Проверьте, что данные сохранены корректно
}

int main() {
    testLoadRoomsAndBookings();
    testLoadCustomersAndBills();
    testSaveRoomsAndBookings();
    testSaveCustomersAndBills();
    HotelSystem hotelSystem;
    menu(hotelSystem);
    return 0;
}
