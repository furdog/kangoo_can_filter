rm ./a* 2> /dev/null

gcc -I.. kangoo_can_filter.test.c -Wall -Wextra -g -std=c89 -pedantic
./a
