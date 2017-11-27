 extern "C" {
//#include "c_types.h"
//#include "ets_sys.h"
//#include "os_type.h"
//#include "osapi.h"
//#include "spi_flash.h"
#include "sntp.h"
}

//============ SNTP TIME ===========

void  SNTP_Init(int time_zone)
{
  sntp_setservername(0, "us.pool.ntp.org"); // set server 0 by domain name
  sntp_setservername(1, "nl.pool.ntp.org"); // set server 1 by domain name
  sntp_setservername(2, "0.nl.pool.ntp.org"); // set server 2 by domain name
  
  sntp_stop();                           
  if( true == sntp_set_timezone(time_zone) ) 
  {
     sntp_init();
  }
  
  sntp_init();
  return;
}
