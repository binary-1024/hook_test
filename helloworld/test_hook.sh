#!/bin/bash
# test_hook.sh - 测试hook是否工作

echo "========== 测试 Hook 功能 =========="

# 获取当前目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 设置环境变量
export DYLD_INSERT_LIBRARIES="${SCRIPT_DIR}/gcc_spawn_tracer.so"
export GCC_TRACE_LOG="${SCRIPT_DIR}/test_trace.jsonl"

echo "测试配置:"
echo "DYLD_INSERT_LIBRARIES: $DYLD_INSERT_LIBRARIES"
echo "GCC_TRACE_LOG: $GCC_TRACE_LOG"
echo ""

# 清理之前的测试日志
rm -f "$GCC_TRACE_LOG"

echo "执行简单的gcc命令进行测试..."
echo "命令: gcc --version"
echo ""

# 执行测试命令
gcc --version

echo ""
echo "========== 测试结果 =========="
if [ -f "$GCC_TRACE_LOG" ]; then
    echo "✅ Hook工作正常! 捕获到的调用:"
    cat "$GCC_TRACE_LOG"
else
    echo "⚠️  Hook可能没有工作"
    echo ""
    echo "可能的原因和解决方案:"
    echo "1. macOS安全限制 (SIP) - 可能需要在恢复模式下禁用SIP"
    echo "2. 检查动态库是否存在:"
    ls -la "$DYLD_INSERT_LIBRARIES"
    echo "3. 检查动态库依赖是否正确:"
    otool -L "$DYLD_INSERT_LIBRARIES"
fi
echo "=========================="