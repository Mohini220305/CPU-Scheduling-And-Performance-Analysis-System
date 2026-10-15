#ifndef JSON_H
#define JSON_H

#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

// ---------- writing ----------
inline std::string jsonString(const std::string &s)
{
    std::string out = "\"";
    for (size_t i = 0; i < s.size(); i++)
    {
        unsigned char c = (unsigned char)s[i];
        switch (c)
        {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (c < 0x20)
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            }
            else
            {
                out += (char)c;
            }
        }
    }
    out += "\"";
    return out;
}

// ---------- reading ----------
struct JsonValue
{
    enum Type
    {
        NUL,
        BOOL,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    };
    Type type = NUL;
    bool boolean = false;
    double number = 0;
    std::string text;
    std::vector<JsonValue> items;                           // ARRAY
    std::vector<std::pair<std::string, JsonValue>> members; // OBJECT

    bool isObject() const { return type == OBJECT; }
    bool isArray() const { return type == ARRAY; }
    bool isString() const { return type == STRING; }
    bool isNumber() const { return type == NUMBER; }

    // returns the member called 'key' or nullptr
    const JsonValue *get(const std::string &key) const
    {
        for (size_t i = 0; i < members.size(); i++)
            if (members[i].first == key)
                return &members[i].second;
        return nullptr;
    }
};

class JsonParser
{
public:
    explicit JsonParser(const std::string &input) : s(input), pos(0) {}

    bool parse(JsonValue &out, std::string &error)
    {
        skipSpaces();
        if (!parseValue(out, error))
            return false;
        skipSpaces();
        if (pos != s.size())
        {
            error = "Unexpected characters after the JSON data";
            return false;
        }
        return true;
    }

private:
    std::string s;
    size_t pos;

    void skipSpaces()
    {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
            pos++;
    }

    bool fail(std::string &error, const std::string &msg)
    {
        error = msg;
        return false;
    }

    bool parseValue(JsonValue &out, std::string &error)
    {
        if (pos >= s.size())
            return fail(error, "Unexpected end of JSON data");
        char c = s[pos];
        if (c == '{')
            return parseObject(out, error);
        if (c == '[')
            return parseArray(out, error);
        if (c == '"')
        {
            out.type = JsonValue::STRING;
            return parseString(out.text, error);
        }
        if (s.compare(pos, 4, "true") == 0)
        {
            pos += 4;
            out.type = JsonValue::BOOL;
            out.boolean = true;
            return true;
        }
        if (s.compare(pos, 5, "false") == 0)
        {
            pos += 5;
            out.type = JsonValue::BOOL;
            out.boolean = false;
            return true;
        }
        if (s.compare(pos, 4, "null") == 0)
        {
            pos += 4;
            out.type = JsonValue::NUL;
            return true;
        }
        return parseNumber(out, error);
    }

    bool parseNumber(JsonValue &out, std::string &error)
    {
        const char *start = s.c_str() + pos;
        char *end = nullptr;
        double v = std::strtod(start, &end);
        if (end == start)
            return fail(error, "Invalid JSON value");
        pos += (size_t)(end - start);
        out.type = JsonValue::NUMBER;
        out.number = v;
        return true;
    }

    static void appendUtf8(std::string &out, unsigned int cp)
    {
        if (cp < 0x80)
        {
            out += (char)cp;
        }
        else if (cp < 0x800)
        {
            out += (char)(0xC0 | (cp >> 6));
            out += (char)(0x80 | (cp & 0x3F));
        }
        else if (cp < 0x10000)
        {
            out += (char)(0xE0 | (cp >> 12));
            out += (char)(0x80 | ((cp >> 6) & 0x3F));
            out += (char)(0x80 | (cp & 0x3F));
        }
        else
        {
            out += (char)(0xF0 | (cp >> 18));
            out += (char)(0x80 | ((cp >> 12) & 0x3F));
            out += (char)(0x80 | ((cp >> 6) & 0x3F));
            out += (char)(0x80 | (cp & 0x3F));
        }
    }

