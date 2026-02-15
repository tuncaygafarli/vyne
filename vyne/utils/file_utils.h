#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

namespace FileUtils {
    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("IO Error: Could not open file at '" + path + "'");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}