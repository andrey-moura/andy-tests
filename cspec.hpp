#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <format>
#include <sstream>

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

        class test_not_passed : public std::runtime_error
        {
        public:
            test_not_passed(const std::string& what)
                : std::runtime_error(what.c_str())
            {

            }
        };

        template<class T>
        class eq
        {
        public:
            eq(const T& t)
                : m_expected(&t)
            {

            }
        public:
            const T* m_expected = nullptr;
        };

        template<typename T>
        class expect
        {
        public:
            expect(const T& t)
                : m_expected(&t)
            {

            }
        public:
            const T* m_expected = nullptr;
            bool m_expect_result = true;
        public:
            const expect<T>& to() const
            {
                expect<T>* this_non_const = (expect<T>*)this;
                this_non_const->m_expect_result = true;
                return *this;
            }
            const expect<T>& to_not() const
            {
                expect<T>* this_non_const = (expect<T>*)this;
                this_non_const->m_expect_result = true;
                return *this;
            }
            template<typename OtherT>
            friend const expect<T>& operator<<(const expect<T>& expect, const eq<OtherT>& matcher)
            {
                bool match = *expect.m_expected == *matcher.m_expected;
                bool passed = match ? expect.m_expect_result : !expect.m_expect_result;

                if(!passed) {
                    throw test_not_passed(std::format("Expected {}\ngot      {}", *expect.m_expected, *matcher.m_expected));
                }

                return expect;
            }
        };

        class core
        {
        public:
            static std::vector<test_group*>& get_groups();
        public:
            static void run_tests();
        };

        class test
        {
        public:
            std::string m_name;
            std::function<void()> m_body;
        public:
            test() = default;
            test(const std::string& name, const std::function<void()> body)
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
            test_group(const std::string& name, const std::vector<test*>& tests);
        };
    };
};