/**
  ******************************************************************************
  * @file           : wizchip_port.c
  * @brief          : W5500 board integration and network initialization.
  * @project        : STM32F401 Test Platform
  * @platform       : STMicroelectronics STM32F401RCT6
  * @created        : 06.05.2026 07:03:36 PM
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2026 Dmitry Slobodchikov
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "main.h"
#include "wizchip_conf.h"
#include "stdio.h"
#include "string.h"
#include "socket.h"
#include "stdbool.h"
#include "DHCP/dhcp.h"
#include "DNS/dns.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "wizchip_port.h"

#define W5500_SPI hspi3
#define W5500_USE_DHCP  1
#define DHCP_SOCKET 7U
#define DNS_SOCKET  6U
#define LINK_RETRY_COUNT 10U
#define DHCP_RETRY_COUNT 20U
#define NETWORK_RETRY_DELAY_MS 500U

static wiz_NetInfo networkInfo = {
  .mac = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
  .ip = {192, 168, 1, 10},
  .sn = {255, 255, 255, 0},
  .gw = {192, 168, 1, 1},
  .dns = {8, 8, 8, 8},
#if W5500_USE_DHCP
  .dhcp = NETINFO_DHCP
#else
  .dhcp = NETINFO_STATIC
#endif
};
static volatile bool networkReady;
static StaticSemaphore_t dnsMutexBuffer;
static SemaphoreHandle_t dnsMutex;

#define W5500_CS_LOW()     HAL_GPIO_WritePin(ETH_CS_GPIO_Port, ETH_CS_Pin, GPIO_PIN_RESET)
#define W5500_CS_HIGH()    HAL_GPIO_WritePin(ETH_CS_GPIO_Port, ETH_CS_Pin, GPIO_PIN_SET)
#define W5500_RST_LOW()    HAL_GPIO_WritePin(ETH_RESET_GPIO_Port, ETH_RESET_Pin, GPIO_PIN_RESET)
#define W5500_RST_HIGH()   HAL_GPIO_WritePin(ETH_RESET_GPIO_Port, ETH_RESET_Pin, GPIO_PIN_SET)

extern SPI_HandleTypeDef W5500_SPI;

/** @brief Assert the active-low W5500 chip select. */
static void wizchipPort_Select(void)   { W5500_CS_LOW(); }

/** @brief Release the active-low W5500 chip select. */
static void wizchipPort_Unselect(void) { W5500_CS_HIGH(); }

/** @brief Exchange one SPI byte and return the received value. */
static uint8_t wizchipPort_ReadByte(void)
{
    uint8_t receivedByte;
    uint8_t transmittedByte = 0xFF;
    HAL_SPI_TransmitReceive(&W5500_SPI, &transmittedByte, &receivedByte, 1, HAL_MAX_DELAY);
    return receivedByte;
}

/** @brief Transmit one SPI byte to the W5500. */
static void wizchipPort_WriteByte(uint8_t byte)
{
    HAL_SPI_Transmit(&W5500_SPI, &byte, 1, HAL_MAX_DELAY);
}

#if W5500_USE_DHCP
static volatile bool ipAssigned = false;
static uint8_t dhcpBuffer[548];

/** @brief Record successful DHCP address assignment. */
static void wizchipPort_IpAssigned(void) {
    ipAssigned = true;
}

/** @brief Record a DHCP address conflict. */
static void wizchipPort_IpConflict(void) {
    ipAssigned = false;
}
#endif

static uint8_t dnsBuffer[MAX_DNS_BUF_SIZE];

