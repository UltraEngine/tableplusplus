#include "UltraEngine.h"

using namespace UltraEngine;

struct SomeFuckedUpShit;

class tableKey
{
    friend SomeFuckedUpShit;

    enum KeyType
    {
        KEY_STRING,
        KEY_INDEX
    };
    KeyType t;

    std::string s;
    int i;

public:

    operator std::string() const
    {
        if (t == KEY_STRING) return s;
        return String(i);
    }

    bool operator<(const tableKey k) const
    {
        if (t == KEY_INDEX and k.t == KEY_STRING) return true;
        if (t == KEY_STRING and k.t == KEY_INDEX) return false;
        if (t == KEY_INDEX) return i < k.i;
        return s < k.s;
    }

    bool operator==(const tableKey k) const
    {
        if (t != k.t) return false;
        if (t == KEY_INDEX) return i == k.i;
        return s == k.s;
    }

    tableKey(const int index)
    {
        t = KEY_INDEX;
        i = index;
    }

    tableKey(const std::string& s)
    {
        t = KEY_STRING;
        this->s = s;
    }
};

enum tableType
{
    TABLE_NULL,
    TABLE_INTEGER,
    TABLE_FLOAT,
    TABLE_BOOLEAN,
    TABLE_STRING,
    TABLE_OBJECT
};

class table
{
    friend SomeFuckedUpShit;

    double f;
    int64_t i;
    bool b;
    std::string s;
    tableType t;
    std::map<tableKey, table> m;

public:

    table()
    {
        i = 0;
        f = 0;
        b = false;
        t = TABLE_OBJECT;
    }

    std::map<tableKey, table>::iterator begin()
    {
        return m.begin();
    }

    std::map<tableKey, table>::iterator end()
    {
        return m.end();
    }

    tableType GetType() const
    {
        return t;
    }

    operator bool() const
    {
        if (t == TABLE_BOOLEAN) return b;
        return false;
    }

    operator short int() const
    {
        return int64_t(*this);
    }

    operator unsigned short int() const
    {
        return int64_t(*this);
    }

    operator unsigned int() const
    {
        return int64_t(*this);
    }
    
    operator int() const
    {
        return int64_t(*this);
    }

    operator int64_t() const
    {
        if (t == TABLE_INTEGER) return i;
        if (t == TABLE_FLOAT) return f;
        if (t == TABLE_BOOLEAN) return b;
        return 0;
    }

    operator float() const
    {
        return double(*this);
    }

    operator double() const
    {
        if (t == TABLE_FLOAT) return f;
        if (t == TABLE_INTEGER) return i;
        if (t == TABLE_BOOLEAN) return b;
        return 0.0f;
    }

    operator std::string() const
    {
        if (t == TABLE_STRING) return s;
        if (t == TABLE_FLOAT) return String(f);
        if (t == TABLE_INTEGER) return String(i);
        if (t == TABLE_BOOLEAN) return String(b);
        return "";
    }

    void clear()
    {
        t = TABLE_NULL;
        i = 0;
        f = 0;
        b = false;
        s.clear();
        m.clear();
    }

    table(const int i_)
    {
        clear();
        i = i_;
        t = TABLE_INTEGER;
    }

    table(const int64_t i_)
    {
        clear();
        i = i_;
        t = TABLE_INTEGER;
    }

    table(const bool b_)
    {
        clear();
        b = b_;
        t = TABLE_BOOLEAN;
    }

    table(const float f_)
    {
        clear();
        f = f_;
        t = TABLE_FLOAT;
    }

    table(const double f_)
    {
        clear();
        f = f_;
        t = TABLE_FLOAT;
    }

    table(const std::string& s_)
    {
        clear();
        s = s_;
        t = TABLE_STRING;
    }

    table& operator[](const char* c)
    {
        return (*this)[std::string(c)];
    }

    table& operator[](const std::string& key)
    {
        return m[key];
    }

    table& operator[](const size_t key)
    {
        return m[key];
    }

    //Designed for convenience, not performance
    size_t size()
    {
        size_t sz = 0;
        while (true)
        {
            if (m.find(sz) == m.end()) break;
            ++sz;
        }
        return sz;
    }

    void push_back(const table& j3)
    {
        auto sz = size();
        m[sz] = j3;
    }

    //This is really slow but whatever
    void resize(const size_t sz)
    {
        auto current = size();
        if (sz > current)
        {
            for (size_t n = 0; n < sz - current; ++n)
            {
                push_back({});
            }
        }
        else if (sz < current)
        {
            for (size_t n = current; n < sz; ++n)
            {
                auto it = m.find(n - 1);
                if (it != m.end()) m.erase(it);
            }
        }
    }

    friend SomeFuckedUpShit;
};

struct SomeFuckedUpShit
{
    struct lua_iterator_state {
        typedef std::map<tableKey, table>::iterator it_t;
        it_t it;
        it_t last;

