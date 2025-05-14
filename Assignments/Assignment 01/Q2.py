import socket
import struct

def build_dns_query(domain):
    """Constructs a DNS query for the given domain."""
    transaction_id = b'\xaa\xaa'  # Random transaction ID
    flags = b'\x01\x00'  # Standard query with recursion
    questions = b'\x00\x01'  # One question
    answer_rrs = b'\x00\x00'
    authority_rrs = b'\x00\x00'
    additional_rrs = b'\x00\x00'
    
    # Convert domain to DNS format (e.g., "google.com" -> "\x06google\x03com\x00")
    query = b''.join(bytes([len(x)]) + x.encode() for x in domain.split('.')) + b'\x00'
    
    # Query Type (A record) and Query Class (IN - Internet)
    qtype = b'\x00\x01'
    qclass = b'\x00\x01'
    
    # Full DNS Query
    dns_query = transaction_id + flags + questions + answer_rrs + authority_rrs + additional_rrs + query + qtype + qclass
    return dns_query

def parse_dns_response(response):
    """Extracts the IP address from the DNS response."""
    ip_start = response.find(b'\xc0') + 12  # Find start of the Answer section
    ip = struct.unpack('!4B', response[ip_start:ip_start+4])  # Unpack 4 bytes as IPv4
    return ".".join(map(str, ip))

def resolve_domain(domain):
    """Sends a DNS query to Google's public DNS and parses the response."""
    dns_server = ('8.8.8.8', 53)  # Google's public DNS server
    query = build_dns_query(domain)
    
    # Create UDP socket and send the query
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(5)  # 5-second timeout
    sock.sendto(query, dns_server)
    
    # Receive the response
    response, _ = sock.recvfrom(512)  # Max DNS response size
    sock.close()
    
    return parse_dns_response(response)

# User Input and Execution
domain = input("Enter domain name: ")
try:
    ip_address = resolve_domain(domain)
    print(f"Resolved IP address of {domain}: {ip_address}")
except Exception as e:
    print(f"Error: {e}")
