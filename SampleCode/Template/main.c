/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "misc_config.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_ADC_Data_Ready                   		(flag_PROJ_CTL.bit1)
#define FLAG_PROJ_PREPARE_DATA            				(flag_PROJ_CTL.bit2)
#define FLAG_PROJ_SWAP_ADC_DATA                         (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_QSPI_PDMA_160                         (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_QSPI_PDMA_1600                        (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_QSPI_NORMAL_160                       (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_QSPI_NORMAL_1600                      (flag_PROJ_CTL.bit7)


/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned int counter_systick = 0;
volatile uint32_t counter_tick = 0;


volatile uint32_t g_u32Vref = 0;
volatile uint32_t g_u32AdcIntFlag = 0;

#define MANUAL_SS

#define BUFFER_LEN                                      (2048)

#define BridgeSpiPortNum				                (QSPI0)
#define BridgeSpiPort_RST				                (QSPI0_RST)
#define BridgeSpiPort_PDMA_TX			                (PDMA_QSPI0_TX)

#define QSPI_MASTER_TX_DMA_CH  			                (3)
#define QSPI_MASTER_OPENED_CH   			            ((1 << QSPI_MASTER_TX_DMA_CH))


uint8_t g_au8MasterTxBuffer[BUFFER_LEN]={0};
uint8_t g_au8MasterRxBuffer[BUFFER_LEN]={0};

uint16_t packetlen = 0;

uint16_t za = 0;
uint16_t zb = 0;
uint16_t zc = 0;
uint16_t zx = 0;


const uint8_t CRC8TAB[256] = 
{ 
	//0
	0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97, 
	0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E, 
	//1
	0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4,
	0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
	//2
	0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11,
	0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
	//3
	0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52, 
	0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
	//4 
	0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA,
	0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
	//5
	0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9,
	0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
	//6
	0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C,
	0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
	//7
	0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
	0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
	//8
	0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED,
	0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
	//9
	0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE,
	0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
	//A
	0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B,
	0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
	//B
	0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28,
	0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
	//C
	0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0,
	0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
	//D
	0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93,
	0xBD, 0x8C, 0xDF, 0xFE, 0x79, 0x48, 0x1B, 0x2A,
	//E
	0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56,
	0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
	//F
	0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15,
	0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC
}; 

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

unsigned int get_systick(void)
{
	return (counter_systick);
}

void set_systick(unsigned int t)
{
	counter_systick = t;
}

void systick_counter(void)
{
	counter_systick++;
}

void SysTick_Handler(void)
{

    systick_counter();

    if (get_systick() >= 0xFFFFFFFF)
    {
        set_systick(0);      
    }

    // if ((get_systick() % 1000) == 0)
    // {
       
    // }

    #if defined (ENABLE_TICK_EVENT)
    TickCheckTickEvent();
    #endif    
}

void SysTick_delay(unsigned int delay)
{  
    
    unsigned int tickstart = get_systick(); 
    unsigned int wait = delay; 

    while((get_systick() - tickstart) < wait) 
    { 
    } 

}

void SysTick_enable(unsigned int ticks_per_second)
{
    set_systick(0);
    if (SysTick_Config(SystemCoreClock / ticks_per_second))
    {
        /* Setup SysTick Timer for 1 second interrupts  */
        printf("Set system tick error!!\n");
        while (1);
    }

    #if defined (ENABLE_TICK_EVENT)
    TickInitTickEvent();
    #endif
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

// void delay_ms(uint16_t ms)
// {
// 	TIMER_Delay(TIMER0, 1000*ms);
// }


uint8_t CRC8( uint8_t *buf, uint16_t len)     
{               
	uint8_t  crc=0;

	while ( len-- )     
	{   
		crc = CRC8TAB[crc^*buf]; 

		buf++;   
	}     

	return crc;     
}  

void EADC_INT0_IRQHandler(void)
{
    g_u32AdcIntFlag = 1;
    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);      /* Clear the A/D ADINT0 interrupt flag */
}

