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
