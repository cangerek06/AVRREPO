/* Nokia 5110 LCD AVR Library example
 *
 * Copyright (C) 2015 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version 3
 * of the Licence, or (at your option) any later version.
 *
 * Original library written by SkewPL, http://skew.tk
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "nokia5110.h"

#define DHT11_PIN 0    // DHT11 veri pini (PB0)
#define BUZZER_PIN PD2 // Buzzer pini (PD2)

void DHT11_Request() {
	DDRB |= (1 << DHT11_PIN); // Çıkış pini yap
	PORTB &= ~(1 << DHT11_PIN); // Düşük yap
	_delay_ms(18); // 18ms boyunca bekle
	PORTB |= (1 << DHT11_PIN); // Yüksek yap
	_delay_us(20); // 20-40us boyunca bekle
	DDRB &= ~(1 << DHT11_PIN); // Giriş pini yap
}

uint8_t DHT11_Response() {
	_delay_us(40);
	if (!(PINB & (1 << DHT11_PIN))) {
		_delay_us(80);
		if (PINB & (1 << DHT11_PIN)) _delay_us(80);
		return 1;
		} else {
		return 0;
	}
}

uint8_t DHT11_Read() {
	uint8_t i, data = 0;
	for (i = 0; i < 8; i++) {
		while (!(PINB & (1 << DHT11_PIN))); // Yüksek olana kadar bekle
		_delay_us(30);
		if (PINB & (1 << DHT11_PIN)) {
			data = (data << 1) | 1;
			} else {
			data = (data << 1);
		}
		while (PINB & (1 << DHT11_PIN)); // Düşük olana kadar bekle
	}
	return data;
}

void Buzzer_On() {
	PORTD |= (1 << BUZZER_PIN); // Buzzer'ı aç
}

void Buzzer_Off() {
	PORTD &= ~(1 << BUZZER_PIN); // Buzzer'ı kapat
}

int main(void) {
	uint8_t I_RH, D_RH, I_Temp, D_Temp, CheckSum;
	uint8_t max_temp_int = 0, max_temp_dec = 0; // Maksimum sıcaklık tam ve ondalık kısımları
	uint8_t max_rh = 0; // Maksimum nem değişkeni

	DDRD |= (1 << BUZZER_PIN); // Buzzer pini çıkış olarak ayarla
	Buzzer_Off(); // Başlangıçta buzzer kapalı

	nokia_lcd_init();
	nokia_lcd_clear();

	while (1) {
		DHT11_Request(); // DHT11 sensörüne istek gönder
		if (DHT11_Response()) { // Sensörden yanıt bekle
			I_RH = DHT11_Read(); // Nem tam kısmı
			D_RH = DHT11_Read(); // Nem ondalık kısmı
			I_Temp = DHT11_Read(); // Sıcaklık tam kısmı
			D_Temp = DHT11_Read(); // Sıcaklık ondalık kısmı
			CheckSum = DHT11_Read(); // Checksum

			if ((I_RH + D_RH + I_Temp + D_Temp) == CheckSum) {
				// Maksimum sıcaklık ve nemi güncelle
				if (I_Temp > max_temp_int || (I_Temp == max_temp_int && D_Temp > max_temp_dec)) {
					max_temp_int = I_Temp;
					max_temp_dec = D_Temp;
				}
				if (I_RH > max_rh) max_rh = I_RH;

				char buffer[40];
				snprintf(buffer, sizeof(buffer), "Sicaklik: %d.%dC", I_Temp, D_Temp);
				nokia_lcd_clear();
				nokia_lcd_write_string(buffer, 1);
				nokia_lcd_set_cursor(0, 10);
				snprintf(buffer, sizeof(buffer), "Nem: %d.%d%%", I_RH, D_RH);
				nokia_lcd_write_string(buffer, 1);
				nokia_lcd_set_cursor(0, 20);
				snprintf(buffer, sizeof(buffer), "Mx Temp: %d.%dC", max_temp_int, max_temp_dec);
				nokia_lcd_write_string(buffer, 1);
				nokia_lcd_set_cursor(0, 30);
				snprintf(buffer, sizeof(buffer), "Mx Nem: %d%%", max_rh);
				nokia_lcd_write_string(buffer, 1);
				nokia_lcd_render();

				// Sıcaklık 40°C'nin üzerinde mi?
				if (I_Temp >= 40) {
					Buzzer_On(); // Buzzer'ı aç
					} else {
					Buzzer_Off(); // Buzzer'ı kapat
				}
				} else {
				nokia_lcd_clear();
				nokia_lcd_write_string("Checksum hatasi", 1);
				nokia_lcd_render();
			}
			} else {
			nokia_lcd_clear();
			nokia_lcd_write_string("Sensor yanit yok", 1);
			nokia_lcd_render();
		}
		_delay_ms(2000); // 2 saniye bekle
	}
}
