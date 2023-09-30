#!/bin/bash

# very quick & dirty test for now...

SCRIPT_NAME=$(basename "$0")
SAFEC_PATH=../../build/bin/SafeCTranspiler
TMP_FILE=/tmp/safec_test_file.AST
GENERATED_AST_DIR=testfiles_generated_asts

AST_FILE_PREFIX="AST_"

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
                echo "[+] testing $file AST..."
                $($SAFEC_PATH -f $file -o . -a -n > $TMP_FILE)
                diff_output=`diff $FILE_GENERATED $TMP_FILE`
                if [ "$diff_output" = "" ];
                then
                    echo "[+] passed"
                else
                    echo "[-] FAILED"
                    echo " ------ DIFF START ------"
                    echo $diff_output
                    echo " ------ DIFF END ------"
                fi
            fi
        fi
    fi
done
