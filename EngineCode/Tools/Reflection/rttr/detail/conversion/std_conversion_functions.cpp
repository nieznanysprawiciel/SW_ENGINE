/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014, 2015 - 2016 Axel Menzel <info@rttr.org>                     *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#include "rttr/detail/conversion/std_conversion_functions.h"

#include <sstream>
#include <locale>
#include <limits>
#include <algorithm>
#include <climits>
#include <iomanip>

namespace rttr
{
namespace detail
{

/////////////////////////////////////////////////////////////////////////////////////////

bool char_to_bool(const char* source, bool* ok)
{
    return string_to_bool(source, ok);
}

/////////////////////////////////////////////////////////////////////////////////////////

int char_to_int(const char* source, bool* ok)
{
    char *end_ptr;
    errno = 0;
    long n = strtol(source, &end_ptr, 0);
 
    if (ERANGE != errno && (n > INT_MIN && n < INT_MAX) &&
        end_ptr != source && *end_ptr == '\0')
    {
        if (ok)
            *ok = true;
        return static_cast<int>(n);
    }

    if (ok)
        *ok = false;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

long long char_to_long_long(const char* source, bool* ok)
{
    char *end_ptr;
    errno = 0;
    long long n = strtoll(source, &end_ptr, 0);
     if (errno != ERANGE && 
        end_ptr != source && 
        *end_ptr == '\0')
    {
        if (ok)
            *ok = true;
        return n;
    }

    if (ok)
        *ok = false;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

unsigned int char_to_uint(const char* source, bool* ok)
{
    char *end_ptr;
    errno = 0;
    long n = strtoul(source, &end_ptr, 0);
    if (errno != ERANGE && 
        end_ptr != source && 
        *end_ptr == '\0')
    {
        if (ok)
            *ok = true;
        return static_cast<unsigned int>(n);
    }

    if (ok)
        *ok = false;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

float char_to_float(const char* source, bool* ok)
{
    char *end_ptr;
    errno = 0;
    float n = strtof(source, &end_ptr);
 
    if (errno != ERANGE && 
        end_ptr != source && 
        *end_ptr == '\0')
    {
        if (ok)
            *ok = true;
        return n;
    }

    if (ok)
        *ok = false;
    return 0.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////

double char_to_double(const char* source, bool* ok)
{
    char *end_ptr;
    errno = 0;
    double n = strtod(source, &end_ptr);
 
    if (errno != ERANGE && 
        end_ptr != source && 
        *end_ptr == '\0')
    {
        if (ok)
            *ok = true;
        return n;
    }

    if (ok)
        *ok = false;
    return 0.0;
}

/////////////////////////////////////////////////////////////////////////////////////////

std::string int_to_string(int value, bool* ok)
{
    try
    {
        std::string text = std::to_string(value);
        if (ok)
            *ok = true;
        return text;
    }
    catch (...)
    {
        if (ok)
            *ok = false;
        return std::string();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

std::string float_to_string(float value, bool* ok)
{
    try
    {
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<float>::digits10) << value;
        if (ok)
            *ok = true;
        return ss.str();
    }
    catch (...)
    {
        if (ok)
            *ok = false;
        return std::string();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

std::string double_to_string(double value, bool* ok)
{
    try
    {
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<double>::digits10) << value;
        if (ok)
            *ok = true;
        return ss.str();
    }
    catch (...)
    {
        if (ok)
            *ok = false;
        return std::string();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

bool string_to_bool(std::string text, bool* ok)
{
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    text.erase( std::remove_if( text.begin(), text.end(), []( char ch ) { return std::isspace<char>( ch, std::locale::classic() ); } ), text.end() );

    if (text == "false" || text == "0" || text.empty())
    {
        if (ok)
            *ok = true;
        return false;
    }

    if (ok)
        *ok = true;
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

int string_to_int(const std::string& source, bool* ok)
{
    try
    {
        std::size_t pos = 0;
        const int value = std::stoi(source, &pos);
        if (pos == source.length())
        {
            if (ok)
                *ok = true;
            return value;
        }
    }
    catch (...)
    {
        if (ok)
            *ok = false;
        return 0;
    }

    if (ok)
        *ok = false;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

float string_to_float(const std::string& source, bool* ok)
{
    try
    {
        std::size_t pos = 0;
        const float value = std::stof(source, &pos);
        if (pos == source.length())
        {
            if (ok)
                *ok = true;
            return value;
        }
    }
    catch (...)
    {
        if (ok)
            *ok = false;
        return 0;
    }

    if (ok)
        *ok = false;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

double string_to_double(const std::string& source, bool* ok)
{
    try
    {
        std::size_t pos = 0;
        const double value = std::stod(source, &pos);
        if (pos == source.length())
        {
            if (ok)
                *ok = true;
            return value;
        }
    }
    catch (...)
    {
        if (ok)
            *ok = false;
        return 0;
    }

    if (ok)
        *ok = false;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

} // end namespace detail
} // end namespace rttr
