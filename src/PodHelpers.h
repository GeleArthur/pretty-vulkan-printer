#pragma once
#include <fstream>
#include <string>
#include <vector>

// helper functions created with Gemini
// WRITE
template<typename T>
void write_pod(std::ofstream& out, const T& val)
{
    out.write(reinterpret_cast<const char*>(&val), sizeof(T));
}

template<typename T>
void write_vector(std::ofstream& out, const std::vector<T>& vec)
{
    uint32_t size = static_cast<uint32_t>(vec.size());
    out.write(reinterpret_cast<const char*>(&size), sizeof(size));
    if (size > 0)
    {
        out.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
    }
}

void inline write_string(std::ofstream& out, const std::string& str)
{
    uint32_t size = static_cast<uint32_t>(str.size());
    out.write(reinterpret_cast<const char*>(&size), sizeof(size));
    if (size > 0)
    {
        out.write(str.data(), size);
    }
}

// READ
template<typename T>
void read_pod(std::ifstream& in, T& val)
{
    in.read(reinterpret_cast<char*>(&val), sizeof(T));
}

template<typename T>
void read_vector(std::ifstream& in, std::vector<T>& vec)
{
    uint32_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));
    vec.resize(size);
    if (size > 0)
    {
        in.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
    }
}

void inline read_string(std::ifstream& in, std::string& str)
{
    uint32_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));
    str.resize(size);
    if (size > 0)
    {
        in.read(&str[0], size);
    }
}