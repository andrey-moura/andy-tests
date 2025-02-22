#include <andy/tests.hpp>

using namespace andy;
using namespace andy::tests;

extern context_or_describe of;

size_t andy::tests::current_describe_level = 0;
std::vector<andy::tests::test_result> andy::tests::result_list;
bool andy::tests::has_the_first_specification = false;

int main(int argc, char* argv[])
{
    tests::current_describe_level++;

    try {
        of.m_test_function();
    } catch(const std::exception& e) {
        uva::console::log_error("Exception outside examples: {}", e.what());
    }

    std::cout << std::endl;
    int passed = 0;
    int failed = 0;

    for(const auto& result : tests::result_list) {

        if(result.passed) {
            ++passed;
        } else {
            ++failed;
            std::cout << std::endl;

            uva::console::log_error("{}", result.description);
            uva::console::log_error("{}", result.error_message);
        }
    }

    std::cout << std::endl;

    uva::console::log("{} examples. {} passed, {} failures", passed + failed, passed, failed);

    return failed;
}