# Протокол : protocol

## Описание
Тестовый демонстрационный протокол.
## Структура
| 1 байт       | 1 байт        | n байт  | 16 бит  |
|--------------|---------------|---------|---------|
| старт (0x7E) | длина payload | payload | CRC     |

Если в передаваемых данный встречается старт байт (0x7E),
он передается 2 раза подряд.

CRC-16/CCITT-FALSE
https://crccalc.com/

## HW
Демонстрационный проект = CUBEMX+FREERTOS+IAR8.32+STLINK+STM32F103
Проверялся в железе в соответствии со схемой:

<a href="https://jpegshare.net/" target="_blank" title="jpegshare.net - бесплатный хостинг картинок"><img src="https://jpegshare.net/images/98/b3/98b3cd82afb3af99a2c530b9d9917ea4.jpg"/></a>
