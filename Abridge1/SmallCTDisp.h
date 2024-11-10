#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp> // For JSON handling
//#include <xlnt/xlnt.hpp>
#include <curl/curl.h>
#include "UpdateCompDB.h"


class SmallCTDisp
{
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
    SmallCTDisp(const std::string& url) : url(url) {}
    SmallCTDisp() {}
    static int invalidTransactions;
    bool send_operation_status();
    bool sendBranchInfo(const std::string& url);
    bool downloadExcel(const std::string& filePath);
    bool processExcel(const std::string& filePath);
    bool processExcel(const std::string& filePath, const std::string& directive);
    void printResults() const;
    void uploadTranErrorLog();
    void insertDataIntoDatabase(const std::string& csvFilePath, const std::string& dbPath);
    void invalidTransaction(const std::string& dbPath,const std::string& transaction);
    std::string calculatePBT(const std::string& proTaxRate, const std::string& forwardCharges);
    int getExchangeRate();
private:
    
    struct VersionInfo {
        std::string Major = "";
        std::string Minor = "";
        std::string BuildNum = "";

    };
    
    std::string url;
    std::unordered_map<std::string, double> costMap; // Maps Id to total cost
    std::string get_env_variable(const std::string& var);
    std::string escape_json(const std::string& str);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file);
    static size_t WriteCallback_S(void* contents, size_t size, size_t nmemb, std::string* userp);
    std::string get_ip_address();
    //std::string get_architecture();
    //bool GetVersion(VersionInfo& info);
    std::string get_current_time();
};