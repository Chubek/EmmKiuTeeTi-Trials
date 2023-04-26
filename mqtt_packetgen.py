from zinteger import ZUint8 as U8, ZUint32 as U32, ZUint16 as U16, ZUint64 as U64
from random import choices
from time import time_ns
from string import ascii_letters

PROTOCOL_NAME = b'\x00\x04MQTT'
LEN_ASCII_LETTERS = 52

class MQTTHeaderCodes:
	PubD0Q0R0 = 48 #0b110000
	PubD0Q0R1 = 49 #0b110001
	PubD1Q0R0 = 56 #0b111000
	PubD1Q0R1 = 57 #0b111001
	PubD0Q1R0 = 50 #0b110010
	PubD0Q1R1 = 51 #0b110011
	PubD1Q1R0 = 58 #0b111010
	PubD1Q1R1 = 59 #0b111011
	PubD0Q2R0 = 52 #0b110100
	PubD0Q2R1 = 53 #0b110101
	PubD1Q2R0 = 60 #0b111100
	PubD1Q2R1 = 61 #0b111101
	ConnectNF = 16 #0b010000
	DisconnNF = 224 #0b11100000

class MQTTVHeaderCodes:
	ConnProtocolLvl4 = 4  #0b00000100
	ConnC1P0R0W0Q0U0 = 2 #0b10
	ConnC1P0R0W0Q0U1 = 130 #0b10000010
	ConnC1P0R0W0Q1U0 = 10 #0b1010
	ConnC1P0R0W0Q1U1 = 138 #0b10001010
	ConnC1P0R0W0Q2U0 = 18 #0b10010
	ConnC1P0R0W0Q2U1 = 146 #0b10010010
	ConnC1P0R0W1Q0U0 = 6 #0b110
	ConnC1P0R0W1Q0U1 = 134 #0b10000110
	ConnC1P0R0W1Q1U0 = 14 #0b1110
	ConnC1P0R0W1Q1U1 = 142 #0b10001110
	ConnC1P0R0W1Q2U0 = 22 #0b10110
	ConnC1P0R0W1Q2U1 = 150 #0b10010110
	ConnC1P0R1W0Q0U0 = 34 #0b100010
	ConnC1P0R1W0Q0U1 = 162 #0b10100010
	ConnC1P0R1W0Q1U0 = 42 #0b101010
	ConnC1P0R1W0Q1U1 = 170 #0b10101010
	ConnC1P0R1W0Q2U0 = 50 #0b110010
	ConnC1P0R1W0Q2U1 = 178 #0b10110010
	ConnC1P0R1W1Q0U0 = 38 #0b100110
	ConnC1P0R1W1Q0U1 = 166 #0b10100110
	ConnC1P0R1W1Q1U0 = 46 #0b101110
	ConnC1P0R1W1Q1U1 = 174 #0b10101110
	ConnC1P0R1W1Q2U0 = 54 #0b110110
	ConnC1P0R1W1Q2U1 = 182 #0b10110110
	ConnC1P1R0W0Q0U0 = 66 #0b1000010
	ConnC1P1R0W0Q0U1 = 194 #0b11000010
	ConnC1P1R0W0Q1U0 = 74 #0b1001010
	ConnC1P1R0W0Q1U1 = 202 #0b11001010
	ConnC1P1R0W0Q2U0 = 82 #0b1010010
	ConnC1P1R0W0Q2U1 = 210 #0b11010010
	ConnC1P1R0W1Q0U0 = 70 #0b1000110
	ConnC1P1R0W1Q0U1 = 198 #0b11000110
	ConnC1P1R0W1Q1U0 = 78 #0b1001110
	ConnC1P1R0W1Q1U1 = 206 #0b11001110
	ConnC1P1R0W1Q2U0 = 86 #0b1010110
	ConnC1P1R0W1Q2U1 = 214 #0b11010110
	ConnC1P1R1W0Q0U0 = 98 #0b1100010
	ConnC1P1R1W0Q0U1 = 226 #0b11100010
	ConnC1P1R1W0Q1U0 = 106 #0b1101010
	ConnC1P1R1W0Q1U1 = 234 #0b11101010
	ConnC1P1R1W0Q2U0 = 114 #0b1110010
	ConnC1P1R1W0Q2U1 = 242 #0b11110010
	ConnC1P1R1W1Q0U0 = 102 #0b1100110
	ConnC1P1R1W1Q0U1 = 230 #0b11100110
	ConnC1P1R1W1Q1U0 = 110 #0b1101110
	ConnC1P1R1W1Q1U1 = 238 #0b11101110
	ConnC1P1R1W1Q2U0 = 118 #0b1110110
	ConnC1P1R1W1Q2U1 = 246 #0b11110110
	ConnC0P0R0W0Q0U0 = 0 #0b0
	ConnC0P0R0W0Q0U1 = 128 #0b10000000
	ConnC0P0R0W0Q1U0 = 8 #0b1000
	ConnC0P0R0W0Q1U1 = 136 #0b10001000
	ConnC0P0R0W0Q2U0 = 16 #0b10000
	ConnC0P0R0W0Q2U1 = 144 #0b10010000
	ConnC0P0R0W1Q0U0 = 4 #0b100
	ConnC0P0R0W1Q0U1 = 132 #0b10000100
	ConnC0P0R0W1Q1U0 = 12 #0b1100
	ConnC0P0R0W1Q1U1 = 140 #0b10001100
	ConnC0P0R0W1Q2U0 = 20 #0b10100
	ConnC0P0R0W1Q2U1 = 148 #0b10010100
	ConnC0P0R1W0Q0U0 = 32 #0b100000
	ConnC0P0R1W0Q0U1 = 160 #0b10100000
	ConnC0P0R1W0Q1U0 = 40 #0b101000
	ConnC0P0R1W0Q1U1 = 168 #0b10101000
	ConnC0P0R1W0Q2U0 = 48 #0b110000
	ConnC0P0R1W0Q2U1 = 176 #0b10110000
	ConnC0P0R1W1Q0U0 = 36 #0b100100
	ConnC0P0R1W1Q0U1 = 164 #0b10100100
	ConnC0P0R1W1Q1U0 = 44 #0b101100
	ConnC0P0R1W1Q1U1 = 172 #0b10101100
	ConnC0P0R1W1Q2U0 = 52 #0b110100
	ConnC0P0R1W1Q2U1 = 180 #0b10110100
	ConnC0P1R0W0Q0U0 = 64 #0b1000000
	ConnC0P1R0W0Q0U1 = 192 #0b11000000
	ConnC0P1R0W0Q1U0 = 72 #0b1001000
	ConnC0P1R0W0Q1U1 = 200 #0b11001000
	ConnC0P1R0W0Q2U0 = 80 #0b1010000
	ConnC0P1R0W0Q2U1 = 208 #0b11010000
	ConnC0P1R0W1Q0U0 = 68 #0b1000100
	ConnC0P1R0W1Q0U1 = 196 #0b11000100
	ConnC0P1R0W1Q1U0 = 76 #0b1001100
	ConnC0P1R0W1Q1U1 = 204 #0b11001100
	ConnC0P1R0W1Q2U0 = 84 #0b1010100
	ConnC0P1R0W1Q2U1 = 212 #0b11010100
	ConnC0P1R1W0Q0U0 = 96 #0b1100000
	ConnC0P1R1W0Q0U1 = 224 #0b11100000
	ConnC0P1R1W0Q1U0 = 104 #0b1101000
	ConnC0P1R1W0Q1U1 = 232 #0b11101000
	ConnC0P1R1W0Q2U0 = 112 #0b1110000
	ConnC0P1R1W0Q2U1 = 240 #0b11110000
	ConnC0P1R1W1Q0U0 = 100 #0b1100100
	ConnC0P1R1W1Q0U1 = 228 #0b11100100
	ConnC0P1R1W1Q1U0 = 108 #0b1101100
	ConnC0P1R1W1Q1U1 = 236 #0b11101100
	ConnC0P1R1W1Q2U0 = 116 #0b1110100
	ConnC0P1R1W1Q2U1 = 244 #0b11110100


