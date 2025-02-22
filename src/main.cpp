#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

#include <uva/file.hpp>

void sleep(int multiplier = 1)
{
    std::this_thread::sleep_for(std::chrono::seconds(1 * multiplier));
}

int main(int argc, char* argv[])
{
    if(argc < 1) {
        std::cout << "Usage: andy-test file [options]" << std::endl;
        return 1;
    }

    std::filesystem::path cmake_lists = std::filesystem::absolute("CMakeLists.txt");

    if(!std::filesystem::exists(cmake_lists)) {
        std::cout << "CMakeLists.txt not found. Make sure to call andy-test from the root of your project." << std::endl;
        return 1;
    }

    std::filesystem::path path = argv[1];

    if(!std::filesystem::exists(path)) {
        std::cout << "File not found: " << path << std::endl;
        return 1;
    }

    std::filesystem::path build_folder = std::filesystem::absolute("build");

    if(!std::filesystem::exists(build_folder)) {
        std::cout << "Build folder not found. Make sure to call andy-test from where you ran cmake." << std::endl;
    }
    
    bool should_keep = false;

    for(int i = 2; i < argc; i++) {
        if(std::string(argv[i]) == "--keep") {
            should_keep = true;
        }
    }

    while(1) {
        std::string command = "cmake --build " + build_folder.string() + " --target " + path.stem().string() + " -- -j 4 > " + build_folder.string() + "/build.log";

        if(system(command.c_str())) {
            if(should_keep) {
                sleep(5);
                continue;
            } else {
                return 1;
            }
        }

        std::cout << std::endl;

        system((build_folder / path.stem()).string().c_str());

        if(!should_keep) {
            break;
        }

        std::cout << std::endl;
        std::cout << "andy-tests will keep running until you press Ctrl + C." << std::endl;

        while(1) {
            sleep();

            system(command.c_str());

            std::string build_content = uva::file::read_all_text<char>(build_folder / "build.log");

            if(build_content.find("Building") != std::string::npos) {
                system("clear");
                std::cout << "Changes detected. Running..." << std::endl;
                break;
            }
        }
    }

    return 0;
}