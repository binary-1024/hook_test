#!/bin/bash
# hook_make.sh - macOS版本的make hook脚本

# 获取当前脚本所在目录的绝对路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 设置动态库路径 - 使用绝对路径更可靠
export DYLD_INSERT_LIBRARIES="${SCRIPT_DIR}/gcc_spawn_tracer.so"

# 设置日志文件路径
export GCC_TRACE_LOG="${SCRIPT_DIR}/build_trace.jsonl"

# 打印配置信息
echo "========== macOS Hook 配置 =========="
echo "DYLD_INSERT_LIBRARIES: $DYLD_INSERT_LIBRARIES"
echo "GCC_TRACE_LOG: $GCC_TRACE_LOG"
echo "当前工作目录: $(pwd)"
echo "====================================="

# 检查so文件是否存在
if [ ! -f "$DYLD_INSERT_LIBRARIES" ]; then
    echo "错误: 找不到动态库文件: $DYLD_INSERT_LIBRARIES"
    exit 1
fi

# 清理之前的日志
rm -f "$GCC_TRACE_LOG"
rm -f "${SCRIPT_DIR}/hook_make.log"

echo "开始hook make流程..."

# 首先清理编译结果
echo "清理之前的编译结果..."
make clean

# 执行make并保存调试输出
echo "执行make (带hook)..."
make -d > "${SCRIPT_DIR}/hook_make.log" 2>&1

echo "Make执行完成!"

# 显示结果
echo ""
echo "========== Hook 结果 =========="
if [ -f "$GCC_TRACE_LOG" ]; then
    echo "✅ Hook成功! 捕获到以下系统调用:"
    echo "日志文件: $GCC_TRACE_LOG"
    echo "内容:"
    cat "$GCC_TRACE_LOG"
else
    echo "⚠️  未生成hook日志文件"
    echo "可能原因:"
    echo "1. macOS安全限制 (SIP)"
    echo "2. 动态库加载失败"
    echo "3. 没有实际的编译执行"
fi

echo ""
echo "详细的make调试信息保存在: ${SCRIPT_DIR}/hook_make.log"
echo "=============================="