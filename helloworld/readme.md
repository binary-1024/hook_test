### hook 加载库编译
gcc -shared -fPIC -o syscall_hook_fixed.so syscall_hook_fixed.c -ldl

## 加载环境变量  export LD_PRELOAD="$(pwd)/syscall_hook_fixed.so"
## check 一下 echo $LD_PRELOAD

export GCC_TRACE_LOG="./build_trace.jsonl"
