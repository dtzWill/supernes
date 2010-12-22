
.text
.global memset32 # int *dest, int c, int count

# rdi = dest, rsi = c, rdx = count
# stosl: src = eax, dest = es:rdi, rcx = count

memset32:
	movl %esi, %eax
	movl %edx, %ecx
	cld
	rep stosl
	ret

.global memcpy32 # int *dest, int *src, int count

# rdi = dest, rsi = src, rdx = count
# stosl: src = rsi, dest = es:rdi, ecx = count

memcpy32:
	movl %edx, %ecx
	cld
	rep movsl
	ret