/*
    Bandgap/ VREF = ADC Data/ 4095
    Vbg = 0.815 (V) 
*/
unsigned int ADC_ConvertBandGapChannel(void)
{

    int32_t  i32ConversionData;
    uint32_t u32TimeOutCnt;
	
    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);

    /* Enable the sample module 16 interrupt.  */
    EADC_ENABLE_INT(EADC, BIT0);//Enable sample module A/D ADINT0 interrupt.
    EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, BIT16);//Enable sample module 16 interrupt.

    /* Reset the ADC interrupt indicator and trigger sample module 16 to start A/D conversion */
    g_u32AdcIntFlag = 0;
    EADC_START_CONV(EADC, BIT16);

    /* Wait EADC conversion done */
    u32TimeOutCnt = SystemCoreClock; /* 1 second time-out */

    while(g_u32AdcIntFlag == 0)
    {
        if(--u32TimeOutCnt == 0)
        {
            printf("Wait for EADC conversion done time-out!\r\n");
            return 0;
        }
    }

    /* Disable the ADINT0 interrupt */
    EADC_DISABLE_INT(EADC, BIT0);
	
    /* Get the conversion result of the sample module 16 */
    i32ConversionData  = EADC_GET_CONV_DATA(EADC, 16);

    g_u32Vref = (0.815 * 4095 * 1000) / i32ConversionData ;

	#if 0	// debug
    printf("Band-gap: 0x%4X(%4d),VREF : %4dmv\r\n", i32ConversionData, i32ConversionData ,g_u32Vref);
	#endif

	return g_u32Vref;
}

void ADC_InitBandGapChannel(void)
{
    /* Set input mode as single-end and enable the A/D converter */
    EADC_Open(EADC, 0);

    /* Set sample module 16 external sampling time to 160 (M251 suggest total sample band-gap more than 10 us)*/
    EADC_SetExtendSampleTime(EADC, 16, 160);

    NVIC_EnableIRQ(EADC_INT0_IRQn);

    //make EADC priority higher than UART
    // NVIC_SetPriority(EADC_INT0_IRQn, (1 << __NVIC_PRIO_BITS) - 2);
}

// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint16_t rng(void)//void uint8_t __attribute__((always_inline)) rng(void)
{
    zx++;
    za = (za^zc^zx);
    zb = (zb+za);
    zc = (zc+(zb>>1)^za);
    return zc;
}

void prepare_seed(void)
{
    ADC_ConvertBandGapChannel();// init
    za = ADC_ConvertBandGapChannel(); 
    zb = ADC_ConvertBandGapChannel(); 
    zc = ADC_ConvertBandGapChannel(); 
    zx = ADC_ConvertBandGapChannel();     
}

uint32_t random(int min, int max)
{
    uint32_t res= 0;    
    uint32_t length_of_range = 0;
    uint16_t adc_vaule = 0;
    uint16_t seed = 0;

    length_of_range = max - min + 1;

    adc_vaule = ADC_ConvertBandGapChannel(); 
    seed = rng();
    srand(seed + adc_vaule);

    res = (uint32_t)(rand() % length_of_range + min);

    #if 0   // debug
    printf("adc_vaule:0x%4X,res:%5d(min:%5d,max:%5d) [0x%4X/0x%4X/0x%4X/0x%4X/0x%4X]\r\n" ,
            adc_vaule , res , min , max , 
            za , zb ,zc , zx , seed);
    #endif

    return res;
}

/*
	[0]BYTE0 		: head : 0x5A
	[1]BYTE1 		: head : 0x5A
	[2]BYTE2 		: length : 
    [3]
    ..data
	
	BYTE(n-2) 	: checksum :
	BYTE(n-1) 	: 0xA5
	BYTE(n)tail : 0xA5
*/

