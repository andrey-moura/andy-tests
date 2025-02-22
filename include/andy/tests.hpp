#pragma once

#include <string>
#include <functional>
#include <memory>

#include <uva/console.hpp>

namespace andy
{
    namespace tests
    {
        enum describe_what
        {
            no_describe,
            structure,
            static_method,
            static_function,
        };
        const std::vector<std::string> describe_what_strings = {
            "no_describe",
            "class",
            "static method",
            "static function",
        };
        class test_result
        {
        public:
            bool passed = true;
            std::string error_message;
            std::string description;
        };

        extern size_t current_describe_level;
        extern std::vector<test_result> result_list;
        extern bool has_the_first_specification;

#ifdef USE_FMT_FORMT
        template <typename T, typename = void>
        struct is_formattable : std::false_type {};
        template <typename T>
        struct is_formattable<T, 
            std::enable_if_t< 
                std::has_formatter<T, std::format_context>::value
            > 
        > : std::true_type{};

        template <typename T>
        static constexpr auto is_formattable_v = is_formattable<T>::value;
#else
        template <typename T>
        static constexpr auto is_formattable_v = false;
#endif

        template<typename T> struct is_shared_ptr : std::false_type {};
        template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

        template<typename T>
        std::string safe_to_string(const T& value)
        {
            // Overrides of default formatters

            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return "nullptr";
            }

            if constexpr(is_formattable_v<T>) {
                return std::format("{}", value);
            }

            // No formatters found

            //shared_ptr
            if constexpr (is_shared_ptr<T>::value) {
                return std::string("std::shared_ptr<") + typeid(T).name() + ">(" + std::format("{}", (void*)value.get()) + ")";
            } else if constexpr ( std::is_same_v<T, std::string_view>) {
                return std::string(value);
            }
            else if constexpr ( std::is_same_v<T, std::string>) {
                return value;
            }
            // } else if constexpr (std::is_arithmetic_v<T>) {
            //     return std::to_string(value);
            // }
            // else if constexpr(std::is_pointer<T>::value) {
            //     return std::string("<") + typeid(T).name() + "#" + std::format("{}", (void*)value) + ">";
            // } else if c
        

            return std::string("<unformated::") + typeid(T).name() + ">";
        }
        class matcher
        {
        public:
            matcher(bool __negative)
                : negative(__negative)
            {
            }
            void assert(bool result)
            {
                if(result) {
                    status = negative ? assert_failed : assert_passed;
                } else {
                    status = negative ? assert_passed : assert_failed;
                }
            }
            void raise(const std::string& message)
            {
                throw std::runtime_error(message);
            }
            bool passed() const
            {
                return status == assert_passed;
            }
            bool failed() const
            {
                return status == assert_failed;
            }
        public:
            enum assert_status {
                assert_not_done,
                assert_passed,
                assert_failed,
            } status = assert_not_done;
            bool negative = false;
        };
        class eq : public matcher
        {
        public:
            template<typename T, typename OtherT>
            eq(bool negative, const T& actual, const OtherT& expected)
                : matcher(negative)
            {
                assert(actual == expected);

                if(failed()) {
                    std::string formated_expected = safe_to_string(expected);
                    std::string formated_got      = safe_to_string(actual);

                    std::string error_message;
                    error_message.reserve(formated_expected.size() + formated_got.size() + 50);
                    error_message += "expected: ";

                    if(negative) {
                        error_message += "NOT ";
                    }

                    error_message += formated_expected;
                    error_message += "\n     got: ";
                    error_message += formated_got;

                    raise(error_message);
                }
            }
        };
        class be_empty : public matcher
        {
        public:
            template<typename T>
            be_empty(bool negative, const T& actual)
                : matcher(negative)
            {
                assert(actual.empty());

                if(failed()) {
                    std::string formated_actual = safe_to_string(actual);
                    std::string error_message;
                    error_message.reserve(formated_actual.size() + 50);

                    error_message += "expected ";
                    error_message += formated_actual;
                    error_message += " to be empty";

                    raise(error_message);
                }
            }
        };
        template<typename T>
        class expect
        {
        public:
            expect(const T& value)
                : actual(value)
            {
            }
            template <class matcher, typename... Args>
            void to(Args&&... args)
            {
                matcher(false, actual, std::forward<Args>(args)...);
            }
            template <class matcher, typename... Args>
            void to_not(Args&&... args)
            {
                matcher(true, actual, std::forward<Args>(args)...);
            }
        public:
            const T& actual;
        };
        class test
        {
        public:
            test(std::string_view description, std::vector<std::string_view> fargs, std::function<void()> test_function)
            {
                size_t current_arg = 0;

                for(size_t i = 0; i < description.size(); i++) {
                    const char& c = description[i];
                    switch (c)
                    {
                        case '{':
                            if(current_arg >= fargs.size()) {
                                throw std::runtime_error("too many arguments");
                            }

                            m_description += fargs[current_arg];
                            current_arg++;

                            if(i + 1 < description.size() && description[i + 1] == '}') {
                                i++;
                            } else {
                                throw std::runtime_error("expected '}'");
                            }
                        break;
                        default:
                            m_description.push_back(c);
                        break;
                    }
                }

                init(test_function);
            }

            test(std::string description, std::function<void()> test_function)
                : m_description(std::move(description))
            {
                init(test_function);
            }
        public:

        public:
            std::string m_description;
        protected:
            void init(std::function<void()> test_function)
            {
                test_result result;
                result.description = std::format("{}", m_description);
                
                try {
                    test_function();
                } catch (const std::exception& e) {
                    result.error_message = e.what();
                    result.passed = false;
                }

                for(size_t i = 0; i < tests::current_describe_level; i++) {
                    std::cout << "  ";
                }

                if(result.passed) {
                    uva::console::log_success("{}", m_description);
                } else {
                    uva::console::log_error("{}", m_description);
                }

                tests::result_list.push_back(result);
            }
        };
        class context_or_describe
        {
        public:
            context_or_describe(describe_what what, std::string_view description, std::function<void()> test_function)
                : m_what(what), m_description(description), m_test_function(test_function)
            {
                init();
            }
            context_or_describe(std::string_view description, std::function<void()> test_function)
                : m_what(describe_what::no_describe), m_description(description), m_test_function(test_function)
            {
                init();
            }
            context_or_describe(std::function<void()> test_function)
                : m_what(describe_what::no_describe), m_description("no description"), m_test_function(test_function)
            {
                init();
            }
        public:
            void init()
            {
                for(size_t i = 0; i < tests::current_describe_level; i++) {
                    std::cout << "  ";
                }

                if(m_what != describe_what::no_describe) {
                    std::cout << describe_what_strings[m_what] << " ";
                }

                for(const char& c : m_description) {
                    switch (c)
                    {
                    case '\n':
                        std::cout << "\\n";
                        break;
                    case '\r':
                        std::cout << "\\r";
                        break;
                    case '\t':
                        std::cout << "\\t";
                        break;
                    default:
                        std::cout << c;
                        break;
                    }
                }

                std::cout << std::endl;

                if(!tests::has_the_first_specification) {
                    tests::has_the_first_specification = true;
                    return;
                }

                tests::current_describe_level++;

                m_test_function();

                tests::current_describe_level--;
            }
        public:
            std::string_view m_description;
            std::function<void()> m_test_function;
            describe_what m_what;
        };
        using describe  = context_or_describe;
        using context   = context_or_describe;
        using it = test;
    }
};

#define given auto
#define subject(x) auto subject = x
#define its subject.

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)