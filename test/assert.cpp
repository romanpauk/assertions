#define ASSERTIONS_HANDLER_TAG int
#include <assertions/assert.h>

namespace assertions {
    template<> struct handler<int> {
        static void notify(const char* file, const int line, const char* function,
            const char* expr, const char* details) {
            std::stringstream stream;
            handler<void>::format(stream, file, line, function, expr, details);
            throw std::runtime_error(stream.str());
        }
    };
};

int main() {
    int a = 1;
    int b = 20;

    //assert(a == b);
    // a.out: assert.cpp:28: int main(): Assertion `a == b' failed.

    assert(a > 0 && b < 0);
}
