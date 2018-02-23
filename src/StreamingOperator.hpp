#pragma once

template <class T>
inline Print &operator<<(Print &stream, T arg)
{
    stream.print(arg);
    return stream;
}

enum _EndLineCode
{
    endl
};

inline Print &operator<<(Print &stream, _EndLineCode arg)
{
    stream.println();
    return stream;
}

inline Print &operator<<(Print &stream, std::string arg)
{
    for (char c : arg)
        stream.print(c);
    return stream;
}