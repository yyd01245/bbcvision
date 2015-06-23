files=Decoder.cpp TSDecoder.cpp RecvQueue.cpp

libs=-lpthread -lavformat -lavcodec -lavutil

DEBUG=-g -D_DEBUG

libtsdecoder.so: $(files)
	g++ -shared -fPIC $(files) $(libs) $(DEBUG) -D__linux -o libtsdecoder.so

install:libtsdecoder.so
	-strip -x libtsdecoder.so
	cp libtsdecoder.so /usr/local/lib/
	cp TSDecoder.h /usr/local/include/
	
uninstall:
	rm -rf /usr/local/lib/libtsdecoder.so
	rm -rf /usr/local/include/TSDecoder.h
	
test:
	-rm -f test
	g++ test.cpp -fPIC $(files) $(libs) $(DEBUG) -D__linux__ -o test
clean:
	-rm -f libtsdecoder
	-rm -f *.so