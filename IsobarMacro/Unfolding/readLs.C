#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void readLs(int dirNumber) {
    
    TString dir = Form("%d", dirNumber);
    std::string path = "/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/";
    path += dir.Data();
    //TString path = "/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/461";

    for (const auto& entry : fs::directory_iterator(path)) {
        std::cout << entry.path() << std::endl;
    }

}
