#include <time.h>
#include <string.h>
#include <kernel/io_utils.h>
#include <kernel/clock.h>

#define CURRENT_YEAR        2019
 
uint32_t century_register = 0x00;                                // Set by ACPI table parsing code if possible
struct tm system_time;
  
enum {
      cmos_address = 0x70,
      cmos_data    = 0x71
};
 
static uint32_t get_update_in_progress_flag() {
      outb(cmos_address, 0x0A);
      return (inb(cmos_data) & 0x80);
}
 
static uint8_t get_RTC_register(uint32_t reg) {
      outb(cmos_address, reg);
      return inb(cmos_data);
}
 
uint32_t read_rtc() {
      struct tm last_time;
      uint32_t century = 0, last_century;
      uint8_t registerB;
 
      // Note: This uses the "read registers until you get the same values twice in a row" technique
      //       to avoid getting dodgy/inconsistent values due to RTC updates
 
      while (get_update_in_progress_flag());                // Make sure an update isn't in progress
      system_time.tm_sec = get_RTC_register(0x00);
      system_time.tm_min = get_RTC_register(0x02);
      system_time.tm_hour = get_RTC_register(0x04);
      system_time.tm_wday = get_RTC_register(0x06);
      system_time.tm_mday = get_RTC_register(0x07);
      system_time.tm_month = get_RTC_register(0x08);
      system_time.tm_year = get_RTC_register(0x09);
      if(century_register != 0) {
            century = get_RTC_register(century_register);
      }
 
      do {
            memcpy(&last_time, &system_time, sizeof(struct tm));
            last_century = century;
 
            while (get_update_in_progress_flag());           // Make sure an update isn't in progress
            system_time.tm_sec = get_RTC_register(0x00);
            system_time.tm_min = get_RTC_register(0x02);
            system_time.tm_hour = get_RTC_register(0x04);
            system_time.tm_wday = get_RTC_register(0x06);
            system_time.tm_mday = get_RTC_register(0x07);
            system_time.tm_month = get_RTC_register(0x08);
            system_time.tm_year = get_RTC_register(0x09);
            if(century_register != 0) {
                  century = get_RTC_register(century_register);
            }
      } while( memcmp(&last_time, &system_time, sizeof(struct tm) || century != last_century));
 
      registerB = get_RTC_register(0x0B);
 
      // Convert BCD to binary values if necessary
 
      if (!(registerB & 0x04)) {
            system_time.tm_sec = (system_time.tm_sec & 0x0F) + ((system_time.tm_sec / 16) * 10);
            system_time.tm_min = (system_time.tm_min & 0x0F) + ((system_time.tm_min / 16) * 10);
            system_time.tm_hour = ( (system_time.tm_hour & 0x0F) + (((system_time.tm_hour & 0x70) / 16) * 10) ) | (system_time.tm_hour & 0x80);
            system_time.tm_mday = (system_time.tm_mday & 0x0F) + ((system_time.tm_mday / 16) * 10);
            system_time.tm_wday = (system_time.tm_wday & 0x0F) + ((system_time.tm_wday / 16) * 10);
            system_time.tm_month = (system_time.tm_month & 0x0F) + ((system_time.tm_month / 16) * 10);
            system_time.tm_year = (system_time.tm_year & 0x0F) + ((system_time.tm_year / 16) * 10);
            if(century_register != 0) {
                  century = (century & 0x0F) + ((century / 16) * 10);
            }
      }
 
      // Convert 12 hour clock to 24 hour clock if necessary
      if (!(registerB & 0x02) && (system_time.tm_hour & 0x80)) {
            system_time.tm_hour = ((system_time.tm_hour & 0x7F) + 12) % 24;
      }
            
      // Calculate the full (4-digit) year
      if(century_register != 0) {
            system_time.tm_year += century * 100 - 1900;
      } else {
            system_time.tm_year += (CURRENT_YEAR / 100) * 100;
            if(system_time.tm_year < CURRENT_YEAR) system_time.tm_year += 100;
            system_time.tm_year -= 1900;
      }
      
      return (uint32_t) &system_time;
}