

#include "types.h"
#include "rtl_types.h"
#include "rtl_glue.h"
#include "rtl_protocol.h"
#include "l3pkt.h"
#include "utility.h"
#include <rtl_protocol.h>


uint16 modelCksum(uint16 *ptr, uint32 len, uint16 resid) 
{
	uint32 csum = resid;
	uint32 odd = 0;
 	if(len & 1) odd = 1;
	len = len >> 1;
 	for(;len > 0 ; len--,ptr++) 
		csum += ntohs(*ptr);
 
	if(odd)
		csum += (*((uint8 *)ptr) <<8) & 0xff00;
 
	/* take care of 1's complement */
	while(csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);
 
	//if(csum == 0xffff)	csum = 0;		//FIXME:luke
 
	return((uint16)csum);
}

/*********************************************************************************
* Routine Name :  model_ipChecksum                                   
* Description : 
* Input :                                                               
* Output :                                                             
* Return :                                                             
* Note :   
*        ThreadSafe: n
**********************************************************************************/
uint16 model_ipChecksum(struct ip *pip)
{
	uint16 *ptr = (uint16 *)pip;
	uint32 len = ((pip->ip_vhl & 0xf) << 2);	
	uint32 csum = 0;
	uint32 odd = 0;
	uint16 orgcsum=0;

	orgcsum=pip->ip_sum;
 	pip->ip_sum = 0;

	if(len & 1) odd = 1;
	len = len >> 1;
	for(;len > 0 ; len--,ptr++) 
		csum += ntohs(*ptr);

	if(odd)
		csum += (*((uint8 *)ptr) <<8) & 0xff00;

	/* take care of 1's complement */
	while(csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);

	//if(csum == 0xffff)	csum = 0;		//FIXME:luke
 	pip->ip_sum=orgcsum;
	return((uint16)htons(~csum));
}
/*********************************************************************************
* Routine Name :  model_tcpChecksum
* Description : 
* Input :                                                               
* Output :                                                             
* Return :                                                             
* Note :   
*        ThreadSafe: n
**********************************************************************************/
uint16 model_tcpChecksum(struct ip *pip, tcpHdr_t *ptcp)
{
	uint32 len = ntohs(pip->ip_len) - ((pip->ip_vhl & 0xf) << 2);
 	uint32 csum = 0;
	uint16 tmp = 0;
	uint16 orgcsum=0;
	ASSERT(pip!=NULL && ptcp!=NULL);
	
 	csum = ipcsum((uint16 *)&pip->ip_src, 8, 0);
 	tmp = htons(IP_PROTO_TCP);
 	csum = modelCksum((uint16 *)&tmp, 2, csum);
 	tmp = htons(len);
 	csum = modelCksum((uint16 *)&tmp, 2, csum);
 	orgcsum = ptcp->th_sum;
 	ptcp->th_sum = 0;
 	csum = modelCksum((uint16 *)ptcp, len, csum);
 	ptcp->th_sum=orgcsum;
	return htons(~csum);
}

uint16 model_udpChecksum(struct ip *pip, udpHdr_t *pudp)
{
	uint32 len = ntohs(pip->ip_len) - ((pip->ip_vhl & 0xf) << 2);	
	uint32 csum = 0;
	uint16 tmp = 0;
	uint16 orgcsum=0;

	csum = modelCksum((uint16 *)&pip->ip_src, 8, 0);
	tmp = htons(IP_PROTO_UDP);
	csum = modelCksum((uint16 *)&tmp, 2, csum);
	tmp = htons(len);
	csum = modelCksum((uint16 *)&tmp, 2, csum);
	orgcsum = pudp->uh_sum;
	pudp->uh_sum = 0;
	csum = modelCksum((uint16 *)pudp, len, csum);

	if(csum == 0xffff)	csum = 0;		//FIXME:luke

	pudp->uh_sum = orgcsum;

	return htons(~csum);
}

/*********************************************************************************
* Routine Name :  model_icmpChecksum
* Description : 
* Input :                                                               
* Output :                                                             
* Return :                                                             
* Note :   
*        ThreadSafe: n
**********************************************************************************/
/* shoud continue memory */
uint16 model_icmpChecksum(struct ip * pip)
{
	uint16   *ptr;
	int32  nhdr, nbytes;
	int32  sum, i;

	nhdr =  ((pip->ip_vhl &0xf) << 2);
	nbytes = ntohs(pip->ip_len) - nhdr;
	ptr = (uint16 *) ((char *) pip + nhdr);
	sum = 0;

	/* Note: we always padding here!! */
	*(((uint8 *)pip) + nhdr + nbytes) = (uint8)0;
	for (i=0; i<nbytes ; i=i+2){
		sum +=  (unsigned long) *ptr++;
	}

	/* Roll over carry bits */
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	/* Return checksum */
	return ((uint16) ~ sum);
}



uint16 model_tcpChecksumV6(struct ipv6hdr_s *pipv6, tcpHdr_t *ptcp, uint32 v6ExtLen)
{
	uint32 len = ntohs(pipv6->payload_len) - v6ExtLen;
 	uint32 csum = 0;
	uint16 tmp = 0;
	uint16 orgcsum=0;
	ASSERT(pipv6!=NULL && ptcp!=NULL);
 	csum = ipcsum((uint16 *)&pipv6->saddr, 32, 0);
 	tmp = htons(IP_PROTO_TCP);
 	csum = modelCksum((uint16 *)&tmp, 2, csum);
 	tmp = htons(len);
 	csum = modelCksum((uint16 *)&tmp, 2, csum);
 	orgcsum = ptcp->th_sum;
 	ptcp->th_sum = 0;
 	csum = modelCksum((uint16 *)ptcp, len, csum);
 	ptcp->th_sum=orgcsum;
	return htons(~csum);
}

uint16 model_udpChecksumV6(struct ipv6hdr_s *pipv6, udpHdr_t *pudp)
{
	uint32 len = ntohs(pipv6->payload_len);	
	uint32 csum = 0;
	uint16 tmp = 0;
	uint16 orgcsum=0;

	csum = modelCksum((uint16 *)&pipv6->saddr, 32, 0);
	tmp = htons(IP_PROTO_UDP);
	csum = modelCksum((uint16 *)&tmp, 2, csum);
	tmp = htons(len);
	csum = modelCksum((uint16 *)&tmp, 2, csum);
	orgcsum = pudp->uh_sum;
	pudp->uh_sum = 0;
	csum = modelCksum((uint16 *)pudp, len, csum);

	if(csum == 0xffff)	csum = 0;		//FIXME:luke

	pudp->uh_sum = orgcsum;

	return htons(~csum);
}

