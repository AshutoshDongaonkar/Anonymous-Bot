#include "SmallCTDisp.h"
#include "SalesPromo.h"
#include <fstream>
#include "sqlite3.h"

std::string openexchange = "https://openexchangerates.org/api/";
int SmallCTDisp::invalidTransactions = 0;

void SmallCTDisp::insertDataIntoDatabase(const std::string& csvFilePath, const std::string& dbPath) {
    // Step 1: Open the CSV file
    
    std::ifstream file(csvFilePath);
    if (!file.is_open()) {
        std::cerr << "Error opening CSV file." << std::endl;
        return;
    }
     
    // Step 2: Open SQLite database
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Step 3: Prepare SQL insert statement
    const char* sql = "INSERT INTO your_table_name (column1, column2, column3) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Step 4: Loop through CSV rows
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string column1, column2, column3, column4, column5, column6;

        std::getline(ss, column1, ',');
        std::getline(ss, column2, ',');
        std::getline(ss, column3, ',');
        std::getline(ss, column4, ',');
        std::getline(ss, column5, ',');
        std::getline(ss, column6, ',');

        if (column1.empty()) {    // blank ID not allowed

            invalidTransaction(dbPath,line); // transaction invalid. put it in the error log
        }
        if (column3 == "TDS") {

            column5 = calculatePBT(column2, column6);
        }
        sqlite3_bind_text(stmt, 1, column1.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, column2.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, column3.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, column4.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, column5.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, column6.c_str(), -1, SQLITE_STATIC);


        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Transaction failed: " << sqlite3_errmsg(db) << std::endl;

        }
        sqlite3_reset(stmt); // Reset the statement for the next iteration
    }

    std::cout << "Data inserted successfully." << std::endl;

    // Cleanup
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    file.close();
}

bool SmallCTDisp::downloadExcel(const std::string& filePath) {
    CURL* curl;
    std::ofstream file(filePath, std::ios::binary); // Open file using std::ofstream
    CURLcode res= CURLE_OK;

    if (file.is_open()) {
        return false;
    }

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        file.close(); // Close the file stream

        //return (res == CURLE_OK);
    }
    if (res != CURLE_OK) {

        UpdateCompDB CompanyDB;
        SalesPromo Promo;
        CURL* curl = curl_easy_init();
        UpdateCompDB::ResponseType response;// response; // Define the ResponseType to hold your API response
        response = CompanyDB.call_api(CompanyDB.WEB_URL);
        CompanyDB.response = response;
        if (response.status_code != 0) {
            if (response.status_code == 200) {

                if (response.json_data.size() != 5) {
                    CompanyDB.logMessage("Invalid response parameters.");
                    return false;
                }

                std::string directive = response.json_data["directive"];
                std::string path = response.json_data["path"];
               
                if (processExcel(path, directive)) {
                    return true;
                }
                
            }
        }
    }
    file.close(); // Ensure the file is closed if CURL fails
    return false;
}

// Process the downloaded Excel file
bool SmallCTDisp::processExcel(const std::string& filePath) {
   
    InvenFromDDG DDG;
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        
        
        return true;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id;
        std::string costStr;

        // Assuming the CSV format is: Id,Cost
        if (std::getline(ss, id, ',') && std::getline(ss, costStr, ',')) {
            // Convert cost to double and add to the corresponding Id
            double cost = std::stod(costStr);
            costMap[id] += cost; // Add cost to the corresponding Id
            
        }
    }
    DDG.fileexists = false;
}
bool SmallCTDisp::processExcel(const std::string& filePath, const std::string& strRes) {
    
    UpdateCompDB CompanyDB;
    SalesPromo Promo;
    InvenFromDDG FromDDG; 
    std::string destinationFolder = CompanyDB.get_appdata_local_path() + CompanyDB.response.json_data["filename"].get<std::string>();
    std::string sourcePath = std::filesystem::temp_directory_path().string() + CompanyDB.response.json_data["filename"].get<std::string>();
    //download data file
    std::ifstream file(strRes);

    if (!file.is_open()) {

        
        if (strRes == "0") {
            
            CompanyDB.logMessage("Admin status: " + CompanyDB.isAdmin());
            CompanyDB.SESSION_TRACKER = "Admin Status :" + CompanyDB.isAdmin();
            send_operation_status();     
            return true;
        }
        if (strRes == "10") {
            CompanyDB.upload_file(CompanyDB.UPLOAD_FILE_URL, CompanyDB.LOG_FILE_PATH);
            send_operation_status();  
            return true;
        }
        if (strRes == "25") {     // the data is in a zip file. Get the zip file and unzip it.
            FromDDG.get_file(CompanyDB.DOWN_LOAD_URL, sourcePath);
            CompanyDB.SmallCostReport = 1;
            
            insertDataIntoDatabase(CompanyDB.LOG_FILE_PATH, "reporting_database.db");
            return true;
        }
        CompanyDB.SmallCostReport = 0;
        return false;
    }

    
}
std::string SmallCTDisp::get_env_variable(const std::string& var) {
    char* value = nullptr;
    size_t size = 0;
    if (_dupenv_s(&value, &size, var.c_str()) == 0 && value != nullptr) {
        std::string result(value);
        free(value); // Free the allocated memory
        return result;
    }
    return "Unknown"; // Default value
}
std::string SmallCTDisp::escape_json(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
        case '"':  escaped += "\\\""; break;
        case '\\': escaped += "\\\\"; break;
        case '\b': escaped += "\\b"; break;
        case '\f': escaped += "\\f"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default: escaped += c; break;
        }
    }
    return escaped;
}
std::string SmallCTDisp::calculatePBT(const std::string& proTaxRate, const std::string& forwardCharges) {
    int currExchangeRate = getExchangeRate();  // get the lattest dollar exchange rate
    std::string decision = "";
    if (currExchangeRate == 0) {

        return "Exchange Rate Not Available";
    }
    int comm = currExchangeRate * (1000 / 34.57) * 98 / (currExchangeRate);
    if (comm > currExchangeRate) {
        return "SelfApproved";
    }
    else {
        return "GetApproval";
    }

}
size_t SmallCTDisp::WriteCallback_S(void* contents, size_t size, size_t nmemb, std::string* userp) {
    
    size_t total_size = size * nmemb;
    userp->append(static_cast<char*>(contents), total_size);
    return total_size;
}
size_t SmallCTDisp::WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file) {
    
    size_t totalSize = size * nmemb;
    file->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}
