# IN-14 / IN-18 Nixie Clock on ESP32

Прошивка для часов на газоразрядных индикаторах IN-14 или IN-18 с управлением на ESP32.

## Features

- Точное время по NTP (pool.ntp.org, часовой пояс UTC+7)
- Автоматическая синхронизация времени через WiFi
- OTA-обновление прошивки
- Отображение курса Bitcoin и Ethereum (заглушка)
- Отображение температуры (OpenWeatherMap / Narodmon)
- Автоматическая регулировка яркости по датчику VEML7700
- Эффекты переключения цифр (CrossFade, SwitchNumbers, Wipe, Random)
- WS2812B RGB-подсветка (6 светодиодов)
- PWM управление подсветкой через ШИМ (pin BL)
- Веб-портал на GyverPortal
- Работает на двух ядрах ESP32 (FreeRTOS)

## Hardware

| Компонент | Назначение |
|-----------|-----------|
| ESP32 Dev Board | Микроконтроллер |
| IN-14 / IN-18 (6 шт) | Газоразрядные индикаторы |
| HV5522 | High-side драйвер анодов (через HSPI) |
| VEML7700 | Датчик освещённости (I2C) |
| WS2812B (6 шт) | RGB светодиоды подсветки |
| MOSFET + BL pin | ШИМ-регулировка подсветки |

## Pinout

| Pin | Назначение |
|-----|-----------|
| GPIO2 (BL) | ШИМ подсветки |
| GPIO5 | CS для VSPI |
| GPIO12 (HSPI_MISO) | HSPI MISO |
| GPIO13 (HSPI_MOSI) | HSPI MOSI |
| GPIO14 (HSPI_SCLK) | HSPI Clock |
| GPIO15 (HSPI_SS) | HSPI Chip Select (HV5522 LE) |
| GPIO16 | WS2812 Data + N/C |

## Project Structure

```
src/
├── main.cpp         # Точка входа
├── variables.h      # Пины, конфигурация, глобальные переменные
├── setup.h          # Инициализация: WiFi, OTA, SPI, таймеры, VEML7700
├── loop.h           # Главный цикл, переключение режимов отображения
├── effects.h        # Эффекты переключения цифр (CrossFade, SwitchNumbers, Wipe, Random)
├── setting.h        # UpdateDisplay(), NTP, WiFi, OTA, таймеры, яркость
├── task.h           # Задача на Core 0: WS2812 + VEML7700
├── getTemp.h        # Получение температуры с OpenWeatherMap и Narodmon
├── timer2Minim.h    # Простой non-blocking таймер
lib/
├── NTPClient/       # NTP клиент
├── Freenove_WS2812_Lib_for_ESP32/  # Драйвер WS2812 через RMT
├── SPI/             # SPI драйвер для ESP32 (VSPI + HSPI)
├── ArduinoMD5/      # MD5 хеширование (для Narodmon API)
```

## Display Modes

- **Mode 0**: Текущее время (ЧЧ:ММ:СС) с анимированной точкой
- **Mode 1**: Курс Bitcoin (6 цифр) / Дата (ДД.ММ.ГГ)
- **Mode 2**: Курс Ethereum (6 цифр)
- **Mode 3**: Температура (с открытыми старшими разрядами)

Переключение между режимами происходит автоматически каждые 4 секунды.
На 30-й секунде каждой минуты активируется отображение дополнительной информации.

## Effects

- **0 — CrossFade**: Плавный перекрёстный переход цифр
- **1 — SwitchNumbers**: Последовательное переключение каждого разряда
- **2 — Random**: Случайный эффект

При смене режима отображения (display) проигрывается эффект "ухода" (`off_effects`), затем эффект "появления" (`on_effects`):

1. Right-to-left wipe
2. Left-to-right wipe
3. Cathode fade right-to-left
4. Cathode fade left-to-right
5. Random cathode fade

## Brightness Control

Яркость регулируется автоматически по показаниям VEML7700:

| Lux | IN-14 | IN-18 |
|-----|-------|-------|
| 0–12 | 60 | 15 |
| 15–47 | 80 | 40 |
| 50–97 | 110 | 80 |
| 100–297 | 150 | 150 |
| 300–697 | 200 | 200 |
| 700–15000 | 255 | 255 |

## Build & Flash

Установите [PlatformIO](https://platformio.org/):

```bash
pio run -e esp32dev --target upload
pio device monitor
```

OTA:

```bash
pio run -e esp32dev --target upload --upload-port 192.168.x.x
```

## Configuration

WiFi, API ключи и другие настройки задаются в коде (в текущей версии жёстко):

- `variables.h` — тип ламп (IN_14 / IN_18), пины, тайминги
- `setting.h` — WiFi SSID/пароль, NTP сервер, часовой пояс
- `getTemp.h` — API ключи OpenWeatherMap и Narodmon

Планируется интеграция с GyverPortal для настройки через веб-интерфейс.

## Dependencies

- `adafruit/Adafruit VEML7700 Library` — датчик освещённости
- `gyverlibs/GyverPortal` — веб-портал
- `gyverlibs/FileData` — работа с файлами конфигурации
- Встроенные библиотеки: `NTPClient`, `Freenove_WS2812_Lib_for_ESP32`, `SPI`, `ArduinoMD5`