void prepare_data(uint16_t target_size)
{
	uint16_t datalen = 0;
	uint16_t i = 0;
	uint8_t checksum = 0;

	uint8_t head_byte = 2;
	uint8_t length_byte = 2;

	reset_buffer(g_au8MasterTxBuffer , 0x00, BUFFER_LEN);

	datalen = target_size - 4 - 3;  // head * 2 , legnth , checksum , tail *2

	for (i = 0 ; i < datalen  ; i++)
	{
		g_au8MasterTxBuffer[i + 4] = random(0 , 0xFF);;
	}

	checksum = CRC8( (uint8_t *) &g_au8MasterTxBuffer[4] , datalen+(head_byte+length_byte) );

	g_au8MasterTxBuffer[0] = 0xA5;
	g_au8MasterTxBuffer[1] = 0xA5;	
	g_au8MasterTxBuffer[2] = LOBYTE(datalen);	
	g_au8MasterTxBuffer[3] = HIBYTE(datalen);	
	g_au8MasterTxBuffer[target_size-3] = checksum;	
	g_au8MasterTxBuffer[target_size-2] = 0x5A;
	g_au8MasterTxBuffer[target_size-1] = 0x5A;

    printf("QSPI MASTER TX (%d)>>>\r\n",target_size);
    dump_buffer_hex(g_au8MasterTxBuffer , target_size);
}

void PDMA_IRQHandler(void)
{
    uint32_t status = PDMA_GET_INT_STATUS(PDMA);
	
    if (status & PDMA_INTSTS_ABTIF_Msk)   /* abort */
    {
		#if 1
        PDMA_CLR_ABORT_FLAG(PDMA, PDMA_GET_ABORT_STS(PDMA));
		#else
        if (PDMA_GET_ABORT_STS(PDMA) & (1 << QSPI_MASTER_TX_DMA_CH))
        {

        }
        PDMA_CLR_ABORT_FLAG(PDMA, (1 << QSPI_MASTER_TX_DMA_CH));

        if (PDMA_GET_ABORT_STS(PDMA) & (1 << QSPI_SLAVE_RX_DMA_CH))
        {

        }
        PDMA_CLR_ABORT_FLAG(PDMA, (1 << QSPI_SLAVE_RX_DMA_CH));
		#endif
    }
    else if (status & PDMA_INTSTS_TDIF_Msk)     /* done */
    {
		#if 1
        if((PDMA_GET_TD_STS(PDMA) & (1 << QSPI_MASTER_TX_DMA_CH) ) == (1 << QSPI_MASTER_TX_DMA_CH) )
        {
            /* Clear PDMA transfer done interrupt flag */
            PDMA_CLR_TD_FLAG(PDMA, (1 << QSPI_MASTER_TX_DMA_CH) );

			// //insert process	
            PDMA_DisableInt(PDMA, QSPI_MASTER_TX_DMA_CH, PDMA_INT_TRANS_DONE);
            // printf("pdma tx done\r\n");
			
        } 		
		#else
        if((PDMA_GET_TD_STS(PDMA) & QSPI_MASTER_OPENED_CH) == QSPI_MASTER_OPENED_CH)
        {
            /* Clear PDMA transfer done interrupt flag */
            PDMA_CLR_TD_FLAG(PDMA, QSPI_MASTER_OPENED_CH);

			//insert process
			QSPI_DISABLE_TX_RX_PDMA(BridgeSpiPortNum);			
        }    
		#endif    		
    }
    else if (status & (PDMA_INTSTS_REQTOF0_Msk | PDMA_INTSTS_REQTOF1_Msk))     /* Check the DMA time-out interrupt flag */
    {
        PDMA_CLR_TMOUT_FLAG(PDMA,QSPI_MASTER_TX_DMA_CH);
    }
    else
    {

    }	
}

