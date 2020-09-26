import bluetooth
import argparse

character_map = {
    ' ' : 0x20,
    '!' : 0x21,
    '"' : 0x22,
    '#' : 0x23,
    '$' : 0x24,
    '%' : 0x25,
    '&' : 0x26,
    '\'' : 0x27,
    '(' : 0x28,
    ')' : 0x29,
    '*' : 0x2a,
    '+' : 0x2b,
    #'\'' : 0x2c,
    '-' : 0x2d,
    '.' : 0x2e,
    '/' : 0x2f,
    '0' : 0x30,
    '1' : 0x31,
    '2' : 0x32,
    '3' : 0x33,
    '4' : 0x34,
    '5' : 0x35,
    '6' : 0x36,
    '7' : 0x37,
    '8' : 0x38,
    '9' : 0x39,
    ':' : 0x3a,
    ';' : 0x3b,
    '<' : 0x3c,
    #'=' : 0x3d,
    '>' : 0x3e,
    '?' : 0x3f,
    '@' : 0x40,
    'A' : 0x41,
    'B' : 0x42,
    'C' : 0x43,
    'D' : 0x44,
    'E' : 0x45,
    'F' : 0x46,
    'G' : 0x47,
    'H' : 0x48,
    'I' : 0x49,
    'J' : 0x4a,
    'K' : 0x4b,
    'L' : 0x4c,
    'M' : 0x4d,
    'N' : 0x4e,
    'O' : 0x4f,
    'P' : 0x50,
    'Q' : 0x51,
    'R' : 0x52,
    'S' : 0x53,
    'T' : 0x54,
    'U' : 0x55,
    'V' : 0x56,
    'W' : 0x57,
    'X' : 0x58,
    'Y' : 0x59,
    'Z' : 0x5a,
    '[' : 0x5b,
    '\\' : 0x5c,
    ']' : 0x5d,
    '^' : 0x5e,
    '_' : 0x5f,
    '`' : 0x60,
    'a' : 0x61,
    'b' : 0x62,
    'c' : 0x63,
    'd' : 0x64,
    'e' : 0x65,
    'f' : 0x66,
    'g' : 0x67,
    'h' : 0x68,
    'i' : 0x69,
    'j' : 0x6a,
    'k' : 0x6b,
    'l' : 0x6c,
    'm' : 0x6d,
    'n' : 0x6e,
    'o' : 0x6f,
    'p' : 0x70,
    'q' : 0x71,
    'r' : 0x72,
    's' : 0x73,
    't' : 0x74,
    'u' : 0x75,
    'v' : 0x76,
    'w' : 0x77,
    'x' : 0x78,
    'y' : 0x79,
    'z' : 0x7a,
    '{' : 0x7b,
    '|' : 0x7c,
    '}' : 0x7d,
    }
serverMACAddress = '00:20:10:08:0f:4a' 
uuid = "94f39d29-7d6d-437d-973b-fba39e49d4ee"

def parse_args():

    parser = argparse.ArgumentParser()

    parser.add_argument('--text', dest='text', required=True )

    return parser.parse_args()

def main(text):

    vals = []
    for t in text:
        try:
            pat = character_map[t]
            vals.append(pat)
        except ValueError:
            print( 'VAlue %s, not recognized' %t)
            raise

    service_matches = bluetooth.find_service(uuid=uuid, address=serverMACAddress)
    
    if len(service_matches) == 0:
        print("Couldn't find the SampleServer service.")
        sys.exit(0)

    first_match = service_matches[0]
    port = first_match["port"]
    name = first_match["name"]
    host = first_match["host"]
    
    print(first_match)
    
    s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    s.connect((serverMACAddress, port))
    s.send(bytes([len(vals)] + vals))

main( **vars(parse_args()) )

