server: main.c  ./http/http_conn.cpp
	g++ -o server main.c ./http/http_conn.cpp 


clean:
	rm  -r server
