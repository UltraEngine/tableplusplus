#include "tableplusplus.h"

namespace tableplusplus
{
    typedef std::pair<tableKey, table> tpair;

    table::table()
    {
        i = 0;
        f = 0;
        b = false;
        t = TABLE_OBJECT;
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
        m()->insert(tpair(sz, j3));
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
            if (m()->find(n) == m()->end()) m()->insert(tpair(n, nullptr));
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
            m()->insert(tpair(key, nullptr));
        }
        auto it = m()->find(key);
        return it->second;
    }

    table& table::operator[](const size_t key)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        if (m()->find(key) == m()->end())
        {
            m()->insert(tpair(key, nullptr));
        }
        auto it = m()->find(key);
        return it->second;
    }

#ifdef SOL_VERSION
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
            sol::meta_function::to_string, [](const table& v)
            {
                std::string s = v;
                return s;
            },
            sol::meta_method::equal_to, 
                [](const table& a, const table& b) { return a == b; },
            sol::meta_function::index, sol::overload(
                [](sol::this_state LLL, table& v, std::string key) {
                    table val = v[key];
                    switch (val.GetType())
                    {
                    case TABLE_INTEGER:
                        return sol::make_object(LLL, val.i);
                    case TABLE_FLOAT:
                        return sol::make_object(LLL, val.f);
                    case TABLE_BOOLEAN:
                        return sol::make_object(LLL, val.b);
                    case TABLE_STRING:
                        return sol::make_object(LLL, val.s);
                    case TABLE_OBJECT:
                        return sol::make_object(LLL, val);
                    }
                    return sol::make_object(LLL, sol::lua_nil);
                },
                [](sol::this_state LLL, table& v, int64_t index) {
                    if (index < 0 || index >= v.size()) sol::make_object(LLL, sol::lua_nil);
                    --index;
                    table val = v[index];
                    switch (val.GetType())
                    {
                    case TABLE_INTEGER:
                        return sol::make_object(LLL, val.i);
                    case TABLE_FLOAT:
                        return sol::make_object(LLL, val.f);
                    case TABLE_BOOLEAN:
                        return sol::make_object(LLL, val.b);
                    case TABLE_STRING:
                        return sol::make_object(LLL, val.s);
                    case TABLE_OBJECT:
                        return sol::make_object(LLL, val);
                    }
                    return sol::make_object(LLL, sol::lua_nil);
                }
                ),
                sol::meta_function::new_index, sol::overload(
                    [](table& v, int64_t index, double x)
                    {
                        --index;
                        if (index < 0) return;
                        v[index] = x;
                    },
                    [](table& v, std::string key, double x)
                    {
                        v[key] = x;
                    },
                    [](table& v, int64_t index, std::string x)
                    {
                        --index;
                        if (index < 0) return;
                        v[index] = x;
                    },
                    [](table& v, std::string key, std::string x)
                    {
                        v[key] = x;
                    },
                    [](table& v, int64_t index, bool x)
                    {
                        --index;
                        if (index < 0) return;
                        v[index] = x;
                    },
                    [](table& v, std::string key, bool x)
                    {
                        v[key] = x;
                    },
                    [](table& v, int64_t index, const table& x)
                    {
                        --index;
                        if (index < 0) return;
                        v[index] = x;
                    },
                    [](table& v, std::string key, const table& x)
                    {
                        v[key] = x;
                    }
                 )
             );
        L->set_function("Table", []() { return table(); });
    }
#endif
}