import subprocess
import socket
import os
import signal

def start_server(port):
    print(f"Starting HTTP server on port {port}...")
    server_process = subprocess.Popen(
        ["python", "-m", "http.server", str(port)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        preexec_fn=os.setsid
    )
    return server_process

def configure_firewall(port):
    print(f"Allowing port {port} through the firewall...")
    os.system(f"sudo ufw allow {port}")

def get_local_ip():
    hostname = socket.gethostname()
    ip_address = socket.gethostbyname(hostname)
    print(f"Your local IP address is: {ip_address}")
    return ip_address

def stop_server(server_process, port):
    print(f"Stopping HTTP server on port {port}...")
    os.killpg(os.getpgid(server_process.pid), signal.SIGTERM)
    os.system(f"sudo ufw delete allow {port}")

if __name__ == "__main__":
    port = 6969
    server_process = start_server(port)
    configure_firewall(port)
    ip_address = get_local_ip()
    print(f"Access the website at: http://{ip_address}:{port}")

    try:
        input("Press Enter to stop the server and block the port...")
    finally:
        stop_server(server_process, port)
        print("Server stopped and port closed.")

