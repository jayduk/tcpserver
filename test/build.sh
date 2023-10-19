#!/bin/bash
# author:菜鸟教程
# url:www.runoob.com


for dir in */; do
    if [ -d "$dir" ]; then
        echo "$dir"
        # 如果是文件夹
        cd "$dir"  # 进入文件夹
        if [ -e "main.cpp" ]; then
            object_files=$(find ../../build -type f -name '*.o' -not -name 'main.o')

            g++ $object_files main.cpp -o a.out
        fi
        cd ..  # 返回到上级目录
    fi
done