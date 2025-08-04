#!/bin/bash

# 初始化变量
INPUT_DIR=""
EXTENSION=""
OUTPUT_FILE=""

# 使用getopt解析短选项和长选项
OPTIONS=$(getopt -o i:e:o: --long input-dir:,extension:,output-file: -n "$0" -- "$@")

# 检查解析是否成功
if [ $? -ne 0 ]; then
    echo "参数解析错误，请检查输入" >&2
    exit 1
fi

# 处理解析结果
eval set -- "$OPTIONS"

while true; do
    case "$1" in
        -i|--input-dir)
            INPUT_DIR="$2"
            shift 2
            ;;
        -e|--extension)
            EXTENSION="$2"
            shift 2
            ;;
        -o|--output-file)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "内部错误: 未处理的选项 $1" >&2
            exit 1
            ;;
    esac
done

# 检查所有必要参数是否提供
if [ -z "$INPUT_DIR" ] || [ -z "$EXTENSION" ] || [ -z "$OUTPUT_FILE" ]; then
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -i, --input-dir      输入目录（必需）"
    echo "  -e, --extension      图片文件扩展名（必需，不带点）"
    echo "  -o, --output-file    输出HTML文件名（必需）"
    echo "示例:"
    echo "  $0 -i ./tiles/15 -e jpg -o grid_map.html"
    echo "  $0 --input-dir ./tiles/15 --extension png --output-file grid_map.html"
    exit 1
fi

# 检查输入目录是否存在
if [ ! -d "$INPUT_DIR" ]; then
    echo "错误: 输入目录 '$INPUT_DIR' 不存在" >&2
    exit 1
fi

# 获取输出文件所在目录
OUTPUT_DIR=$(dirname "$OUTPUT_FILE")
mkdir -p "$OUTPUT_DIR"

# 提取所有子目录的数字名称并找到最小和最大值
# 只处理名称为纯数字的子目录
SUBDIR_NUMBERS=()
while IFS= read -r subdir; do
    dir_name=$(basename "$subdir")
    # 检查目录名是否为纯数字
    if [[ "$dir_name" =~ ^[0-9]+$ ]]; then
        SUBDIR_NUMBERS+=("$dir_name")
    fi
done < <(find "$INPUT_DIR" -mindepth 1 -maxdepth 1 -type d)

