#!/bin/bash

SCRIPT_NAME=$(basename "$0")

COLOR_RED="\033[31m"
COLOR_GREEN="\033[32m"
COLOR_NC="\033[0m"

passed_test_groups=0
failed_test_groups=0

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
