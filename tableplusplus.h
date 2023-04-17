#pragma once
#ifndef __TABLE_PLUS_PLUS
#define __TABLE_PLUS_PLUS

#include <string>
#include <map>
#include <sstream>
#include <stdexcept>

//Comment this out if you don't want to use Sol
#include <sol/sol.hpp>

namespace tableplusplus
{
    class table;
    class tableKey;

#ifdef SOL_VERSION
    struct SomeFuckedUpShit;
    extern void bind_table_plus_plus(sol::state* L);
#endif

    enum tableType
    {
        TABLE_OBJECT,
        TABLE_NULL,
        TABLE_INTEGER,
        TABLE_FLOAT,
        TABLE_BOOLEAN,
        TABLE_STRING,
        TABLE_INVALID
    };

    class tableKey
    {
        friend table;
#ifdef SOL_VERSION
        friend SomeFuckedUpShit;
#endif

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
            std::stringstream out;
            out << i;
            return out.str();
        }

        bool operator<(const tableKey k) const
        {
            if (t == KEY_INDEX && k.t == KEY_STRING) return true;
            if (t == KEY_STRING && k.t == KEY_INDEX) return false;
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

#ifdef SOL_VERSION
        friend void bind_table_plus_plus(sol::state*);
#endif        
    };

    class table
    {
#ifdef SOL_VERSION
        friend SomeFuckedUpShit;
#endif
        double f;
        int64_t i;
        bool b;
        std::string s;
        tableType t;
        std::map<tableKey, table> m;

    public:

        table();
        ~table();
        
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
            if (t == TABLE_INTEGER)
            {
                std::stringstream out;
                out << i;
                return out.str();
            }
            if (t == TABLE_FLOAT)
            {
                std::stringstream out;
                out << f;
                return out.str();
            }
            if (t == TABLE_BOOLEAN)
            {
                if (b) return "true";
                return "false";
            }
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

        table(const std::nullptr_t)
        {
            clear();
            t = TABLE_NULL;
        }

        table& operator[](const char* c);

        table& operator[](const std::string& key);

        table& operator[](const size_t key);
        
        //This is not optimal, but it's the way Lua tables work
        size_t size();
        
        void push_back(const table& j3);

        //This is extremely inefficient and should not be used for large arrays, but it's the best emulation of Lua table behavior
        void resize(const size_t sz);

#ifdef SOL_VERSION
        friend SomeFuckedUpShit;
        friend void bind_table_plus_plus(sol::state*);
#endif
    };

#ifdef SOL_VERSION
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

#endif
}
#endif