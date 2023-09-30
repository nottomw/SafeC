#!/bin/bash

# very quick & dirty test for now...

# regen all:
# for i in `ls`; do ./bin/SafeCTranspiler -f $i -o . -a -n > ./testfiles_generated_asts/${i##*/}.AST; done

SCRIPT_NAME=$(basename "$0")
SAFEC_PATH=../../build/bin/SafeCTranspiler
TMP_FILE_PREFIX=/tmp/safec_ast_test_file_
GENERATED_AST_DIR=testfiles_generated_asts

AST_FILE_PREFIX="AST_"

tests_passed=0
tests_failed=0

for file in `ls`;
do
    if [ "$file" != "$SCRIPT_NAME" ];
    then
        if [[ "$file" == "$AST_FILE_PREFIX"* ]];
        then
            FILE_GENERATED="$GENERATED_AST_DIR/$file.AST"
            # is there a generated ast for this file
            if [ -e "$FILE_GENERATED" ];
            then
                echo "[+] AST check for $file..."
                $($SAFEC_PATH -f $file -o . -a -n > $TMP_FILE_PREFIX$file)
                diff_output=`diff -q $FILE_GENERATED $TMP_FILE_PREFIX$file`
                if [ "$diff_output" = "" ];
                then
                    echo "[+] passed"
                    ((tests_passed++))
                else
                    echo "[-] FAILED"
                    echo "[-]    run to see differences:"
                    echo "[-]     - diff $FILE_GENERATED $TMP_FILE_PREFIX$file"
                    echo "[-]     - kdiff3 $FILE_GENERATED $TMP_FILE_PREFIX$file"
                    echo "[-]    run to regenerate:"
                    echo "[-]     - $SAFEC_PATH -f $file -o . -a -n > $FILE_GENERATED"
                    ((tests_failed++))
                fi
            fi
        fi
    fi
done

echo ""
echo "[+] passed: $tests_passed, failed: $tests_failed"
