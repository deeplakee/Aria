#include "common.h"
#include "runtime/interpretResult.h"
#include "runtime/vm.h"
#include "util/util.h"

#if ENABLE_READLINE
#include "readline/history.h"
#include "readline/readline.h"
#endif

namespace aria {
static void repl()
{
    VM vm;
    for (;;) {
#if ENABLE_READLINE
        char *line_c_str = readline("> ");
        if (!line_c_str) {
            cout << endl;
            break;
        }
        String line(line_c_str);
        if (!line.empty()) {
            add_history(line_c_str);
        }
        free(line_c_str);
#else
        String line;
        cout << "> ";
        if (!std::getline(std::cin, line)) {
            cout << endl;
            break;
        }
#endif
        vm.interpretByLine(line); // 解释执行输入
    }
}

static void runString(String str)
{
    VM vm;
    interpretResult result = vm.interpret(str);

    if (result == interpretResult::COMPILE_ERROR)
        exit(65);
    if (result == interpretResult::RUNTIME_ERROR)
        exit(70);
}

static void runFile(const char *path)
{
    String source;
    try {
        source = readFile(path);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl << std::endl;
        exit(74);
    }
    return runString(std::move(source));
}
} // namespace aria

int main(int argc, const char *argv[])
{
    if (argc == 1) {
        aria::repl();
    } else if (argc == 2) {
        aria::runFile(argv[1]);
    } else {
        std::cerr << aria::format("Usage: {} [path]\n", aria::programName);
        exit(64);
    }

    return 0;
}
