#include <sstream>

#include <console.hpp>
#include <diagnostics.hpp>

#include <cspec.hpp>

using namespace uva;
using namespace console;

//EXTERN VARIABLES

std::filesystem::path uva::cspec::temp_folder;

std::vector<std::string> failed_messages;

static size_t examples = 0;
static size_t failures = 0;
static size_t identation_size = 4;
static size_t identation_level = 0;

cspec::test_group* last_group = nullptr;

//CSPEC BEGIN

#ifdef UVA_DATABASE_AVAILABLE
void uva::cspec::puts(const uva::database::basic_active_record& output)
{
    puts(output.to_s());
}
#endif

void uva::cspec::puts(const std::string& output)
{
    std::cout.clear();
    std::cerr.clear();
    std::clog.clear();

    std::cout << output << std::endl;

    std::cout.setstate(std::ios_base::failbit);
    std::cerr.setstate(std::ios_base::failbit);
    std::clog.setstate(std::ios_base::failbit);
}

//CORE BEGIN

std::vector<uva::cspec::test_group*>& uva::cspec::core::get_groups()
{
    static std::vector<uva::cspec::test_group*> s_tests_groups;
    return s_tests_groups;
}

void uva::cspec::core::run_tests()
{
    std::vector<uva::cspec::test_group*> groups = uva::cspec::core::get_groups();

    examples = 0;
    failures = 0;
    failed_messages.clear();

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
            test_group->do_test();
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

void uva::cspec::test::do_test() const
{
   m_body();
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
        if(!test) return false;
        
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

std::string identation;
void print_identation(size_t add = 0)
{
    size_t current_identation_size = identation_size*(identation_level+add);
    if(identation.size() != current_identation_size) {
        identation.resize(current_identation_size, ' ');
    }

    std::cout << identation;
}

void uva::cspec::test_group::do_test() const
{
    if(m_beforeAll) {
        m_beforeAll->body();
    }

    if(is_root) {
        identation_level = 0;
    }

    if(is_group) {
        print_identation();
        std::cout << m_name << std::endl;
    }

    for(const uva::cspec::test_base* test : tests)
    {
        if(is_group) {
            if(m_beforeEach) {
                m_beforeEach->body((cspec::test*)test);
            }
        }

        if(test->is_group) {
            //recurse
            ++identation_level;
            test->do_test();
        } else {
            //execute test
            //disable cout to not print garbage while printing tests output
            std::cout.setstate(std::ios_base::failbit);
            std::cerr.setstate(std::ios_base::failbit);
            std::clog.setstate(std::ios_base::failbit);

            cspec::test* __test = ((cspec::test*)test);

            std::string error_message;
            try {
                __test->m_body();
            } catch(uva::cspec::test_not_passed& e)
            {
                failures++;
                error_message = std::format("{}\nTest resulted in following error:\n{}", __test->m_name, e.what());
            }
            catch(std::exception& e)
            {
                failures++;
                error_message = std::format("{}\nTest thrown following exception:\n{}", __test->m_name, e.what());
            }

            std::cout.clear();
            std::cerr.clear();
            std::clog.clear();

            print_identation(1);

            if(error_message.size()) {
                failed_messages.push_back(error_message);
                log_error(__test->m_name);
            } else {
                log_success(__test->m_name);
            }
        }
    }

    if(is_root) {
        std::cout << std::endl;
    }
    
    --identation_level;
}