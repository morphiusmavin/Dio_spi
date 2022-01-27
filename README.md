testing SPI on ts-7250
first tried to get on-chip SPI to work but had wild pointer problems
so decided to just use DIO lines as lines for MOSI, MISO, SCLK & SS 
use a define to set which was MASTER/SLAVE so I could test using 
2 different cards (7200 or 7250 or 7260) - not tested on 7800
