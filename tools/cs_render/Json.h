// Minimal JSON reader for phrase files (objects, arrays, numbers, strings, booleans, null). No dependencies.
#pragma once
#include <cctype>
#include <cstdlib>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace csr {
struct Json {
    enum class T { Null, Bool, Num, Str, Arr, Obj } t = T::Null;
    bool b = false; double n = 0; std::string s; std::vector<Json> a; std::map<std::string, Json> o;
    bool has(const std::string& k) const { return t == T::Obj && o.count(k) > 0; }
    const Json& at(const std::string& k) const { auto it = o.find(k); if (it == o.end()) throw std::runtime_error("missing key: " + k); return it->second; }
    double num(const std::string& k, double def) const { return has(k) && at(k).t == T::Num ? at(k).n : def; }
    std::string str(const std::string& k, const std::string& def) const { return has(k) && at(k).t == T::Str ? at(k).s : def; }
};
class JsonParser {
public:
    explicit JsonParser(const std::string& text) : p_(text.c_str()), end_(text.c_str() + text.size()) {}
    Json parse() { Json v = value(); ws(); if (p_ != end_) fail("trailing characters"); return v; }
private:
    const char* p_; const char* end_;
    [[noreturn]] void fail(const char* m) { throw std::runtime_error(std::string("JSON: ") + m); }
    void ws() { while (p_ < end_ && std::isspace((unsigned char) *p_)) ++p_; }
    bool eat(char c) { ws(); if (p_ < end_ && *p_ == c) { ++p_; return true; } return false; }
    std::string str() {
        if (!eat('"')) fail("expected string"); std::string r;
        while (p_ < end_ && *p_ != '"') { if (*p_ == '\\' && p_ + 1 < end_) { ++p_; const char c = *p_++; r += c == 'n' ? '\n' : c == 't' ? '\t' : c; } else r += *p_++; }
        if (p_ >= end_) fail("unterminated string"); ++p_; return r;
    }
    Json value() {
        ws(); if (p_ >= end_) fail("unexpected end"); Json v;
        if (*p_ == '{') { ++p_; v.t = Json::T::Obj; if (eat('}')) return v; do { std::string k = str(); if (!eat(':')) fail("expected ':'"); v.o[k] = value(); } while (eat(',')); if (!eat('}')) fail("expected '}'"); return v; }
        if (*p_ == '[') { ++p_; v.t = Json::T::Arr; if (eat(']')) return v; do { v.a.push_back(value()); } while (eat(',')); if (!eat(']')) fail("expected ']'"); return v; }
        if (*p_ == '"') { v.t = Json::T::Str; v.s = str(); return v; }
        if (end_ - p_ >= 4 && std::string(p_, 4) == "true") { p_ += 4; v.t = Json::T::Bool; v.b = true; return v; }
        if (end_ - p_ >= 5 && std::string(p_, 5) == "false") { p_ += 5; v.t = Json::T::Bool; return v; }
        if (end_ - p_ >= 4 && std::string(p_, 4) == "null") { p_ += 4; return v; }
        char* e = nullptr; v.n = std::strtod(p_, &e); if (e == p_) fail("bad value"); p_ = e; v.t = Json::T::Num; return v;
    }
};
}
