rm ./a* 2> /dev/null

#cppcheck --dump --std=c89 kangoo_can_filter.test.c
#python ../misra/misra.py kangoo_can_filter.test.c.dump --rule-texts=../misra/misra_c_2023__headlines_for_cppcheck.txt

cppcheck --dump --std=c89 ../kangoo_can_filter.h
python ../misra/misra.py ../kangoo_can_filter.h.dump --rule-texts=../misra/misra_c_2023__headlines_for_cppcheck.txt

cppcheck --dump --std=c89 ../fake_bms.h
python ../misra/misra.py ../fake_bms.h.dump --rule-texts=../misra/misra_c_2023__headlines_for_cppcheck.txt

gcc -I.. kangoo_can_filter.test.c -Wall -Wextra -g -std=c89 -pedantic
./a
