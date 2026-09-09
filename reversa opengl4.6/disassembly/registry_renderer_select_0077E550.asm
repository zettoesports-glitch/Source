
/mnt/data/Main.exe:	file format coff-i386

Disassembly of section .text:

00401000 <.text>:
  77e550:      	xorl	%eax, %eax
  77e552:      	jmp	0x77e61d <.text+0x37d61d>
  77e557:      	movaps	0xbd74a0, %xmm0
  77e55e:      	movl	0x5ddff48, %eax
  77e563:      	movl	$0x3ee, -0x14(%ebp)     # imm = 0x3EE
  77e56a:      	movups	%xmm0, -0x24(%ebp)
  77e56e:      	testl	%eax, %eax
  77e570:      	js	0x77e577 <.text+0x37d577>
  77e572:      	cmpl	$0x4, %eax
  77e575:      	jle	0x77e57e <.text+0x37d57e>
  77e577:      	xorl	%eax, %eax
  77e579:      	movl	%eax, 0x5ddff48
  77e57e:      	movl	0xb83588, %esi
  77e584:      	pushl	$0x1
  77e586:      	pushl	-0x24(%ebp,%eax,4)
  77e58a:      	pushl	%edi
  77e58b:      	calll	*%esi
  77e58d:      	leal	-0x54(%ebp), %eax
  77e590:      	movl	$0x1, -0x48(%ebp)
  77e597:      	pushl	%eax
  77e598:      	pushl	$0x20019                # imm = 0x20019
  77e59d:      	pushl	$0x0
  77e59f:      	pushl	$0xbc8bbc               # imm = 0xBC8BBC
  77e5a4:      	pushl	$0x80000001             # imm = 0x80000001
  77e5a9:      	movl	$0x4, -0x50(%ebp)
  77e5b0:      	calll	*0xb83008
  77e5b6:      	testl	%eax, %eax
  77e5b8:      	jne	0x77e5ed <.text+0x37d5ed>
  77e5ba:      	leal	-0x50(%ebp), %eax
  77e5bd:      	pushl	%eax
  77e5be:      	leal	-0x48(%ebp), %eax
  77e5c1:      	pushl	%eax
  77e5c2:      	pushl	$0x0
  77e5c4:      	pushl	$0x0
  77e5c6:      	pushl	$0xbd71d0               # imm = 0xBD71D0
  77e5cb:      	pushl	-0x54(%ebp)
  77e5ce:      	calll	*0xb83010
  77e5d4:      	movl	-0x48(%ebp), %ecx
  77e5d7:      	testl	%eax, %eax
  77e5d9:      	pushl	-0x54(%ebp)
  77e5dc:      	movl	$0x1, %eax
  77e5e1:      	cmovnel	%eax, %ecx
  77e5e4:      	movl	%ecx, -0x48(%ebp)
  77e5e7:      	calll	*0xb83004
  77e5ed:      	movl	-0x48(%ebp), %eax
  77e5f0:      	subl	$0x0, %eax
  77e5f3:      	je	0x77e60d <.text+0x37d60d>
  77e5f5:      	subl	$0x1, %eax
  77e5f8:      	je	0x77e606 <.text+0x37d606>
  77e5fa:      	subl	$0x1, %eax
  77e5fd:      	jne	0x77e606 <.text+0x37d606>
  77e5ff:      	movl	$0x3ef, %eax            # imm = 0x3EF
  77e604:      	jmp	0x77e612 <.text+0x37d612>
  77e606:      	movl	$0x3ed, %eax            # imm = 0x3ED
  77e60b:      	jmp	0x77e612 <.text+0x37d612>
  77e60d:      	movl	$0x3ec, %eax            # imm = 0x3EC
  77e612:      	pushl	$0x1
  77e614:      	pushl	%eax
  77e615:      	pushl	%edi
  77e616:      	calll	*%esi
  77e618:      	movl	$0x1, %eax
  77e61d:      	movl	-0xc(%ebp), %ecx
  77e620:      	movl	%ecx, %fs:0x0
  77e627:      	popl	%ecx
  77e628:      	popl	%edi
  77e629:      	popl	%esi
  77e62a:      	movl	-0x10(%ebp), %ecx
  77e62d:      	xorl	%ebp, %ecx
  77e62f:      	calll	0xafa83b <GetSharedValue+0x378aab>
  77e634:      	movl	%ebp, %esp
  77e636:      	popl	%ebp
  77e637:      	retl	$0x10
