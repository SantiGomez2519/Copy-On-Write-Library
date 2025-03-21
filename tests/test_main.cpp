#include "Versioning.h"
#include <iostream>

int main() {
    Versioning versionSystem;
    
    versionSystem.create("juanes.txt");
    versionSystem.open("juanes.txt");
    versionSystem.write("Juanes te saluda");
    versionSystem.read();
    versionSystem.close();

    return 0;
}
