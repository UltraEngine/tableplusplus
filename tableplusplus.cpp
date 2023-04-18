/*----------------------------------------------------------------------------------------

table++

by Josh Klint
https://www.ultraengine.com

LICENSE

This code may be used freely for any purpose.

If you use this code you must include this unmodified README file somewhere in your application directory or subdirectory where it is accessible to the end user.

This unmodified readme file must be included in any source code distribution, or distribution of code derived from this code.

You may not misrepresent the origin of this code.

You may not use this code in AI training models.

----------------------------------------------------------------------------------------*/

#include "tableplusplus.h"

namespace tableplusplus
{
    table::table()
    {
        i = 0;
        f = 0;
        b = false;
        t = TABLE_OBJECT;
        m();
    }

    table::~table()
    {
        t = TABLE_INVALID;
        _m = nullptr;
    }

    std::shared_ptr<std::map<tableKey, table> > table::m()
    {
        if (_m == nullptr) _m = std::make_shared<std::map<tableKey, table> >();
        return _m;
    }

    bool table::operator==(const table& o) const
    {
        if (t != o.t)
        {
            if (t == TABLE_INTEGER || t == TABLE_FLOAT)
            {
                if (t == TABLE_INTEGER || t == TABLE_FLOAT)
                {
                    return double(*this) == double(o);
                }
            }
            return false;
        }
        switch (t)
        {
        case TABLE_NULL:
            return true;
            break;
        case TABLE_BOOLEAN:
            return b == o.b;
            break;
        case TABLE_INTEGER:
            return i == o.i;
            break;
        case TABLE_STRING:
            return s == o.s;
            break;
        case TABLE_OBJECT:
            return _m == o._m;
            break;
        }
        return false;
    }
    
    bool table::operator!=(const table& o) const
    {
        return !((*this) == o);
    }

    void table::clear()
    {
        t = TABLE_NULL;
        i = 0;
        f = 0;
        b = false;
        s.clear();
        //_m = nullptr;
        //_v = nullptr;
    }

    size_t table::size()
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        if (_m == nullptr) return 0;
        size_t sz = 0;
        while (true)
        {
            if (m()->find(sz) == m()->end()) break;
            ++sz;
        }
        return sz;
    }

    void table::push_back(const table& j3)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        auto sz = size();
        m()->insert_or_assign(sz, j3);
    }

    void table::resize(const size_t sz)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        auto current = size();
        if (current == sz) return;

        //TODO: Get rid of negative indexes

        //Remove indexes beyond the max
        if (sz < current)
        {
            auto it = m()->lower_bound(sz);
            while (it != m()->end())
            {
                if (it->first.t != tableKey::KeyType::KEY_INDEX) break;// index keys are ordered first so this is fine
                if (it->first.i >= sz)
                {
                    it = m()->erase(it);
                    continue;
                }
                ++it;
            }
        }

        //Fill in missing indexes
        for (size_t n = 0; n < sz; ++n)
        {
            if (m()->find(n) == m()->end()) m()->insert_or_assign(n, nullptr);
        }
    }

    table& table::operator[](const char* c)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        return (*this)[std::string(c)];
    }

    table& table::operator[](const std::string& key)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        if (m()->find(key) == m()->end())
        {
            m()->insert_or_assign(key, nullptr);
        }
        auto it = m()->find(key);
        return it->second;
    }

    table& table::operator[](const int key)
    {
        return (*this)[size_t(key)];
    }

    table& table::operator[](const size_t key)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        if (m()->find(key) == m()->end())
        {
            m()->insert_or_assign(key, nullptr);
        }
        auto it = m()->find(key);
        return it->second;
    }

    std::string table::to_json(const std::string indent)
    {
        auto type = GetType();
        if (type != TABLE_OBJECT)
        {
            if (type == TABLE_NULL) return indent + "null";
            return indent + std::string(*this);
        }
        std::string j3;
        bool isarray = size() == m()->size();
        if (isarray)
        {
            j3 += indent + "[\n";
            auto count = size();
            for (int n = 0; n < count; ++n)
            {
                j3 += (*this)[n].to_json(indent + "	");
                if (n != count - 1) j3 += ",";
                j3 += "\n";
            }
            j3 += indent + "]";
        }
        else
        {
            j3 += indent + "{\n";
            int n = 0;
            int count = m()->size();
            for (auto& pair : *this)
            {
                j3 += indent + "	\"" + std::string(pair.first) + "\":";
                if (pair.second.GetType() == TABLE_OBJECT)
                {
                    j3 += "\n";
                    j3 += pair.second.to_json(indent + "	");
                }
                else
                {
                    j3 += " " + pair.second.to_json();
                }
                if (n != count - 1) j3 += ",";
                j3 += "\n";
                ++n;
            }
            j3 += indent + "}";
        }
        return j3;
    }

