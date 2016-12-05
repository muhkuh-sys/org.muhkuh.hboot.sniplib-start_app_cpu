#ifndef __SHA384_H__
#define __SHA384_H__


void sha384_initialize(void);
#define sha384_update_ul(ulData) {ptHashArea->ulHash_din = ulData;}
void sha384_finalize(unsigned long *pulHash, unsigned int sizHash, unsigned long ulDataSizeDw);


#endif  /* __SHA384_H__ */

