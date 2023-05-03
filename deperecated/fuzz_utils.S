.data
	.global seek_zero
	.global sum_array
	.global starter_gun


.text
	seek_zero:
		#define ARR_ADDR %rdi
		#define TYY_SIZE %rsi
		#define FOR_NUMB %rdx	
		#define HAS_ZERO %rax
		#define LDR_TEMP %r11
		#define CNT_TEMP %r10
		#define RES_TRUE $1

		xor CNT_TEMP, CNT_TEMP
		xor HAS_ZERO, HAS_ZERO
		jmp 1f
	1:
		xor LDR_TEMP, LDR_TEMP
		mov (ARR_ADDR), LDR_TEMP
		add TYY_SIZE, ARR_ADDR
		inc CNT_TEMP
		test LDR_TEMP, LDR_TEMP
		jnz 2f
		jz 3f
	2:
		cmp CNT_TEMP, FOR_NUMB
		jge 4f
		jmp 1f
	3:
		mov RES_TRUE, HAS_ZERO
		jmp 4f
	4:
		ret

		#undef ARR_ADDR %rdi
		#undef TYY_SIZE %rsi
		#undef FOR_NUMB %rdx	
		#undef HAS_ZERO %rax
		#undef LDR_TEMP %r11
		#undef CNT_TEMP %r10
		#undef RES_TRUE $1


	sum_array:
		#define ARR_ADDR %rdi
		#define TTY_SIZE %rsi
		#define FOR_NUMB %rdx
		#define FIN_SUMM %rax
		#define LDR_TEMP %r11
		#define CNT_TEMP %r10

		xor CNT_TEMP, CNT_TEMP
		xor FIN_SUMM, FIN_SUMM
		jmp 1f
	1:
		xor LDR_TEMP, LDR_TEMP
		mov (ARR_ADDR), LDR_TEMP
		add TTY_SIZE, ADR_ADDR
		inc CNT_TEMP
		add LDR_TEMP, FIN_SUMM
		cmp CNT_TEMP, FOR_NUMB
		jge 2f
		jmp 1b
	2:
		ret

		#undef ARR_ADDR %rdi
		#undef TYY_SIZE %rsi
		#undef FOR_NUMB %rdx	
		#undef FIN_SUMM %rax
		#undef LDR_TEMP %r11
		#undef CNT_TEMP %r10


	starter_gun:
		#define CNT_DOWN %rdi
		#define JMP_ADDR %rsi
		#define UPP_TSCR %rdx
		#define INI_TSCR %r10

		xor INI_TSCR, INI_TSCR
		rdtsc
		mov UPP_TSCR, INI_TSCR
	1:
		rdtsc
		sub INI_TSCR, UPP_TSCR
		cmp UPP_TSCR, CNT_DOWN
		jge 2f
		jmp 1b
	2:
		jmp *JMP_ADDR
		ret

		#undef CNT_DOWN %rdi
		#undef JMP_ADDR %rsi
		#undef UPP_TSCR %rdx
		#undef INI_TSCR %r10
