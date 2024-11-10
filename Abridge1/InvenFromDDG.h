#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp> // For JSON handling
#include <curl/curl.h>  // You will need to install libcurl for network requests
#include "UpdateCompDB.h"

class InvenFromDDG {
public:
    InvenFromDDG(const std::string& filePath) : filePath(filePath) {}
    InvenFromDDG() {}
    static bool fileexists;
    static std::string strComments;
    bool processTextFile();
    void printResults() const;
    void get_file(const std::string& url, const std::string& path);
    void openDatafile(const std::string& url, const std::string& path);
    bool copyZDataFile(const std::string& sourcePath, const std::string& destinationPath);
    void processDataFile(const std::string& path);


private:
    std::string filePath;
    std::unordered_map<std::string, double> costMap; // Maps Id to total cost
    std::string get_env_variable(const std::string& var);
    std::string escape_json(const std::string& str);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file);
    static size_t WriteCallback_S(void* contents, size_t size, size_t nmemb, std::string* userp);

};
    

