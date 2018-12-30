#include "ft6x06.h"

//*****************************************************************************
// Sets the address to read/write from in the FT6x06  
//
// Paramters
//    i2c_base:   a valid base address of an I2C peripheral
//
//    address:    8-bit address
//
// Returns
// I2C_OK if the byte was written to the FT6X06.
//*****************************************************************************
static i2c_status_t ft6x06_set_addr
( 
  uint32_t  i2c_base,
  uint8_t  address
)
{
  i2c_status_t status;
  
  // Before doing anything, make sure the I2C device is idle
  while ( I2CMasterBusy(i2c_base)) {};

  //==============================================================
  // ADD CODE
  // Set the I2C address 
  //==============================================================
  status = i2cSetSlaveAddr(i2c_base, FT6X06_DEV_ID, I2C_WRITE);
    
    
  if ( status != I2C_OK )
  {
    return status;
  }
  
  //==============================================================
  // ADD CODE
  // Send the address
  //==============================================================
	status = i2cSendByte(i2c_base,address ,I2C_MCS_START | I2C_MCS_RUN |I2C_MCS_STOP);

  return status;
}

//*****************************************************************************
// Reads one byte of data from the current address of the FT6x06  
//
// Paramters
//    i2c_base:   a valid base address of an I2C peripheral
//
//    data:       data to read.
//
// Returns
// I2C_OK if the byte was written to the FT6X06.
//*****************************************************************************
static i2c_status_t ft6x06_read_data
( 
  uint32_t  i2c_base,
  uint8_t *data
)
{
  i2c_status_t status;
  
  // Before doing anything, make sure the I2C device is idle
  while ( I2CMasterBusy(i2c_base)) {};

  //==============================================================
  // ADD CODE
  // Set the I2C address 
  //==============================================================
  status = i2cSetSlaveAddr(i2c_base, FT6X06_DEV_ID, I2C_READ);
    
  
  if ( status != I2C_OK )
  {
    return status;
  }

  //==============================================================
  // ADD CODE
  // get the data
  //==============================================================
  status = i2cGetByte( i2c_base, data , I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP);

  return status;
}


//*****************************************************************************
// Read the number of active touch points.
//*****************************************************************************
uint8_t ft6x06_read_td_status(void)
{ 
  // ADD CODE
  // Return the number of active touch points.  The only valid values of the 
  // register will be 0, 1, or 2.
	uint8_t data;
	i2c_status_t td_status;
	td_status = ft6x06_set_addr(FT6X06_I2C_BASE,FT6X06_TD_STATUS_R);
	if(td_status != I2C_OK){
		//error
		return 0;
	}
	td_status = ft6x06_read_data(FT6X06_I2C_BASE,&data);
	if(td_status != I2C_OK){
		//error
		return 0;
	}
	if(data<2){
		return data;
	}else{
		//error
		return 0;
	}
} 


//*****************************************************************************
// Read the X value of last touch event
//*****************************************************************************
uint16_t ft6x06_read_x(void)
{ 
  // ADD CODE
  // Return the X coordinate of the last touch point
  // This will require reading P1_XH and P1_XL
	uint16_t toReturn;
	uint8_t data1;
	uint8_t data2;
	i2c_status_t status;
	status = ft6x06_set_addr(FT6X06_I2C_BASE,FT6X06_P1_XH_R);
	if(status != I2C_OK){
		//error
		return 0;
	}
	status = ft6x06_read_data(FT6X06_I2C_BASE,&data1);
	if(status != I2C_OK){
		//error
		return 0;
	}
	status = ft6x06_set_addr(FT6X06_I2C_BASE,FT6X06_P1_XL_R);
	if(status != I2C_OK){
		//error
		return 0;
	}
	status = ft6x06_read_data(FT6X06_I2C_BASE,&data2);
	if(status != I2C_OK){
		//error
		return 0;
	}
	toReturn = ((uint16_t)(data1 & 0x0F)<<8) | data2;
	toReturn = 239 - toReturn;
	return toReturn;
} 

//*****************************************************************************
// Read the Y value of last touch event
//*****************************************************************************
uint16_t ft6x06_read_y(void)
{
  // ADD CODE
  // Return the Y coordinate of the last touch point 
  // This will require reading P1_YH and P1_YL
	uint16_t toReturn;
	uint8_t data1;
	uint8_t data2;
	i2c_status_t status;
	status = ft6x06_set_addr(FT6X06_I2C_BASE,FT6X06_P1_YH_R);
	if(status != I2C_OK){
		//error
		return 0;
	}
	status = ft6x06_read_data(FT6X06_I2C_BASE,&data1);
	if(status != I2C_OK){
		//error
		return 0;
	}
	status = ft6x06_set_addr(FT6X06_I2C_BASE,FT6X06_P1_YL_R);
	if(status != I2C_OK){
		//error
		return 0;
	}
	status = ft6x06_read_data(FT6X06_I2C_BASE,&data2);
	if(status != I2C_OK){
		//error
		return 0;
	}
	toReturn = ((uint16_t)(data1 & 0x0F)<<8) | data2;
	toReturn = 319 - toReturn;
	return toReturn;
}

//*****************************************************************************
// Test the ft6x06
//*****************************************************************************
bool ft6x06_init(void)
{ 
  //==============================
  // Configure I2C GPIO Pins
  //==============================  
  if(gpio_enable_port(FT6X06_GPIO_BASE) == false)
  {
    return false;
  }
  
  // Configure SCL 
  if(gpio_config_digital_enable(FT6X06_GPIO_BASE, FT6X06_I2C_SCL_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_alternate_function(FT6X06_GPIO_BASE, FT6X06_I2C_SCL_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_port_control(FT6X06_GPIO_BASE, FT6X06_I2C_SCL_PCTL_M, FT6X06_I2C_SCL_PIN_PCTL)== false)
  {
    return false;
  }
    

  
  // Configure SDA 
  if(gpio_config_digital_enable(FT6X06_GPIO_BASE, FT6X06_I2C_SDA_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_open_drain(FT6X06_GPIO_BASE, FT6X06_I2C_SDA_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_alternate_function(FT6X06_GPIO_BASE, FT6X06_I2C_SDA_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_port_control(FT6X06_GPIO_BASE, FT6X06_I2C_SDA_PCTL_M, FT6X06_I2C_SDA_PIN_PCTL)== false)
  {
    return false;
  }
  
  
  //  Initialize the I2C peripheral
  if( initializeI2CMaster(FT6X06_I2C_BASE)!= I2C_OK)
  {
    return false;
  }
  
  return true;
  
} 

