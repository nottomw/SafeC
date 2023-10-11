#!/bin/bash

SCRIPT_NAME=$(basename "$0")

COLOR_RED="\033[31m"
COLOR_GREEN="\033[32m"
COLOR_NC="\033[0m"

if [ -z "$SAFEC_PATH" ];
then
    SAFEC_PATH=../../build/bin/SafeCTranspiler
fi

passed_test_groups=0
failed_test_groups=0

if [ "$1" = "regen" ];
then
    echo -e "${COLOR_RED}REGENERATING ALL TEST CASES${COLOR_NC}"

    for i in `ls AST*`;
    do
        ${SAFEC_PATH} -f $i -o . -a -n > ./testfiles_generated_asts/${i##*/}.AST
        ${SAFEC_PATH} -f $i -o . -c -n > ./testfiles_generated_coverage/${i##*/}.COV
    done

    ${SAFEC_PATH} -f AST_defer.sc -o /tmp -n --astdump-mod --generate > testfiles_generated_defer_ast/AST_defer.sc.AST

    echo -e "${COLOR_GREEN}REGENERATING ALL TEST CASES DONE${COLOR_NC}"

    exit 0
fi

for i in `ls *.sh`;
do
    if [ "$i" != "$SCRIPT_NAME" ];
    then
        echo "Running test group $i..."
        bash ./$i

        if [ $? -eq 0 ];
        then
            ((passed_test_groups++))
        else
            ((failed_test_groups++))
        fi

        echo ""
    fi
done

echo ""
if [ "$failed_test_groups" -eq 0 ];
then
    echo -e "[+] ${COLOR_GREEN}test groups passed: $passed_test_groups, failed: $failed_test_groups${COLOR_NC}"
    exit 0
else
    echo -e "[-] ${COLOR_RED}test groups passed: $passed_test_groups, failed: $failed_test_groups${COLOR_NC}"
    exit 1
fi
