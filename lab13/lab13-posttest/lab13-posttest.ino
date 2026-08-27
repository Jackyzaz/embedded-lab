#include <avr/io.h>
#include <util/twi.h>
#include "my_twi.h"

void TWI_stop_scan(void) {            // ฟังก์ชันส่ง STOP condition โดยตรง
  TWCR = (1 << TWINT) | 
         (1 << TWEN) | 
         (1 << TWSTO);
  while (TWCR & (1 << TWSTO));        // รอจนกว่า STOP condition จะเสร็จสิ้น
}

bool check_i2c_device(uint8_t sla_w) {  // ฟังก์ชันทดสอบว่า Address นั้นมีการตอบ ACK หรือไม่
                                        // รับค่าเป็น 8-bit SLA+W (เช่น 0x40, 0x42, ... หรือ 0x70, 0x72, ...)               
  TWI_send_start_condition();           // 1. ส่ง START Condition
  TWI_wait_until_start_has_been_sent();

  TWI_send_slave_address(sla_w);        // 2. ส่ง Slave Address + Write bit
  TWI_wait_until_sla_transmitted();

  uint8_t status = TWSR & 0xF8;         // 3. ตรวจสอบสถานะการตอบรับ (ACK) จาก Slave
  bool is_ack = (status == TW_MT_SLA_ACK); // TW_MT_SLA_ACK = 0x18
  
  TWI_stop_scan();                      // 4. ส่ง STOP Condition เพื่อคืนบัส I2C

  return is_ack;
}

  
void scan_pcf8574_devices(void) {       // ฟังก์ชันสแกนเฉพาะ PCF8574 และ PCF8574A 
  uint8_t found_count = 0;
  
  Serial.println("--- Scanning PCF8574 / PCF8574A Devices ---");
  Serial.println("\n[1] Checking PCF8574 (0x20 - 0x27)...");
  for (uint8_t a = 0; a < 8; a++) {
    uint8_t sla_w = 0x40 | (a << 1);    // 8-bit SLA+W: 0x40, 0x42, ..., 0x4E
    uint8_t addr_7bit = 0x20 | a;       // 7-bit I2C Address: 0x20 - 0x27
    if (check_i2c_device(sla_w)) {
      found_count++;
      Serial.print("  >> FOUND: PCF8574 at 7-bit Addr: 0x");
      Serial.print(addr_7bit, HEX);
      Serial.print(" (8-bit SLA_W: 0x");
      Serial.print(sla_w, HEX);
      Serial.print(") | Pin config [A2 A1 A0] = ");
      Serial.print((a >> 2) & 1);
      Serial.print(" ");
      Serial.print((a >> 1) & 1);
      Serial.print(" ");
      Serial.println(a & 1);
    }
  }
  Serial.println("\n[2] Checking PCF8574A (0x38 - 0x3F)...");
  for (uint8_t a = 0; a < 8; a++) {
    uint8_t sla_w = 0x70 | (a << 1); // 8-bit SLA+W: 0x70, 0x72, ..., 0x7E
    uint8_t addr_7bit = 0x38 | a;    // 7-bit I2C Address: 0x38 - 0x3F
    if (check_i2c_device(sla_w)) {
      found_count++;
      Serial.print("  >> FOUND: PCF8574A at 7-bit Addr: 0x");
      Serial.print(addr_7bit, HEX);
      Serial.print(" (8-bit SLA_W: 0x");
      Serial.print(sla_w, HEX);
      Serial.print(") | Pin config [A2 A1 A0] = ");
      Serial.print((a >> 2) & 1);
      Serial.print(" ");
      Serial.print((a >> 1) & 1);
      Serial.print(" ");
      Serial.println(a & 1);
    }
  }
  Serial.println("------------------------------------------");
  if (found_count == 0) {
    Serial.println("Result: No PCF8574 or PCF8574A found on I2C bus.");
  } else {
    Serial.print("Result: Total ");
    Serial.print(found_count);
    Serial.println(" device(s) found.");
  }
  Serial.println("==========================================\n");
}

void setup() {
  Serial.begin(38400);
  init_twi_module(); // กำหนดบิตเรตและเปิดใช้งานมอดูล TWI
  
  delay(500);
  scan_pcf8574_devices();
}

void loop() {
  // สแกนซ้ำทุก ๆ 5 วินาที
  delay(5000);
  scan_pcf8574_devices();
}
