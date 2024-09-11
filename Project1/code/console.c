// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

// #include <stdio.h>
#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"


static void consputc(int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  // use lapiccpunum so that we can call panic from mycpu()
  cprintf("lapicid %d: panic: ", lapicid());
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define BACK 0x101
#define FORW 0x102
#define DELALL 0x103
#define CRTPORT 0x3d4
#define ARROW_UP 226
#define ARROW_DOWN 227

#define X 0x104 // for testing

static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if (c ==  BACK){
    if(pos > 0) --pos;
  }
  else if (c==X) {
    pos = (pos / 80) *80 + 3;
  }
  else if (c == DELALL){
    if (pos < (80*25)-1) ++pos;
    int temp = pos;
    for(int i =0; i< temp/80;i++){
      memmove(crt, crt+80, sizeof(crt[0])*23*80);
      pos -= 80;
      memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
    }
    pos--;
  }
  else if (c == FORW){
    if (pos < (80*25)-1) ++pos;
  }
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
    crt[pos] = ' ';
    for(int i = pos; i < (80*25)-1; i++){
      crt[i] = crt[i+1];
    }
  }
   else{
    for(int i = (80*25)-1; i> pos; i--){
      crt[i] = crt[i-1];
    }
    crt[pos++] = (c&0xff) | 0x0700;  // black on white
  }

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");

  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);

    
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else if (c != BACK && c!= FORW && c != DELALL && c!= X)
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
  uint l;
} input;

#define LAST_TEN_COMMAND 10
struct {
  char command_buf[LAST_TEN_COMMAND][INPUT_BUF];
  int rear;
  int size;
  int commmand_read;
  int cursor;
} cbuf;

#define C(x)  ((x)-'@')  // Control-x



void 
save_command()
{

  int i  = 0;
  for(int j = (input.w % INPUT_BUF); j <= (input.e % INPUT_BUF) - 1; j++) {
    cbuf.command_buf[cbuf.rear][i] = input.buf[j % INPUT_BUF];
    ++i;
  }
  
  ++cbuf.rear;
  cbuf.rear = cbuf.rear % 10;
  cbuf.cursor = cbuf.rear;

  if (cbuf.size < 10)
    ++cbuf.size;
  cbuf.commmand_read = cbuf.size;
}

void
getinput(int c)
{
  if(c != 0 && input.l-input.r < INPUT_BUF){
        consputc(c); 
        c = (c == '\r') ? '\n' : c;
        if(c == '\n' || c == C('D') || input.l == input.r+INPUT_BUF){

          save_command();
          

          input.l = input.e;
          input.buf[input.l++ % INPUT_BUF] = c;
          input.e++;
          input.w = input.e;
          wakeup(&input.r);
        }
        else{
          for(int i = input.e; i > input.l%INPUT_BUF; i--){
            input.buf[(i)%INPUT_BUF] = input.buf[(i-1)%INPUT_BUF];
          }
          input.e++;
          input.buf[input.l++ % INPUT_BUF] = c;
        }
      }
}

void
kill_line(int c)
{
  int i = input.l;
      while (i != input.e){
        consputc(FORW);
        i++;
      }
      i = input.e;
      while(i != input.w &&
            input.buf[(i) % INPUT_BUF] != '\n'){
        i--;
        consputc(BACKSPACE);
      }
      input.e = input.w;
      input.l = input.w;

    
}

void
consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;

  acquire(&cons.lock);
  while((c = getc()) >= 0){
    switch(c){
    case C('P'):  // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case ARROW_UP:
      if (cbuf.commmand_read > 0) {
        kill_line(c);
        --cbuf.cursor;
        if (cbuf.cursor < 0)
          cbuf.cursor = 9;
        for (int i = 0; (i < 128) ; i++ ) {
          getinput(cbuf.command_buf[cbuf.cursor][i]);
        }
        --cbuf.commmand_read;
      }
      
    break;

    case ARROW_DOWN:
      if (cbuf.commmand_read < (cbuf.size) - 1) {
        kill_line(c);
        ++cbuf.cursor;
        if (cbuf.cursor > 9)
          cbuf.cursor = 0;
        for (int i = 0; (i < 128) ; i++ ) {
          getinput(cbuf.command_buf[cbuf.cursor][i]);
         }
        ++cbuf.commmand_read;
      }
      else{
        kill_line(c);
        cbuf.cursor = cbuf.rear;
        cbuf.commmand_read = cbuf.size;
      }
    break;

    case C('B'):
      if (input.l != input.w){
        input.l--;
        consputc(BACK);
      }
      break;
    case C('L'):
      int j = input.l;
      while (j != input.e){
        consputc(FORW);
        j++;
      }
      j = input.e;
      while(j != input.w &&
            input.buf[(j) % INPUT_BUF] != '\n'){
        j--;
        consputc(BACKSPACE);
      }
      input.e = input.w;
      input.l = input.w;
      consputc(DELALL);
      break;
    case C('F'):
      if (input.l < input.e){
        input.l++;
        consputc(FORW);
      }
      break;
    case C('U'):  // Kill line.
      kill_line(c);
    break;
    case C('H'): case '\x7f':  // Backspace
      if(input.l != input.w){
        input.l--;
        for(int i = input.l; i < input.e; i++){
          input.buf[(i)%INPUT_BUF] = input.buf[(i+1)%INPUT_BUF];
        }
        input.e--;
        consputc(BACKSPACE);
      }
    break;
    default:
      getinput(c);
      break;
    }

  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input.r == input.w){
      if(myproc()->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  ioapicenable(IRQ_KBD, 0);
}

