/*
 * Utility functions for trap handling in Supervisor mode.
 */

#include "riscv.h"
#include "process.h"
#include "strap.h"
#include "syscall.h"

#include "spike_interface/spike_utils.h"

//
// handling the syscalls. will call do_syscall() defined in kernel/syscall.c
//
static void handle_syscall(trapframe *tf) {
  // tf->epc points to the address that our computer will jump to after the trap handling.
  // for a syscall, we should return to the NEXT instruction after its handling.
  // in RV64G, each instruction occupies exactly 32 bits (i.e., 4 Bytes)
  tf->epc += 4;
  //TODOSolve: lab1-1 第二个思考题: 下一条指令, 栈+4

  // TODOSolve (lab1_1): remove the panic call below, and call do_syscall (defined in
  // kernel/syscall.c) to conduct real operations of the kernel side for a syscall.
  // IMPORTANT: return value should be returned to user app, or else, you will encounter
  // problems in later experiments!
//  panic( "call do_syscall to accomplish the syscall and lab1_1 here.\n" );
/*
 * 调用 do_syscall() kernel/syscall.c
 * long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7);
 */

    //regs.a1存的是字符串的首地址
    long returnCode=do_syscall(tf->regs.a0,tf->regs.a1,tf->regs.a2,tf->regs.a3,tf->regs.a4,tf->regs.a5,tf->regs.a6,tf->regs.a7);
//    long returnCode=do_syscall(tf->regs.a0,tf->regs.a1,0,0,0,0,0,0);
    //a0保存(函数调用时) 的参数/函数的返回值
    tf->regs.a0=returnCode;

    //重新运行 make clean;make
    //spike ./obj/riscv-pke ./obj/app_helloworld
    /*
     *In m_start, hartid:0
HTIF is available!
(Emulated) memory size: 2048 MB
Enter supervisor mode...
Application: ./obj/app_helloworld
Application program entry point (virtual address): 0x0000000081000000
Switch to user mode...
Hello world!=============>成功!
User exit with code:0.
System is shutting down with exit code 0.
     */
}

//
// kernel/smode_trap.S will pass control to smode_trap_handler, when a trap happens
// in S-mode.
//
//1-判断是否进入之前是U模式
void smode_trap_handler(void) {
  // make sure we are in User mode before entering the trap handling.
  // we will consider other previous case in lab1_3 (interrupt).
  if ((read_csr(sstatus) & SSTATUS_SPP) != 0) panic("usertrap: not from user mode");

  assert(current);
  // save user process counter.
  current->trapframe->epc = read_csr(sepc);

  // if the cause of trap is syscall from user application
  //2-如果是系统调用就选handle_syscall()
  if (read_csr(scause) == CAUSE_USER_ECALL) {
      //3-kernel/process.h
    handle_syscall(current->trapframe);
  } else {
    sprint("smode_trap_handler(): unexpected scause %p\n", read_csr(scause));
    sprint("            sepc=%p stval=%p\n", read_csr(sepc), read_csr(stval));
    panic( "unexpected exception happened.\n" );
  }

  // continue the execution of current process.
  switch_to(current);
}
