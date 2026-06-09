#include <stdio.h>
//! [include]
#include <drivers/rtc.h>
//! [include]

RTC_Handle gRTCHandle;

void open(void)
{
//! [open]
    RTC_Params         params;

    /* Initialize parameters */
    RTC_Params_init(&params);

    params.isfreezemode = true,
    params.onOffCallback = false,
    params.offOnCallback = false,

    /* Update object Parameters mode settings */
    gRTCObjects[CONFIG_RTC0].On_OffCallbackFunction = NULL,
    gRTCObjects[CONFIG_RTC0].Off_OnCallbackFunction = NULL,

    gRTCHandle = RTC_open(CONFIG_RTC0, &params);
    DebugP_assert(gRTCHandle != NULL);
//! [open]
}

void close(void)
{
//! [close]
    RTC_close(gRTCHandle);
//! [close]
}
