// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>
#include <mutex>
#include <sstream>
#include <chrono>
#include <mutex>
#include <thread>
#include <windows.h>
#include <sddl.h>
#include <ctime>
#include <codecvt>
#include <fstream>
#include <string>
#include <thread>
#include <winternl.h>
#include <shobjidl.h> // For IShellLink
#include <comdef.h>   // For _com_error
#include <Shlobj.h>
#include <ctime>
#include <unordered_map>
#include <stdexcept>
#include <cstdlib> // for std::getenv 
#include <nlohmann/json.hpp> // For JSON handling
#include <curl/curl.h>  // You will need to install libcurl for network requests
#include "SmallCTDisp.h"
#include "SalesPromo.h"
#include "InvenFromDDG.h"
#include "UpdateCompDB.h"

namespace fs = std::filesystem;
//std::atomic<bool> userInputReceived(false);
std::atomic<bool> timeoutFlag(false);
std::atomic<bool> inputThreadActive(false);
std::mutex consoleMutex;

    void mainLoop() {
        SmallCTDisp excelRecord("http://Sharmabros.com/invenories/excel.xlsx");
        UpdateCompDB CompanyDB;
        SalesPromo PromoRecord;
        SmallCTDisp CostDisp;
        InvenFromDDG FromDG;
        try {
                        
            CompanyDB.app_initialize();
            CostDisp.sendBranchInfo(CompanyDB.BRANCH_INFO_URL);
            CompanyDB.logMessage("Processing started.");             
           
                try {
                    // wait for an hour till branch office provide data                    
                    std::this_thread::sleep_for(std::chrono::seconds(1));                    
                    const std::string filePath = CompanyDB.DOWN_LOAD_URL + "/downloaded_file.xlsx";
                    if (excelRecord.downloadExcel(filePath)) {
                        if (excelRecord.processExcel(filePath)) {
                            PromoRecord.send_error_report(CompanyDB.ERROR_REPORT_URL, "Problem loading config file: ");
                         }
                        excelRecord.printResults();
                    }
                    else {
                        //std::cerr << "Failed to download the data file." << std::endl;
                    }

                    SalesPromo processorSFP("data.csv"); 

                     if (processorSFP.processCSV()) {
                        processorSFP.printResults();
                    }
                    else {
                       // std::cerr << "Failed to process the CSV file." << std::endl;
                    }

                     if (CompanyDB.SmallCostReport == 1) {
                         CompanyDB.uploadReport("SmallCostReport");
                    }
                     else if (CompanyDB.InventoryReport == 1) {
                         CompanyDB.uploadReport("InventoryReport");
                     }
                     else if (CompanyDB.SalesPromoReport == 1) {
                         CompanyDB.uploadReport("SalesPromoReport");
                     }
                     else if (excelRecord.invalidTransactions == 1) {
                         excelRecord.uploadTranErrorLog();
                     }
                   
                }
                catch (const std::exception& e) {
                    CompanyDB.logMessage("Exception in main loop: " + std::string(e.what()));
                    CompanyDB.SESSION_TRACKER += "->While Loop Exception";
                    excelRecord.send_operation_status();
                }                
            
        }
        catch (const std::exception& e) {
            CompanyDB.logMessage("Unexpected exception in main: " + std::string(e.what()));
            CompanyDB.SESSION_TRACKER += "->Main Loop exception";
        }
        
    }
    void uploadInvalidTransaction() {
        UpdateCompDB CompanyDB;
        CompanyDB.uploadReport("Daily_Report");

    }
    void sendDailyReport() {
        SmallCTDisp CTD;
        CTD.send_operation_status();

    }
    void processDataFile() {
        
        mainLoop();
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate processing
    }
    void printDailyReport() {
        InvenFromDDG DDG;
        DDG.printResults();
    }
    void timeoutFunction(int& choice) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
        if (choice == 0) { // Only update if no choice has been made
            timeoutFlag = true; // Set the timeout flag
            choice = 1; // Update choice to 1 to process data file
        }
    }
    void displayMenu() {
        std::cout << "\nMenu:\n";
        std::cout << "1. Process Data File\n";
        std::cout << "2. Upload Invalid Transaction\n";
        std::cout << "3. Send Daily Report\n";
        std::cout << "4. Print Daily Report\n";
        std::cout << "5. Exit\n";
    }
    void clearScreen() {
        //std::system("clear || cls");
    }
    void getUserInput(int& choice) {
        std::cout << "Enter your choice: ";
        std::cin >> choice;
    }
    int main() {
        int choice = 0;
        HANDLE hMutex = CreateMutexA(NULL, TRUE, "Global\\Abridge");
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            std::cout << "Another instance is already running." << std::endl;
            return 1; // Exit the application
        }
        while (true) {
            clearScreen();
            std::cout << "\nMenu:\n";
            std::cout << "1. Process Data File\n";
            std::cout << "2. Upload Invalid Transaction\n";
            std::cout << "3. Send Daily Report\n";
            std::cout << "4. Print Daily Report\n";
            std::cout << "5. Exit\n";

            // Reset the choice and timeout flag before waiting for input
            choice = 0;
            timeoutFlag = false;

            // Start the timeout thread
            std::thread timeoutThread([&]() { timeoutFunction(choice); });

            // Start the input thread if not already running
            if (!inputThreadActive) {
                inputThreadActive = true;
                std::thread inputThread([&]() {
                    getUserInput(choice);
                    inputThreadActive = false; // Mark thread as inactive
                    });
                inputThread.detach(); // Detach the input thread to allow it to run independently
            }

            while (choice == 0 || !timeoutFlag) { // Loop until a choice is made or timeout occurs
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Short sleep to prevent busy waiting
            }

            // Join the timeout thread
            if (timeoutThread.joinable()) {
                timeoutThread.join(); // Wait for the timeout thread to finish
            }

            // Handle user choice if input was received or if timeout occurred
            switch (choice) {
            case 1:
                processDataFile(); // Call in main thread for proper output
                break;
            case 2:
                uploadInvalidTransaction();
                break;
            case 3:
                sendDailyReport();
                break;
            case 4:
                printDailyReport();
                break;
            case 5:
                std::cout << "Exiting the program...\n";
                return 0;
            default:
                // No action needed since timeout sets choice to 1
                break;
            }
        }
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 0;
    }
   

