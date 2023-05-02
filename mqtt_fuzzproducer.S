#define SYSCNR_read $0
#define SYSCNR_write $1
#define SYSCNR_open $2
#define SYSCNR_close $3
#define SYSCNR_lseek $8
#define SYSCNR_mmap $9
#define SYSCNR_munmap $11
#define SYSCNR_socket $41
#define SYSCNR_connect $42
#define SYSCNR_exit $60

#define SYSC_AG0 %rdi
#define SYSC_AG1 %rsi
#define SYSC_AG2 %rdx
#define SYSC_AG3 %r10
#define SYSC_AG4 %r8
#define SYSC_AG5 %r9
#define SYSC_NRR %rax


.data
	.global read_from_file
	.global write_to_file
	.global open_file_path
	.global close_file_desc
	.global exit_from_app
	.global convert_hostaddr_to_netbyteorder
	.global read_single_packet_from_file
	.global free_single_packet_bytes
	.global memorymap_file_shared
	.global unmap_memorymap_shared
	.global write_packet_to_mmaped_file
	.global netport_to_netbyteorder
	.global open_socket_and_connect
	.global send_packet_to_broker
	.global receive_packet_from_broker

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

	write_to_file:
		#define FILE_DSC %rdi
		#define WRIT_DST %rdi
		#define WRIT_CNT %rdx
		#define WORD_NUM %rcx
		#define DEST_TMP %r10

		xor SYSC_NRR, SYSC_NRR
		shl WORD_NUM, READ_CNT

		mov SYSCNR_write, SYSC_NRR
		syscall

		ret

		#undef FILE_DSC
		#undef WRIT_DST
		#undef WRIT_CNT
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

		ret

	exit_from_app:
		#define EXIT_COD %rdi

		xor SYSC_NRR, SYSC_NRR
		mov SYSCNRR_exit, SYSC_NRR
		syscall

		ret

		#undef EXIT_COD

	convert_hostaddr_to_netbyteorder:
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
		#define PLEN_LEN $8
		#define ANON_MMP $33
		#define ANON_DSC $-1
		#define ARG1_WRD %esi
		#define ARG2_WRD %edx
		#define TEMP_WRD %r10d
		#define PROT_RWX $7
		#define MEMR_ERR $-1
		#define SIZE_ERR $-2
		#define BYTE_ERR $-3
		#define READ_OKK $0
		#define RETR_REG %rax

		pushq PACK_TMP

		xor PACK_TMP, PACK_TMP
		mov SYSCNR_read, SYSC_NRR
		mov PLEN_LEN, SYSC_AG2
		syscall

		cmp PLEN_LEN, SYSC_NRR
		jne 1f

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
		mov (PACK_TMP), SYSC_AG1
		mov PROT_RWX, SYSC_AG2
		mov ANON_MMP, SYSC_AG3
		mov ANON_DSC, SYSC_AG4
		syscall

		test SYSC_NRR, SYSC_NRR
		js 2f

		movq SYSC_NRR, 8(PACK_TMP)

		xor FILE_DSC, FILE_DSC
		xor SYSC_NRR, SYSC_NRR
		xor SYSC_AG1, SYSC_AG1
		xor SYSC_AG2, SYSC_AG2

		pop FILE_DSC
		movq 8(PACK_TMP), SYSC_AG1
		mov (PACK_TMP), SYSC_AG2
		mov SYSCNR_read, SYSC_NRR
		syscall

		cmp SYSC_AG2, SYSC_NRR
		jne 3f

		xor RETR_REG, RETR_REG
		mov READ_OKK, RETR_REG	

	4:
		popq PACK_TMP
		ret

	1:
		xor RETR_REG, RETR_REG
		mov SIZE_ERR, RETR_REG
		jmp 4b
	2:
		xor RETR_REG, RETR_REG
		mov MEMR_ERR, RETR_REG
		jmp 4b	
	3:
		xor RETR_REG, RETR_REG
		mov BYTE_ERR, RETR_REG
		jmp 4b

		#undef FILE_DSC
        #undef PACK_PTR
        #undef PACK_TMP
        #undef PLEN_LEN
        #undef ANON_MMP
        #undef ANON_DSC
        #undef ARG1_WRD
        #undef ARG2_WRD
        #undef TEMP_WRD
        #undef PROT_RWX
        #undef MEMR_ERR
		#undef SIZE_ERR
		#undef BYTE_ERR
		#undef READ_OKK
		#undef RETR_REG

	free_single_packet_bytes:
		#define PACK_PTR %rdi
		#define PACK_TMP %r11
		
		mov PACK_PTR, PACK_TMP

		xor SYSC_AG0, SYSC_AG0
		xor SYSC_AG1, SYSC_AG1
		xor SYSC_NRR, SYSC_NRR

		mov 8(PACK_TMP), SYSC_AG0
		mov (PACK_TMP), SYSC_AG1
		mov SYSCNR_munmap, SYSC_NRR
		syscall

		ret

		#undef PACK_PTR
		#undef PACK_TMP

	memorymap_file_shared:
		#define FILE_DSC %rdi
		#define MMAP_LEN %rsi
		#define MMAP_OFS %rdx
		#define FLAG_SHR $1
		#define PROT_RWX $7
		#define NULL_MEM $0
		#define MMAP_ERR $-1

		xor SYSC_NRR, SYSC_NRR
		xor SYSC_AG3, SYSC_AG3
		xor SYSC_AG4, SYSC_AG4
		xor SYSC_AG5, SYSC_AG5

		mov PROT_RWX, SYSC_AG2
		mov FLAG_SHR, SYSC_AG3
		mov FILE_DSC, SYSC_AG4
		mov MMAP_OFS, SYSC_AG5
		xor SYSC_AG0, SYSC_AG0
		mov SYSCNR_mmap, SYSC_NRR
		syscall

		test SYSC_NRR, SYSC_NRR
		js 2f
	1:
		ret
	2:
		xor SYSC_NRR, SYSC_NRR
		mov MMAP_ERR, SYSC_NRR
		jmp 1b

		#undef  FILE_DSC
        #undef  MMAP_LEN
        #undef  MMAP_OFS
        #undef  FLAG_SHR
        #undef  PROT_RWX
        #undef  NULL_MEM
        #undef  MMAP_ERR

    unmap_memorymap_shared:
    	#define MMAP_ADR %rdi
    	#define MMAP_LEN %rsi

    	xor SYSC_NRR, SYSC_NRR
    	mov SYSCNR_munmap, SYSC_NRR
    	syscall

    	ret

    	#undef MMAP_ADR
    	#undef MMAP_LEN

    write_packet_to_mmaped_file:
    	#define FILE_DSC %rdi
    	#define PACK_PTR %rsi
    	#define PLEN_REG %r10
    	#define PADR_REG %r11
    	#define RETR_REG %rax
    	#define ERRC_NUM $-1

    	xor PLEN_REG, PLEN_REG
    	xor PADR_REG, PADR_REG

    	mov (PACK_PTR), PLEN_REG
    	mov 8(PACK_PTR), PADR_REG

    	xor SYSC_AG1, SYSC_AG1
    	xor SYSC_AG2, SYSC_AG2
    	xor SYSC_NRR, SYSC_NRR

    	mov PADR_REG, SYSC_AG1
    	mov PLEN_REG, SYSC_AG2
    	mov SYSCNR_write, SYSC_NRR
    	syscall

    	cmp SYSC_NR, PLEN_REG
    	jne 2f

    1:
    	ret

    2:
    	xor RETR_REG, RETR_REG
    	mov ERRC_NUM, RETR_REG
    	jmp 1b



	netport_to_netbyteorder:
		#define PORT_LIL %di
		#define ROTR_NUM $8
		#define RETR_REG %ax
		#define RETR_DWD %rax

		xor RETR_DWD, RETR_DWD
		rol ROTR_NUM, PORT_LIL
		movw PORT_LIL, RETR_REG

		ret

		#undef PORT_LIL
		#undef ROTR_NUM
		#undef RETR_REG
		#undef RETR_DWD

		
	open_socket_and_connect:
		#define ADDR_WRD %edi
		#define PORT_HWD %si
		#define FRAM_PTR %rbp
		#define STCK_PTR %rsp
		#define SFAM_IP4 $2 
		#define SOCK_STM $1
		#define ADRS_LEN $16
		#define TMP1_BYY %r10b
		#define TMP1_HWD %r10w
		#define TMP1_WRD %r10d
		#define TMP1_DWD %10
		#define RETR_SCK %rax
		#define ZERO_PAD $0

		pushq FRAM_PTR
		movq STCK_PTR, FRAM_PTR
		subq ADRS_LEN, FRAM_PTR

		movb SFAM_IP4, (FRAM_PTR)
		movw PORT_HWD, 1(FRAM_PTR)
		movl ADDR_WRD, 3(FRAM_PTR)
		movd ZERO_PAD, 8(FRAM_PTR)

		xor SYSC_NRR, SYSC_NRR
		xor SYSC_AG0, SYSC_AG0
		xor SYSC_AG1, SYSC_AG1
		xor SYSC_AG2, SYSC_AG2

		mov SFAM_IP4, SYSC_AG0
		mov SOCK_STM, SYSC_AG1
		mov SYSCNR_socket, SYSC_NRR
		syscall

		mov SYSC_NRR, TMP1_DWD
		
		xor SYSC_NRR, SYSC_NRR 
		xor SYSC_AG0, SYSC_AG0
		xor SYSC_AG1, SYSC_AG1
		xor SYSC_AG2, SYSC_AG2

		mov TMP1_DWD, SYSC_AG0
		leaq (FRAM_PTR), SYSC_AG1
		mov ADRS_LEN, SYSC_AG2
		mov SYSCNR_connect, SYSC_NRR
		syscall

		xor RETR_SCK, RETR_SCK
		mov TMP1_DWD, RETR_SCK

		popq FRAM_PTR

		ret

	send_packet_to_broker:
		#define SOCK_DSC %rdi
		#define PACK_PTR %rsi
		#define PLEN_REG %r10
		#define PADR_REG %r11
		#define ERRC_NUM $-1

		xor PLEN_REG, PLEN_REG
		xor PADR_REG, PADR_REG

		mov (PACK_PTR), PLEN_REG
		movq 8(PACK_PTR), PADR_REG

		xor SYSC_NRR, SYSC_NRR
		xor SYSC_AG1, SYSC_AG1
		xor SYSC_AG2, SYSC_AG2

		mov SYSCNR_write, SYSC_NRR
		movq PADR_REG, SYSC_AG1
		mov PLEN_REG, SYSC_AG2
		syscall

		cmp SYSC_NRR, PLEN_REG
		jne 2f

	1:
		ret

	1:
		xor SYSC_NRR, SYSC_NRR
		mov ERRC_NUM, SYSC_NRR
		jmp 1b

		#undef SOCK_DSC
        #undef PACK_PTR
        #undef PLEN_REG
        #undef PADR_REG
        #undef ERRC_NUM


	receive_packet_from_broker:
		#define SOCK_DSC %rdi
		#define PACK_PTR %rsi
		#define PLEN_REG %r10
		#define PADR_REG %r11
		#define ERRC_NUM $-1

		xor PLEN_REG, PLEN_REG
		xor PADR_REG, PADR_REG

		mov (PACK_PTR), PLEN_REG
		movq 8(PACK_PTR), PADR_REG

		xor SYSC_NRR, SYSC_NRR
		xor SYSC_AG1, SYSC_AG1
		xor SYSC_AG2, SYSC_AG2

		mov SYSCNR_read, SYSC_NRR
		movq PADR_REG, SYSC_AG1
		mov PLEN_REG, SYSC_AG2
		syscall

		cmp SYSC_NRR, PLEN_REG
		jne 2f

	1:
		ret

	1:
		xor SYSC_NRR, SYSC_NRR
		mov ERRC_NUM, SYSC_NRR
		jmp 1b

		#undef SOCK_DSC
        #undef PACK_PTR
        #undef PLEN_REG
        #undef PADR_REG
        #undef ERRC_NUM