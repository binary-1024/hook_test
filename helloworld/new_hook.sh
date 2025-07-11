#!/bin/bash
# hook_make.sh - macOS版本的make hook脚本

# 设置动态库路径
export DYLD_INSERT_LIBRARIES="./gcc_spawn_tracer.so"

# 设置日志文件路径
export GCC_TRACE_LOG="./build_trace.jsonl"

# 清理之前的日志
rm -f ./build_trace.jsonl

# 运行make并记录详细调试信息
echo "开始hook make流程..."
echo "日志将保存到: $GCC_TRACE_LOG"
echo "详细的make调试信息将保存到: hook_make.log"

# 执行make并保存调试输出
make -d > hook_make.log 2>&1

echo "Make执行完成!"
echo "查看hook到的编译命令:"
if [ -f "$GCC_TRACE_LOG" ]; then
    cat "$GCC_TRACE_LOG"
else
    echo "未生成hook日志文件, 可能是因为没有实际的编译执行"
fi