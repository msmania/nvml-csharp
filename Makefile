all:
	make -C lib
	make -C app

clean:
	make -C lib clean
	make -C app clean
