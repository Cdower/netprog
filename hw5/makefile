PROGRAM_NAME = client.out

target: HTTP\(S\)client.cpp
	g++ -o $(PROGRAM_NAME) HTTP\(S\)client.cpp -lcrypto -lssl

run: HTTP\(S\)client.cpp
	g++ -o $(PROGRAM_NAME) HTTP\(S\)client.cpp -lcrypto -lssl
	./$(PROGRAM_NAME) https://www.google.com/about.html
