#include "app.h"

int app_core_init(void)
{
    //for hold the SWD before sleep
    //delay_n_10us(300*1000);
    NS_LOG_INIT();

    #if  (CFG_APP_NS_IUS)
    if(CURRENT_APP_START_ADDRESS == NS_APP1_START_ADDRESS){
        NS_LOG_INFO("application 1 start new ...\r\n");
    }else if(CURRENT_APP_START_ADDRESS == NS_APP2_START_ADDRESS){
        NS_LOG_INFO("application 2 start new ...\r\n");
    }
    #endif

    app_ble_init();
    NS_LOG_INFO("board init ...\r\n");

	return true;
}








