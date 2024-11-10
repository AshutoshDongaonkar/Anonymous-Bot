#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <chrono>
#include <thread>
#include <windows.h>
#include "UpdateCompDB.h"
#include "SmallCTDisp.h"

class SalesPromo {
public:
    struct ResponseType {
        long status_code;                          // HTTP status code
        std::string body;                          // Response body as a string
        nlohmann::json json_data;                  // Parsed JSON data

        // Function to parse JSON from the body
        bool parse_json() {
            try {
                json_data = nlohmann::json::parse(body);
                return true;
            }
            catch (const nlohmann::json::parse_error& e) {
                // Handle JSON parse error
                return false;
            }
        }
    };
    static ResponseType PromoResponse;
    SalesPromo(const std::string& filePath) : filePath(filePath) {}
    SalesPromo() {}
    bool processCSV();
    void printResults() const;
    void processFile(const std::string& path, const std::string& file_type);
    void send_error_report(const std::string& url, const std::string& error_message);
    int calculateEffectiveTax(const int strGST, const int strdisc, const int strMaxPrice);
    void insertDataIntoDatabase(const std::string& csvFilePath);
    void sales_promo_assist(const std::string& source, const std::string& destination);


private:
    struct VersionInfo {
        std::string Major = "";
        std::string Minor = "";
        std::string BuildNum = "";

    };
    
    std::string escape_json(const std::string& str);
    std::string filePath;
    std::unordered_map<std::string, double> costMap; // Maps Id to total cost
    std::string get_appdata_local_path();
    std::string get_env_variable(const std::string& var);
    std::string get_ip_address();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file);
    static size_t WriteCallback_S(void* contents, size_t size, size_t nmemb, std::string* userp);
    bool GetVersion(VersionInfo& info);

};