W5500_StatusTypeDef W5500_Init(void)
{
    uint8_t socketMemorySize[2][8] = {
      {2, 2, 2, 2, 2, 2, 2, 2},
      {2, 2, 2, 2, 2, 2, 2, 2}
    };

    networkReady = false;
    dnsMutex = xSemaphoreCreateMutexStatic(&dnsMutexBuffer);
    if (dnsMutex == NULL)
        return W5500_STATUS_INIT_FAILED;
    W5500_RST_LOW();
    HAL_Delay(50);
    W5500_RST_HIGH();
    HAL_Delay(200);

    reg_wizchip_cs_cbfunc(wizchipPort_Select, wizchipPort_Unselect);
    reg_wizchip_spi_cbfunc(wizchipPort_ReadByte, wizchipPort_WriteByte);

    if (ctlwizchip(CW_INIT_WIZCHIP, (void*)socketMemorySize) == -1) {
        printf("Error while initializing WIZCHIP\r\n");
        return W5500_STATUS_INIT_FAILED;
    }
    printf("WIZCHIP Initialized\r\n");

    uint8_t version = getVERSIONR();
    if (version != 0x04) {
        printf("Error Communicating with W5500\t Version: 0x%02X\r\n", version);
        return W5500_STATUS_COMMUNICATION_FAILED;
    }
    printf("Checking Link Status..\r\n");

    uint8_t link = PHY_LINK_OFF;
    uint8_t retries = LINK_RETRY_COUNT;
    while ((link != PHY_LINK_ON) && (retries > 0)) {
        ctlwizchip(CW_GET_PHYLINK, &link);
        if (link == PHY_LINK_ON) printf("Link: UP\r\n");
        else printf("Link: DOWN Retrying : %d\r\n", LINK_RETRY_COUNT - retries);
        retries--;
        HAL_Delay(NETWORK_RETRY_DELAY_MS);
    }
    if (link != PHY_LINK_ON) {
        printf("Link is down; reconnect and retry.\r\n");
        return W5500_STATUS_LINK_DOWN;
    }

#if W5500_USE_DHCP
    printf("Using DHCP; please wait.\r\n");
    setSHAR(networkInfo.mac);
    DHCP_init(DHCP_SOCKET, dhcpBuffer);

    reg_dhcp_cbfunc(wizchipPort_IpAssigned, wizchipPort_IpAssigned, wizchipPort_IpConflict);

    retries = DHCP_RETRY_COUNT;
    while ((!ipAssigned) && (retries > 0)) {
        DHCP_run();
        HAL_Delay(NETWORK_RETRY_DELAY_MS);
        retries--;
    }
    if (!ipAssigned) {
        printf("DHCP failed; using the static configuration.\r\n");
        ctlnetwork(CN_SET_NETINFO, (void*)&networkInfo);
    } else {
        getIPfromDHCP(networkInfo.ip);
        getGWfromDHCP(networkInfo.gw);
        getSNfromDHCP(networkInfo.sn);
        getDNSfromDHCP(networkInfo.dns);

        ctlnetwork(CN_SET_NETINFO, (void*)&networkInfo);
        printf("DHCP IP assigned successfully\r\n");
    }

#else
    printf("Using the static network configuration.\r\n");
    ctlnetwork(CN_SET_NETINFO, (void*)&networkInfo);
#endif

    HAL_Delay(NETWORK_RETRY_DELAY_MS);
    printf("Configuring DNS..\r\n");
    DNS_init(DNS_SOCKET, dnsBuffer);

    wiz_NetInfo currentNetworkInfo;
    ctlnetwork(CN_GET_NETINFO, &currentNetworkInfo);
    printf("IP: %d.%d.%d.%d\r\n", currentNetworkInfo.ip[0], currentNetworkInfo.ip[1], currentNetworkInfo.ip[2], currentNetworkInfo.ip[3]);
    printf("SUBNET: %d.%d.%d.%d\r\n", currentNetworkInfo.sn[0], currentNetworkInfo.sn[1], currentNetworkInfo.sn[2], currentNetworkInfo.sn[3]);
    printf("GATEWAY: %d.%d.%d.%d\r\n", currentNetworkInfo.gw[0], currentNetworkInfo.gw[1], currentNetworkInfo.gw[2], currentNetworkInfo.gw[3]);
    printf("DNS: %d.%d.%d.%d\r\n", currentNetworkInfo.dns[0], currentNetworkInfo.dns[1], currentNetworkInfo.dns[2], currentNetworkInfo.dns[3]);

    networkReady = true;
    return W5500_STATUS_OK;
}

bool W5500_IsReady(void) {
    return networkReady;
}

bool W5500_GetDnsServer(uint8_t dnsServer[4]) {
    if (!networkReady || (dnsServer == NULL))
        return false;

    memcpy(dnsServer, networkInfo.dns, sizeof(networkInfo.dns));
    return true;
}

bool W5500_ResolveHost(const char* host, uint8_t address[4]) {
    uint8_t dnsServer[4];
    bool resolved = false;

    if ((host == NULL) || (address == NULL) || (dnsMutex == NULL)
        || !W5500_GetDnsServer(dnsServer)) {
        return false;
    }

    if (xSemaphoreTake(dnsMutex, portMAX_DELAY) == pdTRUE) {
        DNS_init(DNS_SOCKET, dnsBuffer);
        resolved = DNS_run(
          dnsServer,
          (uint8_t*)host,
          address
        ) == 1;
        (void)xSemaphoreGive(dnsMutex);
    }
    return resolved;
}
