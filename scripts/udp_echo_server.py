#!/usr/bin/env python3
import socket
import argparse
import sys
import time

def start_udp_echo_server(host, port):
    """
    Starts a UDP Echo Server that responds to ESP32 Latency Analyzer probes.
    Expected format: "PING <timestamp> <sequence>"
    Response format: "PONG <timestamp> <sequence>"
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        sock.bind((host, port))
        print(f"UDP Echo Server started on {host}:{port}")
        print("Press Ctrl+C to stop")
        print("-" * 40)
        
        while True:
            data, addr = sock.recvfrom(1024)
            message = data.decode('utf-8', errors='ignore').strip()
                        
            # ESP32 sends: "PING <timestamp> <sequence>"
            if message.startswith("PING"):
                # We need to reply with: "PONG <timestamp> <sequence>"
                response = message.replace("PING", "PONG", 1)
                sock.sendto(response.encode('utf-8'), addr)
            else:
                # Basic echo for other formats (or if format changes)
                sock.sendto(data, addr)

    except OSError as e:
        if e.errno == 98 or e.errno == 48: # Address already in use
            print(f"💡 Tip: Port {port} is busy. Try a different port using --port argument.")
    except KeyboardInterrupt:
        pass
    finally:
        sock.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="UDP Echo Server for ESP32 Latency Testing")
    parser.add_argument("--host", default="0.0.0.0", help="Host interface to bind to (default: 0.0.0.0)")
    parser.add_argument("--port", type=int, default=5000, help="UDP Port to listen on (default: 5000)")
    
    args = parser.parse_args()
    
    start_udp_echo_server(args.host, args.port)
