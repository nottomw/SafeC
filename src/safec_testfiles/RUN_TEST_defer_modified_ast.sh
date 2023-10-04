#!/bin/bash

# very quick & dirty test for now...

SCRIPT_NAME=$(basename "$0")
TMP_FILE_PREFIX=/tmp/safec_defer_ast_test_file_
GENERATED_DIR=testfiles_generated_defer_ast
GENERATED_FILE_EXT=AST

if [ -z "$SAFEC_PATH" ];
then
    SAFEC_PATH=../../build/bin/SafeCTranspiler
fi

AST_FILE_PREFIX="AST_defer"

COLOR_RED="\033[31m"
COLOR_GREEN="\033[32m"
COLOR_NC="\033[0m"

tests_passed=0
tests_failed=0

for file in `ls`;
do
    if [ "$file" != "$SCRIPT_NAME" ];
    then
        if [[ "$file" == "$AST_FILE_PREFIX"* ]];
        then
            FILE_GENERATED="$GENERATED_DIR/$file.$GENERATED_FILE_EXT"
            if [ -e "$FILE_GENERATED" ];
            then
                echo "[+] Defer modified AST check for $file..."
                $($SAFEC_PATH -f $file -o /tmp -n --astdump-mod --generate > $TMP_FILE_PREFIX$file)
                diff_output=`diff -q $FILE_GENERATED $TMP_FILE_PREFIX$file`
                if [ "$diff_output" = "" ];
                then
                    echo -e "[+] ${COLOR_GREEN}passed${COLOR_NC}"
                    ((tests_passed++))
                else
                    echo -e "[-] ${COLOR_RED}FAILED${COLOR_NC}"
                    echo "[-]    run to see differences:"
                    echo "[-]     - diff $FILE_GENERATED $TMP_FILE_PREFIX$file"
                    echo "[-]     - kdiff3 $FILE_GENERATED $TMP_FILE_PREFIX$file"
                    echo "[-]    run to regenerate:"
                    echo "[-]     - $SAFEC_PATH -f $file -o /tmp -n --astdump-mod --generate > $FILE_GENERATED"
                    ((tests_failed++))
                fi
            fi
        fi
    fi
done

echo ""
if [ "$tests_failed" -eq 0 ];
then
    echo -e "[+] SUMMARY: ${COLOR_GREEN}passed: $tests_passed, failed: $tests_failed${COLOR_NC}"
    exit 0
else
    echo -e "[-] SUMMARY: ${COLOR_RED}passed: $tests_passed, failed: $tests_failed${COLOR_NC}"
    exit 1
fi
