import bluetooth
#import socket
#
hostMACAddress = 'f4:5c:89:bd:f5:ee' # The MAC address of a Bluetooth adapter on the server. The server might have multiple Bluetooth adapters.
serverMACAddress = '00:20:10:08:0f:4a' 
#
#s = socket.socket(0, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
#s.connect((serverMACAddress,port))
#while 1:
#    text = input()
#    if text == "quit":
#        break
#    s.send(bytes(text, 'UTF-8'))
#s.close()

port = 4
#port = bluetooth.get_available_port( bluetooth.RFCOMM )
nearby_devices = bluetooth.discover_devices(lookup_names=True)
print("Found {} devices.".format(len(nearby_devices)))
print (nearby_devices)
# search for the SampleServer service
uuid = "94f39d29-7d6d-437d-973b-fba39e49d4ee"
service_matches = bluetooth.find_service(uuid=uuid, address=serverMACAddress)

if len(service_matches) == 0:
    print("Couldn't find the SampleServer service.")
    sys.exit(0)

first_match = service_matches[0]
port = first_match["port"]
name = first_match["name"]
host = first_match["host"]

print(first_match)

#print (port)
s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
s.connect((serverMACAddress, port))
s.send(bytes([0xd1,0x55]))

#port = 0
#backlog = 1
#size = 1
#s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
##s.bind((serverMACAddress, port))
#s.bind(("", bluetooth.PORT_ANY))
#s.listen(backlog)
#client, clientInfo = s.accept()
#while 1:
#    data = client.recv(size)
#    if data:
#        print(data)
res = s.recv(1)
print (res)


