all: ../bin/server1b-zlabya ../bin/server2b-zlabya ../bin/server3b-zlabya

#server 1
../bin/server1b-zlabya: initialization.o srtt.o server1b-zlabya.o
	gcc -Wall -pthread server1b-zlabya.o initialization.o srtt.o -o ../bin/server1b-zlabya -lrt

server1b-zlabya.o: server1b-zlabya.c myHeaders.h
	gcc -Wall -pthread -c server1b-zlabya.c -o server1b-zlabya.o

#server 2
../bin/server2b-zlabya: initialization.o srtt.o server2b-zlabya.o
	gcc -Wall -pthread server2b-zlabya.o initialization.o srtt.o -o ../bin/server2b-zlabya -lrt
	
server2b-zlabya.o: server2b-zlabya.c myHeaders.h
	gcc -Wall -pthread -c server2b-zlabya.c -o server2b-zlabya.o

#server 3
../bin/server3b-zlabya: initialization.o srtt.o server3b-zlabya.o
	gcc -Wall -pthread server3b-zlabya.o initialization.o srtt.o -o ../bin/server3b-zlabya -lrt

server3b-zlabya.o: server3b-zlabya.c myHeaders.h
	gcc -Wall -pthread -c server3b-zlabya.c -o server3b-zlabya.o

#Common files
initialization.o: initialization.c myHeaders.h
	gcc -Wall -c initialization.c -o initialization.o

srtt.o: srtt.c myHeaders.h
	gcc -Wall -c srtt.c -o srtt.o

clean:
	rm -rf *.o
	rm -rf ../bin/copy_*
	rm -rf ../bin/log*

mrproper: clean
	rm -rf ../bin/server1b-zlabya ../bin/server2b-zlabya ../bin/server3b-zlabya
