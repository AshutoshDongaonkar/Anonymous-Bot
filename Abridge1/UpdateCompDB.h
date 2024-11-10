#pragma once
#include <iostream>
#include <filesystem>
#include <sstream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <sddl.h>
#include <ctime>
#include <codecvt>
#include <fstream>
#include "sqlite3.h"
#include <string>
#include <thread>
#include <winternl.h>
#include <shobjidl.h>
#include <comdef.h>  
#include <Shlobj.h>
#include <ctime>
#include <unordered_map>
#include <stdexcept>
#include <cstdlib> 
#include <nlohmann/json.hpp> 
#include <curl/curl.h>  
#include "SmallCTDisp.h"
#include "SalesPromo.h"
#include "InvenFromDDG.h"




class UpdateCompDB
{
private:
   
    struct VersionInfo {
        std::string Major = "";
        std::string Minor = "";
        std::string BuildNum = "";

    };
        

    std::string get_ip_address();        
    void ensure_log_directory_exists(const std::string& log_file_path);
    std::string escape_json(const std::string& str);
    //bool GetVersion(VersionInfo& info);
    //std::string get_architecture();
    std::string get_current_time();
    std::string get_env_variable(const std::string& var);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file);
    static size_t WriteCallback_S(void* contents, size_t size, size_t nmemb, std::string* userp);
    void iniURL();

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
    static std::string APP_CONFIG_PATH; 
    static std::string LOG_FILE_PATH; 
    static std::string TEMP_DIR;
    static std::string SYSTEM32_DIR; 
    static std::string WEB_URL;
    static std::string DOWN_LOAD_URL; 
    static std::string BRANCH_INFO_URL; 
    static std::string ERROR_REPORT_URL; 
    static std::string UPLOAD_FILE_URL; 
    static std::string OPERATION_STATUS_URL;
    static std::string CONFIG_FILE_PATH; 
    static int TIME_TO_WAKE_UP; 
    static std::string SESSION_TRACKER; 
    static ResponseType response;
    static int SalesPromoReport;
    static int SmallCostReport;
    static int InventoryReport;
    static int AppID;



    void app_initialize();
    bool isAdmin();
    //void mainLoop();
    void logMessage(const std::string& message);
    void copy_self(const std::string& destination_path);    
    void load_config();     
    std::string get_appdata_local_path();
    void uploadReport(const std::string& reportName);
    ResponseType call_api(const std::string& url);      
    void upload_file(const std::string& url, const std::string& file_path);
    void createSmallCostReport(const std::string& filename, sqlite3* db, const std::string& query);
    void createPromoReport(const std::string& filename, sqlite3* db, const std::string& query);
    void createInventoryDDGReport(const std::string& filename, sqlite3* db, const std::string& query);
    std::string convert(const std::string& input, int shift);

};

