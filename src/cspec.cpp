#include <sstream>

#include <console.hpp>
#include <diagnostics.hpp>

#include <cspec.hpp>

//EXTERN VARIABLES

std::filesystem::path uva::cspec::temp_folder;

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

    if(std::filesystem::exists(uva::cspec::temp_folder)) {
        //delete the temp directory
        std::filesystem::remove_all(uva::cspec::temp_folder);
    }

    //create the directory if it not exists
    if(!std::filesystem::create_directories(uva::cspec::temp_folder)) {
        std::cout << uva::console::color(uva::console::color_code::red) << std::format("Error: failed to create directory: {}", uva::cspec::temp_folder.string()) << std::endl;
        return;
    }

    auto elapsed = uva::diagnostics::measure_function([&]() {
        for(const auto& test_group : groups)
        {
            std::cout << test_group->m_name << std::endl;

            test_group->do_test([&](const uva::cspec::test* test)
            {
                examples++;

                try {
                    //disable cout to not print garbage while printing tests output
                    std::cout.setstate(std::ios_base::failbit);
                    std::cerr.setstate(std::ios_base::failbit);
                    test->m_body();
                    std::cout.clear();
                    std::cerr.clear();

                    std::cout << "\t" << uva::console::color(uva::console::color_code::green) << test->m_name << std::endl;
                } catch(uva::cspec::test_not_passed& e)
                {
                    std::cout.clear();
                    std::cerr.clear();
                    failures++;
                    std::cout << "\t" << uva::console::color(uva::console::color_code::red) << test->m_name << std::endl;

                    failed_messages.push_back(std::format("{} {}\nTest resulted in following error:\n{}", test_group->m_name, test->m_name, e.what()));
                }
                catch(std::exception& e)
                {
                    std::cout.clear();
                    std::cerr.clear();
                    failures++;
                    std::cout << "\t" << uva::console::color(uva::console::color_code::red) << test->m_name << std::endl;

                    failed_messages.push_back(std::format("{} {}\nTest thrown following exception:\n{}", test_group->m_name, test->m_name, e.what()));
                }
            });

            std::cout << std::endl;
        }
    });

    for(const std::string& message : failed_messages)
    {
        std::cout << uva::console::color(uva::console::color_code::red) << message << std::endl;
    }

    std::cout << std::endl;
    std::cout << failures << " failures / " << examples << " examples." << std::endl;
    std::cout << "Tests took " << std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() << "s" << std::endl;
}

//CORE END

//TEST BEGIN

//TEST END

uva::cspec::test::test(const std::string& name, const std::function<void()> body)
    : m_name(name), m_body(body)
{
    tests.push_back(this);
}

void uva::cspec::test::do_test(std::function<void(const uva::cspec::test*)> _body) const
{
   _body(this);
}

//TEST GROUP BEGIN

uva::cspec::test_group::test_group(const std::string& name, const std::vector<uva::cspec::test_base*>& _tests, bool _is_root) 
    : m_name(name)
{
    if(_is_root) {
        std::vector<uva::cspec::test_group*>& groups = uva::cspec::core::get_groups();
        groups.push_back(this);
    }

    std::vector<uva::cspec::test_base*> tests_removed_befores = uva::string::select(_tests, [this](uva::cspec::test_base* test){
        if(test->is_before_all) {
            m_beforeAll = (uva::cspec::before_all*)test;
        }
        if(test->is_before_each) {
            m_beforeEach = (uva::cspec::before_each*)test;
        }
        return !test->is_before_all && !test->is_before_each;
    });

    tests.insert(tests.begin(), tests_removed_befores.begin(), tests_removed_befores.end());

    is_group = true;
    is_root = _is_root;
}

void uva::cspec::test_group::do_test(std::function<void(const uva::cspec::test*)> _body) const
{
    if(m_beforeAll) {
        m_beforeAll->body();
    }
    for(const uva::cspec::test_base* test : tests)
    {
        if(is_group && !is_root) {
            std::cout << "\t" << m_name << std::endl;
            std::cout << "\t";

            if(m_beforeEach) {
                m_beforeEach->body((uva::cspec::test*)test);
            }
        }

        test->do_test(_body);
    }
}