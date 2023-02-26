test: test.o
	gcc -o test test.o
test.o: test.c
	gcc -c test.c


data_queue_test: data_queue_test.o
	gcc -o data_queue_test dataqueue_test.o
data_queue_test.o: data_queue_test.c
	gcc -c data_queue_test.c

interest_queue_test: interest_queue_test.o
	gcc -o interest_queue_test interest_queue_test.o
interest_queue_test.o: interest_queue_test.c
	gcc -c interest_queue_test.c


ndn: client.o ndn.o MD5.o RSA.o
	gcc -o client client.o ndn.o MD5.o RSA.o -lpthread
client.o: client.c
	gcc -c client.c
ndn.o: ndn.c
	gcc -c ndn.c
MD5.o: MD5.c
	gcc -c MD5.c
RSA.o: RSA.c
	gcc -c RSA.c
	
	
all:
	gcc -o client client.c ndn.c MD5.c RSA.c -lpthread


clean:
	rm -f client.o client ndn.o test.o test data_queue_test data_queue_test.o interest_queue_test interest_queue_test.o
