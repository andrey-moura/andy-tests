#include <fstream>

#include <andy/tests.hpp>

using namespace andy;
using namespace andy::tests;

size_t andy::tests::current_describe_level = 0;
context_or_describe* andy::tests::first_specification = nullptr;
std::vector<andy::tests::test_result> andy::tests::result_list;
std::vector<std::string_view> andy::tests::current_describe;

void write_safe_str(std::ostream& out, std::string_view str)
{
    for(const char& c : str) {
        if(c == '<') {
            out << "&lt;";
        } else if(c == '>') {
            out << "&gt;";
        } else if(c == '&') {
            out << "&amp;";
        } else if(c == '\'') {
            out << "&apos;";
        } else if(c == '\"') {
            out << "&quot;";
        } else {
            out << c;
        }
    }
}

int andy::tests::run()
{
    auto start = std::chrono::high_resolution_clock::now();
    tests::current_describe_level++;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // append to file
    std::ofstream file("andy_tests.xml", std::ios::app);
    file << "\t<testsuite name=\"";
    write_safe_str(file, first_specification ? first_specification->m_description : "andy tests");
    file << "\" tests=\"" << tests::result_list.size() << "\" failures=\"0\" time=\"" << duration.count() / 1000.0 << "\">" << std::endl;

    std::cout << std::endl;
    int passed = 0;
    int failed = 0;

    for(const auto& result : tests::result_list) {
        file << "\t\t<testcase name=\"";
        if(result.describes.size()) {
            write_safe_str(file, result.describes);
            file << " ";
        }
        write_safe_str(file, result.description);
        file << "\" time=\"" << result.duration / 1000.0 << "\">" << std::endl;
        if(result.passed) {
            ++passed;
        } else {
            ++failed;
            std::cout << std::endl;

            file << "\t\t<failure message=\"";
            write_safe_str(file, result.error_message);
            file << "\" />" << std::endl;

            andy::console::log_error("{} {}", result.describes, result.description);
            andy::console::log_error("{}", result.error_message);
        }
        file << "\t\t</testcase>" << std::endl;
    }

    std::cout << std::endl;

    file << "\t</testsuite>" << std::endl;

    //andy::console::log("{} examples. {} passed, {} failures", passed + failed, passed, failed);

    return failed;
}