    bool parseHex4(unsigned int &value, std::string &error)
    {
        if (pos + 4 > s.size())
            return fail(error, "Invalid \\u escape in JSON string");
        value = 0;
        for (int i = 0; i < 4; i++)
        {
            char h = s[pos++];
            value <<= 4;
            if (h >= '0' && h <= '9')
                value |= (unsigned)(h - '0');
            else if (h >= 'a' && h <= 'f')
                value |= (unsigned)(h - 'a' + 10);
            else if (h >= 'A' && h <= 'F')
                value |= (unsigned)(h - 'A' + 10);
            else
                return fail(error, "Invalid \\u escape in JSON string");
        }
        return true;
    }

    bool parseString(std::string &out, std::string &error)
    {
        pos++; // opening quote
        out.clear();
        while (pos < s.size())
        {
            char c = s[pos++];
            if (c == '"')
                return true;
            if (c != '\\')
            {
                out += c;
                continue;
            }
            if (pos >= s.size())
                break;
            char e = s[pos++];
            switch (e)
            {
            case '"':
                out += '"';
                break;
            case '\\':
                out += '\\';
                break;
            case '/':
                out += '/';
                break;
            case 'b':
                out += '\b';
                break;
            case 'f':
                out += '\f';
                break;
            case 'n':
                out += '\n';
                break;
            case 'r':
                out += '\r';
                break;
            case 't':
                out += '\t';
                break;
            case 'u':
            {
                unsigned int cp = 0;
                if (!parseHex4(cp, error))
                    return false;
                if (cp >= 0xD800 && cp <= 0xDBFF && pos + 1 < s.size() && s[pos] == '\\' && s[pos + 1] == 'u')
                {
                    pos += 2;
                    unsigned int low = 0;
                    if (!parseHex4(low, error))
                        return false;
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                }
                appendUtf8(out, cp);
                break;
            }
            default:
                return fail(error, "Invalid escape in JSON string");
            }
        }
        return fail(error, "Unterminated JSON string");
    }

    bool parseArray(JsonValue &out, std::string &error)
    {
        pos++; // '['
        out.type = JsonValue::ARRAY;
        skipSpaces();
        if (pos < s.size() && s[pos] == ']')
        {
            pos++;
            return true;
        }
        while (true)
        {
            skipSpaces();
            JsonValue item;
            if (!parseValue(item, error))
                return false;
            out.items.push_back(item);
            skipSpaces();
            if (pos >= s.size())
                return fail(error, "Unterminated JSON array");
            if (s[pos] == ',')
            {
                pos++;
                continue;
            }
            if (s[pos] == ']')
            {
                pos++;
                return true;
            }
            return fail(error, "Expected ',' or ']' in JSON array");
        }
    }

    bool parseObject(JsonValue &out, std::string &error)
    {
        pos++; // '{'
        out.type = JsonValue::OBJECT;
        skipSpaces();
        if (pos < s.size() && s[pos] == '}')
        {
            pos++;
            return true;
        }
        while (true)
        {
            skipSpaces();
            if (pos >= s.size() || s[pos] != '"')
                return fail(error, "Expected a string key in JSON object");
            std::string key;
            if (!parseString(key, error))
                return false;
            skipSpaces();
            if (pos >= s.size() || s[pos] != ':')
                return fail(error, "Expected ':' in JSON object");
            pos++;
            skipSpaces();
            JsonValue value;
            if (!parseValue(value, error))
                return false;
            out.members.push_back(std::make_pair(key, value));
            skipSpaces();
            if (pos >= s.size())
                return fail(error, "Unterminated JSON object");
            if (s[pos] == ',')
            {
                pos++;
                continue;
            }
            if (s[pos] == '}')
            {
                pos++;
                return true;
            }
            return fail(error, "Expected ',' or '}' in JSON object");
        }
    }
};

#endif
