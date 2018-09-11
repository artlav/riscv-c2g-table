//############################################################################//
// RISC-V C-to-G table generator
// Made by Artem Litvinovich in 2018
//############################################################################//
#include <stdio.h>
#include <stdint.h>
//############################################################################//
#define CRV_IOP_JALR   0x67
#define CRV_IOP_JAL    0x6F
#define CRV_IOP_IMM    0x13
#define CRV_IOP_LUI    0x37
#define CRV_IOP_STORE  0x23
#define CRV_IOP_LOAD   0x03
#define CRV_IOP_BRANCH 0x63
#define CRV_IOP_OP     0x33
#define CRV_IOP_OP32   0x3B
#define CRV_IOP_IMM32  0x1B
//############################################################################//
//CRV_IOP_IMM
#define CRV_F3_ADDI    0
#define CRV_F3_SLLI    1
#define CRV_F3_SRLSRAI 5
#define CRV_F3_ANDI    7
//CRV_IOP_IMM32
#define CRV_F3_ADDIW   0
//CRV_IOP_STORE
#define CRV_F3_SW      2
#define CRV_F3_SD      3
//CRV_IOP_BRANCH
#define CRV_F3_BEQ     0
#define CRV_F3_BNE     1
//CRV_IOP_LOAD
#define CRV_F3_LW      2
#define CRV_F3_LD      3
//CRV_IOP_OP
#define CRV_F3_ADDSUB  0
#define CRV_F3_XOR     4
#define CRV_F3_OR      6
#define CRV_F3_AND     7
//CRV_IOP_OP32
#define CRV_F3_ADDSUBW 0
//############################################################################//
#define OP_C0 0
#define OP_C1 1
#define OP_C2 2

