/*
 * Copyright (C) 2021 Fern H., RX5808 Arduino-based frequency changer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * IT IS STRICTLY PROHIBITED TO USE THE PROJECT (OR PARTS OF THE PROJECT / CODE)
 * FOR MILITARY PURPOSES. ALSO, IT IS STRICTLY PROHIBITED TO USE THE PROJECT (OR PARTS OF THE PROJECT / CODE)
 * FOR ANY PURPOSE THAT MAY LEAD TO INJURY, HUMAN, ANIMAL OR ENVIRONMENTAL DAMAGE.
 * ALSO, IT IS PROHIBITED TO USE THE PROJECT (OR PARTS OF THE PROJECT / CODE) FOR ANY PURPOSE THAT
 * VIOLATES INTERNATIONAL HUMAN RIGHTS OR HUMAN FREEDOM.
 * BY USING THE PROJECT (OR PART OF THE PROJECT / CODE) YOU AGREE TO ALL OF THE ABOVE RULES.
 */

#include <SPI.h>
#include <EEPROM.h>

const uint8_t SSP_PIN PROGMEM = 10;
const uint8_t BTN_BAND_PIN PROGMEM = 2;
const uint8_t BTN_CHANNEL_PIN PROGMEM = 3;
const uint8_t BTN_LED_GND_PIN PROGMEM = A1;
const uint8_t BTN_LED_VCC_PIN PROGMEM = A3;

// Channels with their Mhz Values
const uint16_t channel_frequency_table[] PROGMEM = {
    // Channel 1 - 8
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // Band A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // Band B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // Band E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // Band F / Airwave
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917  // Raceband
};

uint8_t band, channel;

void setup()
{
    // Initialize pins
    pinMode(BTN_LED_GND_PIN, OUTPUT);
    pinMode(BTN_LED_VCC_PIN, OUTPUT);
    pinMode(BTN_BAND_PIN, INPUT_PULLUP);
    pinMode(BTN_CHANNEL_PIN, INPUT_PULLUP);

    // Initialize EEPROM
    EEPROM.begin();

    // Read band from EEPROM
    if (EEPROM.read(0) > 4)
        EEPROM.write(0, 0);
    band = EEPROM.read(0);

    // Read channel from EEPROM
    if (EEPROM.read(1) > 7)
        EEPROM.write(1, 0);
    channel = EEPROM.read(1);

    // Initialize RX5808
    rx5808_init();

    // Set frequecny from EEPROM
    rx5808_set_frequency(pgm_read_word_near(channel_frequency_table + ((uint8_t)8 * band + channel)));
}

void loop()
{

    //pgm_read_word_near(channelFreqTable + curr_channel);
    if (!digitalRead(BTN_BAND_PIN)) {
        while (!digitalRead(BTN_BAND_PIN));
        if (band < 4)
            band++;
        else
            band = 0;
        EEPROM.write(0, band);
        rx5808_set_frequency(pgm_read_word_near(channel_frequency_table + ((uint8_t)8 * band + channel)));
        blink_ticks(band + 1);
    }

    if (!digitalRead(BTN_CHANNEL_PIN)) {
        while (!digitalRead(BTN_CHANNEL_PIN));
        if (channel < 7)
            channel++;
        else
            channel = 0;
        EEPROM.write(1, channel);
        rx5808_set_frequency(pgm_read_word_near(channel_frequency_table + ((uint8_t)8 * band + channel)));
        blink_ticks(channel + 1);
    }

}

void blink_ticks(uint8_t ticks) {
    delay(500);
    for (uint8_t i = 0; i < ticks; i++) {
        digitalWrite(BTN_LED_VCC_PIN, 1);
        delay(250);
        digitalWrite(BTN_LED_VCC_PIN, 0);
        delay(250);
    }
}

/// <summary>
/// Initializes SPI and RX5808 module
/// </summary>
void rx5808_init() {
    // Initialize SPI
    pinMode(SSP_PIN, OUTPUT);
    SPI.begin();
    SPI.setBitOrder(LSBFIRST);

    // Initialize RX5808
    digitalWrite(SSP_PIN, LOW);
    SPI.transfer((uint8_t)0x10);
    SPI.transfer((uint8_t)0x01);
    SPI.transfer((uint8_t)0x00);
    SPI.transfer((uint8_t)0x00);
    digitalWrite(SSP_PIN, HIGH);
}

/// <summary>
/// Sets a certain frequency for the RX module
/// </summary>
/// <param name="freq">frequency in MHz</param>
void rx5808_set_frequency(uint32_t freq) {
    uint32_t Delitel = (freq - 479) / 2;

    byte DelitelH = Delitel >> 5;
    byte DelitelL = Delitel & 0x1F;

    byte data0 = DelitelL * 32 + 17;
    byte data1 = DelitelH * 16 + DelitelL / 8;
    byte data2 = DelitelH / 16;
    byte data3 = 0;

    digitalWrite(SSP_PIN, LOW);
    SPI.transfer(data0);
    SPI.transfer(data1);
    SPI.transfer(data2);
    SPI.transfer(data3);
    digitalWrite(SSP_PIN, HIGH);
}