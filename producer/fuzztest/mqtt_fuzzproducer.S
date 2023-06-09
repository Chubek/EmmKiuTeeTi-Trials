#define SYSCNR_read $0
#define SYSCNR_write $1
#define SYSCNR_open $2
#define SYSCNR_close $3
#define SYSCNR_lseek $8
#define SYSCNR_mmap $9
#define SYSCNR_munmap $11
#define SYSCNR_socket $41
#define SYSCNR_connect $42
#define SYSCNR_fork $58
#define SYSCNR_exit $60

#define SYSCID_stdout $1
#define SYSCID_stderr $2

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
	.global write_to_stderr_and_exit
	.global write_to_stdout
	.global exit_from_app_or_subproc
	.global zero_out_memory
	.global convert_hostaddr_to_netbyteorder
	.global read_packet_num
	.global store_object_in_shared_memory
	.global read_single_packet_from_file
	.global free_single_packet_bytes
	.global allocate_memory
	.global memorymap_file_shared
	.global unmap_memorymap_shared
	.global write_packet_to_mmaped_file
	.global netport_to_netbyteorder
	.global open_socket_and_connect
	.global send_packet_to_broker
	.global receive_packet_from_broker
	.global packet_bytes_are_same
	.global fork_process

.text
	read_from_file:
		#define FILE_DSC %rdi
		#define READ_DST %rdi
		#define READ_CNT %rdx
		#define WORD_NUM %rcx
		#define DEST_TMP %r10

		shl WORD_NUM, READ_CNT

		movq SYSCNR_read, SYSC_NRR
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

		shl WORD_NUM, WRIT_CNT

		movq SYSCNR_write, SYSC_NRR
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

		movq SYSCNR_open, SYSC_NRR
		syscall

		ret

		#undef FILE_PTH
		#undef OPEN_FLG

	close_file_desc:
		movq SYSCNR_close, SYSC_NRR
		syscall

		ret


	write_to_stderr_and_exit:
		#define ERRS_PTR %rdi
		#define CHAR_REG %r10b
		#define CHAR_DWD %r10
		#define ERRS_TMP %r11
		#define EXIT_STT $1

		xor CHAR_DWD, CHAR_DWD
		xor SYSC_NRR, SYSC_NRR
		xor ERRS_TMP, ERRS_TMP
		
		movq ERRS_PTR, ERRS_TMP

	1:
		movb (ERRS_TMP), CHAR_REG
		inc ERRS_TMP
		test CHAR_REG, CHAR_REG
		jz 2f
		jmp 1b

	2:
		movq ERRS_TMP, SYSC_AG2
		subq ERRS_PTR, SYSC_AG2

		movq ERRS_PTR, SYSC_AG1
		movq SYSCID_stderr, SYSC_AG0
		movq SYSCNR_write, SYSC_NRR
		syscall  

		movq EXIT_STT, SYSC_AG0
		movq SYSCNR_exit, SYSC_NRR
		syscall

		ret

		#undef ERRS_PTR
		#undef CHAR_REG
		#undef CHAR_DWD
		#undef ERRS_TMP
		#undef EXIT_STT


	write_to_stdout:
		#define MSSG_PTR %rdi
		#define CHAR_REG %r10b
		#define CHAR_DWD %r10
		#define MSSG_TMP %r11

		movq MSSG_PTR, MSSG_TMP
		xorq CHAR_DWD, CHAR_DWD
	1:
		movb (MSSG_PTR), CHAR_REG
		inc MSSG_PTR
		test CHAR_REG, CHAR_REG
		jz 2f
		jmp 1b

	2:
		movq MSSG_TMP, SYSC_AG2
		subq MSSG_PTR, SYSC_AG2

		movq MSSG_PTR, SYSC_AG1
		movq SYSCID_stdout, SYSC_AG0
		movq SYSCNR_write, SYSC_NRR
		syscall

		ret

	exit_from_app_or_subproc:
		#define EXIT_COD %rdi

		xor SYSC_NRR, SYSC_NRR
		mov SYSCNR_exit, SYSC_NRR
		syscall

		ret

		#undef EXIT_COD

	zero_out_memory:
		#define ADDR_PTR %rsi
		#define ADDR_LEN %rdi
		#define CNTR_REG %rcx
		#define ZERO_NUM %r10b
		#define ZERO_REG %r10

		movq ADDR_LEN, CNTR_REG
	
	1:
		movb ZERO_NUM, (ADDR_PTR, CNTR_REG)
		loop 1b

		ret

		#undef ADDR_PTR
		#undef ADDR_LEN
		#undef CNTR_REG
		#undef ZERO_NUM

	read_packet_num:
		#define FILE_DSC %rdi
		#define STCK_PTR %rsp
		#define FRAM_PTR %rbp
		#define SIZE_LEN $8

		pushq FRAM_PTR
		movq STCK_PTR, FRAM_PTR
		subq SIZE_LEN, FRAM_PTR

		xor SYSC_AG1, SYSC_AG1

		leaq FRAM_PTR, SYSC_AG1
		movq SIZE_LEN, SYSC_AG2
		movq SYSCNR_read, SYSC_NRR
		syscall

		cmp SIZE_LEN, SYSC_NRR
		jne 1b
		
		movq (FRAM_PTR), SYSC_NRR

	1:
		popq FRAM_PTR
		ret


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

			
	9:
		xorq DIGT_DWD, DIGT_DWD
		xorq TEMP_DWD, TEMP_DWD
	1:
		xorq DIGT_DWD, DIGT_DWD
		movb (HOST_ADR), DIGT_BYY
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
		movq ERRN_NDG, RETR_FIN
		jmp 9f
	7:		
		movq ERRN_BSP, RETR_FIN
		jmp 9f
	8:		
		movq RETR_SUC, RETR_FIN
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

		movq SYSCNR_read, SYSC_NRR
		movq PLEN_LEN, SYSC_AG2
		syscall

		cmp PLEN_LEN, SYSC_NRR
		jne 1f

		pushq FILE_DSC
			
		movq SYSCNR_mmap, SYSC_NRR
		xorq SYSC_AG0, SYSC_AG0
		movq (PACK_TMP), SYSC_AG1
		movq PROT_RWX, SYSC_AG2
		movq ANON_MMP, SYSC_AG3
		movq ANON_DSC, SYSC_AG4
		xorq SYSC_AG5, SYSC_AG5
		syscall

		test SYSC_NRR, SYSC_NRR
		js 2f

		movq SYSC_NRR, 8(PACK_TMP)

		popq FILE_DSC
		movq 8(PACK_TMP), SYSC_AG1
		movq (PACK_TMP), SYSC_AG2
		movq SYSCNR_read, SYSC_NRR
		syscall

		cmp SYSC_AG2, SYSC_NRR
		jne 3f

		xorq RETR_REG, RETR_REG
		movq READ_OKK, RETR_REG	

	4:
		popq PACK_TMP
		ret

	1:
		movq SIZE_ERR, RETR_REG
		jmp 4b
	2:
		movq MEMR_ERR, RETR_REG
		jmp 4b	
	3:
		movq BYTE_ERR, RETR_REG
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

		movq 8(PACK_TMP), SYSC_AG0
		movq (PACK_TMP), SYSC_AG1
		movq SYSCNR_munmap, SYSC_NRR
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

		movq PROT_RWX, SYSC_AG2
		movq FLAG_SHR, SYSC_AG3
		movq FILE_DSC, SYSC_AG4
		movq MMAP_OFS, SYSC_AG5
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

    	movq SYSCNR_munmap, SYSC_NRR
    	syscall

    	ret

    	#undef MMAP_ADR
    	#undef MMAP_LEN

    write_pa cket_to_mmaped_file:
    	#define FILE_DSC %rdi
    	#define PACK_PTR %rsi
    	#define PLEN_REG %r10
    	#define PADR_REG %r11
    	#define RETR_REG %rax
    	#define ERRC_NUM $-1

    	movq (PACK_PTR), PLEN_REG
    	movq 8(PACK_PTR), PADR_REG

    	movq PADR_REG, SYSC_AG1
    	movq PLEN_REG, SYSC_AG2
    	movq SYSCNR_write, SYSC_NRR
    	syscall

    	cmp SYSC_NRR, PLEN_REG
    	jne 2f

    1:
    	ret

    2:
    	xor RETR_REG, RETR_REG
    	mov ERRC_NUM, RETR_REG
    	jmp 1b

    	#undef FILE_DSC
        #undef PACK_PTR
        #undef PLEN_REG
        #undef PADR_REG
        #undef RETR_REG
        #undef ERRC_NUM

    allocate_memory:
    	#define ALOC_SIZ %rdi
    	#define ALOC_TMP %r10
    	#define NULL_FLD $-1
    	#define FLAG_SHR $1
		#define PROT_RWX $7

		movq ALOC_SIZ, ALOC_TMP
		
		movq ALOC_TMP, SYSC_AG1
		movq PROT_RWX, SYSC_AG2
		movq FLAG_SHR, SYSC_AG3
		movq NULL_FLD, SYSC_AG4
		movq SYSCNR_mmap, SYSC_NRR
		syscall

		ret

		#undef ALOC_SIZ
    	#undef ALOC_TMP
    	#undef NULL_FLD
    	#undef FLAG_SHR
		#undef PROT_RWX

	netport_to_netbyteorder:
		#define PORT_LIL %di
		#define ROTR_NUM $8
		#define RETR_DWD %rax

		
		rol ROTR_NUM, PORT_LIL
		movzxw PORT_LIL, RETR_DWD

		ret

		#undef PORT_LIL
		#undef ROTR_NUM
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
		#define TMP1_DWD %r10
		#define RETR_SCK %rax
		#define ZERO_PAD %r11

		pushq FRAM_PTR
		movq STCK_PTR, FRAM_PTR
		subq ADRS_LEN, FRAM_PTR

		xor ZERO_PAD, ZERO_PAD

		movb SFAM_IP4, (FRAM_PTR)
		movw PORT_HWD, 1(FRAM_PTR)
		movl ADDR_WRD, 3(FRAM_PTR)
		mov ZERO_PAD, 8(FRAM_PTR)

		movq SFAM_IP4, SYSC_AG0
		movq SOCK_STM, SYSC_AG1
		movq SYSCNR_socket, SYSC_NRR
		syscall

		mov SYSC_NRR, TMP1_DWD
		
		movq TMP1_DWD, SYSC_AG0
		leaq (FRAM_PTR), SYSC_AG1
		movq ADRS_LEN, SYSC_AG2
		movq SYSCNR_connect, SYSC_NRR
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

		movq (PACK_PTR), PLEN_REG
		movq 8(PACK_PTR), PADR_REG

		movq SYSCNR_write, SYSC_NRR
		movq PADR_REG, SYSC_AG1
		movq PLEN_REG, SYSC_AG2
		syscall

		cmp SYSC_NRR, PLEN_REG
		jne 2f

	1:
		ret

	2:
		movq ERRC_NUM, SYSC_NRR
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

		movq (PACK_PTR), PLEN_REG
		movq 8(PACK_PTR), PADR_REG

		movq SYSCNR_read, SYSC_NRR
		movq PADR_REG, SYSC_AG1
		movq PLEN_REG, SYSC_AG2
		syscall

		cmp SYSC_NRR, PLEN_REG
		jne 2f

	1:
		ret

	2:
		xor SYSC_NRR, SYSC_NRR
		mov ERRC_NUM, SYSC_NRR
		jmp 1b

		#undef SOCK_DSC
        #undef PACK_PTR
        #undef PLEN_REG
        #undef PADR_REG
        #undef ERRC_NUM


   	packet_bytes_are_same:
   		#define PACK_ONE %rdi
   		#define PACK_TWO %rsi
   		#define PCK1_LEN %rdx
   		#define PCK2_LEN %rcx
   		#define REGS_ONE %r10
   		#define REGS_TWO %r11
   		#define DIFF_RET %rax
   		#define DIFF_HAS $0
   		#define DIFF_NON $1
   		#define DWRD_LEN $8

   		movq (PACK_ONE), PCK1_LEN
   		movq (PACK_TWO), PCK2_LEN
   		movq 8(PACK_ONE), REGS_ONE
   		movq 8(PACK_TWO), REGS_TWO

   		xorq PCK1_LEN, PCK2_LEN
   		jnz 2f

   	9:	
   		movq (REGS_ONE, PCK2_LEN), PACK_ONE
   		movq (REGS_TWO, PCK2_LEN), PACK_TWO
   		xorq PACK_ONE, PACK_TWO
   		jnz 2f 
   		subq DWRD_LEN, PCK2_LEN
   		loopnz

   		movq DIFF_NON, DIFF_RET

   	1:
   		ret

   	2:
   		movq DIFF_HAS, %rax
   		jmp 1b


   		#undef PACK_ONE
        #undef PACK_TWO
        #undef PCK1_LEN
        #undef PCK2_LEN
        #undef REGS_ONE
        #undef REGS_TWO
        #undef DIFF_RET
        #undef DIFF_HAS
        #undef DIFF_NON
        #undef DWRD_LEN


    close_broker_socket:
    	#define BCON_PTR %rdi
    	#define TEMP_REG %r10d

    	xor TEMP_REG, TEMP_REG
    	movl (BCON_PTR), TEMP_REG
    	movzx TEMP_REG, SYSC_AG0
    	movq SYSCNR_close, SYSC_NRR
    	syscall

    	ret


    	#undef BCON_PTR
    	#undef TEMP_REG


    fork_process:
    	movq SYSCNR_fork, SYSC_NRR
    	syscall

    	ret