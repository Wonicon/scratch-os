#!/usr/bin/env bash

# $# 参数个数, 从 $1 开始
if [ $# -eq 0 ]
then
    echo "Usage: $0 <image>"
    exit 1
elif [ $1 == "--help" ]
then
    echo "Convert image file to bgr array with C source and header file"
    exit 0
fi

pic="$1" # $1 有空格会被分开视作独立的元素

# 去除路径和扩展名(以第一个 . 为准)
#   e.g. foo/bar.com.net.ext #=> bar
# 这个字符串将作为输出用的文件名
out=$(basename "$pic" | cut -d '.' -f 1 | sed 's/ /_/g')

# 将图片转换成纯像素数组, BGR 排序, 二进制文件
echo "Convert to BGR binary file ${out}.rgb"
rgb="$out.rgb"
convert "$pic" "$rgb"
# 将BGR二进制文件转换成 C 风格数组
echo "Generate C source file ${out}.c"
xxd -i "$rgb" > "$out.c"
rm "$rgb"

# 获取标识符, 防止标识符和 $out 不一致
# 取第一行，去掉类型和固定的后缀等, _rgb 是因为前面用的 .rgb 做扩展名
identifier=$(head -n 1 "$out.c" | sed 's/unsigned char //' | sed 's/_rgb\[\] = {//')
echo "identifier: $identifier"

# 获取宽度和高度, 并加入到源文件中
width=$(identify -format "%[fx:w]" "$pic")
echo "int ${identifier}_w = $width;" >> "$out.c"
echo "${identifier}_width $width"

height=$(identify -format "%[fx:h]" "$pic")
echo "int ${identifier}_h = $height;" >> "$out.c"
echo "${identifier}_height $height"

# 组织生成头文件
echo "Generate C header file"

echo "#ifndef PIC_${out}_H" > "$out.h"
echo -e "#define PIC_${out}_H\n" >> "$out.h"
# 用 <空格>= 做分割符配合 NF > 1, 过滤掉非标识符的行和不需要的初始化语句
# 保留类型和标识符以生成声明
awk -F " =" '{ if (NF > 1) print "extern " $1 ";" }' "$out.c" >> "$out.h"
echo -e "\n#endif  // PIC_${out}_H" >> "$out.h"
