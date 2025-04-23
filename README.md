---
# Author: Evgenii Korolev  
# Hotel System Project

Console-based hotel management system implemented in C++ to simulate real-world hotel operations such as check-in, check-out, room availability, billing, and customer validation.

## 🏨 Project Overview

This project is a terminal-driven hotel management program designed for educational purposes. It provides hotel staff with the ability to manage customer data, assign rooms, track billing, and generate logs using text-based files as storage.

---

## ✨ Key Features

- **Room Management**  
  - View available and booked rooms  
  - Assign customers to rooms  
  - Release rooms upon checkout  

- **Customer Management**  
  - Validate customer names and dates  
  - Track customer IDs and stay duration  
  - Save customer data in `customers.db`  

- **Check-in & Check-out System**  
  - User prompts for entering booking data  
  - Auto-calculated balance updates and billing  
  - Data persistency using `.db` flat files  

- **Billing System**  
  - Bills are appended to `bills.db`  
  - Support for dynamic balance charging and debt tracking  
  - Print historical billing logs for administrators  

- **Data Persistence**  
  - Reads and writes customer and room data using `.db` files  
  - Reconstructs previous bookings from saved data on launch  

---

## 📂 File-Based Data Handling

The system uses the following plain text files:
- `rooms.db` — stores current room status
- `customers.db` — records customer name, ID, dates, and balance
- `bookings.db` — keeps an overview of ongoing bookings
- `bills.db` — logs all billing transactions

All file I/O operations are handled via standard C++ streams, with checks for file validity and graceful error handling.

---

## 🧠 Architecture Breakdown

| Class | Description |
|-------|-------------|
| `Validation` | Validates inputs like names, dates, balances |
| `Customer` | Stores individual guest information |
| `Room` | Handles room availability, billing, and customer assignment |
| `Hotel` | Manages the list of rooms |
| `Reception` | Coordinates room and customer interactions |
| `HotelSystem` | Orchestrates the full program and menu system |

---

## 🖥️ Menu Options

```text
1. Display Hotel Description  
2. Check-in Customer  
3. Check-out Customer  
4. View Available Rooms  
5. View Booked Rooms  
6. Charge Customer  
7. Show Bill Logs  
8. Exit