if [ ${#SUBDIR_NUMBERS[@]} -eq 0 ]; then
    echo "错误: 输入目录 '$INPUT_DIR' 中没有名称为数字的子目录" >&2
    exit 1
fi

# 找到子目录数字的最小值和最大值
MIN_SUBDIR=$(printf "%s\n" "${SUBDIR_NUMBERS[@]}" | sort -n | head -n 1)
MAX_SUBDIR=$(printf "%s\n" "${SUBDIR_NUMBERS[@]}" | sort -n | tail -n 1)
COLUMNS=$((MAX_SUBDIR - MIN_SUBDIR + 1))

# 收集所有文件的数字部分并找到全局最小和最大值
FILE_NUMBERS=()
while IFS= read -r file; do
    # 提取文件名（不含路径和扩展名）
    base_name=$(basename "$file" ".$EXTENSION")
    # 检查是否为纯数字
    if [[ "$base_name" =~ ^[0-9]+$ ]]; then
        FILE_NUMBERS+=("$base_name")
    fi
done < <(find "$INPUT_DIR" -type f -name "*.$EXTENSION")

if [ ${#FILE_NUMBERS[@]} -eq 0 ]; then
    echo "错误: 没有找到名称为数字且扩展名为 '$EXTENSION' 的文件" >&2
    exit 1
fi

# 找到文件数字的最小值和最大值
MIN_FILE=$(printf "%s\n" "${FILE_NUMBERS[@]}" | sort -n | head -n 1)
MAX_FILE=$(printf "%s\n" "${FILE_NUMBERS[@]}" | sort -n | tail -n 1)
ROWS=$((MAX_FILE - MIN_FILE + 1))

# 开始生成HTML文件
echo "<!DOCTYPE html>" > "$OUTPUT_FILE"
echo "<html lang='zh'>" >> "$OUTPUT_FILE"
echo "<head>" >> "$OUTPUT_FILE"
echo "    <meta charset='UTF-8'>" >> "$OUTPUT_FILE"
echo "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>" >> "$OUTPUT_FILE"
echo "    <title>图片网格展示</title>" >> "$OUTPUT_FILE"
echo "    <style>" >> "$OUTPUT_FILE"
echo "        body { margin: 0; padding: 0; background-color: #fff; }" >> "$OUTPUT_FILE"
echo "        .grid-container {" >> "$OUTPUT_FILE"
echo "            display: grid;" >> "$OUTPUT_FILE"
echo "            grid-template-columns: repeat($COLUMNS, auto);" >> "$OUTPUT_FILE"  # 按内容自动调整列宽
echo "            gap: 1px;" >> "$OUTPUT_FILE"  # 1像素间距
echo "            background-color: #ccc;" >> "$OUTPUT_FILE"  # 背景色使间距可见
echo "        }" >> "$OUTPUT_FILE"
echo "        .grid-item {" >> "$OUTPUT_FILE"
echo "            padding: 0;" >> "$OUTPUT_FILE"
echo "            margin: 0;" >> "$OUTPUT_FILE"
echo "            background-color: #fff;" >> "$OUTPUT_FILE"
echo "        }" >> "$OUTPUT_FILE"
echo "        .grid-item img {" >> "$OUTPUT_FILE"
echo "            display: block;" >> "$OUTPUT_FILE"
echo "            /* 不设置宽度和高度，保持图片原始大小 */" >> "$OUTPUT_FILE"
echo "        }" >> "$OUTPUT_FILE"
echo "        .missing {" >> "$OUTPUT_FILE"  # 缺失文件的占位样式
echo "            background-color: #f0f0f0;" >> "$OUTPUT_FILE"
echo "            min-width: 100px;" >> "$OUTPUT_FILE"
echo "            min-height: 100px;" >> "$OUTPUT_FILE"
echo "        }" >> "$OUTPUT_FILE"
echo "    </style>" >> "$OUTPUT_FILE"
echo "</head>" >> "$OUTPUT_FILE"
echo "<body>" >> "$OUTPUT_FILE"
echo "    <div class='grid-container'>" >> "$OUTPUT_FILE"

# 按数字范围生成网格
for ((file_num=MIN_FILE; file_num<=MAX_FILE; file_num++)); do
    for ((dir_num=MIN_SUBDIR; dir_num<=MAX_SUBDIR; dir_num++)); do
        # 构建文件路径
        subdir_path="$INPUT_DIR/$dir_num"
        file_path="$subdir_path/$file_num.$EXTENSION"
        
        # 检查文件是否存在
        if [ -f "$file_path" ]; then
            # 计算相对路径
            abs_file=$(realpath "$file_path")
            abs_output_dir=$(realpath "$OUTPUT_DIR")
            rel_path=$(realpath --relative-to="$abs_output_dir" "$abs_file")
            filename=$(basename "$file_path")
            
            # 添加图片
            echo "        <div class='grid-item'>" >> "$OUTPUT_FILE"
            echo "            <img src=\"$rel_path\" alt=\"$dir_num/$filename\">" >> "$OUTPUT_FILE"
            echo "        </div>" >> "$OUTPUT_FILE"
        else
            # 缺失文件的占位符
            echo "        <div class='grid-item missing'></div>" >> "$OUTPUT_FILE"
        fi
    done
done

# 完成HTML文件
echo "    </div>" >> "$OUTPUT_FILE"
echo "</body>" >> "$OUTPUT_FILE"
echo "</html>" >> "$OUTPUT_FILE"

echo "HTML文件生成成功: $OUTPUT_FILE"
echo "网格尺寸: $ROWS 行 x $COLUMNS 列"
echo "子目录范围: $MIN_SUBDIR 到 $MAX_SUBDIR"
echo "文件范围: $MIN_FILE 到 $MAX_FILE"