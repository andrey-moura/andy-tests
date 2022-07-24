#include <string>
#include <iostream>
#include <vector>
#include <functional>

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)

#define cpsec_configure() \
    int main() {\
        uva::cspec::core::run_tests();\
        return 0;\
    }


#define cspec_describe(group_title, examples) uva::cspec::test_group* CONCATENATE(test_line_, __LINE__) =  new uva::cspec::test_group(group_title, { examples })
#define it(test_title, test_body) new uva::cspec::test(test_title, test_body)

namespace uva
{
    namespace cspec
    {
        class test_group;
        class core
        {
        public:
            static std::vector<test_group*> s_tests_groups;
        public:
            static void run_tests();
        };
        class test
        {
        public:
            std::string m_name;
            std::function<bool()> m_body;
        public:
            test() = default;
            test(const std::string& name, const std::function<bool()> body)
                : m_name(name), m_body(body)
            {

            }
        };
        class test_group
        {
        public:
            std::string m_name;
            std::vector<test*> m_tests;
        public:
            test_group(const std::string& name, const std::vector<test*>& tests) 
                : m_name(name), m_tests(tests)
            {
                uva::cspec::core::s_tests_groups.push_back(this);
            }
        };
    };
};