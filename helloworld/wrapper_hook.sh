#!/bin/bash
# wrapper_hook.sh - 使用编译器包装器的hook方案

echo "========== 编译器包装器 Hook 方案 =========="

# 创建临时目录用于包装器
WRAPPER_DIR="./wrapper_bin"
mkdir -p "$WRAPPER_DIR"

# 获取真实的编译器路径
REAL_GCC=$(which gcc)
REAL_GPP=$(which g++)

echo "真实编译器路径:"
echo "gcc: $REAL_GCC"
echo "g++: $REAL_GPP"
echo ""

# 创建gcc包装器
cat > "$WRAPPER_DIR/gcc" << 'EOF'
#!/bin/bash
# gcc wrapper

# 记录编译命令
echo "{" >> ./compile_trace.jsonl
echo "  \"timestamp\": $(date +%s)," >> ./compile_trace.jsonl
echo "  \"compiler\": \"gcc\"," >> ./compile_trace.jsonl
echo "  \"cwd\": \"$(pwd)\"," >> ./compile_trace.jsonl
echo "  \"command\": \"gcc $*\"," >> ./compile_trace.jsonl
echo "  \"args\": [" >> ./compile_trace.jsonl

# 记录参数
first=true
for arg in "$@"; do
    if [ "$first" = true ]; then
        first=false
    else
        echo "," >> ./compile_trace.jsonl
    fi
    echo -n "    \"$arg\"" >> ./compile_trace.jsonl
done
echo "" >> ./compile_trace.jsonl
echo "  ]" >> ./compile_trace.jsonl
echo "}" >> ./compile_trace.jsonl

# 执行真实的编译器
exec /usr/bin/gcc "$@"
EOF

# 创建g++包装器
cat > "$WRAPPER_DIR/g++" << 'EOF'
#!/bin/bash
# g++ wrapper

# 记录编译命令
echo "{" >> ./compile_trace.jsonl
echo "  \"timestamp\": $(date +%s)," >> ./compile_trace.jsonl
echo "  \"compiler\": \"g++\"," >> ./compile_trace.jsonl
echo "  \"cwd\": \"$(pwd)\"," >> ./compile_trace.jsonl
echo "  \"command\": \"g++ $*\"," >> ./compile_trace.jsonl
echo "  \"args\": [" >> ./compile_trace.jsonl

# 记录参数
first=true
for arg in "$@"; do
    if [ "$first" = true ]; then
        first=false
    else
        echo "," >> ./compile_trace.jsonl
    fi
    echo -n "    \"$arg\"" >> ./compile_trace.jsonl
done
echo "" >> ./compile_trace.jsonl
echo "  ]" >> ./compile_trace.jsonl
echo "}" >> ./compile_trace.jsonl

# 执行真实的编译器
exec /usr/bin/g++ "$@"
EOF

# 给包装器添加执行权限
chmod +x "$WRAPPER_DIR/gcc" "$WRAPPER_DIR/g++"

# 清理之前的日志
rm -f ./compile_trace.jsonl

echo "✅ 包装器创建完成"
echo "包装器位置: $WRAPPER_DIR"

# 修改PATH，让包装器优先
export PATH="$PWD/$WRAPPER_DIR:$PATH"

echo "修改后的PATH: $PATH"
echo ""

# 验证包装器
echo "验证包装器:"
which gcc
which g++
echo ""

# 清理编译结果
echo "清理之前的编译结果..."
make clean

# 执行make
echo "执行make (使用包装器)..."
make -d > wrapper_make.log 2>&1

echo "Make执行完成!"
echo ""

# 显示结果
echo "========== Hook 结果 =========="
if [ -f "./compile_trace.jsonl" ]; then
    echo "✅ 包装器Hook成功! 捕获到的编译命令:"
    cat ./compile_trace.jsonl
else
    echo "⚠️  未捕获到编译命令"
fi

echo ""
echo "详细的make调试信息保存在: wrapper_make.log"
echo "=========================="

# 清理包装器目录
echo ""
echo "清理包装器目录..."
rm -rf "$WRAPPER_DIR"
echo "完成!"