from zinteger import ZUint8 as U8, ZUint32 as U32, ZUint16 as U16
from random import choices
from time import time_ns

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


def pack_fixed_header(tyflag: int, remaining_len: U32) -> bytearray:
	fixed_header_packed = U64(tyflag) | (U64(remaining_len.val) << 8)
	fixed_header_bytes = fixed_header_packed.val.to_bytes(5, byteorder="little")
	zero_index = fixedd_header_bytes.index(0)
	return fixed_header_bytes[:zero_index]


def encode_packet_len(length: int) -> tuple[U32, int]:
	enc_double_word = U32(0)
	shl_amount = 0
	while length > 0:
		enc_byte = U8(length % 128)
		length //= 128
		if length > 0:
			enc_byte |= 128
		enc_double_word |= U32(enc_byte.val) << shl_amount
		shl_amount += 8

	return enc_double_word, shl_amount


def generate_packet_id(mod: int) -> U16:
	assert(mod <= 0xff and mod >= 0, "Modulo for Packet ID generation must be between 0 and 255")
	less_significant_byte = (time_ns() << 2) + time_ns() % mod
	more_significant_byte = (time_ns() << 4) + time_ns() % mod
	return U16(less_significant_byte | (more_significant_byte << 8))


def encode_utf8_string(string: str | bytes) -> tuple[bytearray, int]:
	strlen = U16(len(string))
	assert(strlen <= 0xffff, "UTF-8 string must be smaller than or equal to MAX_U16")
	strlen_bytes = strlen.val.to_bytes(2, byteorder="big")
	str_bytes = bytearray(string, encoding="utf8")
	encoded = strlen_bytes + str_bytes
	return encoded, len(encoded)


def MQTTControlVarHeaderCompose:
	@staticmethod
	def compose_publish_varheader(subject: str | bytes) -> tuple[bytearray, int]:
		subject = bytes(subject) if type(subject) == str else subject
		char1, char2 = subject[0], subject[-1]
		mod = (char1 << 5)  | (char2 >> 4) & 0xff
		packet_id = generate_packet_id(mod)
		packet_id_bytes = packet_id.val.to_bytes(2, byteorder="little")
		subject_encoded = encode_utf8_string(subject)
		composed = subject_encoded + packet_id_bytes
		return composed, len(composed)



def MQTTControlPacketCompose:
	@staticmethod
	def compose_publish_packet(subject: str, message: str, header=MQTTHeaderCodes.PubD0Q1R0) -> bytearray:
		vhead_composed, vhead_len = MTTControlVarHeaderCompose.compose_publish_varheader(subject)
		messg_composed, messg_len = encode_utf8_string(message)
		remaining_length = encode_packet_len(vhead_composed + vhead_len)
		fixed_header_packed = pack_fixed_header(header, remaining_length)
		final_packed = fixed_header_packed + vhead_composed + messg_composed





