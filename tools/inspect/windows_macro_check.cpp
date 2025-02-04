//  windows_macro_check implementation  ------------------------------------------------//
//  Copyright Ste||ar Group

//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <pika/config.hpp>
#include <pika/modules/filesystem.hpp>

#include <functional>
#include <string>
#include "boost/regex.hpp"
#include "function_hyper.hpp"
#include "windows_macro_check.hpp"

namespace fs = pika::filesystem;

namespace {
    boost::regex windows_macro_regex(
        "("
        "^\\s*#\\s*undef\\s*"    // # undef
        "\\b(small)\\b"          // followed by windows macro name, whole word
        ")"
        "|"    // or (ignored)
        "("
        "//[^\\n]*"    // single line comments (//)
        "|"
        "/\\*.*?\\*/"    // multi line comments (/**/)
        "|"
        "\"(?:\\\\\\\\|\\\\\"|[^\"])*\""    // string literals
        ")"
        "|"    // or
        "("
        "\\b(small)\\b"    // windows macro name, whole word
        ")",
        boost::regex::normal);

}    // unnamed namespace

namespace boost { namespace inspect {
    windows_macro_check::windows_macro_check()
      : m_files_with_errors(0)
    {
        register_signature(".c");
        register_signature(".cpp");
        register_signature(".cu");
        register_signature(".cxx");
        register_signature(".h");
        register_signature(".hpp");
        register_signature(".hxx");
        register_signature(".ipp");
    }

    void windows_macro_check::inspect(const string& library_name,
        const path& full_path,     // example: c:/foo/boost/filesystem/path.hpp
        const string& contents)    // contents of file to be inspected
    {
        if (contents.find("pikainspect:"
                          "nowindows_macros") != string::npos)
            return;

        boost::sregex_iterator cur(
            contents.begin(), contents.end(), windows_macro_regex),
            end;

        long errors = 0;

        for (; cur != end; ++cur /*, ++m_files_with_errors*/)
        {
            if (!(*cur)[3].matched)
            {
                string::const_iterator it = contents.begin();
                string::const_iterator match_it = (*cur)[0].first;

                string::const_iterator line_start = it;

                string::size_type line_number = 1;
                for (; it != match_it; ++it)
                {
                    if (string::traits_type::eq(*it, '\n'))
                    {
                        ++line_number;
                        line_start = it + 1;    // could be end()
                    }
                }
                ++errors;
                error(library_name, full_path,
                    "Windows macro clash: " +
                        std::string((*cur)[0].first, (*cur)[0].second - 1),
                    line_number);
            }
        }
        if (errors > 0)
        {
            ++m_files_with_errors;
        }
    }
}}    // namespace boost::inspect
