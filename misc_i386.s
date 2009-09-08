
.text
.global memset32 # int *dest, int c, int count

memset32:
	movl %edi, %edx
	movl 4(%esp), %edi
	movl 8(%esp), %eax
	movl 12(%esp), %ecx
	cld
	rep stosl
	movl %edx, %edi
	ret

.global memcpy32 # int *dest, int *src, int count

memcpy32:
	movl %edi, %edx
	movl 4(%esp), %edi
	movl 8(%esp), %eax
	movl 12(%esp), %ecx
	cld
	rep movsl
	movl %edx, %edi
	ret

