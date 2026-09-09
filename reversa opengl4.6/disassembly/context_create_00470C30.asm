
/mnt/data/Main.exe:	file format coff-i386

Disassembly of section .text:

00401000 <.text>:
  470c30:      	pushl	%ebp
  470c31:      	movl	%esp, %ebp
  470c33:      	subl	$0x1c, %esp
  470c36:      	xorl	%eax, %eax
  470c38:      	pushl	%ebx
  470c39:      	movl	0xc(%ebp), %ebx
  470c3c:      	pushl	%esi
  470c3d:      	movl	0x18(%ebp), %esi
  470c40:      	movw	%ax, -0xf(%ebp)
  470c44:      	movb	%al, -0xd(%ebp)
  470c47:      	movl	%eax, -0x1c(%ebp)
  470c4a:      	movl	%eax, -0x18(%ebp)
  470c4d:      	movl	%eax, -0x14(%ebp)
  470c50:      	movb	%al, -0x10(%ebp)
  470c53:      	movups	-0x1c(%ebp), %xmm0
  470c57:      	movl	%eax, -0xc(%ebp)
  470c5a:      	movl	%eax, -0x8(%ebp)
  470c5d:      	movups	%xmm0, (%esi)
  470c60:      	pushl	%edi
  470c61:      	movq	-0xc(%ebp), %xmm0
  470c66:      	movl	0x10(%ebp), %edi
  470c69:      	movq	%xmm0, 0x10(%esi)
  470c6e:      	movl	%eax, 0x18(%esi)
  470c71:      	movl	%ebx, (%esi)
  470c73:      	movl	%eax, 0x4(%esi)
  470c76:      	cmpl	$0x2, %edi
  470c79:      	jne	0x470c84 <.text+0x6fc84>
  470c7b:      	cmpb	%al, 0x14(%ebp)
  470c7e:      	je	0x470c84 <.text+0x6fc84>
  470c80:      	movb	$0x1, %al
  470c82:      	jmp	0x470c86 <.text+0x6fc86>
  470c84:      	xorb	%al, %al
  470c86:      	cmpl	$0x0, 0x8(%ebp)
  470c8a:      	movb	%al, 0xa(%esi)
  470c8d:      	je	0x470d84 <.text+0x6fd84>
  470c93:      	testl	%ebx, %ebx
  470c95:      	je	0x470d84 <.text+0x6fd84>
  470c9b:      	testl	%edi, %edi
  470c9d:      	je	0x470d7b <.text+0x6fd7b>
  470ca3:      	pushl	$0xb8cdfc               # imm = 0xB8CDFC
  470ca8:      	calll	*0xb833b0
  470cae:      	movl	%eax, %edx
  470cb0:      	testl	%edx, %edx
  470cb2:      	movl	%edx, 0xc(%ebp)
  470cb5:      	setne	%cl
  470cb8:      	movb	%cl, 0x9(%esi)
  470cbb:      	testl	%edx, %edx
  470cbd:      	je	0x470d77 <.text+0x6fd77>
  470cc3:      	movl	$0x0, 0x18(%ebp)
  470cca:      	cmpl	$0x2, %edi
  470ccd:      	jne	0x470d0e <.text+0x6fd0e>
  470ccf:      	leal	0x10(%esi), %eax
  470cd2:      	movb	$0x1, 0xb(%esi)
  470cd6:      	pushl	%eax
  470cd7:      	leal	0x18(%ebp), %eax
  470cda:      	pushl	%eax
  470cdb:      	movzbl	0x14(%ebp), %eax
  470cdf:      	xorl	$0x1, %eax
  470ce2:      	incl	%eax
  470ce3:      	pushl	%eax
  470ce4:      	pushl	$0x6
  470ce6:      	pushl	$0x4
  470ce8:      	pushl	%edx
  470ce9:      	pushl	%ebx
  470cea:      	pushl	0x8(%ebp)
  470ced:      	calll	0x470d90 <.text+0x6fd90>
  470cf2:      	addl	$0x20, %esp
  470cf5:      	cmpl	%edi, %eax
  470cf7:      	je	0x470d84 <.text+0x6fd84>
  470cfd:      	testl	%eax, %eax
  470cff:      	jne	0x470d04 <.text+0x6fd04>
  470d01:      	movl	%edi, 0x4(%esi)
  470d04:      	movl	0x18(%ebp), %edi
  470d07:      	testl	%edi, %edi
  470d09:      	jne	0x470d44 <.text+0x6fd44>
  470d0b:      	movl	0xc(%ebp), %edx
  470d0e:      	leal	0x14(%esi), %eax
  470d11:      	movb	$0x1, 0xc(%esi)
  470d15:      	pushl	%eax
  470d16:      	leal	0x18(%ebp), %eax
  470d19:      	pushl	%eax
  470d1a:      	pushl	$0x2
  470d1c:      	pushl	$0x3
  470d1e:      	pushl	$0x3
  470d20:      	pushl	%edx
  470d21:      	pushl	%ebx
  470d22:      	pushl	0x8(%ebp)
  470d25:      	calll	0x470d90 <.text+0x6fd90>
  470d2a:      	addl	$0x20, %esp
  470d2d:      	cmpl	$0x2, %eax
  470d30:      	je	0x470d84 <.text+0x6fd84>
  470d32:      	testl	%eax, %eax
  470d34:      	jne	0x470d3d <.text+0x6fd3d>
  470d36:      	movl	$0x1, 0x4(%esi)
  470d3d:      	movl	0x18(%ebp), %edi
  470d40:      	testl	%edi, %edi
  470d42:      	je	0x470d77 <.text+0x6fd77>
  470d44:      	pushl	$0x0
  470d46:      	calll	*0xb8335c
  470d4c:      	pushl	%ebx
  470d4d:      	calll	*0xb83408
  470d53:      	testl	%eax, %eax
  470d55:      	jne	0x470d60 <.text+0x6fd60>
  470d57:      	calll	*0xb83358
  470d5d:      	movl	%eax, 0x18(%esi)
  470d60:      	movl	0x10(%ebp), %eax
  470d63:      	cmpl	%eax, 0x4(%esi)
  470d66:      	movl	%edi, (%esi)
  470d68:      	setne	%al
  470d6b:      	popl	%edi
  470d6c:      	movb	%al, 0x8(%esi)
  470d6f:      	movb	$0x1, %al
  470d71:      	popl	%esi
  470d72:      	popl	%ebx
  470d73:      	movl	%ebp, %esp
  470d75:      	popl	%ebp
  470d76:      	retl
  470d77:      	movb	$0x1, 0x8(%esi)
  470d7b:      	popl	%edi
  470d7c:      	popl	%esi
  470d7d:      	movb	$0x1, %al
  470d7f:      	popl	%ebx
  470d80:      	movl	%ebp, %esp
  470d82:      	popl	%ebp
  470d83:      	retl
  470d84:      	popl	%edi
  470d85:      	popl	%esi
  470d86:      	xorb	%al, %al
  470d88:      	popl	%ebx
  470d89:      	movl	%ebp, %esp
  470d8b:      	popl	%ebp
  470d8c:      	retl
  470d8d:      	int3
  470d8e:      	int3
  470d8f:      	int3
  470d90:      	pushl	%ebp
  470d91:      	movl	%esp, %ebp
  470d93:      	subl	$0x34, %esp
  470d96:      	movl	0xcb4550, %eax
  470d9b:      	xorl	%ebp, %eax
  470d9d:      	movl	%eax, -0x4(%ebp)
  470da0:      	movl	0xc(%ebp), %eax
  470da3:      	movl	%eax, -0x30(%ebp)
  470da6:      	movl	0x20(%ebp), %eax
  470da9:      	pushl	%ebx
  470daa:      	movl	0x8(%ebp), %ebx
  470dad:      	movl	%eax, -0x34(%ebp)
  470db0:      	movl	0x14(%ebp), %eax
  470db3:      	pushl	%esi
  470db4:      	movl	0x10(%ebp), %esi
  470db7:      	movl	%eax, -0x24(%ebp)
  470dba:      	movl	0x18(%ebp), %eax
  470dbd:      	pushl	%edi
  470dbe:      	movl	0x24(%ebp), %edi
  470dc1:      	movl	%eax, -0x1c(%ebp)
  470dc4:      	movl	0x1c(%ebp), %eax
  470dc7:      	pushl	$0x0
  470dc9:      	movl	%ebx, -0x2c(%ebp)
  470dcc:      	movl	$0x2091, -0x28(%ebp)    # imm = 0x2091
  470dd3:      	movl	$0x2092, -0x20(%ebp)    # imm = 0x2092
  470dda:      	movl	$0x2094, -0x18(%ebp)    # imm = 0x2094
  470de1:      	movl	$0x0, -0x14(%ebp)
  470de8:      	movl	$0x9126, -0x10(%ebp)    # imm = 0x9126
  470def:      	movl	%eax, -0xc(%ebp)
  470df2:      	movl	$0x0, -0x8(%ebp)
  470df9:      	calll	*0xb8335c
  470dff:      	leal	-0x28(%ebp), %eax
  470e02:      	pushl	%eax
  470e03:      	pushl	$0x0
  470e05:      	pushl	%ebx
  470e06:      	calll	*%esi
  470e08:      	movl	%eax, %esi
  470e0a:      	testl	%esi, %esi
  470e0c:      	jne	0x470e2a <.text+0x6fe2a>
  470e0e:      	calll	*0xb83358
  470e14:      	movl	%eax, (%edi)
  470e16:      	leal	0x1(%esi), %eax
  470e19:      	popl	%edi
  470e1a:      	popl	%esi
  470e1b:      	popl	%ebx
  470e1c:      	movl	-0x4(%ebp), %ecx
  470e1f:      	xorl	%ebp, %ecx
  470e21:      	calll	0xafa83b <GetSharedValue+0x378aab>
  470e26:      	movl	%ebp, %esp
  470e28:      	popl	%ebp
  470e29:      	retl
  470e2a:      	movl	0xb8340c, %ebx
  470e30:      	pushl	$0x0
  470e32:      	pushl	$0x0
  470e34:      	calll	*%ebx
  470e36:      	testl	%eax, %eax
  470e38:      	jne	0x470e5f <.text+0x6fe5f>
  470e3a:      	calll	*0xb83358
  470e40:      	pushl	%esi
  470e41:      	movl	%eax, (%edi)
  470e43:      	calll	*0xb83408
  470e49:      	popl	%edi
  470e4a:      	popl	%esi
  470e4b:      	movl	$0x1, %eax
  470e50:      	popl	%ebx
  470e51:      	movl	-0x4(%ebp), %ecx
  470e54:      	xorl	%ebp, %ecx
  470e56:      	calll	0xafa83b <GetSharedValue+0x378aab>
  470e5b:      	movl	%ebp, %esp
  470e5d:      	popl	%ebp
  470e5e:      	retl
  470e5f:      	pushl	%esi
  470e60:      	pushl	-0x2c(%ebp)
  470e63:      	calll	*%ebx
  470e65:      	testl	%eax, %eax
  470e67:      	jne	0x470e98 <.text+0x6fe98>
  470e69:      	calll	*0xb83358
  470e6f:      	pushl	%esi
  470e70:      	movl	%eax, (%edi)
  470e72:      	calll	*0xb83408
  470e78:      	pushl	-0x30(%ebp)
  470e7b:      	pushl	-0x2c(%ebp)
  470e7e:      	calll	*%ebx
  470e80:      	negl	%eax
  470e82:      	popl	%edi
  470e83:      	sbbl	%eax, %eax
  470e85:      	popl	%esi
  470e86:      	addl	$0x2, %eax
  470e89:      	popl	%ebx
  470e8a:      	movl	-0x4(%ebp), %ecx
  470e8d:      	xorl	%ebp, %ecx
  470e8f:      	calll	0xafa83b <GetSharedValue+0x378aab>
  470e94:      	movl	%ebp, %esp
  470e96:      	popl	%ebp
  470e97:      	retl
  470e98:      	movl	-0x34(%ebp), %eax
  470e9b:      	movl	-0x4(%ebp), %ecx
  470e9e:      	xorl	%ebp, %ecx
  470ea0:      	movl	%esi, (%eax)
  470ea2:      	xorl	%eax, %eax
  470ea4:      	movl	$0x0, (%edi)
  470eaa:      	popl	%edi
  470eab:      	popl	%esi
  470eac:      	popl	%ebx
  470ead:      	calll	0xafa83b <GetSharedValue+0x378aab>
  470eb2:      	movl	%ebp, %esp
  470eb4:      	popl	%ebp
  470eb5:      	retl
