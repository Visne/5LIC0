#include "custom-schedule.h"
#include "tsch.h"

void initialize_tsch_schedule(void)
{
    struct tsch_slotframe *sf = tsch_schedule_add_slotframe(0, 1);

    tsch_schedule_add_link(sf,
                           LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED,
                           LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
                           0, 0, 1);
}
