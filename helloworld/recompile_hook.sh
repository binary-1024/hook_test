#!/bin/bash
# recompile_hook.sh - 重新编译hook动态库

echo "========== 重新编译 Hook 动态库 =========="

# 进入源码目录
cd ../posix_spawn

# 显示编译信息
echo "编译信息:"
echo "源文件: gcc_spawn_tracer.c"
echo "目标文件: gcc_spawn_tracer.so"
echo "编译器: $(which gcc)"
echo "架构: $(uname -m)"
echo ""

# macOS M4 (arm64) 编译命令
echo "正在编译..."
gcc -shared -fPIC -o gcc_spawn_tracer.so gcc_spawn_tracer.c \
    -ldl \
    -ljson-c \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib

# 检查编译结果
if [ $? -eq 0 ]; then
    echo "✅ 编译成功!"

    # 复制到helloworld目录
    cp gcc_spawn_tracer.so ../helloworld/
    echo "✅ 已复制到 helloworld 目录"

    # 显示文件信息
    echo ""
    echo "========== 文件信息 =========="
    file gcc_spawn_tracer.so
    echo ""
    echo "依赖库:"
    otool -L gcc_spawn_tracer.so
    echo "=========================="
else
    echo "❌ 编译失败!"
    exit 1
fi

cd ../helloworld
echo "返回到 helloworld 目录"