#ifdef NLOHMANN_JSON_VERSION_MAJOR

    table::table(const nlohmann::json& j3) : i(0), f(0), b(false), t(TABLE_NULL)
    {
        if (j3.is_array())
        {
            t = TABLE_OBJECT;
            for (int n = 0; n < j3.size(); ++n)
            {
                m()->insert_or_assign(n, table(j3[n]));
            }
        }
        else if (j3.is_object())
        {
            t = TABLE_OBJECT;
            for (const auto& pair : j3.items())
            {
                m()->insert_or_assign(pair.key(), table(pair.value()));
            }
        }
        else if (j3.is_string())
        {
            t = TABLE_STRING;
            s = j3;
        }
        else if (j3.is_boolean())
        {
            t = TABLE_BOOLEAN;
            b = j3;
        }
        else if (j3.is_number_float())
        {
            t = TABLE_FLOAT;
            f = j3;
        }
        else if (j3.is_number_integer() || j3.is_number_unsigned())
        {
            t = TABLE_INTEGER;
            i = j3;
        }
    }

#endif

#ifdef SOL_VERSION

    void table::dynamic_sets(const std::string& key, const sol::object& value)
    {
        dynamic_set(tableKey(key), value);
    }

    void table::dynamic_seti(const int key, const sol::object& value)
    {
        dynamic_set(tableKey(key - 1), value);
    }

    void table::dynamic_set(const tableKey& key, const sol::object& value)
    {
        double f;
        switch (value.get_type())
        {
        case sol::type::number:
            f = value.as<double>();
            m()->insert_or_assign(key, f);
            break;
        case sol::type::string:
            m()->insert_or_assign(key, value.as<std::string>());
            break;
        case sol::type::boolean:
            m()->insert_or_assign(key, value.as<bool>());
            break;
        case sol::type::userdata:
            if (value.is<tablewrapper>())
            {
                auto tbl = value.as<tablewrapper*>();
                m()->insert_or_assign(key, tbl->totable());
            }
            else
            {
                throw(std::runtime_error("userdata must be a C++ table"));
            }
            break;
        case sol::type::none:
            f = 4;
            break;
        case sol::type::nil:
            m()->insert_or_assign(key, nullptr);
            break;
        case sol::type::table:
            throw(std::runtime_error("cannot assign a Lua table to a C++ table"));
            break;
        default:
            throw(std::runtime_error("assigning illegal type to C++ table"));
            break;
        }
    }

    sol::object table::dynamic_geti(sol::this_state L, const int key)
    {
        if (key < 1) return sol::lua_nil;
        return dynamic_get(L, tableKey(key - 1));
    }

    sol::object table::dynamic_gets(sol::this_state L, const std::string& key)
    {
        return dynamic_get(L, tableKey(key));
    }

    sol::object table::dynamic_get(sol::this_state L, const tableKey& key)
    {
        auto sz = m()->size();
        auto it = m()->find(key);
        if (it == m()->end()) return sol::make_object(L, sol::lua_nil);
        switch (it->second.t)
        {
        case TABLE_FLOAT:
            return sol::make_object(L, it->second.f);
            break;
        case TABLE_INTEGER:
            return sol::make_object(L, it->second.i);
            break;
        case TABLE_BOOLEAN:
            return sol::make_object(L, it->second.b);
            break;
        case TABLE_STRING:
            return sol::make_object(L, it->second.s);
            break;
        case TABLE_OBJECT:
            return sol::make_object(L, tablewrapper(it->second));
            break;
        case TABLE_NULL:
        default:
            return sol::make_object(L, sol::lua_nil);
            break;
        }
        return sol::lua_nil;
    }

    void bind_table_plus_plus(sol::state* L)
    {
        L->new_usertype<tablekeywrapper>("tableplusplus_tablekey",
            sol::meta_method::type, [](const tablekeywrapper& v)
            {
                if (v.t == TABLE_OBJECT) return "userdata";
                if (v.t == TABLE_NULL) return "nil";
                if (v.t == TABLE_INTEGER || v.t == TABLE_FLOAT) return "number";
                if (v.t == TABLE_BOOLEAN) return "boolean";
                return "userdata";
            },
            sol::meta_function::to_string, [](const tablekeywrapper& v)
            {
           //     if (v.t == tableKey::KeyType::KEY_INDEX) throw(std::runtime_error("value is not a string."));
                if (v.t == tableKey::KeyType::KEY_STRING) return v.s;
                return std::string("");
            },
            sol::meta_function::concatenation, [](const tablekeywrapper& v, std::string s)
            {
            //    if (v.t == tableKey::KeyType::KEY_INDEX) throw(std::runtime_error("value is not a string."));
                std::string ss;
                if (v.t == tableKey::KeyType::KEY_STRING) ss = v.s;
                return ss + s;
            }
            );

        L->new_usertype<tablewrapper>("tableplusplus_table",
            sol::meta_function::pairs, &IDKWTFLOL::my_pairs,
            sol::meta_function::to_string, [](const tablewrapper& v) { std::string s = v; return s; },
            sol::meta_method::equal_to, [](const tablewrapper& a, const tablewrapper& b) { return a == b; },
            sol::meta_method::new_index, sol::overload(
                [](tablewrapper& t, int k, sol::object o) { t.dynamic_seti(k, o); },
                [](tablewrapper& t, std::string k, sol::object o) { t.dynamic_sets(k, o); }
            ),
            sol::meta_method::index, sol::overload(
                [](sol::this_state L, tablewrapper& t, int k) { return t.dynamic_geti(L, k); },
                [](sol::this_state L, tablewrapper& t, std::string k) { return t.dynamic_gets(L, k); }
            )
        );
        L->set_function("ctable", []() { return tablewrapper(table()); });
    }
#endif
}
