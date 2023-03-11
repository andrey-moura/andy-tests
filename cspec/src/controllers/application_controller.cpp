#include <application_controller.hpp>

#include <iostream>

#include <console.hpp>
#include <file.hpp>
#include <core.hpp>
#include <json.hpp>

#ifdef __UVA_WIN__
    #include <Windows.h>
    #include <shlobj_core.h>

    //possibly add to uva.hpp
    #define popen _popen
    #define pclose _pclose
    #define WEXITSTATUS(x) (x)
#endif

std::string exec_buffer;

size_t exec_exit_code;
std::string exec(const std::string& __cmd, bool log = false, const std::string& args = "") {
    std::string cmd;
    size_t to_reserve = __cmd.size()+args.size()+30;
    cmd.reserve(to_reserve);
    cmd.push_back('"');
    cmd += __cmd;
    cmd.push_back('"');
    cmd.push_back(' ');
    cmd += args;

    UVA_CHECK_RESERVED_BUFFER(cmd, to_reserve);

    if(log) {
        std::cout << "Executing " << cmd << std::endl;
    }

    exec_buffer.resize(1024);

    std::string result;
    auto deleter=[](FILE* ptr) {
        exec_exit_code = WEXITSTATUS(pclose(ptr));
    };

    std::unique_ptr<FILE, decltype(deleter)> pipe(popen(cmd.c_str(), "r"), deleter);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(exec_buffer.data(), exec_buffer.size(), pipe.get()) != nullptr) {
        result += exec_buffer.data();
        std::cout << exec_buffer.data();
    }
    
    //std::cout << result << std::endl;
    return result;
}

void application_controller::run()
{
    std::string cmake_cmd;
    bool cmake_found = false;
#ifdef __UVA_WIN__
    TCHAR pf[MAX_PATH];
    SHGetSpecialFolderPath(0, pf, CSIDL_PROGRAM_FILES, FALSE); 

    std::filesystem::path program_files = pf;

    if(std::filesystem::exists(program_files)) {
        std::filesystem::path visual_studio_path = program_files / "Microsoft Visual Studio";

        if(std::filesystem::exists(visual_studio_path)) {
            //iterate through all VS versions
            for (auto const& version_entry : std::filesystem::directory_iterator(visual_studio_path)) 
            {
                std::filesystem::path version_path = version_entry.path();

                if (std::filesystem::is_directory(version_path)) {
                    //std::string version = version_path.filename();
                    for (auto const& product_entry : std::filesystem::directory_iterator(version_path)) 
                    {
                        std::filesystem::path product_path = product_entry.path();

                        if (std::filesystem::is_directory(product_path)) {
                            //(Prefessional, Community, etc)
                            //std::string product = version_path.filename();

                            std::filesystem::path cmake_bin_path = product_path / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake" / "CMake" / "bin";

                            if(std::filesystem::exists(cmake_bin_path))
                            {
                                cmake_cmd = (cmake_bin_path / "cmake.exe").string();

                                if(std::filesystem::exists(cmake_cmd) && !std::filesystem::is_directory(cmake_cmd))
                                {
                                    cmake_found = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Todo: try to grab from path

    if(!cmake_found) {
        log_error("cannot find cmake. Try setting cmake in your PATH or install Visual Studio.");
        return;
    }

    std::string cmake_version = exec(cmake_cmd, true, "--version");

    if(exec_exit_code || !cmake_version.starts_with("cmake version")) {
        log_error("currupted cmake installation.");
        return;
    }

    std::vector<std::string> targets_names;

    std::string codemodel;

    //std::filesystem::path build_path = std::filesystem::absolute("build");
    std::filesystem::path build_path = std::filesystem::current_path();
    if(build_path.filename() != "build") {
        build_path = build_path.parent_path();
    }

    std::filesystem::path reply_folder = build_path / ".cmake" / "api" / "v1" / "reply";

    if(!std::filesystem::exists(reply_folder))
    {
        log_error("error: cannot find responses from cmake at '{}'", reply_folder.string());
        return;
    }

    const std::string codemodel_prefix = "codemodel-v2-";
    const std::string codemodel_sufix = ".json"; 
  
    for (auto const& entry : std::filesystem::directory_iterator(reply_folder)) 
    {
        std::filesystem::path entry_path = entry.path();

        if (!std::filesystem::is_directory(entry_path)) {
            std::string entry_name = entry_path.filename().string();

            if(entry_name.starts_with(codemodel_prefix) && entry_name.ends_with(codemodel_sufix)) {
                codemodel = uva::file::read_all_text<char>(entry_path);
                break;

                // constexpr size_t suffix_size = sizeof(".dir")-1;
                // target_name.erase(target_name.size()-suffix_size, target_name.size());
                // exec(cmake_cmd, true, "--build " + build_path.string() + " --config Debug --target " + target_name + " -j 6 --");

                // if(!)

                // targets.push_back(target_name);
            }
        }
    }

    if(codemodel.empty()) {
        log_error("error: file '{}' cannot be found, is empty or cannot be read.", (reply_folder / (codemodel_prefix + "*" + codemodel_sufix)).string());
        return;
    }

    var jcodemodel;

    try {
        jcodemodel = uva::json::decode(codemodel);
    } catch(std::exception e) {
        log_error("error: failed to parse codemodel");
        return;
    }

    var configurations = jcodemodel["configurations"];

    if(!configurations || configurations.type != var::var_type::array || !configurations.size()) {
        log_error("error: cannot find a configuration");
        return;
    }

    std::string filter;

    if(params.size()) {
        filter = params[0];
    }

    if(!filter.ends_with("-spec")) {
        filter += "-spec";
    }

    std::string configuration_filter;
    
    if(params.size() > 1) {
        configuration_filter = params[1];
    }

    for(size_t i = 0; i < configurations.size(); ++i)
    {
        var configuration = configurations[i];

        if(configuration.type == var::var_type::map) {
            if(configuration_filter.size() && configuration["name"] != configuration_filter) {
                continue;
            }
            var targets = configuration["targets"];

            if(targets.type == var::var_type::array) {
                for(size_t target_i = 0; target_i < targets.size(); ++target_i) {
                    var target = targets[target_i];

                    var name = target["name"];

                    if(name.type == var::var_type::string) {
                        if(filter.size() && name != filter) {
                            continue;
                        }
                        else if(!name.ends_with("-spec")) {
                            continue;
                        }

                        std::string __name = name.as<var::var_type::string>();

                        if(std::find(targets_names.begin(), targets_names.end(), __name) == targets_names.end()) {
                            targets_names.push_back(std::move(__name));
                        }

                        if(filter.size()) {
                            break;
                        }
                    }
                }

                if(filter.size() && targets_names.size()) {
                    break;
                }
            }
        }
    }

    std::cout << "Found " << targets_names.size() << " targets." << std::endl;

    for(const std::string& target : targets_names)
    {
        exec(cmake_cmd, true, "--build . --target " + target + (configuration_filter.size() ? " --config " + configuration_filter : ""));

        if(exec_exit_code) {
            //the cmake has already shown error for us, just exit
            return;
        }
    }

    log_success("build sucessfully, starting tests...");

    for(const std::string& target : targets_names)
    {
        std::filesystem::path executable_path = build_path;

        if(configuration_filter.size()) {
#ifdef __UVA_WIN__
            executable_path = executable_path / configuration_filter;
#endif
        }

        executable_path = executable_path / target;

#ifdef __UVA_WIN__
        executable_path.replace_extension(".exe");
#endif

        exec(executable_path.string(), true);
    }
#endif
}