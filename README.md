# dry-food-dispenser-alpha
Dry food dispenser for small pets. This was the project for my bachelor thesis. It's an alpha version that I plan on redesigning from the ground up in my spare time as I have some new, possibly better, ideas. All files are here, but I don't recommend building this version yourself.<br>

Project is based on Raspberry Pi Zero 2 W and is powered by a brushed DC motor. Communicaton with user is done via a simple 16x2 LCD display and 5 buttons in a pretty intuitive way. In 99% of cases button up is to go up or increase the value, button down is to go down or decrease the value, button left is to go back or cancel and button right is to go in or confirm. The fifth button called "feed" is used to give the pet a treat and can release a single portion of food when pressed by the user.<br>

Funcionalities of the system are:
- Dispensing food on schedule, fully configured by the user including the time of the day and the amount of portions of food. Limited to 10 feeding times per day, as it seemed more than enough, but it can be adjusted in the code if needed.
- Dispensing food on demand, by pressing the "feed" button.
- Configuring the amount of food per portion, by changing the feeding wheel and then setting the currently used wheel in the user menu.

<center><image src="Models/Pictures/front.jpg" height="400"></center>

The dispenser is a fully closed system and it doesn't need any app or internet connection to work, but it's advised to connect the RPi to the internet once at boot to get the time and date for correct working of the scheduler.<br>

Whole system is written in C and it's GPIO functionality relies on [WiringPi](https://github.com/WiringPi/WiringPi) library. Just like the physical design, the code is also planned to be improved, especially the LCD handler as it's hard to read at times and some fragmentation is needed.<br>

The dispenser is powered by a 5V 4A power supply and the motor is controlled by Cytron MDD3A driver via step-up converter that bumps up the voltage to 6V for the motor. The motor is a 6V LP with 75:1 gearbox with and encoder providing 0.67Nm of torque.<br>

All other parts are 3D printed and the idea behind them is to be easily mountable and demountable to allow for easy cleaning.

### BOM list
- 1x Raspberry Pi Zero 2 W
- 1x Cytron MDD3A motor driver
- 1x Pololu 2286 motor
- 1x 3-35V to 5-40V 5A step-up converter
- 1x 5V 4A DC power supply
- 1x 16x2 LCD display
- 1x LCD I2C adapter
- 4x Tact Switch TACT-67N
- 1x 12mm push button
- 1x DC-022b power socket
- 1x USB Micro B cabel
- 0.5mm² and 1.5mm² wires
- PLA filament
- PETG filament
- 12x M3x4 screw
- 3x M3x6 screw
- 6x M4x10 screw (+4 optional)
- 1x WaveShare Raspberry Pi Zero/Zero 2 W Heatsink
- 4x rubber feet