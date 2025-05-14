import socketserver
import http.server
import requests
import logging

# Set up logging
logging.basicConfig(filename="proxy.log", level=logging.INFO, format="%(asctime)s - %(message)s")

class ProxyHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        logging.info(f"Received GET request for: {self.path}")

        try:
            # Forwarding the request to the target server
            response = requests.get(self.path, headers=self.headers, allow_redirects=True)

            # Sending back response to the client
            self.send_response(response.status_code)
            for key, value in response.headers.items():
                self.send_header(key, value)
            self.end_headers()
            self.wfile.write(response.content)

            logging.info(f"Response: {response.status_code}, {len(response.content)} bytes")
        except Exception as e:
            self.send_error(500, f"Internal Server Error: {e}")
            logging.error(f"Error handling GET request: {e}")

    def do_POST(self):
        logging.info(f"Received POST request for: {self.path}")

        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length)

        try:
            # Forwarding the request to the target server
            response = requests.post(self.path, headers=self.headers, data=post_data)

            self.send_response(response.status_code)
            for key, value in response.headers.items():
                self.send_header(key, value)
            self.end_headers()
            self.wfile.write(response.content)

            logging.info(f"Response: {response.status_code}, {len(response.content)} bytes")
        except Exception as e:
            self.send_error(500, f"Internal Server Error: {e}")
            logging.error(f"Error handling POST request: {e}")

# Start the proxy server
def run_proxy_server(port=8080):
    with socketserver.ThreadingTCPServer(("", port), ProxyHandler) as httpd:
        logging.info(f"Proxy server running on port {port}")
        print(f"Proxy server running on port {port}...")
        httpd.serve_forever()

if __name__ == "__main__":
    run_proxy_server(8080)
