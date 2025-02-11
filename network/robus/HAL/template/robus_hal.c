/******************************************************************************
 * @file robus_HAL
 * @brief Robus Hardware Abstration Layer. Describe Low layer fonction
 * @MCU Family XXX
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "robus_hal.h"
#include "luos_hal.h"

#include <stdbool.h>
#include <string.h>
#include "reception.h"
#include "context.h"

// MCU dependencies this HAL is for family XXX you can find

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEFAULT_TIMEOUT 20
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t Timer_Prescaler = (MCUFREQ / DEFAULTBAUDRATE) / TIMERDIV; //(freq MCU/freq timer)/divider timer clock source

typedef struct
{
    uint16_t Pin;
    GPIO_TypeDef *Port;
    uint8_t IRQ;
} Port_t;

Port_t PTP[NBR_PORT];

volatile uint16_t data_size_to_transmit = 0;
volatile uint8_t *tx_data               = 0;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void RobusHAL_CRCInit(void);
static void RobusHAL_TimeoutInit(void);
static void RobusHAL_GPIOInit(void);
static void RobusHAL_RegisterPTP(void);

/////////////////////////Luos Library Needed function///////////////////////////

/******************************************************************************
 * @brief Luos HAL general initialisation
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_Init(void)
{
    // IO Initialization
    RobusHAL_GPIOInit();

    // CRC Initialization
    RobusHAL_CRCInit();

    // Com Initialization
    RobusHAL_ComInit(DEFAULTBAUDRATE);
}
/******************************************************************************
 * @brief Luos HAL Initialize Generale communication inter node
 * @param Select a baudrate for the Com
 * @return none
 ******************************************************************************/
void RobusHAL_ComInit(uint32_t Baudrate)
{
    LUOS_COM_CLOCK_ENABLE();

    // Initialise USART1

    // Enable Reception interrupt

    // Enable NVIC IRQ

    // enable DMA
#ifndef USE_TX_IT
    // if DMA possible initialize DMA for a data transmission
#endif
    // Timeout Initialization
    Timer_Prescaler = (MCUFREQ / Baudrate) / TIMERDIV;
    RobusHAL_TimeoutInit();
}
/******************************************************************************
 * @brief Tx enable/disable relative to com
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_SetTxState(uint8_t Enable)
{
    if (Enable == true)
    {
        // put Tx COM pin in push pull
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            // Tx enable
        }
    }
    else
    {
        // put Tx COM pin in Open drain
        if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
        {
            // Tx Disable
        }
#ifdef USE_TX_IT
        // Stop current transmit operation
        data_size_to_transmit = 0;
        // Disable IT tx empty
#else

        // stop DMA transmission DMA disable
#endif
        // disable tx complet IT
    }
}
/******************************************************************************
 * @brief Rx enable/disable relative to com
 * @param
 * @return
 ******************************************************************************/
void RobusHAL_SetRxState(uint8_t Enable)
{
    if (Enable == true)
    {
        // clear data register
        //  Enable Rx com
        //  Enable Rx IT
    }
    else
    {
        // disable Rx com
        // disable Rx IT
    }
}
/******************************************************************************
 * @brief Process data send or receive
 * @param None
 * @return None
 ******************************************************************************/