void QSPI_Master_Tx_PDMA_transmit(QSPI_T *qspi , unsigned char *pBuffer , unsigned short size)
{

    QSPI_DISABLE_TX_PDMA(BridgeSpiPortNum);

    #if defined (MANUAL_SS)
    // /CS: active
    QSPI_SET_SS_LOW(qspi);
    #endif

    PDMA_SetTransferCnt(PDMA,QSPI_MASTER_TX_DMA_CH, PDMA_WIDTH_8, size);
    PDMA_SetTransferAddr(PDMA,QSPI_MASTER_TX_DMA_CH, (uint32_t)pBuffer, PDMA_SAR_INC, (uint32_t)&qspi->TX, PDMA_DAR_FIX);
    PDMA_SetTransferMode(PDMA,QSPI_MASTER_TX_DMA_CH, BridgeSpiPort_PDMA_TX, FALSE, 0);    
    
	QSPI_TRIGGER_TX_PDMA(qspi);	

	PDMA_EnableInt(PDMA, QSPI_MASTER_TX_DMA_CH, PDMA_INT_TRANS_DONE);
    
    while (QSPI_IS_BUSY(qspi));

     #if defined (MANUAL_SS)
	// CS: de-active
    QSPI_SET_SS_HIGH(qspi);
    #endif   

    printf("%s : transmit done\r\n",__FUNCTION__);
}

void QSPI_Master_Tx_PDMA_Init(void)
{
	reset_buffer(g_au8MasterTxBuffer , 0x00, BUFFER_LEN);
	
    PDMA_Open(PDMA, QSPI_MASTER_OPENED_CH);
   
	/* Single request type. SPI only support PDMA single request type. */
    PDMA_SetBurstType(PDMA,QSPI_MASTER_TX_DMA_CH, PDMA_REQ_SINGLE, 0);
    
	/* Disable table interrupt */
    PDMA->DSCT[QSPI_MASTER_TX_DMA_CH].CTL |= PDMA_DSCT_CTL_TBINTDIS_Msk;

    QSPI_TRIGGER_TX_PDMA(BridgeSpiPortNum);

    PDMA_EnableInt(PDMA, QSPI_MASTER_TX_DMA_CH, PDMA_INT_TRANS_DONE);
    NVIC_EnableIRQ(PDMA_IRQn);		
}

void QSPI_WriteData(QSPI_T *qspi , uint8_t* pData , uint16_t num_bytes)
{
    uint16_t i = 0;
    
    #if defined (MANUAL_SS)
    // /CS: active
    QSPI_SET_SS_LOW(qspi);
    #endif

    while (i < num_bytes)
    {
        QSPI_WRITE_TX(qspi, pData[i++]);
        while(QSPI_IS_BUSY(qspi));
    }
    
    while(QSPI_IS_BUSY(qspi));

    #if defined (MANUAL_SS)
	// CS: de-active
    QSPI_SET_SS_HIGH(qspi);
    #endif

    QSPI_ClearRxFIFO(qspi);

    printf("%s : transmit done\r\n",__FUNCTION__);
}

void QSPI_WriteReadData(QSPI_T *qspi , uint8_t *pData , uint16_t num_bytes)
{
    uint16_t i = 0;
	uint16_t j = 0;

    #if defined (MANUAL_SS)
    // /CS: active
    QSPI_SET_SS_LOW(qspi);
    #endif

    while (i < num_bytes)
    {
        QSPI_WRITE_TX(qspi, pData[i++]);
        while(QSPI_IS_BUSY(qspi));		

        while (QSPI_GET_RX_FIFO_EMPTY_FLAG(qspi) == 0)
		{
        	g_au8MasterRxBuffer[j++] = QSPI_READ_RX(qspi);
		}
    }
	
    while(QSPI_IS_BUSY(qspi));

    #if defined (MANUAL_SS)
	// CS: de-active
    QSPI_SET_SS_HIGH(qspi);
    #endif

    QSPI_ClearRxFIFO(qspi);
    
    printf("%s : transmit done\r\n",__FUNCTION__);
}

