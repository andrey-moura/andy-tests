#include <iostream>
#include <filesystem>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>

int main(int argc, char* argv[])
{
    if(argc < 2) {
        std::cout << "Usage: andy-test folder [options]" << std::endl;
        return 1;
    }

    std::string_view folder = argv[1];

    std::filesystem::path tests_folder = std::filesystem::absolute(folder);
    std::vector<std::filesystem::path> test_files;

    for(auto& entry : std::filesystem::recursive_directory_iterator(tests_folder)) {
        if(entry.is_regular_file()) {
            std::filesystem::path path = entry.path();
            if(path.stem().string().ends_with("_spec")) {
                test_files.push_back(path);
            }
        }
    }

    std::filesystem::path cmake_lists = std::filesystem::absolute("CMakeLists.txt");

    if(!std::filesystem::exists(cmake_lists)) {
        std::cout << "CMakeLists.txt not found. Make sure to call andy-test from the root of your project." << std::endl;
        return 1;
    }

    std::filesystem::path build_folder = std::filesystem::absolute("build");

    if(!std::filesystem::exists(build_folder)) {
        std::cout << "Build folder not found. Make sure to call andy-test from where you ran cmake." << std::endl;
    }

    std::ofstream file("andy_tests.xml");
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    file << "<testsuites>" << std::endl;
    file.close();

    for(auto& path : test_files) {
        std::string command = "cmake --build " + build_folder.string() + " --target " + path.stem().string() + " -- -j 4 > " + build_folder.string() + "/build.log 2>&1";

        if(system(command.c_str())) {
            return 1;
        }

        std::cout << std::endl;

        system((build_folder / path.stem()).string().c_str());
    }

    file.open("andy_tests.xml", std::ios::app);
    file << "</testsuites>" << std::endl;

    return 0;
}