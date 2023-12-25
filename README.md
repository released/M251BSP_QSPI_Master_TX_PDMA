# M251BSP_QSPI_Master_TX_PDMA
 M251BSP_QSPI_Master_TX_PDMA

update @ 2023/12/25

1. initial QSPI0 with PA3 : CS , PA2 : CLK , PA0 : MOSI , PA1 : MISO 

- BYTE0 : head 0 , 0x5A

- BYTE1 : head 1 , 0x5A

- BYTE2 : length , low byte 

- BYTE3 : length , high byte 

- BYTE(n-2) : CRC8 checksum

- BYTE(n-1) : TAIL 0 , 0xA5 

- BYTE(n-0) : TAIL 1 , 0xA5


2. press digit Q/q , will generate 160 bytes data , press agagin will generate 1600 bytes data

3. press digit 1 , will send QSPI TX by PDMA 160 BYTES 

![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/log_qspi_pdma_160.jpg)	


![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/LA_qspi_pdma_160_head.jpg)	


![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/LA_qspi_pdma_160_tail.jpg)	


4. press digit 3 , will send QSPI TX 160 BYTES 

![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/log_qspi_normal_160.jpg)	


![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/LA_qspi_normal_160_head.jpg)	


![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/LA_qspi_normal_160_tail.jpg)	


5. press digit 2 , will send QSPI TX by PDMA 1600 BYTES 

![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/log_qspi_pdma_1600.jpg)	


![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/LA_qspi_pdma_1600_head.jpg)	


![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/LA_qspi_pdma_1600_tail.jpg)	


6. press digit 4 , will send QSPI TX 1600 BYTES 

![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/log_qspi_normal_1600.jpg)	


![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/LA_qspi_normal_1600_head.jpg)	


![image](https://github.com/released/M251BSP_QSPI_Master_TX_PDMA/blob/main/LA_qspi_normal_1600_tail.jpg)	

