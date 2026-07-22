
#include <rtl_glue.h>

#include <l4pkt.h>
#include <rtk/debug.h>
#include <linux/net.h>



static void memDump (void *start, uint32 size, int8 * strHeader)
{
	int32 row, column, index, index2, max;
	uint8 *buf, *line, ascii[17];
	int8 empty = ' ';

	if(!start ||(size==0))
		return;
	line = (uint8*)start;

	/*
	16 bytes per line
	*/
	if (strHeader)
		rtlglue_printf (KERN_CONT "%s", strHeader);
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16) 
	{

		buf = (uint8*)line;

		memset (ascii, 0, 17);

		max = (index == row - 1) ? column : 16;
		if ( max==0 ) break; /* If we need not dump this line, break it. */
#if 0
		rtlglue_printf ("\n%08x ", (uint32)(memaddr) line);
#else
        rtlglue_printf (KERN_CONT "\n%08x ", index<<4);
#endif
		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
			rtlglue_printf (KERN_CONT "  ");
			rtlglue_printf (KERN_CONT "%02x ", (uint8) buf[index2]);
			ascii[index2] = ((uint8) buf[index2] < 32) ? empty : buf[index2];
		}

		if (max != 16)
		{
			if (max < 8)
				rtlglue_printf (KERN_CONT "  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				rtlglue_printf (KERN_CONT "   ");
		}

		//ASCII
		rtlglue_printf (KERN_CONT "  %s", ascii);
	}
	rtlglue_printf (KERN_CONT "\n");
	return;
}


#ifdef __KERNEL__
#ifndef MAX_PROC_PRINT_SIZE
#define MAX_PROC_PRINT_SIZE	1024
#endif

char proc_print_buf_test[MAX_PROC_PRINT_SIZE];
char *proc_printf_test(struct seq_file *s, char *fmt, ...)
{
    int n;
    int size = MAX_PROC_PRINT_SIZE;     /* Guess we need no more than 512 bytes */
    va_list ap;

    while (1) {
        va_start(ap, fmt);
        n = vsnprintf(proc_print_buf_test, size, fmt, ap);

	printk("%s",proc_print_buf_test);
		
        va_end(ap);

		if (n < 0)
		    return NULL;

		if (n < size)
		    return proc_print_buf_test;

		size = n + 1;

    }
	return NULL;
}
#endif


#if 0
void DEBUG_PKT(uint8 *pkt,uint32 size,char *memo)
{
	if(debug_level&RTK_FB_DEBUG_LEVEL_DEBUG)
		dump_packet(pkt,size,memo);
}
#endif


int DUMP_PACKET_EN=1;	// set as 1 for debug, default is disabled

