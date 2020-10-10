CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=bruno-raissa-thayna

all: server client
server: 
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp HTTPResp.cpp HTTPReq.cpp 
client: 
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp HTTPResp.cpp HTTPReq.cpp 
clean: 
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz
tarball: clean
	tar -cvf lab02-$(USERID).tar.gz *
