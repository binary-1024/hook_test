#!/bin/bash
# syscall_hook_fixed.sh - 修复版本的系统调用Hook脚本

echo "🚀 ========== 修复版系统调用LD_PRELOAD Hook脚本 =========="
echo ""

# 获取当前目录
CURRENT_DIR=$(pwd)
echo "📁 当前工作目录: $CURRENT_DIR"
echo ""

# 编译修复版hook库
echo "🔨 编译修复版系统调用hook库..."
gcc -shared -fPIC -o syscall_hook_fixed.so syscall_hook_fixed.c -ldl

if [ ! -f "syscall_hook_fixed.so" ]; then
    echo "❌ hook库编译失败!"
    exit 1
fi

echo "✅ hook库编译成功: syscall_hook_fixed.so"
echo ""

# 清理之前的日志和临时文件
echo "🧹 清理之前的日志和临时文件..."
rm -f syscall_hook.log
rm -f temp_test.txt
echo "✅ 清理完成"
echo ""

# 编译测试程序
echo "🔨 编译hello程序..."
gcc -o hello hello.c

if [ ! -f "hello" ]; then
    echo "❌ hello程序编译失败!"
    exit 1
fi

echo "✅ hello程序编译成功"
echo ""

# 使用LD_PRELOAD运行hook
echo "🎯 ========== 开始Hook执行 =========="
echo "🔍 使用LD_PRELOAD执行hello程序..."
echo "📊 系统调用跟踪已启动..."
echo ""

# 设置LD_PRELOAD并运行程序
export LD_PRELOAD="$CURRENT_DIR/syscall_hook_fixed.so"
./hello

echo ""
echo "📈 ========== Hook执行结果 =========="

if [ -f "syscall_hook.log" ]; then
    echo "✅ 成功捕获系统调用! 详细日志:"
    echo "📋 ----------------------------------------"
    cat syscall_hook.log
    echo "📋 ----------------------------------------"

    # 统计各类系统调用数量
    echo ""
    echo "📊 系统调用统计:"
    echo "总调用次数: $(wc -l < syscall_hook.log)"

    for syscall in fork execl wait getpid getuid getcwd open write close access sleep unlink; do
        count=$(grep -c "$syscall:" syscall_hook.log)
        if [ $count -gt 0 ]; then
            echo "  $syscall: $count 次"
        fi
    done

    echo ""
    echo "📄 完整日志已保存到: syscall_hook.log"
else
    echo "⚠️  未生成hook日志文件"
    echo "💡 可能的原因:"
    echo "   1. LD_PRELOAD 加载失败"
    echo "   2. 权限问题"
    echo "   3. 动态库路径错误"
fi

# 检查生成的临时文件
echo ""
echo "📁 检查生成的文件:"
if [ -f "temp_test.txt" ]; then
    echo "✅ 临时文件已创建: temp_test.txt"
    echo "📄 文件内容:"
    cat temp_test.txt
else
    echo "⚠️  临时文件未创建"
fi

echo ""
echo "🎉 Hook执行完成!"
echo "🧹 清理环境变量..."
unset LD_PRELOAD
echo "✨ 完成!"