/*
	QSPI MASTER SPI0 : PA0(MOSI)/PA1(MISO)/PA2(CLK)/PA3(SS)
*/
void QSPI_Master_Init(void)
{
	SYS_ResetModule(BridgeSpiPort_RST);

    QSPI_Open(BridgeSpiPortNum, QSPI_MASTER, QSPI_MODE_0, 8, 2000000);

	QSPI_ClearRxFIFO(BridgeSpiPortNum);

    #if defined (MANUAL_SS)
    /* Disable auto SS function, control SS signal manually. */
    QSPI_DisableAutoSS(BridgeSpiPortNum);
    #else
    QSPI_EnableAutoSS(BridgeSpiPortNum, QSPI_SS, QSPI_SS_ACTIVE_LOW);
    #endif
}

void TMR1_IRQHandler(void)
{
	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
            FLAG_PROJ_TIMER_PERIOD_1000MS = 1;//set_flag(flag_timer_period_1000ms ,ENABLE);
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}

void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void loop(void)
{
	// static uint32_t LOG1 = 0;
	// static uint32_t LOG2 = 0;

    if ((get_systick() % 1000) == 0)
    {
        // printf("%s(systick) : %4d\r\n",__FUNCTION__,LOG2++);    
    }

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)//(is_flag_set(flag_timer_period_1000ms))
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0;//set_flag(flag_timer_period_1000ms ,DISABLE);

        // printf("%s(timer) : %4d\r\n",__FUNCTION__,LOG1++);
        PB14 ^= 1;        
    }

    if (FLAG_PROJ_PREPARE_DATA)
    {
        FLAG_PROJ_PREPARE_DATA = 0;

        FLAG_PROJ_SWAP_ADC_DATA ^= 1;
        if (FLAG_PROJ_SWAP_ADC_DATA)
        {
            prepare_data(160);
        }
        else
        {
            prepare_data(1600);
        }
    }

    if (FLAG_PROJ_QSPI_PDMA_160)
    {   
        FLAG_PROJ_QSPI_PDMA_160 = 0;
        QSPI_Master_Tx_PDMA_transmit(BridgeSpiPortNum,g_au8MasterTxBuffer,160);
    }
    if (FLAG_PROJ_QSPI_PDMA_1600)
    {   
        FLAG_PROJ_QSPI_PDMA_1600 = 0;
        QSPI_Master_Tx_PDMA_transmit(BridgeSpiPortNum,g_au8MasterTxBuffer,1600);
    }

    if (FLAG_PROJ_QSPI_NORMAL_160)
    {   
        FLAG_PROJ_QSPI_NORMAL_160 = 0;
        QSPI_WriteData(BridgeSpiPortNum,g_au8MasterTxBuffer,160);
    }
    if (FLAG_PROJ_QSPI_NORMAL_1600)
    {   
        FLAG_PROJ_QSPI_NORMAL_1600 = 0;
        QSPI_WriteData(BridgeSpiPortNum,g_au8MasterTxBuffer,1600);
    }
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		printf("press : %c\r\n" , res);
		switch(res)
		{
			case '1':
                FLAG_PROJ_QSPI_PDMA_160 = 1;
				break;
			case '2':
                FLAG_PROJ_QSPI_PDMA_1600 = 1;
				break;
			case '3':
                FLAG_PROJ_QSPI_NORMAL_160 = 1;
				break;
			case '4':
                FLAG_PROJ_QSPI_NORMAL_1600 = 1;
				break;

			case 'Q':
			case 'q':
                FLAG_PROJ_PREPARE_DATA = 1;

                break;
			case 'X':
			case 'x':
			case 'Z':
			case 'z':
                SYS_UnlockReg();
				// NVIC_SystemReset();	// Reset I/O and peripherals , only check BS(FMC_ISPCTL[1])
                // SYS_ResetCPU();     // Not reset I/O and peripherals
                SYS_ResetChip();    // Reset I/O and peripherals ,  BS(FMC_ISPCTL[1]) reload from CONFIG setting (CBS)	
				break;
		}
	}
}

