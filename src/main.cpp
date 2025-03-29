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
    bool has_one_cpp = false;

    std::filesystem::path tests_folder = std::filesystem::absolute(folder);
    std::vector<std::filesystem::path> test_files;
    std::filesystem::path cmake_lists = std::filesystem::absolute("CMakeLists.txt");
    std::filesystem::path build_folder = std::filesystem::absolute("build");

    for(auto& entry : std::filesystem::recursive_directory_iterator(tests_folder)) {
        if(entry.is_regular_file()) {
            std::filesystem::path path = entry.path();
            if(path.stem().string().ends_with("_spec")) {
                if(path.extension() == ".cpp") {
                    if(!has_one_cpp) {
                        if(!std::filesystem::exists(cmake_lists)) {
                            std::cout << "CMakeLists.txt not found. Make sure to call andy-test from the root of your project." << std::endl;
                            return 1;
                        }
                
                        if(!std::filesystem::exists(build_folder)) {
                            std::cout << "Build folder not found. Make sure to call andy-test from where you ran cmake." << std::endl;
                        }
                    }

                    std::string command = "cmake --build " + build_folder.string() + " --target " + path.stem().string() + " -- -j 4 > " + build_folder.string() + "/build.log";

                    if(system(command.c_str())) {
                        std::cout << std::endl;
                        std::cerr << "Failed to build " << path.stem().string() << std::endl;
                        return 1;
                    }
        
                    has_one_cpp = true;
                }
                test_files.push_back(path);
            }
        }
    }

    std::ofstream file("andy_tests.xml");
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    file << "<testsuites>" << std::endl;
    file.close();

    bool has_one_failed = false;

    for(auto& path : test_files) {
        if(path.extension() == ".andy") {
            std::string command;
            std::filesystem::path andy_executable_path = argv[0];
            andy_executable_path.replace_filename("andy");
            if(!std::filesystem::exists(andy_executable_path)) {
                std::cout << "andy executable not found. Expected to be at " << andy_executable_path.string() << std::endl;
                return 1;
            }
            command.append(andy_executable_path.string());
            command.append(" ");
            command.append(path.string());
            if(system(command.c_str())) {
                has_one_failed = true;
            }
        }
        else {
            if(system((build_folder / path.stem()).string().c_str())) {
                has_one_failed = true;
            }
        }
    }

    file.open("andy_tests.xml", std::ios::app);
    file << "</testsuites>" << std::endl;

    return (has_one_failed ? 1 : 0);
}