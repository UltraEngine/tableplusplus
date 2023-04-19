# table++

By Josh Klint

https://www.ultraengine.com

## About

This code is meant to bridge the gaps between dynamic data in C++, Lua, and JSON with an STL-like implementation of Lua tables.

This is useful for dynamic application settings and user-defined settings added by script.

Tables can be accessed by Lua using the [sol](https://github.com/ThePhD/sol2) library, but the tables themselves do not use Lua.

Tables can be saved in JSON format or loaded from JSON using the [nlohmann::json](https://github.com/nlohmann/json) library.

Only data types that can be written to JSON files are supported.

## Programming Guide

Usage in C++ generally follows the same rules as STL containers, but tables have properties of both STL maps and vectors. The following methods are available:

- begin
- clear
- empty
- end
- erase
- find
- operator[]
- push_back
- size

Note that size() will return the number of entries when the table is treated as a Lua array, which may be different from the number of entries that iterating through begin()...end() will provide.

Values can be retrieved using either a string or an integer for the key.

Usage in Lua is identical to Lua tables, with the following exceptions:
- The only values that can be inserted into C++ tables are booleans, numbers, strings, nil, and other C++ tables.
- By default, there is no way to create a C++ table in Lua, because these usually are declared as a member of another class in C++. However, you can change the definition of TABLEPLUSPLUS_LUATABLEFUNCTION to 1 in the header file, and a creation function called "ctable" will be declared for use in Lua.

## Lua Integration

When declaring C++ classes in sol that use a table as a member, it is necessary to use a sol::property and return a tablewrapper, the class that is actually declared to sol:

```c++
L->new_usertype<Entity> (
	"properties", sol::property(
		[](const Entity& e) { return tableplusplus::tablewrapper(e.properties); },
		[](Entity& e, sol::table t) { e.properties = t; }
	)
)
```

## Examples

Here are some of the cool things you can do in C++ and Lua.

### Key / value in C++

```c++
table t;
t["health"] = 100;
t["windowsettings"] = {};
t["windowsettings"]["position"] = 3;

for (auto a : t)
{
    std::string s = a.first;
    Print(s);
    s = a.second;
    Print(s);
}
```

###  Arrays in C++

```c++
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
```
### Mixing key / value and array-style keys

```c++
table t;

t["health"] = 100;
t["windowsettings"] = {};
t["windowsettings"]["position"] = 3;

t.resize(10);
for (int n = 0; n < t.size(); ++n)
{
    t[n] = n;
}

//Insert an out-of-order index and it will get trimmed off in the resize call
t[12] = {};

t.resize(5);

Print("Size: " + String(t.size()));

for (auto a : t)
{
    std::string s = a.first;
    Print(s);
    s = a.second;
    Print(s);
}
```

### C++ copy test

```c++
table t;
t["window"] = {};
t["window"]["position"] = 300;
auto b = t["window"];
t = nullptr;
std::string s = b["position"];
Print(s);
```

###  Key / value in Lua

```lua
local a = ctable()
a["health"] = 100
a["color"] = "blue"

for k,v in pairs(a) do
	Print(k ..": ".. tostring(v))
end
```

### Arrays in Lua

```lua
b = ctable()
b[1] = 1
b[2] = 2
b[3] = 3
b[4] = 4

for n = 1, #b do
	Print(b[n])
end

for k,v in ipairs(b) do
	Print(k ..": ".. tostring(v))
end
```

## Convert to JSON

```c++
table t;

//Pure integer keys get output as a JSON array
for (int n = 0; n < 10; ++n)
{
    t[n] = n;
}
printf(t.to_json() + "\n");

//Mix string and integer keys
t["health"] = 100;
t["money"] = 0;
t["nullvalue"] = nullptr;
t["zvalue"] = true;

t["subtable"] = {};
t["subtable"]["position"] = 50;
t["subtable"]["size"] = 300;
t["subtable1"] = t["subtable"];

t["subarray"] = {};
t["subarray"][0] = 1;
t["subarray"][1] = 2;
t["subarray"][2] = 3;

printf(t.to_json());
```

## License

This code may be used freely for any purpose unless it is stated otherwise below.

If you use this code you must include this unmodified README file somewhere in your application directory or subdirectory where it is accessible to the end user.

This unmodified readme file must be included in any source code distribution, or distribution of code derived from this code.

You may not misrepresent the origin of this code.

You may not use this code or any derivatives thereof in AI training models.