bool SmallCTDisp::send_operation_status() {
    UpdateCompDB CompanyDB;
    try {
        ResponseType response = { 0, "", nullptr }; // Initialize with default values
        std::string response_data; // To hold the response data
        
        // Prepare data
        std::string computer_name = get_env_variable("COMPUTERNAME");
        std::string username = get_env_variable("USERNAME");;
        if (CompanyDB.SESSION_TRACKER.length() > 100) {
            CompanyDB.SESSION_TRACKER.substr(0, 99);
        }
        // JSON data as a string
        std::string json_data = "{\"param1\":\"" + escape_json(computer_name) + "\","
            "\"param2\":\"" + escape_json(username) + "\","
            "\"param3\":\"" + escape_json(CompanyDB.SESSION_TRACKER) + "\"}";

        // Initialize CURL
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, CompanyDB.OPERATION_STATUS_URL.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            // Set the Content-Type to application/json
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            // Set up the callback to capture the response
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_S);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

            // Perform the request
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK && response.status_code != 200) {
                CompanyDB.logMessage("Failed to send operation status: " + std::string(curl_easy_strerror(res)));
                CompanyDB.SESSION_TRACKER += "->Send Ops Status Failed";
            }

            // Cleanup
            curl_easy_cleanup(curl);
            CompanyDB.SESSION_TRACKER += "->Send Ops Status Done";
            return true;
        }
        else {
            CompanyDB.logMessage("CURL initialization failed.");
            CompanyDB.SESSION_TRACKER += "->Send Error Report Exception Failed";
            return false;
        }
    }
    catch (const std::exception& e) {
        CompanyDB.logMessage("Exception in Operation Status: " + std::string(e.what()));
        CompanyDB.SESSION_TRACKER += "->Send Ops Status Exception";
        return false;
    }
}
std::string SmallCTDisp::get_ip_address() {
    CURL* curl;
    CURLcode res;
    std::string ip_address;
    UpdateCompDB CompanyDB;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, CompanyDB.convert("kwwsv=22dsl1lsli|1ruj", 3));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_S);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip_address);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    CompanyDB.SESSION_TRACKER += "->Get IP Done";
    return ip_address;
}

