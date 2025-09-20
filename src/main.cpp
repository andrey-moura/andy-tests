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
    std::vector<std::string> targets;
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

                    std::string command = "cmake --build ";

                    if(!has_one_cpp) {
                        command += build_folder.string();
                        command += " --target help > ";
                        command += build_folder.string();
                        command += "/build.log";

                        if(system(command.c_str())) {
                            std::cout << "Failed to get help for targets." << std::endl;
                            return 1;
                        }

                        std::ifstream file(build_folder.string() + "/build.log");
                        if(!file.is_open()) {
                            std::cout << "Failed to open build log for reading." << std::endl;
                            return 1;
                        }

                        std::string line;

                        while(std::getline(file, line)) {
                            std::string_view line_view = line;
                            while(line_view.size() && isspace(line_view.front())) {
                                line_view.remove_prefix(1);
                            }
                            if(line_view.size() && line_view.starts_with("...") && line_view.ends_with("_spec.o")) {
                                line_view.remove_prefix(3);
                                while(line_view.size() && isspace(line_view.front())) {
                                    line_view.remove_prefix(1);
                                }
                                line_view.remove_suffix(2);
                                size_t pos = line_view.find('/');
                                while(pos != std::string_view::npos) {
                                    line_view.remove_prefix(pos + 1);
                                    pos = line_view.find('/');
                                }
                                targets.push_back(std::string(line_view));
                            }
                        }

                        file.close();
                        std::sort(targets.begin(), targets.end());
                    }

                    if(std::find(targets.begin(), targets.end(), path.stem().string()) == targets.end()) {
#ifdef __linux__
// Log as yellow
                        std::cout << "\033[33m";
#endif
                        std::cout << "Skipping " << path.stem().string() << " as it is not part of the CMake build targets." << std::endl;
#ifdef __linux__
                        std::cout << "\033[0m";
#endif
                        continue;
                    }

                    command = "cmake --build ";
                    command += build_folder.string();
                    command += " --target ";
                    command += path.stem().string();
#ifdef _NDEBUG
                    command += " --config Release";
#else
                    command += " --config Debug";
#endif
#ifdef __linux__
                    command += " -- -j 4";
#endif
                    command += " > ";
                    command += build_folder.string();
                    command += "/build.log";

                    if(system(command.c_str())) {
                        std::cout << std::endl;
                        std::cerr << "Failed to build " << path.stem().string() << ". " << std::endl;
                        std::cerr << "Command run: " << command << std::endl;
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

    for(auto& path : test_files) {
        if(path.extension() == ".andy") {
            std::string command;
            std::filesystem::path andy_executable_path = argv[0];
            andy_executable_path.replace_filename("andy");
#ifdef _WIN32
            andy_executable_path.replace_extension(".exe");
#endif
            if(!std::filesystem::exists(andy_executable_path)) {
                std::cout << "andy executable not found. Expected to be at " << andy_executable_path.string() << std::endl;
                return 1;
            }
            command.append(andy_executable_path.string());
            command.append(" ");
            command.append(path.string());
            if(system(command.c_str())) {
            }
        }
        else {
            std::filesystem::path spec_exec_path = build_folder;
#ifdef NDEBUG
            spec_exec_path /= "Release/";
#else
            spec_exec_path /= "Debug/";
#endif
            spec_exec_path /= path.stem();
#ifdef _WIN32
            spec_exec_path.replace_extension(".exe");
#endif
            if(system(spec_exec_path.string().c_str())) {
            }
        }
    }

    file.open("andy_tests.xml", std::ios::app);
    file << "</testsuites>" << std::endl;
    file.close();

    int tests = 0;
    int failed = 0;

    // Read the XML file to extract the test results
    std::ifstream xml_file("andy_tests.xml");
    std::string line;
    while (std::getline(xml_file, line)) {
        std::string_view line_view = line;
        while(line_view.size() && isspace(line_view.front())) {
            line_view.remove_prefix(1);
        }
        if(line_view.starts_with("<testsuite")) {
            line_view.remove_prefix(10);
            while(line_view.size()) {
                if(line_view.starts_with("tests=\"")) {
                    line_view.remove_prefix(7);
                    std::string_view tests_str = line_view.substr(0, line_view.find('"'));
                    tests += std::stoi(std::string(tests_str));
                }
                if(line_view.starts_with("failures=\"")) {
                    line_view.remove_prefix(10);
                    std::string_view failures_str = line_view.substr(0, line_view.find('"'));
                    failed += std::stoi(std::string(failures_str));
                }
                line_view.remove_prefix(1);
            }
        }
    }

    xml_file.close();

    if(failed) {
        // red
        std::cout << "\033[31m";
    } else {
        // green
        std::cout << "\033[32m";
    }
    std::cout << tests << " examples. " << (tests - failed) << " passed, " << failed << " failures" << std::endl;
    if(failed) {
        // reset color
        std::cout << "\033[0m";
    }

    return failed ? 1 : 0;
}