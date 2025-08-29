#include "soc/soc_caps.h"

#if SOC_PPA_SUPPORTED
#include "driver/ppa.h"
#include "hal/ppa_ll.h"
#include "../src/ppa_priv.h"

void ppa_set_clut(ppa_client_handle_t ppa_client, const uint8_t *colours, int num_cols)
{
    ppa_soc_handle_t dev = ppa_client->engine->platform->hal.dev;
    ppa_ll_configure_clut_access_mode(dev, true);
    ppa_ll_blend_reset_rx_bg_clut_mem(dev);
    ppa_ll_blend_reset_rx_fg_clut_mem(dev);

    for(int i = 0; i < num_cols; i++)
    {
        int r = colours[i * 3 + 0] << 2;
        int g = colours[i * 3 + 1] << 2;
        int b = colours[i * 3 + 2] << 2;
        uint32_t col = r | g << 8 | b << 16 | 0xFF << 24;

        ppa_ll_blend_wr_rx_bg_clut_data_by_fifo(dev, col);
        ppa_ll_blend_wr_rx_fg_clut_data_by_fifo(dev, i == 0 ? 0 : col);
    }

    ppa_ll_configure_clut_access_mode(dev, false);
}

#endif