#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

class FileManager {
public:
    static bool copyFile(const std::string& source, const std::string& destination);
};

#endif