void UART0_IRQHandler(void)
{
    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
			UARTx_Process();
        }
    }

    if(UART0->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(UART0, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART0_IRQn);
   
    // NVIC_SetPriority(UART0_IRQn, 3);

	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHCLKFreq : %8d\r\n",CLK_GetHCLKFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	
	#endif	

    #if 0
    printf("FLAG_PROJ_TIMER_PERIOD_1000MS : 0x%2X\r\n",FLAG_PROJ_TIMER_PERIOD_1000MS);
    printf("FLAG_PROJ_REVERSE1 : 0x%2X\r\n",FLAG_PROJ_REVERSE1);
    printf("FLAG_PROJ_REVERSE2 : 0x%2X\r\n",FLAG_PROJ_REVERSE2);
    printf("FLAG_PROJ_REVERSE3 : 0x%2X\r\n",FLAG_PROJ_REVERSE3);
    printf("FLAG_PROJ_REVERSE4 : 0x%2X\r\n",FLAG_PROJ_REVERSE4);
    printf("FLAG_PROJ_REVERSE5 : 0x%2X\r\n",FLAG_PROJ_REVERSE5);
    printf("FLAG_PROJ_REVERSE6 : 0x%2X\r\n",FLAG_PROJ_REVERSE6);
    printf("FLAG_PROJ_REVERSE7 : 0x%2X\r\n",FLAG_PROJ_REVERSE7);
    #endif

}

void GPIO_Init (void)
{
//    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB14MFP_Msk)) | (SYS_GPB_MFPH_PB14MFP_GPIO);
		
	GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);

//    GPIO_SetMode(PB, BIT15, GPIO_MODE_OUTPUT);	
}


void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);	

//    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);	

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);

    CLK_EnableModuleClock(PDMA_MODULE);

    /* Enable UART clock */
    CLK_EnableModuleClock(UART0_MODULE);
    /* Select UART clock source from HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

    CLK_EnableModuleClock(QSPI0_MODULE);
  	CLK_SetModuleClock(QSPI0_MODULE, CLK_CLKSEL2_QSPI0SEL_HIRC,MODULE_NoMsk);
    /* Setup QSPI0 multi-function pins */
    SYS->GPA_MFPL = SYS_GPA_MFPL_PA0MFP_QSPI0_MOSI0 | SYS_GPA_MFPL_PA1MFP_QSPI0_MISO0 | SYS_GPA_MFPL_PA2MFP_QSPI0_CLK | SYS_GPA_MFPL_PA3MFP_QSPI0_SS;

    /* Enable QSPI0 clock pin (PA2) schmitt trigger */
    PA->SMTEN |= GPIO_SMTEN_SMTEN2_Msk;

    /* Enable EADC module clock */
    CLK_EnableModuleClock(EADC_MODULE);
    /* EADC clock source is 96MHz, set divider to 8, EADC clock is 96/8 MHz */
    CLK_SetModuleClock(EADC_MODULE, 0, CLK_CLKDIV0_EADC(8));	

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}


/*
 * This is a template project for M251 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{
    SYS_Init();

	GPIO_Init();
	UART0_Init();
	TIMER1_Init();

    SysTick_enable(1000);
    #if defined (ENABLE_TICK_EVENT)
    TickSetTickEvent(1000, TickCallback_processA);  // 1000 ms
    TickSetTickEvent(5000, TickCallback_processB);  // 5000 ms
    #endif
    
    ADC_InitBandGapChannel();
    prepare_seed(); 
    QSPI_Master_Init();
    QSPI_Master_Tx_PDMA_Init();

    /* Got no where to go, just loop forever */
    while(1)
    {
        loop();

    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
