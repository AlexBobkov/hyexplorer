/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <QString>

#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>

class CsvRow
{
public:
    CsvRow(const std::string& input)
    {
        std::size_t offset = 0;
        while (offset < input.size())
        {
            std::size_t pos = 0;

            //Все, что внутри двойных кавычек считаем за одно поле
            if (input[offset] == '"')
            {
                pos = input.find('"', offset + 1);
                pos++;
            }
            else
            {
                pos = input.find(',', offset);
            }

            if (pos >= input.size() || pos == std::string::npos)
            {
                break;
            }

            if (input[offset] == '"')
            {
                _tokens.push_back(input.substr(offset + 1, pos - offset - 2));
            }
            else
            {
                _tokens.push_back(input.substr(offset, pos - offset));
            }
            offset = pos + 1;
        }

        if (input[offset] == '"')
        {
            _tokens.push_back(input.substr(offset + 1, input.size() - offset - 2));
        }
        else
        {
            _tokens.push_back(input.substr(offset, input.size() - offset));
        }
    }

    std::size_t size() const
    {
        return _tokens.size();
    }

    template<typename T>
    T as(std::size_t column) const
    {
        assert(column < _tokens.size());
        assert(_tokens[column].size() > 0);

        return boost::lexical_cast<T>(_tokens[column]);
    }

    template<>
    std::string as<std::string>(std::size_t column) const
    {
        assert(column < _tokens.size());

        std::string str = _tokens[column];

        //Заменяем одинарные кавычки на двойные
        std::size_t offset = 0;
        std::size_t pos = 0;
        while (offset < str.size() && (pos = str.find('\'', offset)) != std::string::npos)
        {
            str.insert(pos, 1, '\'');
            offset = pos + 2;
        }

        return str;
    }
        
    template<>
    QString as<QString>(std::size_t column) const
    {
        assert(column < _tokens.size());

        return QString::fromUtf8(as<std::string>(column).c_str());
    }

protected:
    std::vector<std::string> _tokens;
};
