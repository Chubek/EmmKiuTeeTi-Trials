#define SYSCNR_read $0
#define SYSCNR_open $2
#define SYSCNR_close $3

#define SYSC_AG0 %rsi
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
		
