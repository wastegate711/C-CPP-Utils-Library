#include "I2C_Lcd.h"

I2C_HandleTypeDef i2cHandle;
uint16_t address;
bool backlight = true;             //Начальная установка для подсветки вкл/выкл.
uint8_t global_buffer = 0; //глобальная переменная байта данных, отправляемая дисплею.


static void Send_init_Data(uint8_t *init_Data);

/**
 * Инициализация
 * @param handle Дескриптор структуры I2C к которому подключен дисплей.
 * @param address Адрес дисплея.
 */
void I2C_Lcd_InitializationLibrary(I2C_HandleTypeDef handle, uint16_t addressLcd)
{
    i2cHandle = handle;
    address = addressLcd;
}

/**
 * Инициализация дисплея.
 * Для работы с контроллером HD44780
 */
void I2C_Lcd_LcdInitialization()
{
    uint8_t tx_buffer = 0x30;

    HAL_Delay(15);
    Send_init_Data(&tx_buffer);
    HAL_Delay(5);
    Send_init_Data(&tx_buffer);
    HAL_Delay(1);
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x20;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x20;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x80;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x00;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x80;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x00;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x10;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x00;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x30;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x00;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0xC0;
    Send_init_Data(&tx_buffer);
}

/**
 * Функция предназначена для отправки байта данных по шине i2c.
 * Содержит в себе Delay. Без него инициализация дисплея не проходит.
 * @param init_Data  байт, например 0x25, где 2 (0010) это DB7-DB4 или DB3-DB0, а 5(0101)
 * это сигналы LED, E, RW, RS соответственно
 */
static void Send_init_Data(uint8_t *init_Data)
{
    if(backlight)
    {
        *init_Data |= 0x08; //Включить подсветку
    }
    else
    {
        *init_Data &= ~0x08; //Выключить подсветку
    }
    *init_Data |= 0x04; // Устанавливаем стробирующий сигнал E в 1
    HAL_I2C_Master_Transmit(&i2cHandle, address, init_Data, 1, 10);
    HAL_Delay(5);
    *init_Data &= ~0x04; // Устанавливаем стробирующий сигнал E в 0
    HAL_I2C_Master_Transmit(&i2cHandle, address, init_Data, 1, 10);
    HAL_Delay(5);
}

/**
 * Функция отправки байта информации на дисплей.
 * @param Data Байт данныйх.
 */
static void I2C_Lcd_Write_byte(uint8_t Data)
{
    HAL_I2C_Master_Transmit(&i2cHandle, address, &Data, 1, 10);
}

/**
 * Функция отправки пол байта информации.
 * @param Data Байт данных.
 */
static void I2C_Lcd_Send_cmd(uint8_t Data)
{
    Data <<= 4;
    I2C_Lcd_Write_byte(global_buffer |= 0x04); // Устанавливаем стробирующий сигнал E в 1
    I2C_Lcd_Write_byte(global_buffer | Data); // Отправляем в дисплей полученный и сдвинутый байт
    I2C_Lcd_Write_byte(global_buffer &= ~0x04);	// Устанавливаем стробирующий сигнал E в 0.
}

/**
 * Функция отправки байта данных на дисплей
 * @param Data Байт данных.
 * @param mode Отправка команды. 1 - RW = 1(отправка данных). 0 - RW = 0(отправка команды).
 */
static void I2C_Lcd_Send_data_symbol(uint8_t Data, uint8_t mode)
{
    if (mode == 0)
    {
        I2C_Lcd_Write_byte(global_buffer &= ~0x01); // RS = 0
    }
    else
    {
        I2C_Lcd_Write_byte(global_buffer |= 0x01); // RS = 1
    }
    uint8_t MSB_Data = 0;
    MSB_Data = Data >> 4; // Сдвигаем полученный байт на 4 позичии и записываем в переменную
    I2C_Lcd_Send_cmd(MSB_Data);	// Отправляем первые 4 бита полученного байта
    I2C_Lcd_Send_cmd(Data);	   // Отправляем последние 4 бита полученного байта
}


