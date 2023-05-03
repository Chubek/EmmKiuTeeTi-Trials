from socket import socket, AF_INET, AF_UNIX


class SocketInterface:
    def __init__(self, addr="0.0.0.0", port=1883, sockfamily=AF_INET, bufsize=256, max_retries=5):
        self.addr = addr
        self.port = port
        self.bufsize = bufsize
        self.max_retries = max_retries
        self.sock = socket(family=sockfamily)

    def connect(self):
        self.sock.connect( (self.addr, self.port))

    @classmethod
    def spawn_socket_interface(cls, *args, **kwargs):
        instance = cls(*args, **kwargs)
        instance.connect()
        return instance

    def recv(self) -> bytearray:
        data = bytearray([0])
        while True:
            received = self.sock.recv(self.bufsize)
            data += received
            if len(received) < self.bufsize:
                break
        return data

    def send(self, data: bytearray):
        leng_d = len(data)
        sent_n = self.sock.send(data)
        retries = 0
        while sent_n != leng_d and retries <= self.max_retries:
            sent_n = self.sock.send(data)
            retries += 1

    def close(self):
        self.sock.close()

    def send_recv(self, data: bytearray) -> bytearray:
        self.send(data)
        return self.recv()


class PublisherFuzzerPrototype:
    def __init__(self, pub_packets: list[bytearray], connect_packet: bytearray, disconnect_packet: bytearray, **kwargs):
        self.pub_packets = pub_packets
        self.connect_packet = connect_packet
        self.disconnect_packet = disconnect_packet
        self.sock_interface = SocketInterface(**kwargs)
        self.received_packets = []

    def establish_connection(self):
        self.sock_interface.connect()
        self.sock_interface.send(self.connect_packet)

    def destablish_connection(self):
        self.sock_interface.send(self.disconnect_packet)
        self.sock_interface.close()

    def do_fuzz_test(self):
        for packed in self.pub_packets:
            received = self.sock_interface.send_recv(packed)
            self.received_packets.append(received)

    @classmethod
    def spawn_fuzzer(cls, *args, **kwargs):
        instance = cls(*args, **kwargs)
        instance.establish_connection()
        instance.do_fuzz_test()
        instance.destablish_connection()
        return instance.received_packets


print(PublisherFuzzerPrototype.spawn_fuzzer(
    [bytearray(b"2\x12\x00\x03A/B\x0c\x1c\x00\t{\'zz\': 1}")],
    bytearray(b'\x10\x10\x00\x04MQTT\x04\x02\x0e\x10\x00\x04HyrN'),
    bytearray(b'\xe0\x00'),
    port=1888
))