#define OPC_M0 0
#define OPC_M1 1
#define OPC_M2 2
#define OPC_M3 3
#define OPC_M4 4
#define OPC_M5 5
#define OPC_M6 6
#define OPC_M7 7
//############################################################################//
uint32_t sign_extend_6 (const uint32_t w) {if (w & 0x0020) return 0xFFFFFFC0 | w; else return w;}
uint32_t sign_extend_9 (const uint32_t w) {if (w & 0x0100) return 0xFFFFFE00 | w; else return w;}
uint32_t sign_extend_10(const uint32_t w) {if (w & 0x0200) return 0xFFFFFC00 | w; else return w;}
uint32_t sign_extend_12(const uint32_t w) {if (w & 0x0800) return 0xFFFFF000 | w; else return w;}
//############################################################################//
uint32_t crv_compose_j(const int op,const int rd,const uint32_t imm)
{
 return  (op & 0x7F) |
         ((rd  <<  7) & 0x00000F80) |
         ((imm << 11) & 0x80000000) |
         ((imm <<  0) & 0x000FF000) |
         ((imm <<  9) & 0x00100000) |
         ((imm << 20) & 0x7FE00000);
}
//############################################################################//
uint32_t crv_compose_i(const int op,const int rs1,const int rd,const int funct3,const uint32_t imm)
{
 return  (op & 0x7F) |
         ((rd     <<  7) & 0x00000F80) |
         ((funct3 << 12) & 0x00007000) |
         ((rs1    << 15) & 0x000F8000) |
         ((imm    << 20) & 0xFFF00000);
}
//############################################################################//
uint32_t crv_compose_r(const int op,const int rs1,const int rs2,const int rd,const int funct3,const int funct7)
{
 return  (op & 0x7F) |
         ((rd     <<  7) & 0x00000F80) |
         ((funct3 << 12) & 0x00007000) |
         ((rs1    << 15) & 0x000F8000) |
         ((rs2    << 20) & 0x01F00000) |
         ((funct7 << 25) & 0xFE000000);
}
//############################################################################//
uint32_t crv_compose_s(const int op,const int rs1,const int rs2,const int funct3,const uint32_t imm)
{
 return  (op & 0x7F) |
         ((imm    <<  7) & 0x00000F80) |
         ((funct3 << 12) & 0x00007000) |
         ((rs1    << 15) & 0x000F8000) |
         ((rs2    << 20) & 0x01F00000) |
         ((imm    << 20) & 0xFE000000);
}
//############################################################################//
uint32_t crv_compose_u(const int op,const int rd,const uint32_t imm)
{
 return  (op & 0x7F) |
         ((rd << 7) & 0x00000F80) |
         (imm & 0xFFFFF000);
}
//############################################################################//
uint32_t crv_compose_b(const int op,const int rs1,const int rs2,const int funct3,const uint32_t imm)
{
 return  (op & 0x7F) |
         ((funct3 << 12) & 0x00007000) |
         ((rs1    << 15) & 0x000F8000) |
         ((rs2    << 20) & 0x01F00000) |
         ((imm    << 19) & 0x80000000) |
         ((imm    << 20) & 0x7E000000) |
         ((imm    <<  7) & 0x00000F00) |
         ((imm    >>  4) & 0x00000080);
}
//############################################################################//
uint32_t imm_c0m0(const uint16_t cmd)
{
 return ((cmd & 0x1000) >> 7) |
        ((cmd & 0x0800) >> 7) |
        ((cmd & 0x0400) >> 1) |
        ((cmd & 0x0200) >> 1) |
        ((cmd & 0x0100) >> 1) |
        ((cmd & 0x0080) >> 1) |
        ((cmd & 0x0040) >> 4) |
        ((cmd & 0x0020) >> 2);
}
//############################################################################//
uint32_t imm_c0m2(const uint16_t cmd)
{
 return ((cmd & 0x1000) >> 7) | //5
        ((cmd & 0x0800) >> 7) | //4
        ((cmd & 0x0400) >> 7) | //3
        ((cmd & 0x0040) >> 4) | //2
        ((cmd & 0x0020) << 1);  //6
}
//############################################################################//
uint32_t imm_c0m3(const uint16_t cmd)
{
 return ((cmd & 0x1000) >> 7) |
        ((cmd & 0x0800) >> 7) |
        ((cmd & 0x0400) >> 7) |
        ((cmd & 0x0040) << 1) |
        ((cmd & 0x0020) << 1);
}
//############################################################################//
uint32_t imm_c1m0(const uint16_t cmd)
{
 return sign_extend_6(
         ((cmd & 0x1000) >> 7) |
         ((cmd & 0x0040) >> 2) |
         ((cmd & 0x0020) >> 2) |
         ((cmd & 0x0010) >> 2) |
         ((cmd & 0x0008) >> 2) |
         ((cmd & 0x0004) >> 2)
        );
}
//############################################################################//
uint32_t imm_c1m3(const uint16_t cmd)
{
 return sign_extend_10(
         ((cmd & 0x1000) >> 3) |
         ((cmd & 0x0040) >> 2) |
         ((cmd & 0x0020) << 1) |
         ((cmd & 0x0010) << 4) |
         ((cmd & 0x0008) << 4) |
         ((cmd & 0x0004) << 3)
        );
}
//############################################################################//
uint32_t imm_c1m5(const uint16_t cmd)
{
 return sign_extend_12(
         ((cmd & 0x1000) >> 1) | //12 11
         ((cmd & 0x0800) >> 7) | //11  4
         ((cmd & 0x0400) >> 1) | //10  9
         ((cmd & 0x0200) >> 1) | // 9  8
         ((cmd & 0x0100) << 2) | // 8 10
         ((cmd & 0x0080) >> 1) | // 7  6
         ((cmd & 0x0040) << 1) | // 6  7
         ((cmd & 0x0020) >> 2) | // 5  3
         ((cmd & 0x0010) >> 2) | // 4  2
         ((cmd & 0x0008) >> 2) | // 3  1
         ((cmd & 0x0004) << 3)   // 2  5
        );
}
//############################################################################//
uint32_t imm_c1m6(const uint16_t cmd)
{
 return sign_extend_9(
         ((cmd & 0x1000) >> 4) | //8
         ((cmd & 0x0800) >> 7) | //4
         ((cmd & 0x0400) >> 7) | //3
         ((cmd & 0x0040) << 1) | //7
         ((cmd & 0x0020) << 1) | //6
         ((cmd & 0x0010) >> 2) | //2
         ((cmd & 0x0008) >> 2) | //1
         ((cmd & 0x0004) << 3)   //5
        );
}
//############################################################################//
uint32_t imm_c2m2(const uint16_t cmd)
{
 return ((cmd & 0x1000) >> 7) | //5
        ((cmd & 0x0040) >> 2) | //4
        ((cmd & 0x0020) >> 2) | //3
        ((cmd & 0x0010) >> 2) | //2
        ((cmd & 0x0008) << 4) | //7
        ((cmd & 0x0004) << 4);  //6
}
//############################################################################//
uint32_t imm_c2m3(const uint16_t cmd)
{
 return ((cmd & 0x1000) >> 7) | //5
        ((cmd & 0x0040) >> 2) | //4
        ((cmd & 0x0020) >> 2) | //3
        ((cmd & 0x0010) << 4) | //8
        ((cmd & 0x0008) << 4) | //7
        ((cmd & 0x0004) << 4);  //6
}
//############################################################################//
uint32_t imm_c2m6(const uint16_t cmd)
{
 return ((cmd & 0x1000) >> 7) | //5
        ((cmd & 0x0800) >> 7) | //4
        ((cmd & 0x0400) >> 7) | //3
        ((cmd & 0x0200) >> 7) | //2
        ((cmd & 0x0100) >> 1) | //7
        ((cmd & 0x0080) >> 1);  //6
}
//############################################################################//
uint32_t imm_c2m7(const uint16_t cmd)
{
 return ((cmd & 0x1000) >> 7) | //5
        ((cmd & 0x0800) >> 7) | //4
        ((cmd & 0x0400) >> 7) | //3
        ((cmd & 0x0200) >> 1) | //8
        ((cmd & 0x0100) >> 1) | //7
        ((cmd & 0x0080) >> 1);  //6
}
//############################################################################//
//Convert a 16 bit C instruction into a 32 bit G instruction
uint64_t crv_decompress_real(const uint16_t cmd)
{
 uint16_t op,imm3,b12,a,b,a3,b3;

 if (cmd==0) return 0;
 op=cmd & 0x7F;

 imm3= (cmd >> 13) & 0x07;
 b12 = (cmd >> 12) & 0x01;
 a   = (cmd >>  7) & 0x1F;
 b   = (cmd >>  2) & 0x1F;
 a3=8+((cmd >>  7) & 0x07);
 b3=8+((cmd >>  2) & 0x07);

 switch (op & 0x03) {
  case OP_C0:switch (imm3) {
   case OPC_M0:return crv_compose_i(CRV_IOP_IMM  ,2 ,(b & 0x07)+8,CRV_F3_ADDI,imm_c0m0(cmd));  //c.addi4spn
   case OPC_M2:return crv_compose_i(CRV_IOP_LOAD ,a3,b3          ,CRV_F3_LW  ,imm_c0m2(cmd));  //c.lw
   case OPC_M3:return crv_compose_i(CRV_IOP_LOAD ,a3,b3          ,CRV_F3_LD  ,imm_c0m3(cmd));  //c.ld
   case OPC_M6:return crv_compose_s(CRV_IOP_STORE,a3,b3          ,CRV_F3_SW  ,imm_c0m2(cmd));  //c.sw
   case OPC_M7:return crv_compose_s(CRV_IOP_STORE,a3,b3          ,CRV_F3_SD  ,imm_c0m3(cmd));  //c.sd
   default:    return 0;                                                                       //Invalid
  };return 0;

  case OP_C1:switch (imm3) {
   case OPC_M0:switch (cmd & 0xFFFC) {
    case 0: return crv_compose_i(CRV_IOP_IMM,0,0,CRV_F3_ADDI,0);                               //c.nop
    default:return crv_compose_i(CRV_IOP_IMM,a,a,CRV_F3_ADDI,imm_c1m0(cmd));                   //c.addi
   }
   case OPC_M1:return crv_compose_i(CRV_IOP_IMM32,a,a,CRV_F3_ADDIW,imm_c1m0(cmd));             //c.addiw
   case OPC_M2:return crv_compose_i(CRV_IOP_IMM,0,a,CRV_F3_ADDI,imm_c1m0(cmd));                //c.li
   case OPC_M3:switch (a) {
    case 2:  return crv_compose_i(CRV_IOP_IMM,2,2,CRV_F3_ADDI,imm_c1m3(cmd));                  //c.addi16sp
    case 0:  return 0;                                                                         //Invalid
    default: return crv_compose_u(CRV_IOP_LUI,a,imm_c1m0(cmd) << 12);                          //c.lui
   }
   case OPC_M4:switch (a >> 3) {
    case 0:return crv_compose_i(CRV_IOP_IMM,a3,a3,CRV_F3_SRLSRAI,(b12 << 5) | b);              //c.srli
    case 1:return crv_compose_i(CRV_IOP_IMM,a3,a3,CRV_F3_SRLSRAI,0x400 | (b12 << 5) | b);      //c.srai
    case 2:return crv_compose_i(CRV_IOP_IMM,a3,a3,CRV_F3_ANDI,sign_extend_6((b12 << 5) | b));  //c.andi
    case 3:switch (b12) {
     case 0:switch (b >> 3) {
      case 0:return crv_compose_r(CRV_IOP_OP,a3,b3,a3,CRV_F3_ADDSUB,32);                        //c.sub
      case 1:return crv_compose_r(CRV_IOP_OP,a3,b3,a3,CRV_F3_XOR,0);                            //c.xor
      case 2:return crv_compose_r(CRV_IOP_OP,a3,b3,a3,CRV_F3_OR,0);                             //c.or
      case 3:return crv_compose_r(CRV_IOP_OP,a3,b3,a3,CRV_F3_AND,0);                            //c.and
     }
     case 1:switch (b >> 3) {
      case 0:return crv_compose_r(CRV_IOP_OP32,a3,b3,a3,CRV_F3_ADDSUBW,32);                     //c.subw
      case 1:return crv_compose_r(CRV_IOP_OP32,a3,b3,a3,CRV_F3_ADDSUBW,0);                      //c.addw
      case 2:return 0;                                                                          //Invalid
      case 3:return 0;                                                                          //Invalid
     }
    }
   }
   case OPC_M5:return crv_compose_j(CRV_IOP_JAL,0,imm_c1m5(cmd));                               //c.j
   case OPC_M6:return crv_compose_b(CRV_IOP_BRANCH,a3,0,CRV_F3_BEQ,imm_c1m6(cmd));              //c.beqz
   case OPC_M7:return crv_compose_b(CRV_IOP_BRANCH,a3,0,CRV_F3_BNE,imm_c1m6(cmd));              //c.bnez
   default:return 0;                                                                            //Invalid
  };return 0;

  case OP_C2:switch (imm3) {
   case OPC_M0:return crv_compose_i(CRV_IOP_IMM,a,a,CRV_F3_SLLI,(b12 << 5) | b);                //c.slli
   case OPC_M2:return crv_compose_i(CRV_IOP_LOAD,2,a,CRV_F3_LW,imm_c2m2(cmd));                  //c.lwsp
   case OPC_M3:return crv_compose_i(CRV_IOP_LOAD,2,a,CRV_F3_LD,imm_c2m3(cmd));                  //c.ldsp
   case OPC_M4:switch (b12) {
    case 0:switch (b) {
     case 0: return crv_compose_i(CRV_IOP_JALR,a,0,0,0);                                        //c.jr
     default:return crv_compose_r(CRV_IOP_OP,0,b,a,CRV_F3_ADDSUB,0);                            //c.mv
    }
    case 1:switch (b) {
     case 0:switch (a) {
      case 0: return 0;                                                                         //c.ebreak
      default:return crv_compose_i(CRV_IOP_JALR,a,1,0,0);                                       //c.jalr
     }
     default: return crv_compose_r(CRV_IOP_OP,a,b,a,CRV_F3_ADDSUB,0);                           //c.add
    }
   }
   case OPC_M6:return crv_compose_s(CRV_IOP_STORE,2,b,CRV_F3_SW,imm_c2m6(cmd));                 //c.swsp
   case OPC_M7:return crv_compose_s(CRV_IOP_STORE,2,b,CRV_F3_SD,imm_c2m7(cmd));                 //c.sdsp
   default:return 0;                                                                            //Invalid
  }
  default:return 0;                                                                             //Invalid
 }
}
//############################################################################//
//Print out the tables
void compose_table()
{
 int i,q;
 uint16_t w;
 uint32_t d;

 for (q=0;q<=2;q++) {
  printf("//############################################################################//\n");
  printf("const uint32_t riscv_c2g_c%d[16384]={\n ",q);
  for (i=0;i<16384;i++) {
   if (i!=0) {
    printf(",");
    if ((i % 8)==0) printf("  //%04X\n ",i-8);
   }

   w=(i << 2) | q;
   d=crv_decompress_real(w);
   printf("0x%08X",d);
  }
  printf("   //%04X\n",16384-8);
  printf("};\n");
 }
 printf("//############################################################################//\n");
}
//############################################################################//
int main(int argc,char **argv)
{
 compose_table();
 return 0;
}
//############################################################################//
