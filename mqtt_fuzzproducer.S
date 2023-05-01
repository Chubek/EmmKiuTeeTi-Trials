#define SYSCNR_read $0
#define SYSCNR_open $2
#define SYSCNR_close $3
#define SYSCNR_mmap $9
#define SYSCNR_munmap $11

#define SYSC_AG0 %rdi
#define SYSC_AG1 %rsi
#define SYSC_AG2 %rdx
#define SYSC_AG3 %r10
#define SYSC_AG4 %r8
#define SYSC_AG5 %r9
#define SYSC_NRR %rax


.data
	.global read_from_file
	.global open_file_path
	.global close_file_desc
	.global convert_hostaddr_to_network_int
	.global read_single_packet_from_file
	.global free_single_packet_bytes


.text
	read_from_file:
		#define FILE_DSC %rdi
		#define READ_DST %rdi
		#define READ_CNT %rdx
		#define WORD_NUM %rcx
		#define DEST_TMP %r10

		xor SYSC_NRR, SYSC_NRR
		shl WORD_NUM, READ_CNT

		mov SYSCNR_read, SYSC_NRR
		syscall

		ret

		#undef FILE_DSC
		#undef READ_DST
		#undef READ_CNT
		#undef WORD_NUM
		#undef DEST_TMP

	open_file_path:
		#define FILE_PTH %rdi
		#define OPEN_FLG %rsi

		xor SYSC_NRR, SYSC_NRR

		mov SYSCNR_open, SYSC_NRR
		syscall

		ret

		#undef FILE_PTH
		#undef OPEN_FLG

	close_file_desc:
		xor SYSC_NRR, SYSC_NRR
		mov SYSCNR_close, SYSC_NRR
		syscall


	convert_hostaddr_to_network_int:
		#define HOST_ADR %rdi
		#define DEST_INT %rsi
		#define RETR_FIN %rax
		#define ACCM_REG %rax
		#define CNTR_REG %rcx
		#define DIGT_BYY %r10b
		#define RETR_WRD %r10w
		#define DIGT_DWD %r10
		#define TEMP_DWD %r11
		#define TEMP_BYY %r11b
		#define SHFL_REG %cl
		#define SHFL_BEG $0
		#define CNTR_MAX $4
		#define SHFL_STP $8
		#define MULT_NUM $10
		#define ASCI_PER $46
		#define ASCI_ZRO $48
		#define ASCI_NIN $57
		#define ERRN_NDG $-1
		#define ERRN_BSP $-2
		#define RETR_SUC $0

		xor TEMP_DWD, TEMP_DWD
		xor DIGT_DWD, DIGT_DWD
		xor RETR_FIN, RETR_FIN
		xor SHFL_REG, SHFL_REG
		xor CNTR_REG, CNTR_REG
	
	9:
		xor DIGT_DWD, DIGT_DWD
		xor TEMP_DWD, TEMP_DWD
	1:
		xor DIGT_DWD, DIGT_DWD
		mov (HOST_ADR), DIGT_BYY
		inc HOST_ADR
		test DIGT_BYY, DIGT_BYY
		jz 5f
		cmp ASCI_ZRO, DIGT_BYY
		jl 4f
		jmp 3f		
	2:
		sub ASCI_ZRO, DIGT_BYY
		xor ACCM_REG, ACCM_REG
		mov MULT_NUM, ACCM_REG
		mul TEMP_DWD
		add DIGT_DWD, ACCM_REG
		xor TEMP_DWD, TEMP_DWD
		mov ACCM_REG, TEMP_DWD
		jmp 1b
	3:
		cmp ASCI_NIN, DIGT_BYY
		jg 6f
		jmp 2b
	4:
		cmp ASCI_PER, DIGT_BYY
		jne 7f
		jmp 5f
	5:
		movb TEMP_BYY, (DEST_INT, CNTR_REG)
		inc CNTR_REG
		cmp CNTR_MAX, CNTR_REG
		je 8f
		jne 9b
	6:
		xor RETR_FIN, RETR_FIN
		mov ERRN_NDG, RETR_FIN
		jmp 9f
	7:
		xor RETR_FIN, RETR_FIN
		mov ERRN_BSP, RETR_FIN
		jmp 9f
	8:
		xor RETR_FIN, RETR_FIN
		mov RETR_SUC, RETR_FIN
		jmp 9f
	9:
		ret

		#undef HOST_ADR
		#undef DEST_INT
		#undef RETR_FIN
		#undef ACCM_REG
		#undef CNTR_REG
		#undef DIGT_BYY
		#undef RETR_WRD
		#undef DIGT_DWD
		#undef TEMP_DWD
		#undef TEMP_BYY
		#undef SHFL_REG
		#undef SHFL_BEG
		#undef CNTR_MAX
		#undef SHFL_STP
		#undef MULT_NUM
		#undef ASCI_PER
		#undef ASCI_ZRO
		#undef ASCI_NIN
		#undef ERRN_NDG
		#undef ERRN_BSP
		#undef RETR_SUC

	read_single_packet_from_file:
		#define FILE_DSC %rdi
		#define PACK_PTR %rsi
		#define PACK_TMP %r12
		#define PLEN_LEN $4
		#define ANON_MMP $33
		#define ANON_DSC $-1
		#define ARG1_WRD %esi
		#define ARG2_WRD %edx
		#define TEMP_WRD %r10d
		#define PROT_RWX $7

		push PACK_TMP

		xor PACK_TMP, PACK_TMP
		mov SYSCNR_read, SYSC_NRR
		mov PLEN_LEN, SYSC_AG2
		syscall

		push FILE_DSC
		mov PACK_PTR, PACK_TMP
		xor SYSC_NRR, SYSC_NRR
		xor SYSC_AG0, SYSC_AG0
		xor SYSC_AG1, SYSC_AG1
		xor SYSC_AG2, SYSC_AG2
		xor SYSC_AG3, SYSC_AG3
		xor SYSC_AG4, SYSC_AG4
		xor SYSC_AG5, SYSC_AG5
			
		mov SYSCNR_mmap, SYSC_NRR
		movl (PACK_TMP), ARG1_WRD
		mov PROT_RWX, SYSC_AG2
		mov ANON_MMP, SYSC_AG3
		mov ANON_DSC, SYSC_AG4
		syscall

		mov SYSC_NRR, 4(PACK_TMP)

		mov $0x414141, %r10
		mov %r10, (SYSC_NRR)

		xor PACK_PTR, PACK_PTR
		xor FILE_DSC, FILE_DSC
		xor SYSC_AG2, SYSC_AG2
		xor SYSC_NRR, SYSC_NRR
		pop FILE_DSC
		xor SYSC_AG1, SYSC_AG1
		xor SYSC_AG2, SYSC_AG2
		mov 4(PACK_TMP), SYSC_AG1
		movl (PACK_TMP), ARG2_WRD
		mov SYSCNR_read, SYSC_NRR
		syscall

		pop PACK_TMP

		ret
