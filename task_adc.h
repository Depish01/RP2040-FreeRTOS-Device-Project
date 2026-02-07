#pragma once
#include <Arduino.h>

void vTaskADC(void *pvParameters);
void vTaskFFT(void *pvParameters);
void vTaskBattery(void *pvParameters);

void ADC_Stop();
void ADC_Start();