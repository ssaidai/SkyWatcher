#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    // Check if the log file was provided as an argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <logfile>" << std::endl;
        return 1;
    }

    std::string logfile = argv[1];
    std::ifstream logFileStream(logfile);

    // Check if the log file was successfully opened
    if (!logFileStream.is_open()) {
        std::cerr << "Failed to open log file: " << logfile << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> errorLogs;

    // Read the log file line by line
    while (std::getline(logFileStream, line)) {
        // Check if the line contains an ERROR log
        if (line.find("[ERROR]") != std::string::npos) {
            errorLogs.push_back(line);
        }
    }

    logFileStream.close();

    // Output the results
    if (errorLogs.empty()) {
        std::cout << "No errors." << std::endl;
    } else {
        std::cout << "Error logs found:" << std::endl;
        for (const auto& errorLine : errorLogs) {
            std::cout << errorLine << std::endl;
        }
    }

    return 0;
}
