#include "spi.hpp"

SPI::SPI(int miso_port, int mosi_port, int clk_port) {
      spi_bus_config_t spi_bus_config = {
        .mosi_io_num = mosi_port,
        .miso_io_num = miso_port,
        .sclk_io_num = clk_port,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
      };
  
    #ifdef DEBUG
      ESP_LOGI(TAG_SPI, "spi_therm_bus_cfg: Inicializando o barramento.");
    #endif
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &spi_bus_config, 1));
}

THERM::THERM(int cs_port, int clk, int unit) {
  spi_device_interface_config_t spi_interface_cfg = {
    .mode = 0,
    .clock_speed_hz = clk,
    .spics_io_num=CS_PORT,
    .queue_size=1
  };
  
  #ifdef DEBUG
    ESP_LOGI(TAG_SPI, "spi_therm_init: Adicionando o dispositivo ao bus.");
  #endif
  ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &spi_interface_cfg, &spi_dev_handle));
  this->offset = 0;
  this->unit = CELSIUS;
}

float THERM::spi_therm_read(void) {
    uint16_t bits = 0;
    spi_transaction_t tM = {
        .length = 16,
        .rxlength = 16,
        .tx_buffer = NULL,
        .rx_buffer = &bits
    };

  spi_device_acquire_bus(spi_dev_handle, portMAX_DELAY); // Probably unnecessary
  spi_device_transmit(spi_dev_handle, &tM);
  spi_device_release_bus(spi_dev_handle);

  uint16_t res = SPI_SWAP_DATA_RX(bits,16);
  res >>= 3;

  if(this->unit == CELSIUS) {
    return res*0.25 + this->offset;
  } else if (this->unit == FAHRENHEIT) {
    return (res*0.25 + this->offset)*9/5+32;
  } else {
      return (res*0.25 + this->offset) + 273;
  } 
}

float THERM::setOffset(float offset){ this->offset = offset; return offset;}
float THERM::getOffset(void){ return this->offset;}
int THERM::getUnit(void){return this->unit;}
int THERM::setUnit(unsigned int unit){
  ESP_LOGI(TAG_SPI, "Valor recebido %d",unit);
  switch (unit) {
    case 0:
      this->unit = unit;
      return unit;
      break;
    case 1:
      this->unit = unit;
      return unit;
      break;
    case 2:
      this->unit = unit;
      return unit;
      break;
    default:
      return -1;
      break;
  }
}