def hex_escape_packet(packet: bytearray) -> str:
	return "".join([f"\\x{b:02x}" for b in packet])


def pack_fixed_header(tyflag: int, remaining_len: U32) -> bytearray:
	remaining_len_bytes = remaining_len.val.to_bytes(4, byteorder="little")
	remaining_len_bytes = remaining_len_bytes[:remaining_len_bytes.index(0)]
	tyflag_bytes = tyflag.to_bytes(1, byteorder="little")
	return tyflag_bytes + remaining_len_bytes


def encode_packet_len(length: int) -> U32:
	enc_double_word = U32(0)
	shl_amount = 0
	while length > 0:
		enc_byte = U8(length % 128)
		length //= 128
		if length > 0:
			enc_byte |= 128
		enc_double_word |= U32(enc_byte.val) << shl_amount
		shl_amount += 8
	return enc_double_word


def generate_packet_id(mod: int) -> U16:
	assert mod <= 0xff and mod >= 0, "Modulo for Packet ID generation must be between 0 and 255"
	less_significant_byte = ((time_ns() << 2) + time_ns()) % mod
	more_significant_byte = ((time_ns() << 4) + time_ns()) % mod
	return U16(less_significant_byte | (more_significant_byte << 8))

def generate_client_id(length=4) -> bytearray:
	client_id = ""
	for _ in range(length):
		idx = ((time_ns() << 2) + time_ns()) % LEN_ASCII_LETTERS
		client_id += ascii_letters[idx] 
	return client_id.encode('utf8')

