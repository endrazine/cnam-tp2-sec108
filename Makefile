CC := gcc 
CFLAGS := -W -Wall -Wextra -Wno-unused-parameter -fno-stack-protector -z execstack -no-pie -m32

all::
	$(CC) $(CFLAGS) bo.c -o bo
test:
	./bo `perl -e 'print "A"x1024 '`
clean:
	rm -f bo

disableaslr:
	echo 0 | sudo tee /proc/sys/kernel/randomize_va_space