        lua_iterator_state(table& mt)
            : it(mt.m.begin()), last(mt.m.end()) {
        }
    };

    static std::tuple<sol::object, sol::object> my_next(
        sol::user<lua_iterator_state&> user_it_state,
        sol::this_state l) {
        // this gets called
        // to start the first iteration, and every
        // iteration there after

        // the state you passed in my_pairs is argument 1
        // the key value is argument 2, but we do not
        // care about the key value here
        lua_iterator_state& it_state = user_it_state;
        auto& it = it_state.it;
        if (it == it_state.last) {
            // return nil to signify that
            // there's nothing more to work with.
            return std::make_tuple(sol::object(sol::lua_nil),
                sol::object(sol::lua_nil));
        }
        auto itderef = *it;
        // 2 values are returned (pushed onto the stack):
        // the key and the value
        // the state is left alone
        auto r = std::make_tuple(
            sol::object(l, sol::in_place, it->first),
            sol::object(l, sol::in_place, it->second));
        // the iterator must be moved forward one before we return
        std::advance(it, 1);
        return r;
    }

    static auto my_pairs(table& mt) {
        // pairs expects 3 returns:
        // the "next" function on how to advance,
        // the "table" itself or some state,
        // and an initial key value (can be nil)

        // prepare our state
        lua_iterator_state it_state(mt);
        // sol::user is a space/time optimization over regular
        // usertypes, it's incompatible with regular usertypes and
        // stores the type T directly in lua without any pretty
        // setup saves space allocation and a single dereference
        return std::make_tuple(&my_next,
            sol::user<lua_iterator_state>(std::move(it_state)),
            sol::lua_nil);
    }
};

int main(int argc, const char* argv[])
{
    auto L = GetLuaState();

    L->new_usertype<tableKey>("JSONtableKEY",
        sol::meta_function::to_string, [](const tableKey& v)
        {
            std::string s = v;
            return s;
        },
        sol::meta_function::concatenation, [](const tableKey & v, std::string s)
        {
            std::string ss = std::string(v);
            return ss + s;
        }
    );

    L->new_usertype<table>("JSONtable",
        sol::meta_function::pairs, &SomeFuckedUpShit::my_pairs,
        sol::meta_function::ipairs, &SomeFuckedUpShit::my_pairs,
        sol::meta_function::to_string, [](const table& v)
        {
            std::string s = v;
            return s;
        },
        sol::meta_function::index, sol::overload(
            [](table& v, std::string key) {
                auto L = GetLuaState()->lua_state();
                auto val = v[key];
                switch (val.GetType())
                {
                case TABLE_INTEGER:
                    return sol::make_object(L, int64_t(val));
                case TABLE_FLOAT:
                    return sol::make_object(L, double(val));
                case TABLE_BOOLEAN:
                    return sol::make_object(L, bool(val));
                case TABLE_STRING:
                    return sol::make_object(L, std::string(val));
                case TABLE_OBJECT:
                    return sol::make_object(L, val);
                }
            },
            [](table& v, int64_t index) {
                auto L = GetLuaState()->lua_state();
                if (index < 0 or index >= v.size()) sol::make_object(L, sol::lua_nil);
                --index;
                auto val = v[index];
                switch (val.GetType())
                {
                case TABLE_INTEGER:
                    return sol::make_object(L, int64_t(val));
                case TABLE_FLOAT:
                    return sol::make_object(L, double(val));
                case TABLE_BOOLEAN:
                    return sol::make_object(L, bool(val));
                case TABLE_STRING:
                    return sol::make_object(L, std::string(val));
                case TABLE_OBJECT:
                    return sol::make_object(L, val);
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

    //----------------------------------
    // C++ Test
    //----------------------------------

    table t;
    t["health"] = 100;
    t["windowsettings"] = table();
    t["windowsettings"]["position"] = 3;

    for (auto a : t)
    {
        std::string s = a.first;
        Print(s);
        s = a.second;
        Print(s);
    }

    table arr;
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    arr.push_back(4);
    arr.push_back(5);

    for (int n = 0; n < arr.size(); ++n)
    {
        Print(std::string(arr[n]));
    }

    //----------------------------------
    // Lua Test
    //----------------------------------

    //Get commandline settings
    auto settings = ParseCommandLine(argc, argv);

    //Enable the debugger if needed
    shared_ptr<Timer> debugtimer;
    if (settings["debug"].is_boolean() and settings["debug"] == true)
    {
        RunScript("Scripts/System/Debugger.lua");
        debugtimer = CreateTimer(510);
        ListenEvent(EVENT_TIMERTICK, debugtimer, std::bind(PollDebugger, 500));
    }

    //Run main script
    RunScript("Scripts/Main.lua");

    return 0;
}
