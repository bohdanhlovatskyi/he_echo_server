package main

// taken from https://dev.to/0xbf/golang-a-simple-echo-server-1den
// as a little benchmark of adequacy for out servers
import (
    "io"
    "log"
    "net"
)

func main() {
    addr := "192.168.0.101:9000"
    server, err := net.Listen("tcp", addr)
    if err != nil {
        log.Fatalln(err)
    }
    defer server.Close()

    // log.Println("Server is running on:", addr)

    for {
        conn, err := server.Accept()
        if err != nil {
            log.Println("Failed to accept conn.", err)
            continue
        }

        go func(conn net.Conn) {
            defer func() {
                conn.Close()
            }()
            io.Copy(conn, conn)
        }(conn)
    }
}