void LUOS_COM_IRQHANDLER()
{
    // Reset timeout to it's default value
    RobusHAL_ResetTimeout();

    // reception management
    // if IT receive and IT receive enable
    // get data from register
    ctx.rx.callback(&data); // send reception byte to state machine
    if (data_size_to_transmit == 0)
    {
        // clear error IT
        return;
    }
    // else if Framming error IT
    ctx.rx.status.rx_framing_error = true;

    // Transmission management
    // if IT transmit complete and IT transmit complete enable
    RobusHAL_SetRxState(true);
    RobusHAL_SetTxState(false);
    // diasble It tx complete

#ifdef USE_TX_IT
    // else if IT transmit empty and IT transmit empty enable
    //  Transmit buffer empty (this is a software DMA)
    data_size_to_transmit--;
    // put data to register
    if (data_size_to_transmit == 0)
    {
        // Transmission complete, stop loading data and watch for the end of transmission
        // Disable Transmission empty buffer interrupt
        // Enable Transmission complete interrupt
    }
}
#endif
// clear error flag
}
/******************************************************************************
 * @brief Process data transmit
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComTransmit(uint8_t *data, uint16_t size)
{
    // wait tx empty
    RobusHAL_SetTxState(true);
    // Reduce size by one because we send one directly
    data_size_to_transmit = size - 1;
    if (size > 1)
    {
        // Start the data buffer transmission
        // **** NO DMA
        // Copy the data pointer globally alowing to keep it and run the transmission.
        tx_data = data;
#ifdef USE_TX_IT
        // put data in register
        // enable IT tx empty
        // disable IT tx complete
#else
        data_size_to_transmit = 0; // Reset this value avoiding to check IT TC during collision
        // set up DMA transfert
        // enable IT tx complete
#endif
    }
    else
    {
        // Transmit the only byte we have
        // put data in register
        // Enable Transmission complete interrupt because we only have one.
    }
}
/******************************************************************************
 * @brief set state of Txlock detection pin
 * @param None
 * @return Lock status
 ******************************************************************************/
void RobusHAL_SetRxDetecPin(uint8_t Enable)
{
    if (TX_LOCK_DETECT_IRQ != DISABLE)
    {
        // clear tx detect IT
        if (Enable == true)
        {
            // clear flag
        }
        else
        {
            // set flag
        }
    }
}
/******************************************************************************
 * @brief get Lock Com transmit status this is the HW that can generate lock TX
 * @param None
 * @return Lock status
 ******************************************************************************/
uint8_t RobusHAL_GetTxLockState(void)
{
    uint8_t result = false;

#ifdef USART_ISR_BUSY
    // check busy flag
    RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
    result = true;
#else
    if ((TX_LOCK_DETECT_PIN != DISABLE) && (TX_LOCK_DETECT_PORT != DISABLE))
    {
        // if pin low
        result = true;
        if (TX_LOCK_DETECT_IRQ == DISABLE)
        {
            if (result == true)
            {
                RobusHAL_ResetTimeout(DEFAULT_TIMEOUT);
            }
        }
    }
#endif
    return result;
}
/******************************************************************************
 * @brief Luos Timeout initialisation
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_TimeoutInit(void)
{
    // initialize clock
    LUOS_TIMER_CLOCK_ENABLE();

    // timer init

    // NVIC IT
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ResetTimeout(uint16_t nbrbit)
{
    // clear NVIC IT pending
    // clear IT flag
    // reset counter Timer
    // relaod value counter
    // if nbrbit != 0
    // enable timer
}
/******************************************************************************
 * @brief Luos Timeout communication
 * @param None
 * @return None
 ******************************************************************************/
void LUOS_TIMER_IRQHANDLER()
{
    // if It timeout
    // clear IT
    if ((ctx.tx.lock == true) && (RobusHAL_GetTxLockState() == false))
    {
        Recep_Timeout();
    }
}
/******************************************************************************
 * @brief Initialisation GPIO
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_GPIOInit(void)
{
    // Activate Clock for PIN choosen in RobusHAL
    PORT_CLOCK_ENABLE();

    if ((RX_EN_PIN != DISABLE) || (RX_EN_PORT != DISABLE))
    {
        // Configure GPIO pins : RxEN_Pin
        // output
        // no pull
    }

    if ((TX_EN_PIN != DISABLE) || (TX_EN_PORT != DISABLE))
    {
        // Configure GPIO pins : TxEN_Pin
        // output
        // no pull
    }

    // Configure GPIO pin : TxPin
    // ALTERNATE function USART Tx
    // open drain
    // pull up

    // Configure GPIO pin : RxPin
    // ALTERNATE function USART Rx
    // open drain
    // pull up

    // configure PTP
    RobusHAL_RegisterPTP();
    for (uint8_t i = 0; i < NBR_PORT; i++) /*Configure GPIO pins : PTP_Pin */
    {
        // IT falling
        //  pull down
        //  Setup PTP lines
        RobusHAL_SetPTPDefaultState(i);
        // activate NVIC IT for PTP
    }

    /*Configure GPIO pins : TX_LOCK_DETECT_Pin */
    if ((TX_LOCK_DETECT_PIN != DISABLE) || (TX_LOCK_DETECT_PORT != DISABLE))
    {
        // pull up
        // input
        if (TX_LOCK_DETECT_IRQ != DISABLE)
        {
            // it falling
            // NVIC enable
        }
    }
}
/******************************************************************************
 * @brief Register PTP
 * @param void
 * @return None
 ******************************************************************************/
