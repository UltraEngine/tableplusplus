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
            if (t == TABLE_INTEGER or t == TABLE_FLOAT)
            {
                if (t == TABLE_INTEGER or t == TABLE_FLOAT)
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
        //if (_m) _m->clear();
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

#ifdef SOL_VERSION

    void table::dynamic_sets(const std::string& key, const sol::object& value)
    {
        dynamic_set(tableKey(key), value);
    }

    void table::dynamic_seti(const int key, const sol::object& value)
    {
        if (key < 1) return;
        dynamic_set(tableKey(key - 1), value);
    }

    void table::dynamic_set(const tableKey& key, const sol::object& value)
    {
        void* p;
        double f;
        switch (value.get_type())
        {
        case sol::type::number:
            f = value.as<double>();
            p = m().get();
            m()->insert_or_assign(key, f);
            break;
        case sol::type::string:
            m()->insert_or_assign(key, value.as<std::string>());
            break;
        case sol::type::boolean:
            m()->insert_or_assign(key, value.as<bool>());
            break;
        case sol::type::userdata:
            if (value.is<table>())
            {
                auto tbl = value.as<table*>();
                m()->insert_or_assign(key, *tbl);
            }
            else
            {
                throw(std::runtime_error("userdata must be a C++ table"));
            }
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
            return sol::make_object(L, it->second);
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
        L->new_usertype<tableKey>("tableplusplus::tablekey",
            sol::meta_method::type, [](const tableKey& v)
            {
                if (v.t == TABLE_OBJECT) return "userdata";
                if (v.t == TABLE_NULL) return "nil";
                if (v.t == TABLE_INTEGER or v.t == TABLE_FLOAT) return "number";
                if (v.t == TABLE_BOOLEAN) return "boolean";
                return "userdata";
            },
            sol::meta_function::to_string, [](const tableKey& v)
            {
                std::string s = v;
                return s;
            },
            sol::meta_function::concatenation, [](const tableKey& v, std::string s)
            {
                std::string ss = std::string(v);
                return ss + s;
            }
            );

        L->new_usertype<table>("tableplusplus::table",
            sol::meta_function::pairs, &SomeFuckedUpShit::my_pairs,
            sol::meta_function::ipairs, &SomeFuckedUpShit::my_pairs,
            sol::meta_function::to_string, [](const table& v) { std::string s = v; return s; },
            sol::meta_method::equal_to, [](const table& a, const table& b) { return a == b; },
            sol::meta_function::index, sol::overload(&table::dynamic_gets, &table::dynamic_geti),
            sol::meta_function::new_index, sol::overload(&table::dynamic_sets, &table::dynamic_seti)//,
            //sol::meta_function::static_index, sol::overload(&table::dynamic_gets, &table::dynamic_geti),
            //sol::meta_function::static_new_index, sol::overload(&table::dynamic_sets, &table::dynamic_seti)
        );
        L->set_function("Table", []() { return table(); });
    }
#endif
}
