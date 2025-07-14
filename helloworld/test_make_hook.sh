#!/bin/bash
# test_make_hook.sh - 测试make调用gcc的hook功能

echo "🚀 ========== 测试Make调用GCC的Hook功能 =========="
echo ""

# 获取当前目录
CURRENT_DIR=$(pwd)
echo "📁 当前工作目录: $CURRENT_DIR"
echo ""

# 编译更新后的hook库
echo "🔨 编译更新后的系统调用hook库..."
gcc -shared -fPIC -o syscall_hook_fixed.so syscall_hook_fixed.c -ldl

if [ ! -f "syscall_hook_fixed.so" ]; then
    echo "❌ hook库编译失败!"
    exit 1
fi

echo "✅ hook库编译成功: syscall_hook_fixed.so"
echo ""

# 清理之前的文件
echo "🧹 清理之前的文件..."
rm -f syscall_hook.log
rm -f hello
echo "✅ 清理完成"
echo ""

# 显示makefile内容
echo "📋 Makefile内容:"
echo "=================="
cat makefile
echo "=================="
echo ""

# 使用LD_PRELOAD运行make
echo "🎯 ========== 开始Hook Make执行 =========="
echo "🔍 使用LD_PRELOAD执行make命令..."
echo "📊 系统调用跟踪已启动..."
echo ""

# 设置LD_PRELOAD并运行make
export LD_PRELOAD="$CURRENT_DIR/syscall_hook_fixed.so"
make

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

    # 统计exec系列调用
    echo ""
    echo "🔍 Exec系列调用统计:"
    for syscall in execl execlp execle execv execve execvp execvpe system; do
        count=$(grep -c "$syscall:" syscall_hook.log)
        if [ $count -gt 0 ]; then
            echo "  $syscall: $count 次"
        fi
    done

    # 查找进程创建相关调用
    echo ""
    echo "👶 进程创建相关调用:"
    grep -E "(fork|exec|wait)" syscall_hook.log | head -20

    # 检查是否捕获到gcc调用
    echo ""
    echo "🔍 检查是否捕获到gcc/g++调用:"
    gcc_calls=$(grep -i "g++" syscall_hook.log | wc -l)
    if [ $gcc_calls -gt 0 ]; then
        echo "✅ 成功捕获到 $gcc_calls 个g++相关调用!"
        echo "🎯 G++调用详情:"
        grep -i "g++" syscall_hook.log
    else
        echo "❌ 未捕获到g++调用"
        echo "💡 可能原因:"
        echo "   1. make使用了我们未hook的exec函数"
        echo "   2. make使用了内置命令或其他机制"
        echo "   3. hook被绕过了"
    fi

    echo ""
    echo "📄 完整日志已保存到: syscall_hook.log"
else
    echo "⚠️  未生成hook日志文件"
    echo "💡 可能的原因:"
    echo "   1. LD_PRELOAD 加载失败"
    echo "   2. 权限问题"
    echo "   3. 动态库路径错误"
fi

# 检查生成的可执行文件
echo ""
echo "📁 检查生成的文件:"
if [ -f "hello" ]; then
    echo "✅ 可执行文件已创建: hello"
    echo "📏 文件大小: $(ls -lh hello | awk '{print $5}')"
else
    echo "⚠️  可执行文件未创建"
fi

echo ""
echo "🎉 Make Hook测试完成!"
echo "🧹 清理环境变量..."
unset LD_PRELOAD
echo "✨ 完成!"