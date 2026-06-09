#!/usr/bin/env bash
# 一键检查: clang-format + clang-tidy + build + tests
# 用法: utils/check.sh [--skip-tidy] [--skip-build] [--skip-test]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
RESET='\033[0m'

pass() { echo -e "${GREEN}[PASS]${RESET} $1"; }
fail() { echo -e "${RED}[FAIL]${RESET} $1"; }
info() { echo -e "${BOLD}[....]${RESET} $1"; }
warn() { echo -e "${YELLOW}[WARN]${RESET} $1"; }

SKIP_TIDY=false
SKIP_BUILD=false
SKIP_TEST=false
for arg in "$@"; do
    case "$arg" in
        --skip-tidy)  SKIP_TIDY=true  ;;
        --skip-build) SKIP_BUILD=true ;;
        --skip-test)  SKIP_TEST=true ;;
    esac
done

TOTAL=0
PASSED=0
FAILED=0

run_check() {
    local name="$1"
    TOTAL=$((TOTAL + 1))
    info "$name"
}

mark_pass() {
    PASSED=$((PASSED + 1))
    pass "$1"
}

mark_fail() {
    FAILED=$((FAILED + 1))
    fail "$1"
}

# ============================================================
# 1. clang-format
# ============================================================
run_check "clang-format"

cd "$PROJECT_ROOT"
mapfile -d '' FORMAT_FILES < <(
    find src -name '*.cpp' -print0 -o -name '*.hpp' -print0
)

if clang-format --dry-run --Werror "${FORMAT_FILES[@]}" 2>&1; then
    mark_pass "clang-format"
else
    mark_fail "clang-format (run: find src -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i)"
fi

# ============================================================
# 2. clang-tidy
# ============================================================
if [ "$SKIP_TIDY" = false ]; then
    run_check "clang-tidy"

    cd "$PROJECT_ROOT"

    # 生成 compile_commands.json
    xmake project -k compile_commands >/dev/null 2>&1 || true

    # libc++ 头文件路径 (Clang 18 on Ubuntu)
    LIBCXX_INCLUDE=""
    if [ -d "/usr/lib/llvm-18/include/c++/v1" ]; then
        LIBCXX_INCLUDE="/usr/lib/llvm-18/include/c++/v1"
    elif [ -d "/usr/lib/llvm/include/c++/v1" ]; then
        LIBCXX_INCLUDE="/usr/lib/llvm/include/c++/v1"
    fi

    TIDY_ARGS=(
        -p .
        --extra-arg="-Isrc"
        --extra-arg="-std=c++23"
        --extra-arg="-stdlib=libc++"
    )
    if [ -n "$LIBCXX_INCLUDE" ]; then
        TIDY_ARGS+=(--extra-arg="-I$LIBCXX_INCLUDE")
    fi

    TIDY_OUTPUT=$(clang-tidy "${TIDY_ARGS[@]}" \
        src/main.cpp \
        src/benchmark/benchmark.cpp \
        src/benchmark/blas_functions.cpp \
        src/config/config_parser.cpp \
        src/output/output_formatter.cpp \
        src/utils/system_info.cpp \
        2>&1) || true

    # 只提取用户代码的 warning/error (跳过 suppressed 和 "warnings generated")
    TIDY_ISSUES=$(echo "$TIDY_OUTPUT" | grep -E "warning:|error:" | grep -v "Suppressed" || true)

    if [ -z "$TIDY_ISSUES" ]; then
        mark_pass "clang-tidy"
    else
        mark_fail "clang-tidy"
        echo "$TIDY_ISSUES" | sed 's/^/  /'
    fi
else
    warn "clang-tidy (skipped)"
fi

# ============================================================
# 3. build
# ============================================================
if [ "$SKIP_BUILD" = false ]; then
    run_check "build (xmake release)"

    cd "$PROJECT_ROOT"
    if xmake f -m release >/dev/null 2>&1 && xmake >/dev/null 2>&1; then
        mark_pass "build"
    else
        mark_fail "build"
        xmake 2>&1 | sed 's/^/  /' || true
    fi
else
    warn "build (skipped)"
fi

# ============================================================
# 4. tests
# ============================================================
if [ "$SKIP_TEST" = false ]; then
    run_check "tests (smoke run)"

    cd "$PROJECT_ROOT"
    TEST_FAIL=0

    if ! xmake run blas_benchmark -1 1000 -2 64,64 -3 64,64,64 -c 1 -w 0 -t 1 >/dev/null 2>&1; then
        TEST_FAIL=$((TEST_FAIL + 1))
        echo "  FAIL: smoke test (benchmark run)"
    fi

    if [ "$TEST_FAIL" -eq 0 ]; then
        mark_pass "tests"
    else
        mark_fail "tests ($TEST_FAIL failed)"
    fi
else
    warn "tests (skipped)"
fi

# ============================================================
# Summary
# ============================================================
echo ""
echo "========================================"
if [ "$FAILED" -eq 0 ]; then
    echo -e "${GREEN}All $TOTAL checks passed.${RESET}"
else
    echo -e "${RED}$FAILED of $TOTAL checks failed.${RESET}"
fi
echo "========================================"

exit "$FAILED"