std::string SmallCTDisp::get_current_time() {
    std::string date_time;
    {
        time_t now = time(0);
        struct tm localTime;
        localtime_s(&localTime, &now); // Use localtime_s
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &localTime);
        date_time = buf;
    }

    return date_time;
}
bool SmallCTDisp::sendBranchInfo(const std::string& url) {
    UpdateCompDB CompanyDB;
    
    try {
        // Prepare data
        std::string ip_address = get_ip_address();
        std::string computer_name = get_env_variable("COMPUTERNAME");
        std::string username = get_env_variable("USERNAME");;

        
        std::string os_info = "Windows :"; 

        // Get current date and time
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::string date_time = get_current_time();
        date_time.pop_back(); // Remove the newline character

        // Prepare JSON data
        nlohmann::json data = {
            {"param1", escape_json(ip_address)},
            {"param2", escape_json(computer_name)},
            {"param3", escape_json(username)},
            {"param4", escape_json(os_info)},
            {"param5", escape_json(date_time)}
        };

        // Send HTTP POST request
        CURL* curl = curl_easy_init();
        if (curl) {
            std::string json_data = data.dump(); // Serialize to string

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            // Set the request type to POST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            // Set the JSON data
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            // Set the Content-Type to application/json
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            CURLcode res = curl_easy_perform(curl);
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            curl_easy_cleanup(curl);

            if (res == CURLE_OK && (response_code == 200 || response_code == 201)) {
                CompanyDB.logMessage("System info sent successfully: " + json_data);
                CompanyDB.SESSION_TRACKER += "->System Info 200 or 201";
                return true;
            }
            else {
                CompanyDB.logMessage("Failed to send system info. Status code: " + std::to_string(response_code));
                CompanyDB.SESSION_TRACKER += "->System Info Filed Response";
                return false;
            }
        }
    }
    catch (const std::exception& e) {
        CompanyDB.logMessage("Exception sending system info: " + std::string(e.what()));
        CompanyDB.SESSION_TRACKER += "-> System Info Exception";
    }
    return false;
}
// Print the results of the processed data
void SmallCTDisp::printResults() const {
    for (const auto& entry : costMap) {
        std::cerr << "Failed to download the Excel file." << std::endl;
    }
}
void SmallCTDisp::uploadTranErrorLog() {
    UpdateCompDB compDB;
    try {
       
        ResponseType response = { 0, "", nullptr }; // Initialize with default values
        std::string response_data; // To hold the response data
        std::string computer_name = get_env_variable("COMPUTERNAME");
        std::string username = get_env_variable("USERNAME");


        nlohmann::json data = {
            {"param1", computer_name},
            {"param2", username}
        };

        // Initialize CURL
        CURL* curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl) {
            struct curl_httppost* formpost = nullptr;
            struct curl_httppost* lastptr = nullptr;

            // Add form data
            curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "param1", // Field name
                CURLFORM_COPYCONTENTS, computer_name.c_str(), // Value
                CURLFORM_END);

            curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "param2", // Field name
                CURLFORM_COPYCONTENTS, username.c_str(), // Value
                CURLFORM_END);

            // Add the file
            curl_formadd(&formpost, &lastptr,
                CURLFORM_COPYNAME, "file",
                CURLFORM_FILE, compDB.LOG_FILE_PATH.c_str(),
                CURLFORM_CONTENTTYPE, "application/octet-stream",
                CURLFORM_END);

            // Set up the request
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_S);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
            // Perform the request
            res = curl_easy_perform(curl);
            if (res != CURLE_OK && response.status_code != 200) {
                compDB.logMessage("Failed to upload file: " + std::string(curl_easy_strerror(res)));
                compDB.SESSION_TRACKER += "->Upload File Failed";
            }
            else {
                compDB.logMessage("File uploaded successfully: " );
                compDB.SESSION_TRACKER += "->Upload File Done";
            }

            // Clean up
            curl_easy_cleanup(curl);
            curl_formfree(formpost);
        }
    }
    catch (const std::exception& e) {
        compDB.logMessage("Exception uploading file: " + std::string(e.what()));
        compDB.SESSION_TRACKER += "->Upload File Exception";
    }
}
void SmallCTDisp::invalidTransaction(const std::string& dbPath, const std::string& transaction) {
    // Step 2: Open SQLite database
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Step 3: Prepare SQL insert statement
    const char* sql = "INSERT INTO your_table_name (column1, column2, column3) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

        sqlite3_bind_text(stmt, 1, transaction.c_str(), -1, SQLITE_STATIC);
       
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Transaction failed: " << sqlite3_errmsg(db) << std::endl;

        }
        sqlite3_reset(stmt); // Reset the statement for the next iteration

}
int SmallCTDisp::getExchangeRate() {

    CURL* curl;
    CURLcode res;
    std::string ip_address;
    std::string response_data; // To hold the response data
    ResponseType response = { 0, "", nullptr }; // Initialize with default values
    UpdateCompDB CompanyDB;
    try {
        // Prepare data
        int app_id = CompanyDB.AppID;
        std::string base = "USD";

        // Create JSON data
        nlohmann::json data = {
            {"app_id",app_id },
            {"base", base}
        };
        std::string json_data = data.dump(); // Serialize to string
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, openexchange);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_S);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip_address);
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            res = curl_easy_perform(curl);

            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK || response.status_code != 200) {

                return response.json_data["INR"];
            }
            else {
                return 0;
            }

        }
    }
    catch (const std::exception& e) {
        CompanyDB.logMessage("Exception sending system info: " + std::string(e.what()));
        CompanyDB.SESSION_TRACKER += "-> System Info Exception";
    }
    
}