
# 安装必要的组件
sudo apt install cmake

## 编译并运行程序
```shell
cd build 
meson setup ../
meson compile  # 或者 ninja
export DISKEY_DEBUG=1  && ./diskey/diskey
```

## 为了查看内存泄露问题 使用 valgrind 工具
编译时添加 '-g' 选项，然后使用如下命令查看分析结果
```shell
valgrind --leak-check=full --log-file=./go.txt --show-leak-kinds=all -s ./diskey/diskey
```
