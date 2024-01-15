FILES=$(find . -print | grep -i "\.c" | tr -s '\n' ' ')
WARNS="-W -Wall -Wextra -Wuninitialized -Wno-multichar"
FSANS="-fsanitize=address -fsanitize=undefined -fsanitize=leak -fsanitize-address-use-after-scope"
CGENS=""
LINKS=""
DEBUG="-g3"

echo "Executing with..."
echo "FILES: $FILES"
echo "WARNS: $WARNS"
echo "FSANS: $FSAN"
echo "CGENS: $CGEN"
echo "LINKS: $LINKS"
echo "DEBUG: $DEBUG"

echo "\n\nCounting..."
if command -v scc &> /dev/null; then
    scc
else
    find ./ -type f \( -iname \*.c -o -iname \*.h \) \
        | xargs wc -l \
        | sort -nr
fi


gcc $FILES -o ./bin/convolution $WARNS $LINKS $DEBUG $FSANS $CGENS -ftime-report \
    > tmp.txt 2>&1

echo "\nReporting..."
cat tmp.txt \
    | grep -E --color=never '^(Time variable| [[:alnum:]])' \
    | cut -c1-36,69-79 \
    | sed 's/   wall/ /' \
    | awk -v list="$FILES" 'BEGIN { file_index=1 }
        {
            if (gsub("Time variable", (file_index <= split(list, arr) ? arr[file_index] : "File"))) {
                file_index = (file_index < length(arr) ? file_index + 1 : 1)
            }
            print 
        }'

echo "\n\nBuilding..."
cat tmp.txt \
    | grep -v -E '^(Time variable| [[:alnum:]])' \
    | grep -v '^$'

rm tmp.txt

chmod a+x ./bin/convolution
echo "\n\nExecute ./bin/convolution to start convolution."
