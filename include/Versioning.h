#ifndef VERSIONING_H
#define VERSIONING_H

#include <string>

class Versioning {
    public:
        Versioning();
        void create(const std::string& fileName);
        void open(const std::string& fileName);
        void read();
        void write(const std::string& data);
        void close();
        
        void listVersions();
        void checkout(int versionNumber);

    private:
        std::string currentFile;
        int versionCounter;
};

#endif

