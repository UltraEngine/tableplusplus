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
#ifdef SOL_VERSION
    std::map<int, table> table::copiedluatables;
#endif

    table::table()
    {
        i = 0;
        f = 0;
        b = false;
        t = type::object;
        m();
    }

    table::~table()
    {
        t = type::null;
        _m = nullptr;
    }

    bool table::is_string()
    {
        return t == type::string;
    }

    bool table::is_object()
    {
        return t == type::object;
    }

    bool table::is_number()
    {
        return t == type::number_integer || t == type::number_float;
    }

    bool table::is_boolean()
    {
        return t == type::boolean;
    }

    bool table::is_null()
    {
        return t == type::null;
    }

    bool table::is_integer()
    {
        return t == type::number_integer;
    }

    bool table::is_float()
    {
        return t == type::number_float;
    }

    bool table::is_array()
    {
        if (find(0) == end()) return false;
        return (size() == m()->size()) && !m()->empty();
    }

    std::map<tablekey, table>::iterator table::erase(const std::map<tablekey, table>::iterator& it)
    {
        return m()->erase(it);
    }

    std::map<tablekey, table>::iterator table::find(const int key)
    {
        return m()->find(key);
    }

    std::map<tablekey, table>::iterator table::find(const std::string& key)
    {
        return m()->find(key);
    }

    std::shared_ptr<std::map<tablekey, table> > table::m()
    {
        if (_m == nullptr) _m = std::make_shared<std::map<tablekey, table> >();
        return _m;
    }

    bool table::operator==(const table& o) const
    {
        if (t != o.t)
        {
            if (t == type::number_integer || t == type::number_float)
            {
                if (t == type::number_integer || t == type::number_float)
                {
                    return double(*this) == double(o);
                }
            }
            return false;
        }
        switch (t)
        {
        case type::null:
            return true;
            break;
        case type::boolean:
            return b == o.b;
            break;
        case type::number_integer:
            return i == o.i;
            break;
        case type::string:
            return s == o.s;
            break;
        case type::object:
            return _m == o._m;
            break;
        }
        return false;
    }
    
    bool table::operator!=(const table& o) const
    {
        return !((*this) == o);
    }

    table table::copy(const bool recursive)
    {
        table tbl = *this;
        tbl._m = nullptr;
        if (t == type::object && recursive == true)
        {
            tbl.m();
            for (auto it = m()->begin(); it != m()->end(); ++it)
            {
                tbl.m()->insert_or_assign(it->first, it->second.copy());
            }
        }
        return tbl;
    }

    void table::clear()
    {
        t = type::null;
        i = 0;
        f = 0;
        b = false;
        s.clear();
        //_m = nullptr;
        //_v = nullptr;
    }

    size_t table::size()
    {
        if (t != type::object) throw std::runtime_error("Table value is not an object.");
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
        if (t != type::object) throw std::runtime_error("Table value is not an object.");
        auto sz = size();
        m()->insert_or_assign(sz, j3);
    }

    void table::resize(const size_t sz)
    {
        if (t != type::object) throw std::runtime_error("Table value is not an object.");
        auto current = size();
        if (current == sz) return;

        //TODO: Get rid of negative indexes

        //Remove indexes beyond the max
        if (sz < current)
        {
            auto it = m()->lower_bound(sz);
            while (it != m()->end())
            {
                if (it->first.t != tablekey::type::index) break;// index keys are ordered first so this is fine
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
        return (*this)[std::string(c)];
    }

    table& table::operator[](const std::string& key)
    {
        if (t == type::null) t = type::object;
        if (t != type::object) throw std::runtime_error("Table value is not an object.");
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
        if (t == type::null) t = type::object;
        if (t != type::object) throw std::runtime_error("Table value is not an object.");
        if (m()->find(key) == m()->end())
        {
            m()->insert_or_assign(key, nullptr);
        }
        auto it = m()->find(key);
        return it->second;
    }

    bool table::empty()
    {
        if (t != type::object) return true;
        if (m()->empty()) return true;

        return false;
    }

    std::string table::to_json(const std::string indent)
    {
        auto type = get_type();
        if (type != type::object)
        {
            if (type == type::null) return indent + "null";
            return indent + std::string(*this);
        }
        std::string j3;
        if (is_array())
        {
            j3 += indent + "[\n";
            auto count = size();
            for (int n = 0; n < count; ++n)
            {
                if ((*this)[n].t == type::object && (*this)[n].empty()) continue;
                j3 += (*this)[n].to_json(indent + "	");
                if (n != count - 1) j3 += ",";
                j3 += "\n";
            }
            j3 += indent + "]";
        }
        else
        {
            int n = 0;
            int count = m()->size();
            std::vector<std::pair<tablekey, table> > pairs;
            pairs.reserve(m()->size());
            for (auto& pair : *this)
            {
                if (pair.second.is_null()) continue;
                if (pair.second.is_object() && pair.second.empty()) continue;
                pairs.push_back(pair);
            }
            if (pairs.empty()) return "null";
            j3 += indent + "{\n";
            for (size_t n = 0; n < pairs.size(); ++n)
            {
                j3 += indent + "	\"" + std::string(pairs[n].first) + "\":";
                if (pairs[n].second.get_type() == type::object)
                {
                    j3 += "\n";
                    j3 += pairs[n].second.to_json(indent + "	");
                }
                else
                {
                    j3 += " " + pairs[n].second.to_json();
                }
                if (n != pairs.size() - 1) j3 += ",";
                j3 += "\n";
            }
            j3 += indent + "}";
        }        
        return j3;
    }

#ifdef NLOHMANN_JSON_VERSION_MAJOR

    table::operator nlohmann::json()
    {
        auto data = to_json();
        return nlohmann::json::parse(data);
    }

    table::table(const nlohmann::json& j3) : i(0), f(0), b(false), t(type::null)
    {
        if (j3.is_array())
        {
            t = type::object;
            for (int n = 0; n < j3.size(); ++n)
            {
                m()->insert_or_assign(n, table(j3[n]));
            }
        }
        else if (j3.is_object())
        {
            t = type::object;
            for (const auto& pair : j3.items())
            {
                m()->insert_or_assign(pair.key(), table(pair.value()));
            }
        }
        else if (j3.is_string())
        {
            t = type::string;
            s = j3;
        }
        else if (j3.is_boolean())
        {
            t = type::boolean;
            b = j3;
        }
        else if (j3.is_number_float())
        {
            t = type::number_float;
            f = j3;
        }
        else if (j3.is_number_integer() || j3.is_number_unsigned())
        {
            t = type::number_integer;
            i = j3;
        }
    }

#endif

#ifdef SOL_VERSION

    void table::dynamic_sets(const std::string& key, const sol::object& value)
    {
        dynamic_set(tablekey(key), value);
    }

    void table::dynamic_seti(const int key, const sol::object& value)
    {
        dynamic_set(tablekey(key - 1), value);
    }
     
    void table::dynamic_set(const tablekey& key, const sol::object& value)
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
            {
                copiedluatables.clear();
                m()->insert_or_assign(key, table(value.as<sol::table>(), true));
                copiedluatables.clear();
            }
            //throw(std::runtime_error("cannot assign a Lua table to a C++ table"));
            break;
        default:
            throw(std::runtime_error("assigning illegal type to C++ table"));
            break;
        }
    }

    table::table(const sol::table& tbl)
    {
        copiedluatables.clear();
        *this = table(tbl, true);
        copiedluatables.clear();
    }

    table::table(const sol::table& tbl, const bool handleduplicates)
    {
        if (handleduplicates)
        {
            if (copiedluatables.find(tbl.registry_index()) != copiedluatables.end())
            {
                *this = copiedluatables[tbl.registry_index()];
                return;
            }            
            copiedluatables[tbl.registry_index()] = *this;
        }

        t = type::object;
        m();
        for (const auto& pair : tbl)
        {
            tablekey key;
            switch (pair.first.get_type())
            {
            case sol::type::string:
                key = pair.first.as<std::string>();
                break;
            case sol::type::number:
                key = pair.first.as<int>();
                break;
            default:
                continue;
            }
            switch (pair.second.get_type())
            {
            case sol::type::number:
                m()->insert_or_assign(key, pair.second.as<double>());
                break;
            case sol::type::string:
                m()->insert_or_assign(key, pair.second.as<std::string>());
                break;
            case sol::type::boolean:
                m()->insert_or_assign(key, pair.second.as<bool>());
                break;
            //TODO: prevent infinite recursive loops
            case sol::type::table:
                m()->insert_or_assign(key, table(pair.second.as<sol::table>(), handleduplicates));
                break;
            }
        }       
    }

    sol::object table::dynamic_geti(sol::this_state L, const int key)
    {
        if (key < 1) return sol::lua_nil;
        return dynamic_get(L, tablekey(key - 1));
    }

    sol::object table::dynamic_gets(sol::this_state L, const std::string& key)
    {
        return dynamic_get(L, tablekey(key));
    }

    sol::object table::dynamic_get(sol::this_state L, const tablekey& key)
    {
        auto sz = m()->size();
        auto it = m()->find(key);
        if (it == m()->end()) return sol::make_object(L, sol::lua_nil);
        switch (it->second.t)
        {
        case type::number_float:
            return sol::make_object(L, it->second.f);
            break;
        case type::number_integer:
            return sol::make_object(L, it->second.i);
            break;
        case type::boolean:
            return sol::make_object(L, it->second.b);
            break;
        case type::string:
            return sol::make_object(L, it->second.s);
            break;
        case type::object:
            return sol::make_object(L, tablewrapper(it->second));
            break;
        case type::null:
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
                if (v.t == tablekey::type::name) return "string";
                return "number";
            },
            sol::meta_function::to_string, [](const tablekeywrapper& v)
            {
           //     if (v.t == tablekey::KeyType::key::index) throw(std::runtime_error("value is not a string."));
                //if (v.t == tablekey::type::name) return v.s;
                std::string s = v;
                return s;
            },
            sol::meta_function::concatenation, [](const tablekeywrapper& v, std::string s)
            {
            //    if (v.t == tablekey::KeyType::key::index) throw(std::runtime_error("value is not a string."));
                std::string ss;
                if (v.t == tablekey::type::name) ss = v.s;
                return ss + s;
            }
            );

        L->new_usertype<tablewrapper>("tableplusplus_table",
            sol::meta_method::type, [](const tablewrapper& v)
            {
                if (v.t == table::type::object) return "userdata";
                if (v.t == table::type::null) return "nil";
                if (v.t == table::type::string) return "string";
                if (v.t == table::type::number_integer || v.t == table::type::number_float) return "number";
                if (v.t == table::type::boolean) return "boolean";
                return "userdata";
            },
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
#if TABLEPLUSPLUS_LUATABLEFUNCTION
        L->set_function("ctable", []() { return tablewrapper(table()); });
#endif
    }
#endif
}
