/*
* Copyright 2025 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly
* in accordance with the applicable license terms.  By expressly accepting such terms or by downloading,
* installing, activating and/or otherwise using the software, you are agreeing that you have read,
* and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by
* the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#include <string.h>
#include <stdio.h>
#include "pin_mux.h"
#include "board.h"

#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/opt.h"
#include "lwip/netifapi.h"
#include "netif/ethernet.h"
#include "ethernetif.h"
#include "fsl_phy.h"

#include "freemaster.h"
#include "network.h"

#ifndef EXAMPLE_MAC_ADDR
#include "fsl_silicon_id.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////


#define EXAMPLE_IP_ADDR  LWIP_MAKEU32(192, 168, 1, 20)  /* IP address configuration. */
#define EXAMPLE_NET_MASK LWIP_MAKEU32(255, 255, 255, 0) /* Netmask configuration. */
#define EXAMPLE_GW_ADDR  LWIP_MAKEU32(192, 168, 1, 1)   /* Gateway address configuration. */


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static struct netif netif;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

static void print_ipaddr(struct netif *itf, unsigned short port)
{
    PRINTF("IPv4 Address     : %s\n", ipaddr_ntoa(&itf->ip_addr));
    PRINTF("IPv4 Subnet mask : %s\n", ipaddr_ntoa(&itf->netmask));
    PRINTF("IPv4 Gateway     : %s\n\n", ipaddr_ntoa(&itf->gw));

    if (port != 0U)
    {
        PRINTF(
            "Run the FreeMASTER PC Host application, select the network communication\n"
            "plug-in and connect to this board's IP Address specified above at port %d.\n\n",
            port);
    }
}



void Network_Init(uint32_t csrClock_Hz)
{
    static EXAMPLE_PHY_RES phyResource;

    static phy_handle_t phyHandle;
    static phy_operations_t phy_ops;
    ip4_addr_t ipaddr, netmask, gw;
    err_t err;

    ethernetif_config_t enet_config = {
        .phyHandle   = &phyHandle,
        .phyResource = &phyResource,
        .phyAddr     = EXAMPLE_PHY_ADDRESS,
        .phyOps      = &phy_ops,
        .srcClockHz  = csrClock_Hz,
    };

    /* Generate and set MAC address. */
    if (SILICONID_ConvertToMacAddr(&enet_config.macAddress) != kStatus_Success)
    {
        /* Fallback MAC addres */
        enet_config.macAddress[0] = 0;
        enet_config.macAddress[1] = 0;
        enet_config.macAddress[2] = 0xfa;
        enet_config.macAddress[3] = 0xfa;
        enet_config.macAddress[4] = 0xdd;
        enet_config.macAddress[5] = 0xdd;
    }

    /* Prepare static IP address */
    ipaddr.addr  = (u32_t)PP_HTONL(EXAMPLE_IP_ADDR);
    netmask.addr = (u32_t)PP_HTONL(EXAMPLE_NET_MASK);
    gw.addr      = (u32_t)PP_HTONL(EXAMPLE_GW_ADDR);

    /* Initialize PHY */
    Network_PhyInit(&phyResource, &phy_ops, csrClock_Hz);

    /* Initialize LWIP stack */
    tcpip_init(NULL, NULL);

    /* Register ETH as network interface */
    err = netifapi_netif_add(&netif, &ipaddr, &netmask, &gw, &enet_config, EXAMPLE_ETHIF_INIT, tcpip_input);

    if (err == ERR_OK)
    {
        err = netifapi_netif_set_default(&netif);

        if (err == ERR_OK)
        {
            err = netifapi_netif_set_up(&netif);

            if (err != ERR_OK)
            {
                PRINTF("ERROR: netifapi_netif_set_up returned %d\n", (int)err);
            }
        }
        else
        {
            PRINTF("ERROR: netifapi_netif_set_default returned %d\n", (int)err);
        }
    }
    else
    {
        PRINTF("ERROR: netifapi_netif_add returned %d\n", (int)err);
    }

    if (err == ERR_OK)
    {
        /* Print static IP address */
        PRINTF("\n======= Static IP address =======\n");
        print_ipaddr(&netif, FMSTR_NET_PORT);

    }
}


