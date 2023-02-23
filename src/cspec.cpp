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
static size_t identation_size = 2;
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

uva::cspec::test_base::test_base(const std::string& __name)
    : name(__name)
{

}

void uva::cspec::core::run_tests()
{
    std::vector<uva::cspec::test_group*>& groups = uva::cspec::core::get_groups();

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

uva::cspec::test::test(const std::string& __name, const std::function<void()> body)
    : test_base(__name), m_body(body)
{
    tests.push_back(this);
}

void uva::cspec::test::do_test() const
{
   m_body();
}

bool uva::cspec::test_base::recursively_has_test(std::string& name, const uva::cspec::test_base* child) const
{
    for(const uva::cspec::test_base* test : tests) {
        if(child == test) {
            return true;
        }

        if(test->is_group && test->recursively_has_test(name, child)) {
            name += test->name;
            test->recursively_has_test(name, child);
            return true;
        }
    }

    return false;
}

void uva::cspec::test_base::recursively_traverse_name(std::string& name, const uva::cspec::test_base *child) const
{
    // const uva::cspec::test_base* test = this;
    // while(test->recursively_has_test(child))
    // {
    //     if(test == child) return;
    //     name += test->name;
    //     name.push_back(' ');

    //     //now the job is traverse to find the next node containing self
    //     for(const uva::cspec::test_base* __test : tests) {
    //         if(__test->recursively_has_test(this)) {
    //             recursively_traverse_name(name, child);
    //         }
    //     }

    //     break;
    // }
}

const uva::cspec::test_base *uva::cspec::test_base::recursively_find_test_parent(const test_base *test, const test_base *last) const
{
    if(is_group) {
        for(size_t i = 0; i < tests.size(); ++i) {
            const uva::cspec::test_base * found = tests[i]->recursively_find_test_parent(test, last);
            if(found) {
                if(last == this) {
                    return found;
                }
                return this;
            }
        }
    }
    else {
        if(this == test) {
            return this;
        }
    }

    return nullptr;
}

std::string uva::cspec::test::full_name()
{
   // uva::cspec::test_group* parent = nullptr;
    std::vector<uva::cspec::test_group*>& groups = uva::cspec::core::get_groups();

    std::string full_name;

    for(const uva::cspec::test_group* group : groups) {
        //if(group->recursively_has_test(full_name, this)) {
           // while(uva::cspec::test_base* parent = (group->recursively_find_test(this))) {
                //full_name += parent->name;
           // }
            //found the root test group with self
            //group->recursively_traverse_name(full_name, this);
            //break;
//}
            const uva::cspec::test_base* parent = nullptr;
            while(parent = group->recursively_find_test_parent(this, parent)) {
                full_name += parent->name;
            }
    }

    return full_name;
}

//TEST GROUP BEGIN

uva::cspec::test_group::test_group(const std::string& __name, const std::vector<uva::cspec::test_base*>& _tests, bool _is_root) 
    : test_base(__name)
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

std::vector<std::string> hierarchy;

std::string get_test_name_on_hierarchy(uva::cspec::test_base* test)
{
    std::string hierarchy_str = uva::string::join(hierarchy, ' ');
    hierarchy_str.push_back(' ');
    hierarchy_str += test->name;
    return hierarchy_str;
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
        std::cout << name << std::endl;
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

            hierarchy.push_back(test->name);
            test->do_test();
            hierarchy.pop_back();
        } else {
            examples++;
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
                error_message = std::format("{}\nTest resulted in following error:\n{}\n", get_test_name_on_hierarchy(__test), e.what());
            }
            catch(std::exception& e)
            {
                error_message = std::format("{}\nTest thrown following exception:\n{}\n", get_test_name_on_hierarchy(__test), e.what());
            }

            std::cout.clear();
            std::cerr.clear();
            std::clog.clear();

            print_identation(1);

            if(error_message.size()) {
                failures++;
                failed_messages.push_back(error_message);
                log_error(__test->name);
            } else {
                log_success(__test->name);
            }
        }
    }

    if(is_root) {
        std::cout << std::endl;
    }
    
    --identation_level;
}