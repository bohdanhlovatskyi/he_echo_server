package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
)

const (
	port = 9000
	addr = "localhost:"
)

func main() {
	server, err := net.Listen("tcp", addr+fmt.Sprint(port))
	if err != nil {
		log.Fatalln("could not start server: ", err)
	}
	defer server.Close()

	for {
		conn, err := server.Accept()
		if err != nil {
			log.Println("failed to accept connection: ", err)
			continue
		}

		go func(conn net.Conn) {
			defer conn.Close()

			b := bufio.NewReader(conn)
			line, err := b.ReadBytes('\n')
			if err != nil {
				return
			}

			// or this can be done via io.copy()
			conn.Write(line)
		}(conn)
	}
}
