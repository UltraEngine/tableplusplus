# table++

By: Josh Klint

https://www.ultraengine.com

## About

This code is meant to bridge the gaps between dynamic data in C++, Lua, and JSON.

The tables can be accessed by Lua using the [sol](https://github.com/ThePhD/sol2) library, but the tables themselves do not use Lua.

This is useful for dynamic application settings and user-defined settings added by script.

## Examples

Here are some of the cool things you can do in C++ and Lua.

### Key / value in C++

```c++
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
```

###  Array in C++

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
### Mixing Key / value and array-style keys

```c++
table t;

t["health"] = 100;
t["windowsettings"] = table();
t["windowsettings"]["position"] = 3;

t.resize(10);
for (int n = 0; n < t.size(); ++n)
{
    t[n] = n;
}
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

###  Key / value in Lua

```lua
local a = Table()
a["health"] = 100
a["color"] = "blue"

for k,v in pairs(a) do
	Print(k ..": ".. tostring(v))
end
```

### Arrays in Lua

```lua
b = Table()
b[1] = 1
b[2] = 2
b[3] = 3
b[4] = 4

for n = 1, #b do
	Print(b[n])
end
```

## License

If you use this code you must include this README file somewhere in your application directory or subdirectory where it is accessible to the end user.
