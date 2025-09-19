//
// This file is part of assertions project <https://github.com/romanpauk/assertions>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: MIT
//

#include <iostream>
#include <sstream>
#include <string>

namespace assertions {
    struct result {
        std::string expression;
        operator bool() const { return expression.empty(); }
    };
   
    struct formatter {
        template<typename LHS> static std::string handler(const LHS& lhs) {
            std::stringstream stream;
            stream << lhs << std::endl;
            return stream.str();
        }

        template<typename LHS, typename RHS>
            static std::string handler(const LHS& lhs, const RHS& rhs, const char* op) {
            std::stringstream stream;
            stream << lhs << ' ' << op << ' ' << rhs << "'." << std::endl;
            return stream.str();
        }
    };

    template<typename Formatter> struct expression {
        expression() = default;

        template<typename LHS> struct check {
            const LHS& lhs_;

            check(const LHS& lhs)
                : lhs_(lhs) {}

            operator result() const {
                if (lhs_) return {};
                return { Formatter::handler(lhs_) };
            }

        #define ASSERTION_OP(op) \
            template<typename RHS> result operator op (RHS&& rhs) const { \
                if (lhs_ op rhs) return {}; \
                return { Formatter::handler(lhs_, rhs, #op) }; \
            }

            ASSERTION_OP(==)
            ASSERTION_OP(!=)
            ASSERTION_OP(<=)
            ASSERTION_OP(<)
            ASSERTION_OP(>=)
            ASSERTION_OP(>)
            ASSERTION_OP(||)
            ASSERTION_OP(&&)

        #undef ASSERTION_OP
        };

	    template<typename T> check<T> operator << (T&& lhs) { return check<T>(lhs); }
    };
}

#undef assert
#define assert(expr) \
    do { \
        using namespace assertions; \
        if (result res = expression<formatter>() << expr; !res) { \
            std::cerr \
                << __FILE__ << ':' \
                << __LINE__ << ": " \
                << __PRETTY_FUNCTION__ \
                << ": Assertion `" << #expr << '\'' \
                << "' failed with `" << res.expression << "'." \
                << std::endl; \
        } \
    } while(0)

