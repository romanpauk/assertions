#include <gtest/gtest.h>

#define ASSERTIONS_HANDLER_TAG int
#include <assertions/assert.h>

namespace assertions {
    template<> struct handler<int> {
        static void notify(const char* file, const int line, const char* function,
            const char* msg, const char* expr, const char* ctx) {
            std::stringstream stream;
            handler<void>::format(stream, file, line, function, msg, expr, ctx);
            throw std::runtime_error(stream.str());
        }
    };
};

TEST(AssertionTest, FailingAssertionThrows) {
    int a = 1;
    int b = 20;
    bool exception_caught = false;
    try {
        assert(a > 0 && b < 0);
    }
    catch (const std::runtime_error& e) {
        exception_caught = true;
        std::string what = e.what();
        EXPECT_NE(what.find("Assertion `a > 0 && b < 0' failed with `(1 > 0) && (20 < 0)'."), std::string::npos);
    }
    catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
    EXPECT_TRUE(exception_caught);
}