def encode_utf8_string(string: str | bytes) -> tuple[bytearray, int]:
	strlen = U16(len(string))
	assert strlen <= 0xffff, "UTF-8 string must be smaller than or equal to MAX_U16"
	strlen_bytes = strlen.val.to_bytes(2, byteorder="big")
	str_bytes = bytearray(string, encoding="utf8") if type(string) == str else bytearray(string)
	encoded = strlen_bytes + str_bytes
	return encoded, len(encoded)


class MQTTControlVHeaderCompose:
	@staticmethod
	def compose_publish_varheader(topic: str | bytes) -> tuple[bytearray, int]:
		topic = bytes(topic, encoding="utf8") if type(topic) == str else topic
		char1, char2 = topic[0], topic[-1]
		mod = ((char1 << 5)  | (char2 >> 4)) & 0xff
		packet_id = generate_packet_id(mod)
		packet_id_bytes = packet_id.val.to_bytes(2, byteorder="little")
		topic_encoded, _ = encode_utf8_string(topic)
		composed = topic_encoded + packet_id_bytes
		return composed, len(composed)

	@staticmethod
	def compose_connect_varheader(keepalive=3600, flags=MQTTVHeaderCodes.ConnC1P0R0W0Q0U0):
		lvl_bytes = MQTTVHeaderCodes.ConnProtocolLvl4.to_bytes(1, byteorder="little")
		flags_bytes = flags.to_bytes(1, byteorder="little")
		keepalive_bytes = keepalive.to_bytes(2, byteorder="big")
		composed = PROTOCOL_NAME + lvl_bytes + flags_bytes + keepalive_bytes
		return composed, len(composed)


class MQTTControlPacketCompose:
	@staticmethod
	def compose_publish_packet(topic: str, message: str, header=MQTTHeaderCodes.PubD0Q1R0) -> bytearray:
		vhead_composed, vhead_len = MQTTControlVHeaderCompose.compose_publish_varheader(topic)
		messg_composed, messg_len = encode_utf8_string(message)
		remaining_length = encode_packet_len(messg_len + vhead_len)
		fixed_header_packed = pack_fixed_header(header, remaining_length)
		final_packet = fixed_header_packed + vhead_composed + messg_composed
		return bytearray(final_packet)

	@staticmethod
	def compose_connect_packet(body=None, header=MQTTHeaderCodes.ConnectNF):
		client_id, cid_len = encode_utf8_string(generate_client_id())
		vhead_composed, vhead_len = MQTTControlVHeaderCompose.compose_connect_varheader()
		remaining_len = encode_packet_len(vhead_len + cid_len)
		fixed_header_packed = pack_fixed_header(header, remaining_len)
		final_packet = fixed_header_packed + vhead_composed + client_id
		return bytearray(final_packet)

	@staticmethod
	def compose_disconnect_packet() -> bytearray:
		return bytearray(MQTTHeaderCodes.DisconnNF.to_bytes(1, byteorder="little") + b"\0")


print(MQTTControlPacketCompose.compose_publish_packet("A/B", "{'zz': 1}"))
print(MQTTControlPacketCompose.compose_connect_packet())
print(MQTTControlPacketCompose.compose_disconnect_packet())

