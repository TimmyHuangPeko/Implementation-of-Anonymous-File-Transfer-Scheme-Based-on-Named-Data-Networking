kgc: KGC.o ndn.o MD5.o RSA.o
	gcc -o kgc KGC.o ndn.o MD5.o RSA.o
KGC.o: KGC.c
	gcc -c KGC.c
ndn.o: ndn.c
	gcc -c ndn.c
MD5.o: MD5.c
	gcc -c MD5.c
RSA.o: RSA.c
	gcc -c RSA.c
	
clean:
	rm -f KGC KGC.o ndn.o
