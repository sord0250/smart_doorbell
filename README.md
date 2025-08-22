# ECEn 225 Doorbell Lab

Author: Spencer Ord

Last Updated: May 21 2025

## Description

This project is creating a working doorbell from a raspberry pi. It has a display screen and a joystick button. The doorbell uses a daemon to start the program when power is given to it. It presents a welcome screen, and upon pressing the button, "Ding Dong" will print on the screen and a picture will be taken and sent to the server to be saved onto a website. Then the welcome screen reappears. There is also a secret menu, and when it is accessed you can take pictures, put filters on it, and open and close several files.

## Usage Instructions

To access this lab, you'll need to ensure that you have the BCM2835 Library installed. To install this, run this command " wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz " and then uncompress the file, run the configure file, input the "make" command, and finall input "sudo make install".

To put the lab repo onto your pi, you will copy the ssh link. Then, access the part of your pi where you want to clone the repo. Then run the following command:

```
git clone your_github_repository
```

Once you have this library and the lab, to run the doorbell you will do the following:

```
run make main
run sudo ./main
```

The doorbell uses a Makefile which compiles everything for you. When you run the file, your doorbell will be running.

You can also add the executable as a service file to make it run. In the /etc/systemd/system directory, add a file that ends in .service. The contents of the file must have [Unit], [Service], and [Install] sections.

First enable the service with the "sudo systemctl disable my-service.service" command.
```
sudo systemctl start sshd     # Starts the sshd daemon
sudo systemctl stop sshd      # Stops the sshd daemon
```

## Updates & Changelog

### 05/21/25

- implemented the display features
- includes the following options: Clear screen, print Hello World!, display random Chars, display Stars, display a Flag
- test file was the only one edited


### 05/27/25

- added the ability to read text files
- implemented the ability to display a bitmap picture from a file
- refined functionality of the menu and button's abilities

### 05/29/25

- used the camera to take a picture and save it to a file
- applied the filters to the picture using the buttons
- learned how to use malloc function and clear the space after the picture is taken

### 06/05/25

- added functionality that sends image to a server
- uses network sockets and uses client_connect(), send(), etc.
- although slow, it gets the job done

### 06/10/25

- used threads to allow multiple parts of code at once
- frees the menu while the image is sending
- presents a bar that shows the status of the image

### 06/12/25

- created the doorbell functionality
- if the secret combination is pressed, it will go to the hidden menu
- used daemon to have it load upon starting the raspberry pi
