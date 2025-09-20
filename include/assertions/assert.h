//
// This file is part of assertions project <https://github.com/romanpauk/assertions>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: MIT
//

#include <iostream>
#include <sstream>

namespace assertions {
    template<typename T> struct operator_traits;

    struct context {
        void push() {}
        template<typename T> void emit(T value) {}
        template<typename T> void call() {}
        void pop() {}
    };

    struct tracing_context {
        void push() { if (depth++) stream << '('; }
        template<typename T> void emit(const T& value) { stream << value; }
        template<typename T> void call() { stream << ' ' << operator_traits<T>::name << ' '; }
        void pop() { if (--depth) stream << ')'; }

        std::stringstream stream;
        std::size_t depth = 0;
    };

    template<typename T> struct literal {
        literal(T value): value_(value) {}
        T value_;
    };

    template<typename LHS, typename RHS, typename Op> struct binary_expression {
        binary_expression(LHS lhs, RHS rhs)
            : lhs_(lhs), rhs_(rhs) {}
        LHS lhs_;
        RHS rhs_;
    };

    template<> struct operator_traits<std::equal_to<>> {
        static constexpr const char* name = "==";
    };
    
    template<> struct operator_traits<std::not_equal_to<>> {
        static constexpr const char* name = "!=";
    };
    
    template<> struct operator_traits<std::less<>> {
        static constexpr const char* name = "<";
    };
    
    template<> struct operator_traits<std::less_equal<>> {
        static constexpr const char* name = "<=";
    };

    template<> struct operator_traits<std::greater<>> {
        static constexpr const char* name = ">";
    };

    template<> struct operator_traits<std::greater_equal<>> {
        static constexpr const char* name = ">=";
    };

    template<> struct operator_traits<std::logical_or<>> {
        static constexpr const char* name = "||";
    };
    
    template<> struct operator_traits<std::logical_and<>> {
        static constexpr const char* name = "&&";
    };
    
    template<typename LHS, typename RHS>
    auto operator == (LHS lhs, RHS rhs) {
        return binary_expression<LHS, RHS, std::equal_to<>>(lhs, rhs);
    }
    
    template<typename LHS, typename RHS>
    auto operator != (LHS lhs, RHS rhs) {
        return binary_expression<LHS, RHS, std::not_equal_to<>>(lhs, rhs);
    }

    template<typename LHS, typename RHS>
    auto operator || (LHS lhs, RHS rhs) {
        return binary_expression<LHS, RHS, std::logical_or<>>(lhs, rhs);
    }

    template<typename LHS, typename RHS>
    auto operator && (LHS lhs, RHS rhs) {
        return binary_expression<LHS, RHS, std::logical_and<>>(lhs, rhs);
    }

    template<typename LHS, typename RHS>
    auto operator < (LHS lhs, RHS rhs) {
        return binary_expression<LHS, RHS, std::less<>>(lhs, rhs);
    }

    template<typename LHS, typename RHS>
    auto operator <= (LHS lhs, RHS rhs) {
        return binary_expression<LHS, RHS, std::less_equal<>>(lhs, rhs);
    }

    template<typename LHS, typename RHS>
    auto operator > (LHS lhs, RHS rhs) {
        return binary_expression<LHS, RHS, std::greater<>>(lhs, rhs);
    }

    template<typename LHS, typename RHS>
    auto operator >= (LHS lhs, RHS rhs) {
        return binary_expression<LHS, RHS, std::greater_equal<>>(lhs, rhs);
    }

    template<typename Context, typename T> T eval(Context&& ctx, T v) {
        ctx.emit(v);
        return v;
    }
    
    template<typename Context, typename T> std::remove_reference_t<T> eval(Context&& ctx, literal<T> l) {
        return eval(ctx, l.value_);
    }

    template<typename Context, typename LHS, typename RHS, typename Op>
    auto eval(Context&& ctx, binary_expression<LHS, RHS, Op> e) {
        ctx.push();
        auto lhs = eval(ctx, e.lhs_);
        ctx.template call<Op>();
        auto rhs = eval(ctx, e.rhs_);
        ctx.pop();
        return Op()(lhs, rhs);
    }

    struct capture {};
	template<typename T> auto operator << (capture, T&& rhs) { return literal<T>(rhs); }
    template<typename T> auto operator << (T&& lhs, capture) { return literal<T>(lhs); }
    template<typename T> auto operator << (literal<T>&& lhs, capture) { return lhs; }
}

#undef assert
#define assert(expr) \
    do { \
        using namespace assertions; \
        if (!static_cast<bool>(expr)) { \
            tracing_context ctx; \
            eval(ctx, capture() << expr << capture()); \
            std::cerr \
                << __FILE__ << ':' \
                << __LINE__ << ": " \
                << __PRETTY_FUNCTION__ \
                << ": Assertion `" << #expr << '\'' \
                << "' failed with `" << ctx.stream.str() << "'." \
                << std::endl; \
        } \
    } while(0)