void dump_packet_header(uint8 *pkt,uint32 size,char *memo)
{
    int off;
    uint8 protocol=0;
    int i;
    int pppoeif=0;

	if(DUMP_PACKET_EN==0)
	return;

    if(pkt == NULL){
		rtlglue_printf("No packet buffer could be dumped @ %s\n", __FUNCTION__);
		return;
    }
    for(i=0; i<78; i++)
        rtlglue_printf("=");
    rtlglue_printf("\n");

    if(size==0)
    {
        rtlglue_printf("%s\npacket_length=0\n",memo);
        return;
    }
	rtlglue_printf("\n");
	off=0;
VXLAN_INNER:	
	rtlglue_printf(""COLOR_Y "DA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\t" COLOR_Y "SA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\n",pkt[off+0],pkt[off+1],pkt[off+2],pkt[off+3],pkt[off+4],pkt[off+5]
				   ,pkt[off+6],pkt[off+7],pkt[off+8],pkt[off+9],pkt[off+10],pkt[off+11]);
	off+=12;

	if(ntohs(*(unsigned short *)(&pkt[off])) < 0x600) //SNAP format 2
	{
		rtlglue_printf("SNAP_2:[" COLOR_Y "Len" COLOR_NM "=%d(0x%x)][" COLOR_Y "LLC" COLOR_NM "=0x%04x%04x%04x]\n",
					   ntohs(*(unsigned short *)(&pkt[off])),ntohs(*(unsigned short *)(&pkt[off])),((uint32)pkt[off+2]<<8)|((uint32)pkt[off+3]),((uint32)pkt[off+4]<<8)|((uint32)pkt[off+5]),((uint32)pkt[off+6]<<8)|((uint32)pkt[off+7]));
        off+=8;
	}

    if((pkt[off]==0x88)&&(pkt[off+1]==0xa8))
    {
        rtlglue_printf("SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
        off+=4;
    }

    if((pkt[off]==0x81)&&(pkt[off+1]==0x00))
    {
        rtlglue_printf("CVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
        off+=4;
    }

	if(ntohs(*(unsigned short *)(&pkt[off])) < 0x600) //SNAP format 1
	{
		rtlglue_printf("SNAP_1:[" COLOR_Y "Len" COLOR_NM "=%d(0x%x)][" COLOR_Y "LLC" COLOR_NM "=0x%04x%04x%04x]\n",
					   ntohs(*(unsigned short *)(&pkt[off])),ntohs(*(unsigned short *)(&pkt[off])),((uint32)pkt[off+2]<<8)|((uint32)pkt[off+3]),((uint32)pkt[off+4]<<8)|((uint32)pkt[off+5]),((uint32)pkt[off+6]<<8)|((uint32)pkt[off+7]));
        off+=8;
	}

    if((pkt[off]==0x88)&&((pkt[off+1]==0x63)||(pkt[off+1]==0x64))) //PPPoE
    {
        rtlglue_printf("PPPoE:[" COLOR_Y "Code" COLOR_NM "=0x%02x][" COLOR_Y "SessionID" COLOR_NM "=0x%04x] [" COLOR_Y "Len" COLOR_NM "=%d]\n",
                       pkt[off+3],((uint32)pkt[off+4]<<8)|pkt[off+5],((uint32)pkt[off+6]<<8)|pkt[off+7]);

        off+=8;
        pppoeif=1;
    }
DUAL_HEADER:
    if(((pkt[off]==0x86)&&(pkt[off+1]==0xdd)) || ((pkt[off]==0x00)&&(pkt[off+1]==0x57)) || (pkt[off]==0x57))		//IPv6 or IPv6 with PPPoE
    {
	if(pkt[off]==0x57)	off-=1;
        rtlglue_printf("IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=0x%02x(DSCP=%d,ECN=%d)][" COLOR_Y "FL" COLOR_NM "=%x%02x%02x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
                       ,pkt[off+2]>>4, (((pkt[off+2]&0xf)<<4) | (pkt[off+3]>>4)),(((pkt[off+2]&0xf)<<4) | ((pkt[off+3]>>4)&0xc))>>2,((pkt[off+3]>>4)&0x3), (pkt[off+3]&0xf), pkt[off+4], pkt[off+5], (pkt[off+6]<<8)+pkt[off+7], pkt[off+8], pkt[off+9]);
        rtlglue_printf("     [" COLOR_Y "SIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                       ,pkt[off+10], pkt[off+11], pkt[off+12], pkt[off+13], pkt[off+14], pkt[off+15], pkt[off+16], pkt[off+17]
                       ,pkt[off+18], pkt[off+19], pkt[off+20], pkt[off+21], pkt[off+22], pkt[off+23], pkt[off+24], pkt[off+25]);
        rtlglue_printf("     [" COLOR_Y "DIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                       ,pkt[off+26], pkt[off+27], pkt[off+28], pkt[off+29], pkt[off+30], pkt[off+31], pkt[off+32], pkt[off+33]
                       ,pkt[off+34], pkt[off+35], pkt[off+36], pkt[off+37], pkt[off+38], pkt[off+39], pkt[off+40], pkt[off+41]);

        protocol=pkt[off+8];
        if(protocol==0)	//hop-by-hop
        {
            rtlglue_printf("Hop-By-Hop:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Length" COLOR_NM "=%d]\n"
                           ,pkt[off+42], pkt[off+43]);
            rtlglue_printf("          [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x]\n"
                           ,pkt[off+44], pkt[off+45], pkt[off+46], pkt[off+47], pkt[off+48], pkt[off+49]);
            for(i=0; i<pkt[off+43]; i++)
            {
                rtlglue_printf("         [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x %02x %02x]\n"
                               ,pkt[off+50+i*8], pkt[off+51+i*8], pkt[off+52+i*8], pkt[off+53+i*8]
                               ,pkt[off+54+i*8], pkt[off+55+i*8], pkt[off+56+i*8], pkt[off+57+i*8]);
            }

            protocol=pkt[off+42];
            off+=(50+pkt[off+43]*8);
			if(protocol == 4){	// IPIP
				off-=2;
				goto DSLITE_INNER;
			}
        }
		else if(protocol == 44)	//fragment hdr
       	{
       		int tmp;
			tmp=(pkt[off+44]<<5)+((pkt[off+45]&0xf8));
       		protocol=pkt[off+42];
			rtlglue_printf("Frag:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Rsv" COLOR_NM "=%d][" COLOR_Y "Offset" COLOR_NM "=%d][" COLOR_Y "MF" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=0x%02x%02x%02x%02x]\n",
				pkt[off+42],pkt[off+43],tmp,pkt[off+45]&0x1,pkt[off+46],pkt[off+47],pkt[off+48],pkt[off+49]);
			off+=50;
       	}
		else if(protocol == 4)	// IPIP
        {
        	off +=40;
		goto DSLITE_INNER;
        }
        else
            off+=42;
    }

	if(((pkt[off]==0x08)&&(pkt[off+1]==0x00))||((pkt[off]==0x00)&&(pkt[off+1]==0x21))||(pkt[off]==0x21))
	{
		if(pkt[off]==0x21)	off-=1;
 DSLITE_INNER:
		rtlglue_printf("IPv4:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "HLen" COLOR_NM "=%d][" COLOR_Y "TOS" COLOR_NM "=0x%02x(DSCP=%d,ECN=%d)][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=%d][" COLOR_Y "R" COLOR_NM "=%d," COLOR_Y "DF" COLOR_NM "=%d," COLOR_Y "MF" COLOR_NM "=%d]\n"
                       ,pkt[off+2]>>4,(pkt[off+2]&0xf)*4,pkt[off+3],pkt[off+3]>>2,pkt[off+3]&0x3,(pkt[off+4]<<8)|pkt[off+5],(pkt[off+6]<<8)|pkt[off+7]
                       ,(pkt[off+8]>>7)&1,(pkt[off+8]>>6)&1,(pkt[off+8]>>5)&1);
		rtlglue_printf("     [" COLOR_Y "FrgOff" COLOR_NM "=%d][" COLOR_Y "TTL" COLOR_NM "=%d][" COLOR_Y "PROTO" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n"
                       ,((pkt[off+8]&0x1f)<<8)|pkt[off+9],pkt[off+10],pkt[off+11],(pkt[off+12]<<8)|pkt[off+13]);
        rtlglue_printf("     [" COLOR_Y "SIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "DIP" COLOR_NM "=%d.%d.%d.%d]\n"
                       ,pkt[off+14],pkt[off+15],pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19],pkt[off+20],pkt[off+21]);

        protocol=pkt[off+11];
        off+=(pkt[off+2]&0xf)*4+2;
    }

    if((pkt[off]==0x08)&&(pkt[off+1]==0x06))
    {
	rtlglue_printf("ARP:[" COLOR_Y "SenderIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "TargetIP=" COLOR_NM "=%d.%d.%d.%d]\n"
                       ,pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19]
                        ,pkt[off+26],pkt[off+27],pkt[off+28],pkt[off+29]);
	off+=30;
    }

    if(protocol==0x6) //TCP
    {
        rtlglue_printf("TCP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x][" COLOR_Y "HLen" COLOR_NM "=%d]\n"
                       ,(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3]),(pkt[off+4]<<24)|(pkt[off+5]<<16)|(pkt[off+6]<<8)|(pkt[off+7]<<0)
                       ,(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|(pkt[off+11]<<0),pkt[off+12]>>4<<2);
        rtlglue_printf("    [" COLOR_Y "URG" COLOR_NM "=%d][" COLOR_Y "ACK" COLOR_NM "=%d][" COLOR_Y "PSH" COLOR_NM "=%d][" COLOR_Y "RST" COLOR_NM "=%d][" COLOR_Y "SYN" COLOR_NM "=%d][" COLOR_Y "FIN" COLOR_NM "=%d][" COLOR_Y "Win" COLOR_NM "=%d]\n"
                       ,(pkt[off+13]>>5)&1,(pkt[off+13]>>4)&1,(pkt[off+13]>>3)&1,(pkt[off+13]>>2)&1,(pkt[off+13]>>1)&1,(pkt[off+13]>>0)&1
                       ,(pkt[off+14]<<8)|pkt[off+15]);
        rtlglue_printf("    [" COLOR_Y "CHM" COLOR_NM "=0x%x][" COLOR_Y "Urg" COLOR_NM "=0x%x]\n",(pkt[off+16]<<8)|(pkt[off+17]<<0),(pkt[off+18]<<8)|(pkt[off+19]<<0));
    }
    else if(protocol==0x11) //UDP
    {
    	uint16 sport = (pkt[off]<<8)|(pkt[off+1]);
		
        rtlglue_printf("UDP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n",(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3])
                       ,(pkt[off+4]<<8)|(pkt[off+5]),(pkt[off+6]<<8)|(pkt[off+7]));
		if(pkt[off+8]==0x8 && pkt[off+9]==0x0 && pkt[off+10]==0x0 && pkt[off+11]==0x0) //vxlan
		{
			off += 8;
			rtlglue_printf("VXLAN:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "VNI" COLOR_NM "=0x%x]\n",
							pkt[off], (pkt[off+4]<<16)|(pkt[off+5]<<8)|(pkt[off+6]));			
			off += 8;
			goto VXLAN_INNER;
		}
		else if(sport==1701)
		{
			do{
				int tmpOff = 0;
				int l2tp_len = 0, l2tp_tunnelid = 0, l2tp_sessionid = 0, l2tp_offset = 0, l2tp_Ns=0, l2tp_Nr=0;
				off += 8;
				//if(pkt[off]&0x80)break;			// skip control packet parsing
				//if((pkt[off+1]&0xf)!=0x2) break;	// only support version 2 now
				//if(pkt[off]&0x08) break;			// Sequence bit
				//if(pkt[off]&0x02) break;			// Offset bit
				//if(pkt[off]&0x01) break;			// Priority bit

				rtlglue_printf("L2TP:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d]", (pkt[off]<<8)|(pkt[off+1]), (pkt[off+1]&0xf));
				tmpOff = 2;						// flag+version
				if(pkt[off]&0x40){				// Length bit
					l2tp_len = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					rtlglue_printf("[" COLOR_Y "Len" COLOR_NM "=%d]", l2tp_len);
				}
				l2tp_tunnelid = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
				l2tp_sessionid = pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
				tmpOff+=4;					// Tunnel ID + Session ID
				rtlglue_printf("[" COLOR_Y"TunnelID" COLOR_NM "=%d][" COLOR_Y"SessionID" COLOR_NM "=%d]", l2tp_tunnelid, l2tp_sessionid);
				if(pkt[off]&0x08){				// Length bit
					l2tp_Ns = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					l2tp_Nr = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;
					rtlglue_printf("[" COLOR_Y"Ns" COLOR_NM "=%d][" COLOR_Y"Nr" COLOR_NM "=%d]", l2tp_Ns, l2tp_Nr);
				}
				if(pkt[off]&0x02){
					l2tp_offset = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;				// offset size
					tmpOff+=l2tp_offset;	// offset padding
					rtlglue_printf("[" COLOR_Y "Offset" COLOR_NM "=%d]", l2tp_offset);
				}
				rtlglue_printf("\n");

				off+= (tmpOff);		// tmpOff: L2TP
				if((pkt[off] == 0xff) && pkt[off+1] == 0x03)	off+=2;	// 2: PPP(address, control)
				goto DUAL_HEADER;
			}while(0);
		}
    }
    else if(protocol==0x2f) //GRE(47)
    {
    	uint32 optionOffset = 0;
    	protocol=(pkt[off+2]<<8) | pkt[off+3] ;
        rtlglue_printf("GRE:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "PLen" COLOR_NM "=0x%x][" COLOR_Y"CallID" COLOR_NM "=0x%x]\n",
			(pkt[off]<<8)|(pkt[off+1]&0xfff8), (pkt[off+1]&0x7), (pkt[off+4]<<8) | pkt[off+5], (pkt[off+6]<<8) | pkt[off+7]);
	if(pkt[off] & GRE_S_BIT)	// seq bit: GRE_S_BIT	0x10
	{
		rtlglue_printf("    [" COLOR_Y "Seq" COLOR_NM "=0x%x]", (pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|pkt[off+11]);
		optionOffset +=4;
	}
	if(pkt[off+1] & 0x80)		// ack bit
	{
	 	rtlglue_printf("    [" COLOR_Y "Ack" COLOR_NM "=0x%x]", (pkt[off+optionOffset+8]<<24)|(pkt[off+optionOffset+9]<<16)|(pkt[off+optionOffset+10]<<8)|pkt[off+optionOffset+11]);
		optionOffset +=4;
	}
	// rtlglue_printf("    [" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x]\n",
	//		(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|pkt[off+11], (pkt[off+12]<<24)|(pkt[off+13]<<16)|(pkt[off+14]<<8)|pkt[off+15]);
	if(optionOffset!=0)	rtlglue_printf("\n");

	if((pkt[off+8+optionOffset] == 0xff) && pkt[off+8+optionOffset+1] == 0x03)	optionOffset+=2;	// 2: PPP(address, control)
	off+= (8+optionOffset);		// 8 GRE basic header; optionOffset 0~10: 8: seq and ac, 2: PPP(address, control)
	goto DUAL_HEADER;
    }
	else if(protocol==0x73)	//l2tpv3(115)
	{
		do{
			uint32 l2tp_sessionid = 0, l2tp_len = 0, l2tp_ctrl_connectionId = 0, l2tp_Ns=0, l2tp_Nr=0;
			int tmpOff = 0;

			off += 8;
			l2tp_sessionid = pkt[off+tmpOff]<<24 | pkt[off+tmpOff+1]<<16 | pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
			tmpOff += 4;
			if(l2tp_sessionid)
			{
				rtlglue_printf("L2TPv3 data:[" COLOR_Y "SessionID" COLOR_NM "=%d]", l2tp_sessionid);
				tmpOff += 8;
			}
			else
			{
				rtlglue_printf("L2TPv3 ctrl:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d]", (pkt[off+4]<<8)|(pkt[off+5]), (pkt[off+5]&0xf));
				tmpOff += 2;
				if(pkt[off+4]&0x40){				// Length bit
					l2tp_len = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					rtlglue_printf("[" COLOR_Y "Len" COLOR_NM "=%d]", l2tp_len);
				}
				l2tp_ctrl_connectionId = pkt[off+tmpOff]<<24 | pkt[off+tmpOff+1]<<16 | pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
				rtlglue_printf("[" COLOR_Y "ctrl connection id" COLOR_NM "=%d]", l2tp_ctrl_connectionId);
				if(pkt[off+4]&0x08){				// seq bit
					l2tp_Ns = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					l2tp_Nr = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;
					rtlglue_printf("[" COLOR_Y"Ns" COLOR_NM "=%d][" COLOR_Y"Nr" COLOR_NM "=%d]", l2tp_Ns, l2tp_Nr);
				}
			}
			off += (tmpOff);		// tmpOff: L2TP
		}while(0);
	}
}