static void RobusHAL_RegisterPTP(void)
{
#if (NBR_PORT >= 1)
    PTP[0].Pin  = PTPA_PIN;
    PTP[0].Port = PTPA_PORT;
    PTP[0].IRQ  = PTPA_IRQ;
#endif

#if (NBR_PORT >= 2)
    PTP[1].Pin  = PTPB_PIN;
    PTP[1].Port = PTPB_PORT;
    PTP[1].IRQ  = PTPB_IRQ;
#endif

#if (NBR_PORT >= 3)
    PTP[2].Pin  = PTPC_PIN;
    PTP[2].Port = PTPC_PORT;
    PTP[2].IRQ  = PTPC_IRQ;
#endif

#if (NBR_PORT >= 4)
    PTP[3].Pin  = PTPD_PIN;
    PTP[3].Port = PTPD_PORT;
    PTP[3].IRQ  = PTPD_IRQ;
#endif
}
/******************************************************************************
 * @brief callback for GPIO IT
 * @param GPIO IT line
 * @return None
 ******************************************************************************/
void PINOUT_IRQHANDLER(uint16_t GPIO_Pin)
{
    ////Process for Tx Lock Detec
    if (GPIO_Pin == TX_LOCK_DETECT_PIN)
    {
        ctx.tx.lock = true;
        // clear flag
    }
    else
    {
        for (uint8_t i = 0; i < NBR_PORT; i++)
        {
            if (GPIO_Pin == PTP[i].Pin)
            {
                PortMng_PtpHandler(i);
                break;
            }
        }
    }
}
/******************************************************************************
 * @brief Set PTP for Detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPDefaultState(uint8_t PTPNbr)
{
    // clear IT
    //  Pull Down / IT mode / Rising Edge
}
/******************************************************************************
 * @brief Set PTP for reverse detection on branch
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_SetPTPReverseState(uint8_t PTPNbr)
{
    // clear IT
    //  Pull Down / IT mode / Falling Edge
}
/******************************************************************************
 * @brief Set PTP line
 * @param PTP branch
 * @return None
 ******************************************************************************/
void RobusHAL_PushPTP(uint8_t PTPNbr)
{
    // Pull Down / Output mode
    // Clean edge/state detection and set the PTP pin as output
}
/******************************************************************************
 * @brief Get PTP line
 * @param PTP branch
 * @return Line state
 ******************************************************************************/
uint8_t RobusHAL_GetPTPState(uint8_t PTPNbr)
{
    // Pull Down / Input mode
}
/******************************************************************************
 * @brief Initialize CRC Process
 * @param None
 * @return None
 ******************************************************************************/
static void RobusHAL_CRCInit(void)
{
    // CRC initialisation
}
/******************************************************************************
 * @brief Compute CRC
 * @param None
 * @return None
 ******************************************************************************/
void RobusHAL_ComputeCRC(uint8_t *data, uint8_t *crc)
{
#ifdef CRC_HW
    // CRC HW calculation
#else
    for (uint8_t i = 0; i < 1; ++i)
    {
        uint16_t dbyte = data[i];
        *(uint16_t *)crc ^= dbyte << 8;
        for (uint8_t j = 0; j < 8; ++j)
        {
            uint16_t mix = *(uint16_t *)crc & 0x8000;
            *(uint16_t *)crc = (*(uint16_t *)crc << 1);
            if (mix)
                *(uint16_t *)crc = *(uint16_t *)crc ^ 0x0007;
        }
    }
#endif
}