void I2C_Lcd_Send_data(uint8_t *Data)
{

    if(backlight)
    {
        *Data |= 0x08;
    }
    else
    {
        *Data &= ~0x08;
    }
    *Data |= 0x04; // устанавливаем стробирующий сигнал E в 1
    HAL_I2C_Master_Transmit(&i2cHandle, address, Data, 1, 10);
    *Data &= ~0x04; // устанавливаем стробирующий сигнал E в 0
    HAL_I2C_Master_Transmit(&i2cHandle, address, Data, 1, 10);
}

/**
 * Функция вывода символа на дисплей.
 * @param symbol символ в кодировке utf-8
 */
void I2C_Lcd_Print_symbol(uint8_t symbol)
{
    uint8_t command;
    command = ((symbol & 0xf0) | 0x09); //Формирование верхнего полубайта в команду для дисплея
    I2C_Lcd_Send_data(&command);
    command = ((symbol & 0x0f) << 4) | 0x09; //Формирование нижнего полубайта в команду для дисплея
    I2C_Lcd_Send_data(&command);
}

/**
 * Функция вывода символа на дисплей.
 * @param message Массив, который отправляем на дисплей.
 */
void I2C_Lcd_Print_text(char *message)
{
    for(int i = 0; i < strlen(message); i++)
    {
        I2C_Lcd_Print_symbol(message[i]);
    }
}

/**
 * Функция установки курсора для вывода текста на дисплей.
 * @param x Координата по оси x. от 0 до 39.
 * @param y Координата по оси y. от 0 до 3.
 */
void I2C_Lcd_SetCursor(uint8_t x, uint8_t y)
{
    uint8_t command, adr;
    if(y > 3)
        y = 3;
    if(x > 39)
        x = 39;
    if(y == 0)
    {
        adr = x;
    }
    if(y == 1)
    {
        adr = x + 0x40;
    }
    if(y == 2)
    {
        adr = x + 0x14;
    }
    if(y == 3)
    {
        adr = x + 0x54;
    }
    command = ((adr & 0xf0) | 0x80);
    I2C_Lcd_Send_data(&command);

    command = (adr << 4);
    I2C_Lcd_Send_data(&command);
}

/**
 * Функция перемещения текста влево, если ее повторять с периодичностью, то получится бегущая строка.
 */
void I2C_Lcd_Move_to_the_left(void)
{
    uint8_t command;
    command = 0x18;
    I2C_Lcd_Send_data(&command);

    command = 0x88;
    I2C_Lcd_Send_data(&command);
}

/**
 * Включения/выключения подсветки.
 * @param state состояние подсветки.
 */
void I2C_Lcd_Backlight(bool state)
{
    if (state)
    {
        backlight = true;
    }
    else
    {
        backlight = false;
    }
}

/**
 * Функция создания своего собственного символа и запись его в память.
 * @param my_Symbol Массив с символом.
 * @param memory_address Номер ячейки: от 1 до 8. Всего 8 ячеек.
 */
void I2C_Lcd_Create_symbol(uint8_t *my_Symbol, uint8_t memory_address)
{
    I2C_Lcd_Send_data_symbol(((memory_address * 8) | 0x40), 0);
    for (uint8_t i = 0; i < 8; i++)
    {
        I2C_Lcd_Send_data_symbol(my_Symbol[i], 1); // Записываем данные побайтово в память
    }
}

/**
 * Аппаратная функция очистки дисплея.
 */
void I2C_Lcd_Clean(void)
{
/// Аппаратная функция очистки дисплея.
/// Удаляет весь текст, возвращает курсор в начальное положение.
    uint8_t tx_buffer = 0x00;
    Send_init_Data(&tx_buffer);
    tx_buffer = 0x10;
    Send_init_Data(&tx_buffer);

}