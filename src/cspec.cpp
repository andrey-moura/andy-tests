#include <cspec.hpp>

std::vector<uva::cspec::test_group*> uva::cspec::core::s_tests_groups;

void uva::cspec::core::run_tests()
{
    for(const auto& test_group : s_tests_groups)
    {
        for(const auto& test : test_group->m_tests)
        {
            try {
                bool passed = test->m_body();

                if(passed) {

                } else {

                }
                
            } catch(std::exception& e) {

            }
        }
    }
}