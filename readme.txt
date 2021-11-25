    This demo is based on the APM32F003 mini board, it shows how to use I2C peripheral.
    It is a example of how to use the I2C software library to ensure the steps of an 
I2C communication between slave Receiver/transmitter and master transmitter/receiver using interrupts.

    This demo need two APM32F003 mini board, they are connected through I2C lines and GND.As follow:
                        
                                    _______VDD_______
                                     |             |    
                                    _|_           _|_
                                   |   |         |   |
                   pull up resistor|   |         |   |pull up resistor
                                   |___|         |___|
                                     |             |
               _________             |             |            _________
              |         |            |             |           |         |
              |         |__SDA(PB5)__|_____________|___________|         |       
              |         |__SCL(PB4)________________|___________|         |           
              |         |_____GND______________________________|         |           
              |         |                                      |         |    
              |_________|                                      |_________|
                Board 1                                          Board 2
                                                
Note:J2 and J3 Should be opend.
