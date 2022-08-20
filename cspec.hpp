#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <format>
#include <filesystem>
#include <sstream>

#include <string.hpp>

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)

#define cspec_configure() \
    int main() {\
        uva::cspec::temp_folder = CSPEC_TEMP_FOLDER;\
        uva::cspec::core::run_tests();\
        return 0;\
    }

#define cspec_describe(group_title, examples) uva::cspec::test_group* CONCATENATE(test_line_, __LINE__) =  new uva::cspec::test_group(group_title, { examples }, true)
#define context(group_title, examples) new uva::cspec::test_group(group_title, { examples }),
#define it(test_title, test_body) new uva::cspec::test(test_title, test_body),

#ifdef UVA_DATABASE_AVAILABLE
    #include <database.hpp>
#endif

namespace uva
{
    namespace cspec
    {
        extern std::filesystem::path temp_folder;
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
        class exist
        {
        
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
            const expect<T>& to_match() const
            {
                expect<T>* this_non_const = (expect<T>*)this;
                this_non_const->m_expect_result = true;
                return *this;
            }
            const expect<T>& to_not_match() const
            {
                expect<T>* this_non_const = (expect<T>*)this;
                this_non_const->m_expect_result = false;
                return *this;
            }
            template<typename OtherT>
            friend const expect<T>& operator<<(const expect<T>& expected, const eq<OtherT>& matcher)
            {
                const T& expect = *expected.m_expected;
                const OtherT& match = *matcher.m_expected;

                bool is_match;

                const constexpr bool expect_is_container = uva::string::is_container<T>::value;
                const constexpr bool match_is_container = uva::string::is_container<OtherT>::value;
                const constexpr bool expect_is_string = std::is_same<std::string, T>::value;
                const constexpr bool match_is_string = std::is_same<std::string, OtherT>::value;
                const constexpr bool is_container = expect_is_container && match_is_container && !expect_is_string && !match_is_string;
                
                if constexpr (is_container)
                {
                    if(expect.size() != match.size())
                    {
                        is_match = false;
                    } else 
                    {
                        is_match = true;

                        for(size_t index = 0; index < expect.size(); ++index)
                        {
                            if(expect[index] != match[index])
                            {
                                is_match = false;
                                break;
                            }
                        }
                    }

                } else {
                    is_match = expect == match;
                }

                bool passed = is_match ? expected.m_expect_result : !expected.m_expect_result;

                if(!passed) {
                    if constexpr (is_container)
                    {
                        std::string expected_str = uva::string::join(*matcher.m_expected, ',');
                        std::string got_str = uva::string::join(*expected.m_expected, ',');

                        throw test_not_passed(std::format("Expected {{ {} }}\ngot      {{ {} }}", expected_str, got_str));

                    } else 
                    {
                       throw test_not_passed(std::format("Expected {}\ngot      {}", *matcher.m_expected, *expected.m_expected));
                    }
                }

                return expected;
            }

            friend const expect<T>& operator<<(const expect<T>& expected, const exist& matcher)
            {
                if constexpr(std::is_same<T, std::filesystem::path>::value)
                {
                    bool is_match = std::filesystem::exists(*expected.m_expected);
                    bool passed = is_match ? expected.m_expect_result : !expected.m_expect_result;

                    if(!passed) {
                        if( expected.m_expect_result ) {
                            throw test_not_passed(std::format("Expected file or directory to exists, but not exists at {}", expected.m_expected->string()));
                        } else {
                            throw test_not_passed(std::format("Expected file or directory to not exists, but exists at {}", expected.m_expected->string()));
                        }
                    }
                }
                #ifdef UVA_DATABASE_AVAILABLE
                    else if constexpr (std::is_same<T, uva::database::active_record_relation>::value)
                    {
                        size_t count = expected.m_expected->count();
                        bool is_match = (bool)count;
                        bool passed = is_match ? expected.m_expect_result : !expected.m_expect_result;

                        if(!passed) {
                            if( expected.m_expect_result ) {
                                throw test_not_passed(std::format("Expected \"COUNT(*) {}\" or directory to be > 0, but it was {}", expected.m_expected->to_sql(), count));
                            } else {
                                throw test_not_passed(std::format("Expected \"COUNT(*) {}\" or directory to be 0, but it was {}", expected.m_expected->to_sql(), count));
                            }
                        }
                    }
                    else
                    {
                        //#error "matcher exist called, but there is no away to know if the expected object exists.";
                    }
                #endif

                return expected;
            }

        };

        class core
        {
        public:
            static std::vector<test_group*>& get_groups();
        public:
            static void run_tests();
        };
        class test;
        class test_base
        {
        public:
            std::vector<const test_base*> tests;
            virtual void do_test(std::function<void(const test*)> _body) const = 0;
            bool is_group = false;
            bool is_root = false;
            bool is_before_all = false;
            bool is_before_each = false;
        };
        class test : public test_base
        {
        public:
            std::string m_name;
            std::function<void()> m_body;
        public:
            test() = default;
            test(const std::string& name, const std::function<void()> body);
            virtual void do_test(std::function<void(const test*)> _body) const override;
        };
        struct before_each : public test_base
        {
            std::function<void(const test*)> body;

            before_each(std::function<void(const test*)> _body)
                : test_base(), body(_body)
            {
                is_before_each = true;
            }

            virtual void do_test(std::function<void(const test*)> _body) const override {};
        };
        struct before_all : public test_base
        {
            std::function<void()> body;

            before_all(std::function<void()> _body)
                : test_base(), body(_body)
            {
                is_before_all = true;
            }

            virtual void do_test(std::function<void(const test*)> _body) const override {};
        };
        class test_group : public test_base
        {
        public:
            std::string m_name;
            const before_all* m_beforeAll = nullptr;
            const before_each* m_beforeEach = nullptr;
        public:
            test_group(const std::string& name, const std::vector<test_base*>& tests, bool is_root = false);
            virtual void do_test(std::function<void(const test*)> _body) const override;
        };
    };
};

using namespace uva::cspec;

#define to to_match() <<
#define to_not to_not_match() <<
#define exist exist()
#define before_all_tests(body) new uva::cspec::before_all(body),
#define before_each_test(body) new uva::cspec::before_each(body),