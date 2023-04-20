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

#pragma once

#ifndef __TABLE_PLUS_PLUS
#define __TABLE_PLUS_PLUS

// Options
#define TABLEPLUSPLUS_INCLUDE_SOL 1
#define TABLEPLUSPLUS_INCLUDE_JSON 1
#define TABLEPLUSPLUS_LUATABLEFUNCTION 0

#include <string>
#include <map>
#include <sstream>
#include <stdexcept>

#if TABLEPLUSPLUS_INCLUDE_SOL
    #define SOL_NO_CHECK_NUMBER_PRECISION 1
    #define SOL_ALL_SAFETIES_ON 1
    #include <sol/sol.hpp>
#endif

#if TABLEPLUSPLUS_INCLUDE_JSON
    #include <json.hpp>
#endif

namespace tableplusplus
{
    class table;
    class tablekey;

#ifdef SOL_VERSION
    struct IDKWTFLOL;
    struct tablewrapper;
    struct tablekeywrapper;
    extern void bind_table_plus_plus(sol::state* L);
#endif

    class tablekey
    {
    public:

        enum type
        {
            index,
            name
        };

    private:
        friend table;


        type t;

        std::string s;
        int i;

        tablekey() {};

    public:

        operator std::string() const
        {
            if (t == type::name) return s;
            std::stringstream out;
            out << i;
            return out.str();
        }

        operator int() const
        {
            if (t == type::name) return 0;
            return i;
        }

        bool operator<(const tablekey k) const
        {
            if (t == type::index && k.t == type::name) return true;
            if (t == type::name && k.t == type::index) return false;
            if (t == type::index) return i < k.i;
            return s < k.s;
        }

        bool operator==(const tablekey k) const
        {
            if (t != k.t) return false;
            if (t == type::index) return i == k.i;
            return s == k.s;
        }

        tablekey(const int index)
        {
            t = type::index;
            i = index;
        }

        tablekey(const std::string& s)
        {
            t = type::name;
            this->s = s;
            i = 0;
        }

        type get_type() { return t; };

#ifdef SOL_VERSION
        friend IDKWTFLOL;
        friend tablekeywrapper;
        friend tablewrapper;
        friend void bind_table_plus_plus(sol::state*);
#endif        
    };

    class table
    {
    public:

        enum type
        {
            object,
            null,
            number_float,
            number_integer,
            boolean,
            string
        };
    private:

        double f;
        int64_t i;
        bool b;
        std::string s;
        type t;
        std::shared_ptr<std::map<tablekey, table> > _m;
        std::shared_ptr<std::map<tablekey, table> > m();
#ifdef SOL_VERSION
        static std::map<int, table> copiedluatables;
#endif

    public:

        type get_type() { return type(t); };

        table();
        ~table();
        
        bool operator==(const table& k) const;
        bool operator!=(const table& k) const;
        table& operator[](const char* c);
        table& operator[](const std::string& key);
        table& operator[](const int key);
        table& operator[](const size_t key);

        bool is_string();
        bool is_object();
        bool is_number();
        bool is_array();
        bool is_integer();
        bool is_float();
        bool is_boolean();
        bool is_null();

        std::map<tablekey, table>::iterator erase(const std::map<tablekey, table>::iterator& it);
        std::map<tablekey, table>::iterator find(const int key);
        std::map<tablekey, table>::iterator find(const std::string& key);
        bool empty();
        void clear();
        size_t size();
        void push_back(const table& j3);
        void resize(const size_t sz);
        std::string to_json(const std::string indent = "");
        table copy(const bool recursive = true);

        std::map<tablekey, table>::iterator begin()
        {
            return m()->begin();
        }

        std::map<tablekey, table>::iterator end()
        {
            return m()->end();
        }

        operator bool() const
        {
            if (t == type::boolean) return b;
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
            if (t == type::number_integer) return i;
            if (t == type::number_float) return f;
            return 0;
        }

        operator float() const
        {
            return double(*this);
        }

        operator double() const
        {
            if (t == type::number_float) return f;
            if (t == type::number_integer) return i;
            return 0.0f;
        }

        operator std::string() const
        {
            if (t == type::string) return s;
            if (t == type::number_integer)
            {
                std::stringstream out;
                out << i;
                return out.str();
            }
            if (t == type::number_float)
            {
                std::stringstream out;
                out << f;
                return out.str();
            }
            if (t == type::boolean)
            {
                if (b) return "true";
                return "false";
            }
            return "null";
        }

        table(const int i_)
        {
            clear();
            i = i_;
            t = type::number_integer;
        }

        table(const int64_t i_)
        {
            clear();
            i = i_;
            t = type::number_integer;
        }

        table(const bool b_)
        {
            clear();
            b = b_;
            t = type::boolean;
        }

        table(const float f_)
        {
            clear();
            f = f_;
            t = type::number_float;
        }

        table(const double f_)
        {
            clear();
            f = f_;
            t = type::number_float;
        }

        table(const std::string& s_)
        {
            clear();
            s = s_;
            t = type::string;
        }

        table(const std::nullptr_t)
        {
            clear();
            t = type::null;
        }

#ifdef SOL_VERSION
        friend IDKWTFLOL;
        table(const sol::table& tbl);
    private:
        table(const sol::table& tbl, const bool handleduplicates);
    public:
#endif
        
#ifdef NLOHMANN_JSON_VERSION_MAJOR

        table(const nlohmann::json& j3);
        operator nlohmann::json();

#endif

#ifdef SOL_VERSION

    private:
        void dynamic_set(const tablekey& key, const sol::object& value);
        void dynamic_sets(const std::string& key, const sol::object& value);
        void dynamic_seti(const int key, const sol::object& value);

        sol::object dynamic_get(sol::this_state L, const tablekey& key);
        sol::object dynamic_gets(sol::this_state L, const std::string& key);
        sol::object dynamic_geti(sol::this_state L, const int key);
       
        friend IDKWTFLOL;
        friend void bind_table_plus_plus(sol::state*);
        friend tablewrapper;
#endif
    };

#ifdef SOL_VERSION

    struct tablekeywrapper : private tablekey
    {
        friend tablekeywrapper;
        friend void bind_table_plus_plus(sol::state* L);

        tablekey tokey()
        {
            return *this;
        }

        tablekeywrapper(const tablekey& t)
        {
            this->t = t.t;
            this->i = t.i;
            this->s = t.s;
        }
    };

    struct tablewrapper : private table
    {
        friend table;
        friend tablewrapper;
        friend void bind_table_plus_plus(sol::state* L);

        auto begin() { return table::begin(); }
        auto end() { return table::end(); }
        size_t size() { return table::size(); }

        table totable()
        {
            return *this;
        }

        tablewrapper(const table& t)
        {
            this->t = t.t;
            this->b = t.b;
            this->i = t.i;
            this->f = t.f;
            this->s = t.s;
            this->_m = t._m;
        }
    };

    struct IDKWTFLOL
    {
        struct lua_iterator_state {
            typedef std::map<tablekey, table>::iterator it_t;
            it_t it;
            it_t last;

            lua_iterator_state(table& mt)
                : it(mt.m()->begin()), last(mt.m()->end()) {
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
                sol::object(l, sol::in_place, tablekeywrapper(it->first)),
                sol::object(l, sol::in_place, tablewrapper(it->second)));
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

//Cleanup
#undef TABLEPLUSPLUS_INCLUDE_SOL
#undef TABLEPLUSPLUS_INCLUDE_JSON
#undef TABLEPLUSPLUS_LUATABLEFUNCTION

#endif
