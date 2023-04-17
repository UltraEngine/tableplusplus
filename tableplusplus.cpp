#include "tableplusplus.h"

namespace tableplusplus
{
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
    }

    size_t table::size()
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        size_t sz = 0;
        while (true)
        {
            if (m.find(sz) == m.end()) break;
            ++sz;
        }
        return sz;
    }

    void table::push_back(const table& j3)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        auto sz = size();
        m[sz] = j3;
    }

    void table::resize(const size_t sz)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        auto current = size();
        if (current == sz) return;

        //Remove indexes beyond the max
        if (sz < current)
        {
            auto it = m.lower_bound(sz);
            while (it != m.end())
            {
                if (it->first.t != tableKey::KeyType::KEY_INDEX) break;// index keys are ordered first so this is fine
                if (it->first.i >= sz)
                {
                    it = m.erase(it);
                    continue;
                }
                ++it;
            }
        }

        //Fill in missing indexes
        for (size_t n = 0; n < sz; ++n)
        {
            m[n];
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
        if (m.find(key) == m.end()) m[key] = nullptr;
        return m[key];
    }

    table& table::operator[](const size_t key)
    {
        if (t != TABLE_OBJECT) throw std::runtime_error("Table value is not an object.");
        if (m.find(key) == m.end()) m[key] = nullptr;
        return m[key];
    }

#ifdef SOL_VERSION
    void bind_table_plus_plus(sol::state* L)
    {
        L->new_usertype<tableKey>("tableplusplus::tablekey",
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
            sol::meta_function::index, sol::overload(
                [](sol::this_state LLL, table& v, std::string key) {
                    auto& val = v[key];
                    switch (val.GetType())
                    {
                    case TABLE_INTEGER:
                        return sol::make_object(LLL, int64_t(val));
                    case TABLE_FLOAT:
                        return sol::make_object(LLL, double(val));
                    case TABLE_BOOLEAN:
                        return sol::make_object(LLL, bool(val));
                    case TABLE_STRING:
                        return sol::make_object(LLL, std::string(val));
                    case TABLE_OBJECT:
                        return sol::make_object(LLL, &val);
                    }
                },
                [](sol::this_state LLL, table& v, int64_t index) {
                    if (index < 0 || index >= v.size()) sol::make_object(LLL, sol::lua_nil);
                    --index;
                    auto& val = v[index];
                    switch (val.GetType())
                    {
                    case TABLE_INTEGER:
                        return sol::make_object(LLL, int64_t(val));
                    case TABLE_FLOAT:
                        return sol::make_object(LLL, double(val));
                    case TABLE_BOOLEAN:
                        return sol::make_object(LLL, bool(val));
                    case TABLE_STRING:
                        return sol::make_object(LLL, std::string(val));
                    case TABLE_OBJECT:
                        return sol::make_object(LLL, &val);
                    }
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