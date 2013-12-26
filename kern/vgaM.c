#include "vgaM.h"
#include "io.h"

int init_graph_vga(int width, int height,int chain4) 
  // returns 1=ok, 0=fail
{
   const byte *w,*h;
   byte val;
   int a;

   switch(width) {
      case 256: w=width_256; val=R_COM+R_W256; break;
      case 320: w=width_320; val=R_COM+R_W320; break;
      case 360: w=width_360; val=R_COM+R_W360; break;
      case 376: w=width_376; val=R_COM+R_W376; break;
      case 400: w=width_400; val=R_COM+R_W400; break;
      default: return 0; // fail
   }
   switch(height) {
      case 200: h=height_200; val|=R_H200; break;
      case 224: h=height_224; val|=R_H224; break;
      case 240: h=height_240; val|=R_H240; break;
      case 256: h=height_256; val|=R_H256; break;
      case 270: h=height_270; val|=R_H270; break;
      case 300: h=height_300; val|=R_H300; break;
      case 360: h=height_360; val|=R_H360; break;
      case 400: h=height_400; val|=R_H400; break;
      case 480: h=height_480; val|=R_H480; break;
      case 564: h=height_564; val|=R_H564; break;
      case 600: h=height_600; val|=R_H600; break;
      default: return 0; // fail
   }

   // chain4 not available if mode takes over 64k

   if(chain4 && (long)width*(long)height>65536L) return 0; 

   // here goes the actual modeswitch

   outbp(0x3c2,val);
   outpw(0x3d4,0x0e11); // enable regs 0-7

   for(a=0;a<SZ(hor_regs);++a) 
      outpw(0x3d4,(word)((w[a]<<8)+hor_regs[a]));
   for(a=0;a<SZ(ver_regs);++a)
      outpw(0x3d4,(word)((h[a]<<8)+ver_regs[a]));

   outpw(0x3d4,0x0008); // vert.panning = 0

   if(chain4) {
      outpw(0x3d4,0x4014);
      outpw(0x3d4,0xa317);
      outpw(0x3c4,0x0e04);
   } else {
      outpw(0x3d4,0x0014);
      outpw(0x3d4,0xe317);
      outpw(0x3c4,0x0604);
   }

   outpw(0x3c4,0x0101);
   outpw(0x3c4,0x0f02); // enable writing to all planes
   outpw(0x3ce,0x4005); // 256color mode
   outpw(0x3ce,0x0506); // graph mode & A000-AFFF

   inb(0x3da);
   outbp(0x3c0,0x30); outbp(0x3c0,0x41);
   outbp(0x3c0,0x33); outbp(0x3c0,0x00);

   for(a=0;a<16;a++) {    // ega pal
      outbp(0x3c0,(byte)a); 
      outbp(0x3c0,(byte)a); 
   } 
   
   outbp(0x3c0, 0x20); // enable video

   return 1;
}
