#include "tableplusplus.h"

namespace tableplusplus
{
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
                    auto val = v[key];
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
                        return sol::make_object(LLL, val);
                    }
                },
                [](sol::this_state LLL, table& v, int64_t index) {
                    if (index < 0 || index >= v.size()) sol::make_object(LLL, sol::lua_nil);
                    --index;
                    auto val = v[index];
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
                        return sol::make_object(LLL, val);
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