#include "Mid_BLE.h"
#include "app_rdtss.h"

void Mid_BLE_SendNotify(const uint8_t *p_data, uint16_t length)
{
    rdtss_send_notify((uint8_t *)p_data, length);
}
