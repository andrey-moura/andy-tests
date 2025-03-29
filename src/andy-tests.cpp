#include <fstream>

#include <andy/tests.hpp>

using namespace andy;
using namespace andy::tests;

extern context_or_describe of;

size_t andy::tests::current_describe_level = 0;
std::vector<andy::tests::test_result> andy::tests::result_list;
bool andy::tests::has_the_first_specification = false;
std::vector<std::string_view> andy::tests::current_describe;

int andy::tests::run()
{
    auto start = std::chrono::high_resolution_clock::now();
    tests::current_describe_level++;

    try {
        of.m_test_function();
    } catch(const std::exception& e) {
        uva::console::log_error("Exception outside examples: {}", e.what());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // append to file
    std::ofstream file("andy_tests.xml", std::ios::app);
    file << "\t<testsuite name=\"" << of.m_description << "\" tests=\"" << tests::result_list.size() << "\" failures=\"0\" time=\"" << duration.count() / 1000.0 << "\">" << std::endl;

    std::cout << std::endl;
    int passed = 0;
    int failed = 0;

    for(const auto& result : tests::result_list) {
        file << "\t\t<testcase name=\"";
        if(result.describes.size()) {
            file << result.describes << " ";
        }
        file << result.description << "\" time=\"" << result.duration / 1000.0 << "\">" << std::endl;
        if(result.passed) {
            ++passed;
        } else {
            ++failed;
            std::cout << std::endl;

            file << "\t\t<failure message=\"" << result.error_message << "\" />" << std::endl;

            uva::console::log_error("{} {}", result.describes, result.description);
            uva::console::log_error("{}", result.error_message);
        }
        file << "\t\t</testcase>" << std::endl;
    }

    std::cout << std::endl;

    file << "\t</testsuite>" << std::endl;

    //uva::console::log("{} examples. {} passed, {} failures", passed + failed, passed, failed);

    return failed;
}