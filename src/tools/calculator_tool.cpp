#include "tool.h"
#include <cmath>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

// ── Recursive descent expression parser ───────────────────────
// Grammar:
//   expr   = term   ( ('+'|'-') term   )*
//   term   = factor ( ('*'|'/') factor )*
//   factor = NUMBER | '(' expr ')' | '-' factor
namespace {

class ExprParser {
    const std::string& s_;
    size_t pos_ = 0;

    void skip() {
        while (pos_ < s_.size() && std::isspace((unsigned char)s_[pos_])) ++pos_;
    }

    double number() {
        skip();
        size_t start = pos_;
        if (pos_ < s_.size() && s_[pos_] == '-') ++pos_;  // unary minus
        while (pos_ < s_.size() &&
               (std::isdigit((unsigned char)s_[pos_]) || s_[pos_] == '.')) ++pos_;
        if (start == pos_) throw std::runtime_error("Expected number at pos " + std::to_string(pos_));
        return std::stod(s_.substr(start, pos_ - start));
    }

    double factor() {
        skip();
        if (pos_ < s_.size() && s_[pos_] == '(') {
            ++pos_;
            double v = expr();
            skip();
            if (pos_ < s_.size() && s_[pos_] == ')') ++pos_;
            else throw std::runtime_error("Missing closing ')'");
            return v;
        }
        return number();
    }

    double term() {
        double v = factor();
        while (pos_ < s_.size()) {
            skip();
            char op = s_[pos_];
            if (op != '*' && op != '/') break;
            ++pos_;
            double r = factor();
            if (op == '/' && r == 0.0) throw std::runtime_error("Division by zero");
            v = (op == '*') ? v * r : v / r;
        }
        return v;
    }

    double expr() {
        double v = term();
        while (pos_ < s_.size()) {
            skip();
            char op = s_[pos_];
            if (op != '+' && op != '-') break;
            ++pos_;
            v = (op == '+') ? v + term() : v - term();
        }
        return v;
    }

public:
    explicit ExprParser(const std::string& s) : s_(s) {}
    double evaluate() { return expr(); }
};

// Format kết quả: hiển thị integer nếu không có phần thập phân
std::string fmtDouble(double v) {
    if (v == std::floor(v) && std::abs(v) < 1e15) {
        return std::to_string((long long)v);
    }
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

} // anonymous namespace

// ── CalculatorTool ────────────────────────────────────────────

std::string CalculatorTool::getName()        const { return "calculator"; }
std::string CalculatorTool::getDescription() const {
    return "Tính biểu thức toán học: + - * / ( ) và số âm. "
           "Args: biểu thức trực tiếp hoặc JSON {\"expression\":\"2*(3+4)\"}";
}

std::optional<std::string> CalculatorTool::execute(const std::string& args) {
    std::string expr = args;
    try {
        auto j = nlohmann::json::parse(args);
        if (j.contains("expression")) expr = j["expression"].get<std::string>();
    } catch (...) {}

    if (expr.empty()) return std::nullopt;

    try {
        double result = ExprParser(expr).evaluate();
        return fmtDouble(result);
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