#if 0
void dump_compare_packet(uint8 *output,uint8 *expect,int out_size,int expect_size)
{
    int off;
    u8 protocol=0;
    int i,j,vlanCnt,pppoeif=0;
    uint8 *pkt=NULL;
    char tmpstr[32];
    int size=max(out_size,expect_size);
	int32 ret=SUCCESS;

    for(j=1; j<3; j++)
    {
		 pppoeif = 0;
        //if(j==0) {pkt=input; sprintf(tmpstr,"INPUT");}
        if(j==1)
        {
            pkt=output;
            sprintf(tmpstr,"OUTPUT(size=%d)",out_size);
        }
        if(j==2)
        {
            pkt=expect;
            sprintf(tmpstr,"EXPECT(size=%d)",expect_size);
        }

        for(i=0; i<78; i++)
            rtlglue_printf(KERN_CONT "=");

        rtlglue_printf(KERN_CONT "\n");

        rtlglue_printf(KERN_CONT "%s\n",tmpstr);
        if(size==0) return;

	if(size>1518)
		size = 1518;

        for(i=0; i<size; i++)
        {
            int diff=0;

            if(j==1)
            {
                if((i>=size)||(size<=4))
                    break;//goto skip_dump;
            }
            if(j==2)
            {
                if((i>=size)||(size<=4))
                    break;//goto skip_dump;
            }
            if((i%16)==0) rtlglue_printf(KERN_CONT "%08x ",(unsigned int)(unsigned long int)(&pkt[i]));
            //if((input[i]!=output[i])||(output[i]!=expect[i])) diff=1;
            if(i<max(out_size,expect_size))
                if(output[i]!=expect[i]) 
				{
					diff=1;
					ret=FAILED;
				}
            rtlglue_printf(KERN_CONT "%s%02x%s ",(diff==1)?COLOR_H:"",(u8)pkt[i],(diff==1)?COLOR_NM:"");
            if((i%16)==7) rtlglue_printf(KERN_CONT "  ");
            if((i%16)==15) rtlglue_printf(KERN_CONT "\n");
        }
        if(i%16!=15)
            rtlglue_printf(KERN_CONT "\n");

		rtlglue_printf(KERN_CONT "\n");
		off=0;
VXLAN_INNER:	
		rtlglue_printf(KERN_CONT ""COLOR_Y "DA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\t" COLOR_Y "SA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\n",pkt[off+0],pkt[off+1],pkt[off+2],pkt[off+3],pkt[off+4],pkt[off+5]
					   ,pkt[off+6],pkt[off+7],pkt[off+8],pkt[off+9],pkt[off+10],pkt[off+11]);
		off+=12;


		if(ntohs(*(unsigned short *)(&pkt[off])) < 0x600) //SNAP format 2
		{
			rtlglue_printf(KERN_CONT "SNAP_2:[" COLOR_Y "Len" COLOR_NM "=%d(0x%x)][" COLOR_Y "LLC" COLOR_NM "=0x%04x%04x%04x]\n",
						   ntohs(*(unsigned short *)(&pkt[off])),ntohs(*(unsigned short *)(&pkt[off])),((uint32)pkt[off+2]<<8)|((uint32)pkt[off+3]),((uint32)pkt[off+4]<<8)|((uint32)pkt[off+5]),((uint32)pkt[off+6]<<8)|((uint32)pkt[off+7]));
            off+=8;
		}

		for(vlanCnt=0;vlanCnt<4;vlanCnt++)
		{

	        if((pkt[off]==0x88)&&(pkt[off+1]==0xa8))
	        {
	            rtlglue_printf(KERN_CONT "SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
	            off+=4;
	        }

	        if((pkt[off]==0x91)&&(pkt[off+1]==0x00))
	        {
	            rtlglue_printf(KERN_CONT "SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
	            off+=4;
	        }


	        if((pkt[off]==0x81)&&(pkt[off+1]==0x00))
	        {
	            rtlglue_printf(KERN_CONT "CVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
	            off+=4;
	        }


	        if((pkt[off]==0x55)&&(pkt[off+1]==0x66))
	        {
	            rtlglue_printf(KERN_CONT "OTHER_VLAN_5566:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
	            off+=4;
	        }			
	        if((pkt[off]==0x77)&&(pkt[off+1]==0x88))
	        {
	            rtlglue_printf(KERN_CONT "OTHER_VLAN_7788:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
	            off+=4;
	        }			
			
		}

		if(ntohs(*(unsigned short *)(&pkt[off])) < 0x600) //SNAP format 1
		{
			rtlglue_printf(KERN_CONT "SNAP_1:[" COLOR_Y "Len" COLOR_NM "=%d(0x%x)][" COLOR_Y "LLC" COLOR_NM "=0x%04x%04x%04x]\n",
						   ntohs(*(unsigned short *)(&pkt[off])),ntohs(*(unsigned short *)(&pkt[off])),((uint32)pkt[off+2]<<8)|((uint32)pkt[off+3]),((uint32)pkt[off+4]<<8)|((uint32)pkt[off+5]),((uint32)pkt[off+6]<<8)|((uint32)pkt[off+7]));
            off+=8;
		}

        if((pkt[off]==0x88)&&((pkt[off+1]==0x63)||(pkt[off+1]==0x64))) //PPPoE
        {
			rtlglue_printf(KERN_CONT "PPPoE:[" COLOR_Y "Code" COLOR_NM "=0x%02x][" COLOR_Y "SessionID" COLOR_NM "=0x%04x] [" COLOR_Y "Len" COLOR_NM "=%d]\n",
						   pkt[off+3],((uint32)pkt[off+4]<<8)|pkt[off+5],((uint32)pkt[off+6]<<8)|pkt[off+7]);
            off+=8;
            pppoeif=1;
        }
DUAL_HEADER:
        if(((pkt[off]==0x86)&&(pkt[off+1]==0xdd)) || ((pkt[off]==0x00)&&(pkt[off+1]==0x57)) || (pkt[off]==0x57))		//IPv6 or IPv6 with PPPoE
        {
		if(pkt[off]==0x57)	off-=1;
	
            rtlglue_printf(KERN_CONT "IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=0x%02x(DSCP=%d,ECN=%d)][" COLOR_Y "FL" COLOR_NM "=0x%x%02x%02x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
                           ,pkt[off+2]>>4, (((pkt[off+2]&0xf)<<4) | (pkt[off+3]>>4)),(((pkt[off+2]&0xf)<<4) | ((pkt[off+3]>>4)&0xc))>>2,((pkt[off+3]>>4)&0x3), (pkt[off+3]&0xf), pkt[off+4], pkt[off+5], (pkt[off+6]<<8)+pkt[off+7], pkt[off+8], pkt[off+9]);
            rtlglue_printf(KERN_CONT "     [" COLOR_Y "SIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                           ,pkt[off+10], pkt[off+11], pkt[off+12], pkt[off+13], pkt[off+14], pkt[off+15], pkt[off+16], pkt[off+17]
                           ,pkt[off+18], pkt[off+19], pkt[off+20], pkt[off+21], pkt[off+22], pkt[off+23], pkt[off+24], pkt[off+25]);
            rtlglue_printf(KERN_CONT "     [" COLOR_Y "DIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                           ,pkt[off+26], pkt[off+27], pkt[off+28], pkt[off+29], pkt[off+30], pkt[off+31], pkt[off+32], pkt[off+33]
                           ,pkt[off+34], pkt[off+35], pkt[off+36], pkt[off+37], pkt[off+38], pkt[off+39], pkt[off+40], pkt[off+41]);

            protocol=pkt[off+8];
            if(protocol==0)	//hop-by-hop
            {
                rtlglue_printf(KERN_CONT "Hop-By-Hop:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Length" COLOR_NM "=%d]\n"
                               ,pkt[off+42], pkt[off+43]);
                rtlglue_printf(KERN_CONT "          [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x]\n"
                               ,pkt[off+44], pkt[off+45], pkt[off+46], pkt[off+47], pkt[off+48], pkt[off+49]);
                for(i=0; i<pkt[off+43]; i++)
                {
                    rtlglue_printf(KERN_CONT "         [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x %02x %02x]\n"
                                   ,pkt[off+50+i*8], pkt[off+51+i*8], pkt[off+52+i*8], pkt[off+53+i*8]
                                   ,pkt[off+54+i*8], pkt[off+55+i*8], pkt[off+56+i*8], pkt[off+57+i*8]);
                }

                protocol=pkt[off+42];
                off+=(50+pkt[off+43]*8);
            }else if(protocol == 4)	// IPIP
	        {
	        	off +=40;
			goto DSLITE_INNER;
	        }
            else
                off+=42;
        }
        //rtlglue_printf("###############the pkt off before IPV4 is %02x off+1 is %02x, pppoeif is %d\n",pkt[off],pkt[off+1],pppoeif);
        if(((pkt[off]==0x08)&&(pkt[off+1]==0x00)&&(pppoeif==0))||((pkt[off]==0x00)&&(pkt[off+1]==0x21)/*&&(pppoeif==1)*/) || (pkt[off]==0x21))
        {
		if(pkt[off]==0x21)	off-=1;
DSLITE_INNER:
            rtlglue_printf(KERN_CONT "IPv4:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "HLen" COLOR_NM "=%d][" COLOR_Y "TOS" COLOR_NM "=0x%02x(DSCP=%d,ECN=%d)][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=%d][" COLOR_Y "R" COLOR_NM "=%d," COLOR_Y "DF" COLOR_NM "=%d," COLOR_Y "MF" COLOR_NM "=%d]\n"
                           ,pkt[off+2]>>4/*Ver*/,(pkt[off+2]&0xf)*4/*HLen*/,pkt[off+3]/*TOS*/,pkt[off+3]>>2,pkt[off+3]&0x3,(pkt[off+4]<<8)|pkt[off+5],(pkt[off+6]<<8)|pkt[off+7]
                           ,(pkt[off+8]>>7)&1,(pkt[off+8]>>6)&1,(pkt[off+8]>>5)&1);
            rtlglue_printf(KERN_CONT "	 [" COLOR_Y "FrgOff" COLOR_NM "=%d][" COLOR_Y "TTL" COLOR_NM "=%d][" COLOR_Y "PROTO" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n"
                           ,((pkt[off+8]&0x1f)<<8)|pkt[off+9],pkt[off+10],pkt[off+11],(pkt[off+12]<<8)|pkt[off+13]);
            rtlglue_printf(KERN_CONT "	 [" COLOR_Y "SIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "DIP" COLOR_NM "=%d.%d.%d.%d]\n"
                           ,pkt[off+14],pkt[off+15],pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19],pkt[off+20],pkt[off+21]);

            protocol=pkt[off+11];
            off+=(pkt[off+2]&0xf)*4+2;
        }

	if((pkt[off]==0x08)&&(pkt[off+1]==0x06))
	{
		rtlglue_printf(KERN_CONT "ARP:[" COLOR_Y "SenderIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "TargetIP=" COLOR_NM "=%d.%d.%d.%d]\n"
	                   ,pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19]
	                    ,pkt[off+26],pkt[off+27],pkt[off+28],pkt[off+29]);
		off+=30;
	}
	
        if(protocol==0x6) //TCP
        {
            rtlglue_printf(KERN_CONT "TCP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x][" COLOR_Y "HLen" COLOR_NM "=%d]\n"
                           ,(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3]),(pkt[off+4]<<24)|(pkt[off+5]<<16)|(pkt[off+6]<<8)|(pkt[off+7]<<0)
                           ,(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|(pkt[off+11]<<0),pkt[off+12]>>4<<2);
            rtlglue_printf(KERN_CONT "	[" COLOR_Y "URG" COLOR_NM "=%d][" COLOR_Y "ACK" COLOR_NM "=%d][" COLOR_Y "PSH" COLOR_NM "=%d][" COLOR_Y "RST" COLOR_NM "=%d][" COLOR_Y "SYN" COLOR_NM "=%d][" COLOR_Y "FIN" COLOR_NM "=%d][" COLOR_Y "Win" COLOR_NM "=%d]\n"
                           ,(pkt[off+13]>>5)&1,(pkt[off+13]>>4)&1,(pkt[off+13]>>3)&1,(pkt[off+13]>>2)&1,(pkt[off+13]>>1)&1,(pkt[off+13]>>0)&1
                           ,(pkt[off+14]<<8)|pkt[off+15]);
            rtlglue_printf(KERN_CONT "	[" COLOR_Y "CHM" COLOR_NM "=0x%x][" COLOR_Y "Urg" COLOR_NM "=0x%x]\n",(pkt[off+16]<<8)|(pkt[off+17]<<0),(pkt[off+18]<<8)|(pkt[off+19]<<0));
			if((pkt[off+12]>>4<<2)>20){
				//parsing mss
				if(pkt[off+20]==0x2){
					rtlglue_printf(KERN_CONT "    [" COLOR_Y "Option(%d Byte):MSS" COLOR_NM "=0x%x]\n",pkt[off+21],(pkt[off+22]<<8)|(pkt[off+23]));
				}
			}
        }
        else if(protocol==0x11) //UDP
        {
        	uint16 sport = (pkt[off]<<8)|(pkt[off+1]);
			
            rtlglue_printf(KERN_CONT "UDP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n",(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3])
                           ,(pkt[off+4]<<8)|(pkt[off+5]),(pkt[off+6]<<8)|(pkt[off+7]));
			
			if(pkt[off+8]==0x8 && pkt[off+9]==0x0 && pkt[off+10]==0x0 && pkt[off+11]==0x0) //vxlan
			{
				off += 8;
				rtlglue_printf(KERN_CONT "VXLAN:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "VNI" COLOR_NM "=0x%x]\n",
								pkt[off], (pkt[off+4]<<16)|(pkt[off+5]<<8)|(pkt[off+6]));			
				off += 8;
				goto VXLAN_INNER;
			}
			else if(sport==1701)
			{
				do{
					int tmpOff = 0;
					int l2tp_len = 0, l2tp_tunnelid = 0, l2tp_sessionid = 0, l2tp_offset = 0, l2tp_Ns=0, l2tp_Nr=0;
					off += 8;
					//if(pkt[off]&0x80)break;			// skip control packet parsing
					//if((pkt[off+1]&0xf)!=0x2) break;	// only support version 2 now
					//if(pkt[off]&0x08) break;			// Sequence bit
					//if(pkt[off]&0x02) break;			// Offset bit
					//if(pkt[off]&0x01) break;			// Priority bit

					rtlglue_printf(KERN_CONT "L2TP:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d]", (pkt[off]<<8)|(pkt[off+1]), (pkt[off+1]&0xf));
					tmpOff = 2;						// flag+version
					if(pkt[off]&0x40){				// Length bit
						l2tp_len = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
						tmpOff+=2;	
						rtlglue_printf(KERN_CONT "[" COLOR_Y "Len" COLOR_NM "=%d]", l2tp_len);
					}
					l2tp_tunnelid = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					l2tp_sessionid = pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
					tmpOff+=4;					// Tunnel ID + Session ID
					rtlglue_printf(KERN_CONT "[" COLOR_Y"TunnelID" COLOR_NM "=%d][" COLOR_Y"SessionID" COLOR_NM "=%d]", l2tp_tunnelid, l2tp_sessionid);
					if(pkt[off]&0x08){				// Length bit
						l2tp_Ns = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
						tmpOff+=2;	
						l2tp_Nr = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
						tmpOff+=2;
						rtlglue_printf(KERN_CONT "[" COLOR_Y"Ns" COLOR_NM "=%d][" COLOR_Y"Nr" COLOR_NM "=%d]", l2tp_Ns, l2tp_Nr);
					}
					if(pkt[off]&0x02){
						l2tp_offset = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
						tmpOff+=2;				// offset size
						tmpOff+=l2tp_offset;	// offset padding
						rtlglue_printf(KERN_CONT "[" COLOR_Y "Offset" COLOR_NM "=%d]", l2tp_offset);
					}
					rtlglue_printf(KERN_CONT "\n");

					off+= (tmpOff);		// tmpOff: L2TP
					if((pkt[off] == 0xff) && pkt[off+1] == 0x03)	off+=2;	// 2: PPP(address, control)
					goto DUAL_HEADER;
				}while(0);
			}
        }
	else if(protocol==0x2f) //GRE(47)
	{
		uint32 optionOffset = 0;
		protocol=(pkt[off+2]<<8) | pkt[off+3] ;
	        rtlglue_printf(KERN_CONT "GRE:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Protocol" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "PLen" COLOR_NM "=%d][" COLOR_Y"CallID" COLOR_NM "=0x%x]\n",
				(pkt[off]<<8)|(pkt[off+1]&0xfff8), (pkt[off+2]<<8) | pkt[off+3], (pkt[off+1]&0x7), (pkt[off+4]<<8) | pkt[off+5], (pkt[off+6]<<8) | pkt[off+7]);
		if(pkt[off] & GRE_S_BIT)	// seq bit: GRE_S_BIT	0x10
		{
			rtlglue_printf(KERN_CONT "    [" COLOR_Y "Seq" COLOR_NM "=0x%x]", (pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|pkt[off+11]);
			optionOffset +=4;
		}
		if(pkt[off+1] & 0x80)		// ack bit
		{
			rtlglue_printf(KERN_CONT "    [" COLOR_Y "Ack" COLOR_NM "=0x%x]", (pkt[off+optionOffset+8]<<24)|(pkt[off+optionOffset+9]<<16)|(pkt[off+optionOffset+10]<<8)|pkt[off+optionOffset+11]);
			optionOffset +=4;
		}
		// rtlglue_printf("    [" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x]\n",
		//		(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|pkt[off+11], (pkt[off+12]<<24)|(pkt[off+13]<<16)|(pkt[off+14]<<8)|pkt[off+15]);
		if(optionOffset!=0)	rtlglue_printf(KERN_CONT "\n");

		if((pkt[off+8+optionOffset] == 0xff) && pkt[off+8+optionOffset+1] == 0x03)	optionOffset+=2;	// 2: PPP(address, control)
		off+= (8+optionOffset);		// 8 GRE basic header; optionOffset 0~10: 8: seq and ac, 2: PPP(address, control)
		goto DUAL_HEADER;
    }
	else if(protocol==0x73)	//l2tpv3(115)
	{
		do{
			uint32 l2tp_sessionid = 0, l2tp_len = 0, l2tp_ctrl_connectionId = 0, l2tp_Ns=0, l2tp_Nr=0;
			int tmpOff = 0;
			off += 8;

			l2tp_sessionid = pkt[off+tmpOff]<<24 | pkt[off+tmpOff+1]<<16 | pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
			tmpOff += 4;
			if(l2tp_sessionid)
			{
				rtlglue_printf(KERN_CONT "L2TPv3 data:[" COLOR_Y "SessionID" COLOR_NM "=%d]", l2tp_sessionid);
				tmpOff += 8;
			}
			else
			{
				rtlglue_printf(KERN_CONT "L2TPv3 ctrl:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d]", (pkt[off+4]<<8)|(pkt[off+5]), (pkt[off+5]&0xf));
				tmpOff += 2;
				if(pkt[off+4]&0x40){				// Length bit
					l2tp_len = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					rtlglue_printf(KERN_CONT "[" COLOR_Y "Len" COLOR_NM "=%d]", l2tp_len);
				}
				l2tp_ctrl_connectionId = pkt[off+tmpOff]<<24 | pkt[off+tmpOff+1]<<16 | pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
				rtlglue_printf(KERN_CONT "[" COLOR_Y "ctrl connection id" COLOR_NM "=%d]", l2tp_ctrl_connectionId);
				if(pkt[off+4]&0x08){				// seq bit
					l2tp_Ns = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					l2tp_Nr = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;
					rtlglue_printf(KERN_CONT "[" COLOR_Y"Ns" COLOR_NM "=%d][" COLOR_Y"Nr" COLOR_NM "=%d]", l2tp_Ns, l2tp_Nr);
				}
			}
			off += (tmpOff);		// tmpOff: L2TP
		}while(0);
	}
//skip_dump:
        //      ;
    }

	if(ret ==SUCCESS)
		 rtlglue_printf(KERN_CONT COLOR_G"\tPacket Compare PASS"COLOR_NM"\n");

	return;
	
}
#endif


