#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include "dynamixel_sdk.h"                                  // Uses Dynamixel SDK library
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>



// Control table address
#define ADDR_MX_TORQUE_ENABLE           24                  // Control table address is different in Dynamixel model
#define ADDR_MX_GOAL_POSITION           30
#define ADDR_MX_PRESENT_POSITION        36

// Protocol version
#define PROTOCOL_VERSION                1.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL_ID                          1                   // Dynamixel ID: 1
#define BAUDRATE                        57600

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define DXL_MOVING_STATUS_THRESHOLD     10                  // Dynamixel moving status threshold

#define ESC_ASCII_VALUE                 0x1b

std::vector<std::string> get_available_ports(std::string file)
{
  std::vector<std::string> l;
  int fd;
  std::string port = "";
  for(int i = 0;i < 256;++i)
  {
    port.clear();
    port.append(file);
    port.append(std::to_string(i));
    fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd != -1)
      l.push_back(port);
  }
  return l;
}

std::vector<std::string> l=get_available_ports("/dev/ttyUSB");
#define DEVICENAME                      (char*)l[0].c_str()

int write(int id,int goal_pos)
{
	#define DXL_MAXIMUM_POSITION_VALUE goal_pos
	dynamixel::PortHandler *portHandler=dynamixel::PortHandler::getPortHandler(DEVICENAME);
	dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);
	int index = 1;
	int dxl_comm_result = COMM_TX_FAIL;             // Communication result
	uint8_t dxl_error = 0;                          // Dynamixel error
	uint16_t dxl_present_position = 0;              // Present position

	if (portHandler->openPort())
	{
		printf("Succeeded to open the port!\n");
	}
	else
	{
		printf("Failed to open the port!\n");
		printf("Press any key to terminate...\n");
		//getch();
		return 0;
	}

	if (portHandler->setBaudRate(BAUDRATE))
	{
		printf("Succeeded to change the baudrate!\n");
	}

	dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, id, ADDR_MX_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (dxl_error != 0)
	{
		printf("%s\n", packetHandler->getRxPacketError(dxl_error));
	}
	else
	{
		printf("Dynamixel has been successfully connected \n");
	}

	dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, id, ADDR_MX_PRESENT_POSITION, &dxl_present_position, &dxl_error);
	#define DXL_MINIMUM_POSITION_VALUE dxl_present_position
	int dxl_goal_position[2] = {DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE};         // Goal position
	
	// Write goal position
	dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, id, ADDR_MX_GOAL_POSITION, dxl_goal_position[index], &dxl_error);
	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (dxl_error != 0)
	{
		printf("%s\n", packetHandler->getRxPacketError(dxl_error));
	}

	do
	{
		// Read present position
		dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, id, ADDR_MX_PRESENT_POSITION, &dxl_present_position, &dxl_error);
		if (dxl_comm_result != COMM_SUCCESS)
		{
			printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
		}
		else if (dxl_error != 0)
		{
			printf("%s\n", packetHandler->getRxPacketError(dxl_error));
		}

		printf("[ID:%03d] GoalPos:%03d  PresPos:%03d\n", id, dxl_goal_position[index], dxl_present_position);

	}while((abs(dxl_goal_position[index] - dxl_present_position) > DXL_MOVING_STATUS_THRESHOLD));




	dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, id, ADDR_MX_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
	if (dxl_comm_result != COMM_SUCCESS)
	{
		printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
	}
	else if (dxl_error != 0)
	{
	printf("%s\n", packetHandler->getRxPacketError(dxl_error));
	}

	// Close port
	portHandler->closePort();

	return 1;
}



int main()
{
	int goal_pos;
	scanf("%d",&goal_pos);
	write(DXL_ID,goal_pos);
	return 0;
}
