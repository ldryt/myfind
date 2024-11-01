#!/usr/bin/env bash

test "$#" -ne 1 && echo "Need path of myfind as argument" && exit 1

myfind_exec=$(realpath "$1")

cd $(mktemp -d)

test_find() {
    params="$@"
    echo "Testing with parameters: $params"

    find_output=$(find $params 2>/dev/null)

    myfind_output=$("$myfind_exec" $params 2>/dev/null)

    if diff <(echo "$find_output") <(echo "$myfind_output"); then
        echo "Test passed"
        echo "---"
    else
        echo "------"
        echo "ᕕ(╭ರ╭ ͟ʖ╮•́)⊃¤=(————-"
        echo "Test failed with parameters: $params"
        exit 1
    fi
}

test_find .

test_find . -name "*.txt"

test_find . -type f

test_find . -type ff

test_find . -type d

test_find . -perm 644

touch a
test_find . -newer a

echo "All tests passed ٩(^‿^)۶"