void test_dump_packet(uint8 *pkt,uint32 size,char *memo)
{
    int off;
    uint8 protocol=0;
    int i;
    int pppoeif=0;

	if(DUMP_PACKET_EN==0)
		return;
	
    if(pkt == NULL){
		rtlglue_printf(KERN_CONT "No packet buffer could be dumped @ %s\n", __FUNCTION__);
		return;
    }
    for(i=0; i<78; i++)
        rtlglue_printf(KERN_CONT "=");
    rtlglue_printf(KERN_CONT "\n");

    if(size==0)
    {
        rtlglue_printf(KERN_CONT "%s\npacket_length=0\n",memo);
        return;
    }
	rtlglue_printf(KERN_CONT "%s\n (packet_length=%d)\n",memo,size);
    memDump(pkt,size,memo);
	rtlglue_printf(KERN_CONT "\n");
	off=0;
VXLAN_INNER:	
    rtlglue_printf(KERN_CONT ""COLOR_Y "DA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\t" COLOR_Y "SA" COLOR_NM ":[%02X-%02X-%02X-%02X-%02X-%02X]\n",pkt[off+0],pkt[off+1],pkt[off+2],pkt[off+3],pkt[off+4],pkt[off+5]
                   ,pkt[off+6],pkt[off+7],pkt[off+8],pkt[off+9],pkt[off+10],pkt[off+11]);
    off+=12;

	if(ntohs(*(unsigned short *)(&pkt[off])) < 0x600) //SNAP format 2
	{
		rtlglue_printf(KERN_CONT "SNAP_2:[" COLOR_Y "Len" COLOR_NM "=%d(0x%x)][" COLOR_Y "LLC" COLOR_NM "=0x%04x%04x%04x]\n",
					   ntohs(*(unsigned short *)(&pkt[off])),ntohs(*(unsigned short *)(&pkt[off])),((uint32)pkt[off+2]<<8)|((uint32)pkt[off+3]),((uint32)pkt[off+4]<<8)|((uint32)pkt[off+5]),((uint32)pkt[off+6]<<8)|((uint32)pkt[off+7]));
        off+=8;
	}

	while(1)
	{
		int orig_off=off;
		if((pkt[off]==0x88)&&(pkt[off+1]==0xa8))
		{
			rtlglue_printf(KERN_CONT "SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
			off+=4;
		}
	
		if((pkt[off]==0x91)&&(pkt[off+1]==0x00))
		{
			rtlglue_printf(KERN_CONT "SVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "DEI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
			off+=4;
		}
	
	
		if((pkt[off]==0x81)&&(pkt[off+1]==0x00))
		{
			rtlglue_printf(KERN_CONT "CVLAN:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
			off+=4;
		}
	
	
		if((pkt[off]==0x55)&&(pkt[off+1]==0x66))
		{
			rtlglue_printf(KERN_CONT "OTHER_VLAN_5566:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
			off+=4;
		}			
		if((pkt[off]==0x77)&&(pkt[off+1]==0x88))
		{
			rtlglue_printf(KERN_CONT "OTHER_VLAN_7788:[" COLOR_Y "Pri" COLOR_NM "=%d][" COLOR_Y "CFI" COLOR_NM "=%d][" COLOR_Y "VID" COLOR_NM "=%d]\n",pkt[off+2]>>5,(pkt[off+2]>>4)&1,((pkt[off+2]&0xf)<<8)|(pkt[off+3]));
			off+=4;
		}			
		if(off==orig_off)break;	//no more vlan
	}

	if(ntohs(*(unsigned short *)(&pkt[off])) < 0x600) //SNAP format 1
	{
		rtlglue_printf(KERN_CONT "SNAP_1:[" COLOR_Y "Len" COLOR_NM "=%d(0x%x)][" COLOR_Y "LLC" COLOR_NM "=0x%04x%04x%04x]\n",
					   ntohs(*(unsigned short *)(&pkt[off])),ntohs(*(unsigned short *)(&pkt[off])),((uint32)pkt[off+2]<<8)|((uint32)pkt[off+3]),((uint32)pkt[off+4]<<8)|((uint32)pkt[off+5]),((uint32)pkt[off+6]<<8)|((uint32)pkt[off+7]));
        off+=8;
	}

    if((pkt[off]==0x88)&&((pkt[off+1]==0x63)||(pkt[off+1]==0x64))) //PPPoE
    {
        rtlglue_printf(KERN_CONT "PPPoE:[" COLOR_Y "Code" COLOR_NM "=0x%02x][" COLOR_Y "SessionID" COLOR_NM "=0x%04x] [" COLOR_Y "Len" COLOR_NM "=%d]\n",
                       pkt[off+3],((uint32)pkt[off+4]<<8)|pkt[off+5],((uint32)pkt[off+6]<<8)|pkt[off+7]);
        off+=8;
        pppoeif=1;
    }
DUAL_HEADER:
    if(((pkt[off]==0x86)&&(pkt[off+1]==0xdd)) || ((pkt[off]==0x00)&&(pkt[off+1]==0x57)) || (pkt[off]==0x57))		//IPv6 or IPv6 with PPPoE
    {
	if(pkt[off]==0x57)	off-=1;
        rtlglue_printf(KERN_CONT "IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=0x%02x(DSCP=%d,ECN=%d)][" COLOR_Y "FL" COLOR_NM "=%x%02x%02x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
                       ,pkt[off+2]>>4, (((pkt[off+2]&0xf)<<4) | (pkt[off+3]>>4)),(((pkt[off+2]&0xf)<<4) | ((pkt[off+3]>>4)&0xc))>>2,((pkt[off+3]>>4)&0x3), (pkt[off+3]&0xf), pkt[off+4], pkt[off+5], (pkt[off+6]<<8)+pkt[off+7], pkt[off+8], pkt[off+9]);
        rtlglue_printf(KERN_CONT "     [" COLOR_Y "SIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                       ,pkt[off+10], pkt[off+11], pkt[off+12], pkt[off+13], pkt[off+14], pkt[off+15], pkt[off+16], pkt[off+17]
                       ,pkt[off+18], pkt[off+19], pkt[off+20], pkt[off+21], pkt[off+22], pkt[off+23], pkt[off+24], pkt[off+25]);
        rtlglue_printf(KERN_CONT "     [" COLOR_Y "DIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
                       ,pkt[off+26], pkt[off+27], pkt[off+28], pkt[off+29], pkt[off+30], pkt[off+31], pkt[off+32], pkt[off+33]
                       ,pkt[off+34], pkt[off+35], pkt[off+36], pkt[off+37], pkt[off+38], pkt[off+39], pkt[off+40], pkt[off+41]);

        protocol=pkt[off+8];
        if(protocol==0 || protocol==60)	//hop-by-hop
        {
            rtlglue_printf(KERN_CONT "Hop-By-Hop:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Length" COLOR_NM "=%d]\n"
                           ,pkt[off+42], pkt[off+43]);
            rtlglue_printf(KERN_CONT "          [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x]\n"
                           ,pkt[off+44], pkt[off+45], pkt[off+46], pkt[off+47], pkt[off+48], pkt[off+49]);
            for(i=0; i<pkt[off+43]; i++)
            {
                rtlglue_printf(KERN_CONT "         [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x %02x %02x]\n"
                               ,pkt[off+50+i*8], pkt[off+51+i*8], pkt[off+52+i*8], pkt[off+53+i*8]
                               ,pkt[off+54+i*8], pkt[off+55+i*8], pkt[off+56+i*8], pkt[off+57+i*8]);
            }

            protocol=pkt[off+42];
            off+=(50+pkt[off+43]*8);
			if(protocol == 4){	// IPIP
				off-=2;
				goto DSLITE_INNER;
			}
        }
		else if(protocol == 44)	//fragment hdr
		{
			int tmp;
			tmp=(pkt[off+44]<<5)+((pkt[off+45]&0xf8)>>3);
			protocol=pkt[off+42];
			rtlglue_printf(KERN_CONT "Frag:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Rsv" COLOR_NM "=%d][" COLOR_Y "Offset" COLOR_NM "=%d][" COLOR_Y "MF" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=0x%02x%02x%02x%02x]\n",
				pkt[off+42],pkt[off+43],tmp,pkt[off+45]&0x1,pkt[off+46],pkt[off+47],pkt[off+48],pkt[off+49]);
			off+=50;
			if(protocol==4)
			{
				off-=2;
				goto DSLITE_INNER;
			}
		}
		else if(protocol == 4)	// IPIP
	        {
			off +=40;
			goto DSLITE_INNER;
	}
	else
		off+=42;
    }

    if(((pkt[off]==0x08)&&(pkt[off+1]==0x00))||((pkt[off]==0x00)&&(pkt[off+1]==0x21))||(pkt[off]==0x21))
    {
	if(pkt[off]==0x21)	off-=1;
 DSLITE_INNER:
        rtlglue_printf(KERN_CONT "IPv4:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "HLen" COLOR_NM "=%d][" COLOR_Y "TOS" COLOR_NM "=0x%02x(DSCP=%d,ECN=%d)][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=%d][" COLOR_Y "R" COLOR_NM "=%d," COLOR_Y "DF" COLOR_NM "=%d," COLOR_Y "MF" COLOR_NM "=%d]\n"
                       ,pkt[off+2]>>4,(pkt[off+2]&0xf)*4,pkt[off+3],pkt[off+3]>>2,pkt[off+3]&0x3,(pkt[off+4]<<8)|pkt[off+5],(pkt[off+6]<<8)|pkt[off+7]
                       ,(pkt[off+8]>>7)&1,(pkt[off+8]>>6)&1,(pkt[off+8]>>5)&1);
        rtlglue_printf(KERN_CONT "     [" COLOR_Y "FrgOff" COLOR_NM "=%d][" COLOR_Y "TTL" COLOR_NM "=%d][" COLOR_Y "PROTO" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n"
                       ,((pkt[off+8]&0x1f)<<8)|pkt[off+9],pkt[off+10],pkt[off+11],(pkt[off+12]<<8)|pkt[off+13]);
        rtlglue_printf(KERN_CONT "     [" COLOR_Y "SIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "DIP" COLOR_NM "=%d.%d.%d.%d]\n"
                       ,pkt[off+14],pkt[off+15],pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19],pkt[off+20],pkt[off+21]);

        protocol=pkt[off+11];
        off+=(pkt[off+2]&0xf)*4+2;
    }

	if((pkt[off]==0x08)&&(pkt[off+1]==0x06))
    {
	rtlglue_printf(KERN_CONT "ARP:[" COLOR_Y "SenderIP" COLOR_NM "=%d.%d.%d.%d][" COLOR_Y "TargetIP=" COLOR_NM "=%d.%d.%d.%d]\n"
                       ,pkt[off+16],pkt[off+17],pkt[off+18],pkt[off+19]
                        ,pkt[off+26],pkt[off+27],pkt[off+28],pkt[off+29]);
	off+=30;
    }

    if(protocol==0x6) //TCP
    {
        rtlglue_printf(KERN_CONT "TCP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x][" COLOR_Y "HLen" COLOR_NM "=%d]\n"
                       ,(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3]),(pkt[off+4]<<24)|(pkt[off+5]<<16)|(pkt[off+6]<<8)|(pkt[off+7]<<0)
                       ,(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|(pkt[off+11]<<0),pkt[off+12]>>4<<2);
        rtlglue_printf(KERN_CONT "    [" COLOR_Y "URG" COLOR_NM "=%d][" COLOR_Y "ACK" COLOR_NM "=%d][" COLOR_Y "PSH" COLOR_NM "=%d][" COLOR_Y "RST" COLOR_NM "=%d][" COLOR_Y "SYN" COLOR_NM "=%d][" COLOR_Y "FIN" COLOR_NM "=%d][" COLOR_Y "Win" COLOR_NM "=%d]\n"
                       ,(pkt[off+13]>>5)&1,(pkt[off+13]>>4)&1,(pkt[off+13]>>3)&1,(pkt[off+13]>>2)&1,(pkt[off+13]>>1)&1,(pkt[off+13]>>0)&1
                       ,(pkt[off+14]<<8)|pkt[off+15]);
        rtlglue_printf(KERN_CONT "    [" COLOR_Y "CHM" COLOR_NM "=0x%x][" COLOR_Y "Urg" COLOR_NM "=0x%x]\n",(pkt[off+16]<<8)|(pkt[off+17]<<0),(pkt[off+18]<<8)|(pkt[off+19]<<0));
		if((pkt[off+12]>>4<<2)>20){
			//parsing mss
			if(pkt[off+20]==0x2){
				rtlglue_printf(KERN_CONT "    [" COLOR_Y "Option(%d Byte):MSS" COLOR_NM "=0x%x]\n",pkt[off+21],(pkt[off+22]<<8)|(pkt[off+23]));
			}
		}
    }
    else if(protocol==0x11) //UDP
    {
    	uint16 sport = (pkt[off]<<8)|(pkt[off+1]);
		
        rtlglue_printf(KERN_CONT "UDP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n",(pkt[off]<<8)|(pkt[off+1]),(pkt[off+2]<<8)|(pkt[off+3])
                       ,(pkt[off+4]<<8)|(pkt[off+5]),(pkt[off+6]<<8)|(pkt[off+7]));

		if(pkt[off+8]==0x8 && pkt[off+9]==0x0 && pkt[off+10]==0x0 && pkt[off+11]==0x0) //vxlan
		{
			off += 8;
			rtlglue_printf(KERN_CONT "VXLAN:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "VNI" COLOR_NM "=0x%x]\n",
							pkt[off], (pkt[off+4]<<16)|(pkt[off+5]<<8)|(pkt[off+6]));			
			off += 8;
			goto VXLAN_INNER;
		}
		else if(sport==1701)
		{
			do{

				int tmpOff = 0;
				int l2tp_len = 0, l2tp_tunnelid = 0, l2tp_sessionid = 0, l2tp_offset = 0, l2tp_Ns=0, l2tp_Nr=0;
				//if(pkt[off]&0x80)break;			// skip control packet parsing
				//if((pkt[off+1]&0xf)!=0x2) break;	// only support version 2 now
				//if(pkt[off]&0x08) break;			// Sequence bit
				//if(pkt[off]&0x02) break;			// Offset bit
				//if(pkt[off]&0x01) break;			// Priority bit
				off += 8;
				rtlglue_printf(KERN_CONT "L2TP:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d]", (pkt[off]<<8)|(pkt[off+1]), (pkt[off+1]&0xf));
				tmpOff = 2;						// flag+version
				if(pkt[off]&0x40){				// Length bit
					l2tp_len = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					rtlglue_printf(KERN_CONT "[" COLOR_Y "Len" COLOR_NM "=%d]", l2tp_len);
				}
				l2tp_tunnelid = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
				l2tp_sessionid = pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
				tmpOff+=4;					// Tunnel ID + Session ID
				rtlglue_printf(KERN_CONT "[" COLOR_Y"TunnelID" COLOR_NM "=%d][" COLOR_Y"SessionID" COLOR_NM "=%d]", l2tp_tunnelid, l2tp_sessionid);
				if(pkt[off]&0x08){				// Length bit
					l2tp_Ns = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					l2tp_Nr = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;
					rtlglue_printf(KERN_CONT "[" COLOR_Y"Ns" COLOR_NM "=%d][" COLOR_Y"Nr" COLOR_NM "=%d]", l2tp_Ns, l2tp_Nr);
				}
				if(pkt[off]&0x02){
					l2tp_offset = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;				// offset size
					tmpOff+=l2tp_offset;	// offset padding
					rtlglue_printf(KERN_CONT "[" COLOR_Y "Offset" COLOR_NM "=%d]", l2tp_offset);
				}
				rtlglue_printf(KERN_CONT "\n");

				off+= (tmpOff);		// tmpOff: L2TP
				if((pkt[off] == 0xff) && pkt[off+1] == 0x03)	off+=2;	// 2: PPP(address, control)
				goto DUAL_HEADER;
			}while(0);
		}

    }
    else if(protocol==0x2f) //GRE(47)
    {
	uint32 optionOffset = 0;
	protocol=(pkt[off+2]<<8) | pkt[off+3] ;
        rtlglue_printf(KERN_CONT "GRE:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Protocol" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "PLen" COLOR_NM "=%d][" COLOR_Y"CallID" COLOR_NM "=0x%x]\n",
			(pkt[off]<<8)|(pkt[off+1]&0xfff8), (pkt[off+2]<<8) | pkt[off+3], (pkt[off+1]&0x7), (pkt[off+4]<<8) | pkt[off+5], (pkt[off+6]<<8) | pkt[off+7]);
	if(pkt[off] & GRE_S_BIT)	// seq bit: GRE_S_BIT	0x10
	{
		rtlglue_printf(KERN_CONT "    [" COLOR_Y "Seq" COLOR_NM "=0x%x]", (pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|pkt[off+11]);
		optionOffset +=4;
	}
	if(pkt[off+1] & 0x80)		// ack bit
	{
		rtlglue_printf(KERN_CONT "    [" COLOR_Y "Ack" COLOR_NM "=0x%x]", (pkt[off+optionOffset+8]<<24)|(pkt[off+optionOffset+9]<<16)|(pkt[off+optionOffset+10]<<8)|pkt[off+optionOffset+11]);
		optionOffset +=4;
	}
	// rtlglue_printf("    [" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x]\n",
	//		(pkt[off+8]<<24)|(pkt[off+9]<<16)|(pkt[off+10]<<8)|pkt[off+11], (pkt[off+12]<<24)|(pkt[off+13]<<16)|(pkt[off+14]<<8)|pkt[off+15]);
	if(optionOffset!=0)	rtlglue_printf(KERN_CONT "\n");

	if((pkt[off+8+optionOffset] == 0xff) && pkt[off+8+optionOffset+1] == 0x03)	optionOffset+=2;	// 2: PPP(address, control)
	off+= (8+optionOffset);		// 8 GRE basic header; optionOffset 0~10: 8: seq and ac, 2: PPP(address, control)
	goto DUAL_HEADER;
    }
	else if(protocol==0x73)	//l2tpv3(115)
	{
		do{
			uint32 l2tp_sessionid = 0, l2tp_len = 0, l2tp_ctrl_connectionId = 0, l2tp_Ns=0, l2tp_Nr=0;
			int tmpOff = 0;
			off += 8;
			l2tp_sessionid = pkt[off+tmpOff]<<24 | pkt[off+tmpOff+1]<<16 | pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
			tmpOff += 4;
			if(l2tp_sessionid)
			{
				rtlglue_printf(KERN_CONT "L2TPv3 data:[" COLOR_Y "SessionID" COLOR_NM "=%d]", l2tp_sessionid);
				tmpOff += 8;
			}
			else
			{
				rtlglue_printf(KERN_CONT "L2TPv3 ctrl:[" COLOR_Y "flags" COLOR_NM "=0x%x][" COLOR_Y "Ver" COLOR_NM "=%d]", (pkt[off+4]<<8)|(pkt[off+5]), (pkt[off+5]&0xf));
				tmpOff += 2;
				if(pkt[off+4]&0x40){				// Length bit
					l2tp_len = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					rtlglue_printf(KERN_CONT "[" COLOR_Y "Len" COLOR_NM "=%d]", l2tp_len);
				}
				l2tp_ctrl_connectionId = pkt[off+tmpOff]<<24 | pkt[off+tmpOff+1]<<16 | pkt[off+tmpOff+2]<<8 | pkt[off+tmpOff+3];
				rtlglue_printf(KERN_CONT "[" COLOR_Y "ctrl connection id" COLOR_NM "=%d]", l2tp_ctrl_connectionId);
				if(pkt[off+4]&0x08){				// seq bit
					l2tp_Ns = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;	
					l2tp_Nr = pkt[off+tmpOff]<<8 | pkt[off+tmpOff+1];
					tmpOff+=2;
					rtlglue_printf(KERN_CONT "[" COLOR_Y"Ns" COLOR_NM "=%d][" COLOR_Y"Nr" COLOR_NM "=%d]", l2tp_Ns, l2tp_Nr);
				}
			}
			off += (tmpOff);		// tmpOff: L2TP
		}while(0);
	}
    else if(protocol==0x29) //6RD(41)
    {
		rtlglue_printf(KERN_CONT "IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=%02x][" COLOR_Y "FL" COLOR_NM "=%x%02x%02x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
					   ,pkt[off]>>4, (((pkt[off]&0xf)<<4) | (pkt[off+1]>>4)), (pkt[off+1]&0xf), pkt[off+2], pkt[off+3], (pkt[off+4]<<8)+pkt[off+5], pkt[off+6], pkt[off+7]);
		rtlglue_printf(KERN_CONT "	 [" COLOR_Y "SIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
					   ,pkt[off+8], pkt[off+9], pkt[off+10], pkt[off+11], pkt[off+12], pkt[off+13], pkt[off+14], pkt[off+15]
					   ,pkt[off+16], pkt[off+17], pkt[off+18], pkt[off+19], pkt[off+20], pkt[off+21], pkt[off+22], pkt[off+23]);
		rtlglue_printf(KERN_CONT "	 [" COLOR_Y "DIP" COLOR_NM "=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n"
					   ,pkt[off+24], pkt[off+25], pkt[off+26], pkt[off+27], pkt[off+28], pkt[off+29], pkt[off+30], pkt[off+31]
					   ,pkt[off+32], pkt[off+33], pkt[off+34], pkt[off+35], pkt[off+36], pkt[off+37], pkt[off+38], pkt[off+39]);
	
		protocol=pkt[off+6];
		if(protocol==0) //hop-by-hop
		{
			rtlglue_printf(KERN_CONT "Hop-By-Hop:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Length" COLOR_NM "=%d]\n"
						   ,pkt[off+40], pkt[off+41]);
			rtlglue_printf(KERN_CONT "		  [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x]\n"
						   ,pkt[off+42], pkt[off+43], pkt[off+44], pkt[off+45], pkt[off+46], pkt[off+47]);
			for(i=0; i<pkt[off+41]; i++)
			{
				rtlglue_printf(KERN_CONT "		 [" COLOR_Y "Option" COLOR_NM "=%02x %02x %02x %02x %02x %02x %02x %02x]\n"
							   ,pkt[off+48+i*8], pkt[off+49+i*8], pkt[off+50+i*8], pkt[off+51+i*8]
							   ,pkt[off+52+i*8], pkt[off+53+i*8], pkt[off+54+i*8], pkt[off+55+i*8]);
			}

			#if 0
			protocol=pkt[off+40];
			off+=(50+pkt[off+41]*8);
			if(protocol == 4){	// IPIP
				off-=2;
				goto DSLITE_INNER;
			}
			#endif
		}
		else if(protocol == 44) //fragment hdr
		{
			int tmp;
			tmp=(pkt[off+42]<<5)+((pkt[off+43]&0xf8)>>3);
			protocol=pkt[off+40];
			rtlglue_printf(KERN_CONT "Frag:[" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "Rsv" COLOR_NM "=%d][" COLOR_Y "Offset" COLOR_NM "=%d][" COLOR_Y "MF" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=0x%02x%02x%02x%02x]\n",
				pkt[off+40],pkt[off+41],tmp,pkt[off+43]&0x1,pkt[off+44],pkt[off+45],pkt[off+46],pkt[off+47]);
			off+=50;
		}
    }
}

