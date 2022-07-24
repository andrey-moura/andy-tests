#include <sstream>

#include <cspec.hpp>
#include <console.hpp>
#include <diagnostics.hpp>

//CORE BEGIN

std::vector<uva::cspec::test_group*>& uva::cspec::core::get_groups()
{
    static std::vector<uva::cspec::test_group*> s_tests_groups;
    return s_tests_groups;
}

void uva::cspec::core::run_tests()
{
    std::vector<std::string> failed_messages;
    std::vector<uva::cspec::test_group*> groups = uva::cspec::core::get_groups();

    size_t examples = 0;
    size_t failures = 0;

    auto elapsed = uva::diagnostics::measure_function([&]() {
        for(const auto& test_group : groups)
        {
            std::cout << test_group->m_name << std::endl;

            for(const auto& test : test_group->m_tests)
            {
                examples++;

                try {
                    test->m_body();
                    std::cout << "\t" << uva::console::color(uva::console::color_code::green) << test->m_name << std::endl;
                } catch(uva::cspec::test_not_passed& e)
                {
                    failures++;
                    std::cout << "\t" << uva::console::color(uva::console::color_code::red) << test->m_name << std::endl;

                    failed_messages.push_back(std::format("{} {}\nTest resulted in following error:\n{}", test_group->m_name, test->m_name, e.what()));
                }
                catch(std::exception& e)
                {
                    failures++;
                    std::cout << "\t" << uva::console::color(uva::console::color_code::red) << test->m_name << std::endl;

                    failed_messages.push_back(std::format("{} {}\nTest thrown following exception:\n{}", test_group->m_name, test->m_name, e.what()));
                }
            }

            std::cout << std::endl;
        }
    });

    for(const std::string& message : failed_messages)
    {
        std::cout << uva::console::color(uva::console::color_code::red) << message << std::endl;
    }

    std::cout << std::endl;
    std::cout << failures << " failures / " << examples << " examples." << std::endl;
    std::cout << "Tests took " << elapsed.count() << "s" << std::endl;
}

//CORE END

//TEST GROUP BEGIN

uva::cspec::test_group::test_group(const std::string& name, const std::vector<uva::cspec::test*>& tests) 
    : m_name(name), m_tests(tests)
{
    std::vector<uva::cspec::test_group*>& groups = uva::cspec::core::get_groups();
    groups.push_back(this);
}