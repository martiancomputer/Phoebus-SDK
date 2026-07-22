#include <rtl_types.h>
#include <rtl_glue.h>
#include <stdarg.h>

#ifdef __KERNEL__
#include <linux/slab.h>
#endif

rtk_test_debug_level_t test_debug_level = 0xffffffff;
char printf_buf[512];


void *rtlglue_malloc(uint32 NBYTES){return (void *)kmalloc(NBYTES,GFP_ATOMIC);}
void *rtlglue_zalloc(uint32 NBYTES){return (void *)kmalloc(NBYTES,GFP_ATOMIC|__GFP_ZERO);}
void rtlglue_freeAndNull(void **APTR)
{
	if(*APTR!=NULL)
	{
		kfree(*APTR);
		*APTR=NULL;
	}
}



void common_dump_fctest(rtk_test_debug_level_t level, char *string, int color, int bgcolor,const char *funcs,int line, char *comment,...)
{
	int mt_trace_i;
	va_list a_list;
	va_start(a_list,comment);

	vsprintf( printf_buf, comment, a_list);
	for(mt_trace_i=1;mt_trace_i<512;mt_trace_i++)
	{
		if(printf_buf[mt_trace_i]==0)
		{
			if(printf_buf[mt_trace_i-1]=='\n') printf_buf[mt_trace_i-1]=' ';
			else break;
		}
	}
	rtlglue_printf("\033[1;%d;%dm[%s] %s\033[0;37;40m @ %s(%d)\033[0m\n",color,bgcolor,string,printf_buf,funcs,line);
	
	va_end (a_list);  
}


