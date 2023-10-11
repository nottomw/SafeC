#!/bin/bash

# very quick & dirty test for now...

# regen all:
# for i in `ls AST*`; do ../../build/bin/SafeCTranspiler -f $i -o . -c -n > ./testfiles_generated_coverage/${i##*/}.COV; done

SCRIPT_NAME=$(basename "$0")
TMP_FILE_PREFIX=/tmp/safec_cov_test_file_
GENERATED_DIR=testfiles_generated_coverage
GENERATED_FILE_EXT=COV

if [ -z "$SAFEC_PATH" ];
then
    SAFEC_PATH=../../build/bin/SafeCTranspiler
fi

AST_FILE_PREFIX="AST_"

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
                echo "[+] Coverage check for $file..."
                $($SAFEC_PATH -f $file -o . -c -n > $TMP_FILE_PREFIX$file)
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
                    echo "[-]     - $SAFEC_PATH -f $file -o . -c -n > $FILE_GENERATED"
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
