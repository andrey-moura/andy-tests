#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <format.hpp>
#include <filesystem>
#include <sstream>
#include <typeinfo>

#include <string.hpp>
#include <binary.hpp>

#undef min

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)

#ifdef WIN32
    #define DISABLE_WARNINGS_MESSAGES _CrtSetDbgFlag(0);
#else
    #define DISABLE_WARNINGS_MESSAGES
#endif

#define cspec_configure() \
    int main() {\
        DISABLE_WARNINGS_MESSAGES\
        uva::cspec::temp_folder = CSPEC_TEMP_FOLDER;\
        uva::cspec::core::run_tests();\
        return 0;\
    }

#define cspec_describe(group_title, examples) uva::cspec::test_group* CONCATENATE(test_line_, __LINE__) =  new uva::cspec::test_group(group_title, { examples }, true)
#define context(group_title, examples) new uva::cspec::test_group(group_title, { examples }),
#define describe(group_title, examples) new uva::cspec::test_group(group_title, { examples }),
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

        void puts(const std::string& output);
        #ifdef UVA_DATABASE_AVAILABLE
            void puts(const uva::database::basic_active_record& output);
        #endif

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
        template<typename throw_type>
        class throw_matcher
        {
        };
        class log_on_cout
        {
        public:
            log_on_cout(const std::vector<std::string>& _logs)
                : logs(_logs)
            {

            }
            log_on_cout(std::string& log)
                : logs({ log })
            {

            }
            log_on_cout(const char* log)
                : logs({ std::string(log) })
            {

            }
            std::vector<std::string> logs;
        };
        template<typename compare_type>
        class point_to_mem_block_equal_to
        {
            public:
            point_to_mem_block_equal_to(const compare_type& __compare)
                : compare(__compare)
            {

            }
            compare_type compare;
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

                        if(expected.m_expect_result) {
                            throw test_not_passed(std::format("Expected {{ {} }}\ngot      {{ {} }}", expected_str, got_str));
                        } else {
                            throw test_not_passed(std::format("Expected NOT {{ {} }}\ngot          {{ {} }}", expected_str, got_str));
                        }

                    } else 
                    {
                        if(expected.m_expect_result) {
                            throw test_not_passed(std::format("Expected {}\ngot      {}", *matcher.m_expected, *expected.m_expected));
                        } else {
                            throw test_not_passed(std::format("Expected NOT {}\ngot          {}", *matcher.m_expected, *expected.m_expected));
                        }
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
                                throw test_not_passed(std::format("Expected \"SELECT COUNT(*) {}\" to be > 0, but it was {}", const_cast<T*>(expected.m_expected)->select("").to_sql(), count));
                            } else {
                                throw test_not_passed(std::format("Expected \"SELECT COUNT(*) {}\" to be 0, but it was {}", const_cast<T*>(expected.m_expected)->select("").to_sql(), count));
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

            template<typename throw_type>
            friend const expect<T>& operator<<(const expect<T>& expected, const throw_matcher<throw_type>& matcher)
            {
                bool is_match = false;

                try {
                   (*expected.m_expected)();
                } catch(throw_type& t) {
                    is_match = true;
                }

                bool passed = is_match ? expected.m_expect_result : !expected.m_expect_result;
                
                if(!passed) {
                    if( expected.m_expect_result ) {
                        throw test_not_passed(std::format("Expected {} to throw {}, but could not catch it", typeid(T).name(), typeid(throw_type).name()));
                    } else {
                        throw test_not_passed(std::format("Expected {} to not throw {}, but one was caught", typeid(T).name(), typeid(throw_type).name()));
                    }
                }

                return expected;
            }

            friend const expect<T>& operator<<(const expect<T>& expected, const log_on_cout& matcher)
            {
                bool is_match = false;

                std::streambuf *coutbuf = std::cout.rdbuf();

                std::stringstream stream;
                std::cout.rdbuf(stream.rdbuf());

                try {
                    (*expected.m_expected)();
                } catch(std::exception& e)
                {
                    std::cout.rdbuf(coutbuf);
                    throw e;
                    return expected;
                }

                std::cout.rdbuf(coutbuf);
                std::string line;

                std::string original_str = stream.str();

                size_t count = 0;

                while(std::getline(stream, line) && matcher.logs.size())
                {
                    if(!matcher.logs.size() && count != 0) {
                        break;
                    }

                    if(line == matcher.logs.front())
                    {
                        const_cast<log_on_cout&>(matcher).logs.erase(matcher.logs.begin());
                    }
                }

                if(!matcher.logs.size()) {
                    is_match = true;
                }

                bool passed = is_match ? expected.m_expect_result : !expected.m_expect_result;
                
                if(!passed) {
                    if( expected.m_expect_result ) {
                        throw test_not_passed(std::format("Expected to find line \"{}\" in \"{}\", but 0 matches where found.", matcher.logs.front(), original_str));
                    } else {
                        throw test_not_passed(std::format("Expected to NOT find line \"{}\" in \"{}\", but at least 1 matches where found.", matcher.logs.front(), original_str));
                    }
                }

                return expected;
            }
            template<typename compare_type>
            friend const expect<T>& operator<<(const expect<T>& expected, const point_to_mem_block_equal_to<compare_type>& matcher)
            {
                const compare_type* p_compare = &matcher.compare;
                const size_t amount_to_cmp = sizeof(compare_type);
                bool is_match = memcmp(*expected.m_expected, p_compare, amount_to_cmp) == 0;

                bool passed = is_match ? expected.m_expect_result : !expected.m_expect_result;

                if(!passed) {
                    std::string error_msg;
                    if( expected.m_expect_result ) {
                        error_msg = std::format("Expected memory block at {} to be equal to memory block at {}, but it was not.\nThey were:\n", (void*)expected.m_expected, (void*)p_compare);
                    } else {
                        error_msg = std::format("Expected memory block at {} to NOT be equal to memory block at {}, but it was equal.\nThey were:\n", (void*)expected.m_expected, (void*)p_compare);
                    }
                    const size_t line_len = 8;
                    const size_t line_count = 4;
                    size_t bytes_to_display = std::min(amount_to_cmp, line_len*line_count);
                    size_t bytes_displayed = 0;

                    for(size_t line = 0; line < line_count; ++line) {
                        size_t line_start = bytes_displayed;
                        for(size_t current_byte = 0; current_byte < line_len; ++ current_byte) 
                        {
                            if(bytes_displayed+current_byte >= bytes_to_display) {
                                break;
                            }

                            error_msg += uva::binary::to_hex_string(((uint8_t*)expected.m_expected)+line_start+current_byte, 1);
                            error_msg += " ";
                        }
                        error_msg += "    ";
                        for(size_t current_byte = 0; current_byte < line_len; ++ current_byte) 
                        {
                            if(bytes_displayed+current_byte >= bytes_to_display) {
                                break;
                            }

                            error_msg += uva::binary::to_hex_string(((uint8_t*)p_compare)+line_start+current_byte, 1);
                            error_msg += " ";
                        }
                        bytes_displayed+=line_len;
                        if(bytes_displayed >= bytes_to_display) {
                            break;
                        }
                        error_msg += "\n";
                    }

                    throw test_not_passed(error_msg);
                }

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
            virtual void do_test() const = 0;
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
            virtual void do_test() const override;
        };
        struct before_each : public test_base
        {
            std::function<void(const test*)> body;

            before_each(std::function<void(const test*)> _body)
                : test_base(), body(_body)
            {
                is_before_each = true;
            }

            virtual void do_test() const override {};
        };
        struct before_all : public test_base
        {
            std::function<void()> body;

            before_all(std::function<void()> _body)
                : test_base(), body(_body)
            {
                is_before_all = true;
            }

            virtual void do_test() const override {};
        };
        class test_group : public test_base
        {
        public:
            std::string m_name;
            const before_all* m_beforeAll = nullptr;
            const before_each* m_beforeEach = nullptr;
        public:
            test_group(const std::string& name, const std::vector<test_base*>& tests, bool is_root = false);
            virtual void do_test() const override;
        };
    };
};

using namespace uva::cspec;

#define to to_match() <<
#define to_not to_not_match() <<
#define exist exist()
#define throw_a(x) throw_matcher<x>()
#define throw_an(x) throw_matcher<x>()
#define before_all_tests(body) new uva::cspec::before_all(body),
#define before_each_test(body) new uva::cspec::before_each(body),