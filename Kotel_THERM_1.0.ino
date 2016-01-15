#include <EEPROM.h>
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <RTC.h>

RTC  time; //Часы
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Подключение по I2C LCD дисплея
//Подключение джойстика
int xPin = A0;
int yPin = A1;
int buttonPin = 13;

//Запомним чтобы каждый раз не выводить
String TIME;

//Меню
int NumMenu = 0;
char* myStrings[] = {"EDIT TIME", "EDIT TIMER", "EXIT"}; // 1-3

//Время после истечения которого будет произведен выход из меню
unsigned long TimeAut = 0;

//Значения таймера
byte ChasTimer = 0;
byte MinutTimer = 0;
byte RabotaChasTimer = 0;
byte RabotaMinutTimer = 0;

//Cтатус таймера
bool TimerPower = false;

//порт переключателя
int SRDPin = 12;

void setup() {

  //  Serial.begin(9600);
  //Инициализация джостика
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  //Инициализация СРД
  pinMode(SRDPin, OUTPUT);

  //Инициализация часов
  //  delay(300);
  Serial.begin(9600);
  //time.begin(RTC_DS1302, 12, 10, 11); // на базе чипа DS1302, вывод RST, вывод CLK, вывод DAT   доступны любые выводы
  time.begin(RTC_DS3231);
  // Инициализация дисплея 16 на 2
  lcd.begin(16, 2);
  // Помигаем дисплеем чтобы было понятно что работаем
  for (int i = 0; i < 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight();
  // Переводим курсор в начало координат x=0, y=0
  lcd.setCursor(4, 0);
  // Вывод привет
  lcd.print("THERMO");
  // wait a second.
  delay(1000);
  // set cursor on second line
  lcd.setCursor(5, 1);
  lcd.print("EL 23");
  delay(2000);
  //Читаем данные с EEPROM
  ProchitatTimer();
}

//***************************************************************
void loop() {
  //Проверка положения ждойстика
  char Joy ;
  Joy = position_joystik();

  //Если есть движение джойстика выводим
  if (Joy != '0') {
    //Если нажато B (кнопка) переходим в пункт меню переходим в меню
    if (Joy == 'B') {
      delay(500);
      NumMenu = 1;
      menu();

    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Joy);
      //TIME = ""; //Нужно обновить время
      delay(1000);
    }
  }
  StatusTimer();
  time_display();
  //Основная работа
  if (TimerPower) {
    //if (digitalRead(SRDPin) == LOW) {
    digitalWrite(SRDPin, HIGH);
    EEPROM.write(4, 1);//В 4 ячейке cтатус On
    time.gettime();
    EEPROM.write(5, time.day);//День старта
    //}
  } else {
    //if (digitalRead(SRDPin) == HIGH) {
    digitalWrite(SRDPin, LOW);
    EEPROM.write(4, 0);//В 4 ячейке cтатус Off
    EEPROM.write(5, 0);//День старта
    //}
  }
  delay(100);
}
//***************************************************************

void StatusTimer() {
  //Сравниванием дату часы минуты
  //Состояние системы Off/On
  //Дата старта Должна быть меньше текущей
  time.gettime();
  int StatusOnOff = EEPROM.read(4);
  int DateStart = EEPROM.read(5);
  //  Serial.println(StatusOnOff);
  //  Serial.println(DateStart);
  //Проверим статус
  if (StatusOnOff == 0) {
    //Если статус не запущен проверяем настало ли время для запуска
    //    Serial.println(ChasTimer);
    //    Serial.println(MinutTimer);
    //    Serial.println("VREMYA");
    //    Serial.println(time.Hours);
    //    Serial.println(time.minutes);
    if (ChasTimer <= time.Hours && MinutTimer <= time.minutes) {
      //Проверим возможно начало периода уже настало
      // if (RabotaChasTimer >= time.Hours && RabotaMinutTimer > time.minutes){
      // Serial.println("Power On");
      TimerPower = true;
      //}
    }//а включать и не нужно пока
  } else {
    //Если время выключения меньше времени включения нужно ждать сутки
    if (ChasTimer >= RabotaChasTimer && MinutTimer > RabotaMinutTimer) {
      //Проверяем настал ли следующий день
      if (DateStart < time.day) {
        //Проверим настало ли время выключения
        if (RabotaChasTimer <= time.Hours && RabotaMinutTimer <= time.minutes) {
          // Serial.println("Power Off tomorow");
          TimerPower = false;
        }//Время выключения
      }//Дата текущая и время не наступило
    } else {
      //Время выключения больше времени старта
      if (RabotaChasTimer <= time.Hours && RabotaMinutTimer <= time.minutes) {
        //Serial.println("Power Off today");
        TimerPower = false;
      }//Сегодня включили и сегодня выключили
    }
  }



  //  if (ChasTimer >= time.Hours && MinutTimer >= time.minutes && RabotaChasTimer <= time.Hours && RabotaMinutTimer <= time.minutes) {
  //    TimerPower = true;
  //  } else {
  //    TimerPower = false;
  //  }
}

//7777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777
String NulS(int cheslo = 0) {
  if (String(cheslo).length() == 1)
  {
    return "0" + String(cheslo);
  } else {
    return String(cheslo);
  }
}
//777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777

void ProchitatTimer() {
  ChasTimer = EEPROM.read(0);
  MinutTimer = EEPROM.read(1);
  RabotaChasTimer = EEPROM.read(2);
  RabotaMinutTimer = EEPROM.read(3);
}

//------------------------------------------------------------------------------------------------------------

int izmenitVPredelah(int Znacenie = 0, boolean dobavit = true, int minimum = 0, int maximum = 0) {
  //Если не равно минимум и максимум тогда выполняем действие иначе просто ставим минимум или максимум взависимости от действия
  if (Znacenie == minimum && !dobavit) {
    //Если добавить тогда ставим минимум
    return maximum;
  }
  if (Znacenie == maximum && dobavit) {
    //Если добавить тогда ставим минимум
    return minimum;
  }
  if (dobavit) {
    return Znacenie + 1;
  } else {
    return Znacenie - 1;
  }
}
//******************************************************************************************************************

//Вывод текущее время на экран
void time_display() {
  //String tTIME = time.gettime("d-m-Y H:i");
  String tTIME = time.gettime("dmYH:i:s");
  if (tTIME != TIME) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(time.gettime("d-m-Y"));
    lcd.setCursor(0, 1);
    lcd.print(time.gettime("H:i:s"));
    lcd.setCursor(13, 1);
    if (TimerPower) {
      lcd.print("On");
    } else {
      lcd.print("Off");
    }
    TIME = tTIME;
  }
}

//99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999
//////////////////////////////////////////////////////////////////////
//Получаем положение джойстика, если нажата кнопка она имеет приоритет
//  "B" - нажата кнопка (0)
//  "L" - максимум влево (>503) +10%
//  "R" - максимум вправо (<503) -10%
//  "U" - максимум вверх (<490) -10%
//  "D" - максимум вниз (>490) +10%
//  "0" - пусто
char position_joystik () {
  int xPosition = 0;
  int yPosition = 0;
  int buttonState = 0;
  int xUklon = 0;
  int yUklon = 0;
  int Xotklonenie = 0; //Максимальное значение отклюнения X
  int Yotklonenie = 0; //Максимальное отклюнение Y

  xPosition = analogRead(xPin);
  yPosition = analogRead(yPin);
  buttonState = digitalRead(buttonPin);

  //Отладка калибровка джойстика
  //  Serial.println(xPosition);
  //  Serial.println(yPosition);
  //  Serial.println(buttonState);

  //Получаем отклонение
  xUklon = constrain(xPosition, 502 / 100 * 90, 502 * 1.1);
  yUklon = constrain(yPosition, 494 / 100 * 90, 494 * 1.1);

  //Получаем максимальное отклюнение
  if (xUklon > xPosition)
  {
    Xotklonenie = xUklon - xPosition;
  } else
  {
    Xotklonenie = xPosition - xUklon;
  }
  if (yUklon > yPosition)
  {
    Yotklonenie = yUklon - yPosition;
  } else
  {
    Yotklonenie = yPosition - yUklon;
  }

  //Если нажата кнопка сразу возвращаем значение "B"
  if (buttonState == 0) {
    TimerPower = millis();
    return 'B';
  } else {
    //Если Х и Y погрешность в пределах 10% значит джойстик на месте
    if ((xUklon == xPosition) && ( yUklon == yPosition)) {
      return '0';
    } else {
      //Если и X и Y есть уклон тогда смотрим куда больше отклюнение
      if (Xotklonenie > Yotklonenie)
      {
        //Больше отклюнение по Х осталось опредилить куда
        if (xUklon > xPosition)
        {
          TimerPower = millis();
          return 'U';
        } else
        {
          TimerPower = millis();
          return 'D';
        }
      } else
      {
        //Больше отклюнение по Y осталось опредилить куда
        if (yUklon > yPosition)
        {
          TimerPower = millis();
          return 'L';
        } else
        {
          TimerPower = millis();
          return 'R';
        }
      }
    }
  }
}

//999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999
//Меню настройки
void menu() {
  int menuNaDisplay = 0; //Ранее выведеная позиция меню
  char PosJoy = '0';

  //Настройчи часов - настройки программ
  //Выводим на дисплей время и подмигиваем настраиваемое значение
  while (NumMenu != 0) {
    PosJoy = position_joystik();
    //выводим на экран нужное меню
    if (NumMenu != menuNaDisplay) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(myStrings[NumMenu - 1]);
      lcd.setCursor(1, 1);
      lcd.print(NumMenu);
      menuNaDisplay = NumMenu;
    }
    switch (PosJoy) {
      case 'R':
        delay(500);
        if (NumMenu < 3) {
          NumMenu = NumMenu + 1;
        }
        break;
      case 'L':
        delay(500);
        if (NumMenu > 1) {
          NumMenu = NumMenu - 1;
        }
        break;
      default: { //Проверка выхода по таймауту
          vuhod = wottimeout();
        }
        break;
      case 'B':
        delay(500);
        //Выбор подменю
        switch (NumMenu) {
          //настройка времени выводим дату и время с подсветкой значения настройки
          case 1: {
              int redaktirsimvol = 0;
              boolean vuhod = true;
              boolean podsvetka = true;
              time.gettime();
              int Psekunda = time.seconds;
              int Pminuta = time.minutes;
              int Pchas = time.Hours;
              int Pdata = time.day;
              int Pmesyac = time.month;
              int Pgod = time.year;
              //Нужно постоянно выводить дату и время и с частотой 1с пропадать и появлятся (замена редактируемого символа)
              while (vuhod) {
                //Вывод на экран с подсветкой нужного символа
                lcd.clear();
                lcd.home();
                String StrokaLcd1 = NulS(Pdata) + "-" + NulS(Pmesyac) + "-20" + NulS(Pgod);
                String StrokaLcd2 = NulS(Pchas) + ":" + NulS(Pminuta) + ":" + NulS(Psekunda);
                lcd.print(StrokaLcd1);
                lcd.setCursor(0, 1);
                lcd.print(StrokaLcd2);
                if (podsvetka)
                {
                  switch (redaktirsimvol)
                  {
                    case 0:
                      lcd.setCursor(0, 0);
                      lcd.print("  ");
                      break;
                    case 1:
                      lcd.setCursor(3, 0);
                      lcd.print("  ");
                      break;
                    case 2:
                      lcd.setCursor(6, 0);
                      lcd.print("    ");
                      break;
                    case 3:
                      lcd.setCursor(0, 1);
                      lcd.print("  ");
                      break;
                    case 4:
                      lcd.setCursor(3, 1);
                      lcd.print("  ");
                      break;
                    case 5:
                      lcd.setCursor(6, 1);
                      lcd.print("  ");
                      break;
                  }
                  podsvetka = !podsvetka;
                } else {
                  podsvetka = !podsvetka;
                }
                delay(150);
                //Управление методом перехода влево вправо
                switch (position_joystik()) {
                  case 'R':
                    delay(500);
                    if (redaktirsimvol < 5) {
                      redaktirsimvol = redaktirsimvol + 1;
                    }
                    break;
                  case 'L':
                    delay(500);
                    if (redaktirsimvol > 0) {
                      redaktirsimvol = redaktirsimvol - 1;
                    }
                    break;
                  case 'U':
                    delay(500);
                    //Если редактируемый символ № то соответвенные параметры
                    switch (redaktirsimvol)
                    {
                      case 0: //Дата 1-31
                        Pdata = izmenitVPredelah(Pdata, true, 1, 31);
                        break;
                      case 1:  //Месяц 1-12
                        Pmesyac = izmenitVPredelah(Pmesyac, true, 1, 12);
                        break;
                      case 2:  //Год 2000-2060
                        Pgod = izmenitVPredelah(Pgod, true, 0, 60);
                        break;
                      case 3: //Час 0-23
                        Pchas = izmenitVPredelah(Pchas, true, 0, 23);
                        break;
                      case 4: //Минуты 0-59
                        Pminuta = izmenitVPredelah(Pminuta, true, 0, 59);
                        break;
                      case 5: //Секунды 0-59
                        Psekunda = izmenitVPredelah(Psekunda, true, 0, 59);
                        break;
                      default:  //Проверка выхода по таймауту
                        vuhod = wottimeout();
                        break;
                    }
                    break;
                  case 'D':
                    delay(500);
                    //Если редактируемый символ № то соответвенные параметры
                    switch (redaktirsimvol)
                    {
                      case 0: //Дата 1-31
                        Pdata = izmenitVPredelah(Pdata, false, 1, 31);
                        break;
                      case 1:  //Месяц 1-12
                        Pmesyac = izmenitVPredelah(Pmesyac, false, 1, 12);
                        break;
                      case 2:  //Год 2000-2060
                        Pgod = izmenitVPredelah(Pgod, false, 0, 60);
                        break;
                      case 3: //Час 0-23
                        Pchas = izmenitVPredelah(Pchas, false, 0, 23);
                        break;
                      case 4: //Минуты 0-59
                        Pminuta = izmenitVPredelah(Pminuta, false, 0, 59);
                        break;
                      case 5: //Секунды 0-59
                        Psekunda = izmenitVPredelah(Psekunda, false, 0, 59);
                        break;
                      default:  //Проверка выхода по таймауту
                        vuhod = wottimeout();
                        break;
                    }
                    break;
                  case 'B':
                    delay(500);
                    time.settime(Psekunda, Pminuta, Pchas, Pdata, Pmesyac, Pgod); // 0  сек, 17 мин, 15 час, 1, октября, 2015 года, четверг
                    vuhod = false;
                    menuNaDisplay = 0; //Нужно обновить дисплей
                    break;
                  default:  //Проверка выхода по таймауту
                    vuhod = wottimeout();
                    break;
                }

              }
              break;
            }
          case 2://настройка таймера
            {
              byte tChasTimer = ChasTimer;
              byte tMinutTimer = MinutTimer;
              byte tRabotaChasTimer = RabotaChasTimer;
              byte yRabotaMinutTimer = RabotaMinutTimer;

              //В цикле постоянно выводим на экран данные сподсветкой настроек
              int redaktirsimvol = 0;
              boolean vuhod = true;
              boolean podsvetka = true;
              //Нужно постоянно выводить дату и время и с частотой 1с пропадать и появлятся (замена редактируемого символа)
              while (vuhod) {
                //Вывод на экран с подсветкой нужного символа
                lcd.clear();
                lcd.home();
                String StrokaLcd1 = "NACHALO " + NulS(tChasTimer) + "-" + NulS(tMinutTimer);
                String StrokaLcd2 = "KONEC " + NulS(tRabotaChasTimer) + ":" + NulS(yRabotaMinutTimer);
                lcd.print(StrokaLcd1);
                lcd.setCursor(0, 1);
                lcd.print(StrokaLcd2);
                if (podsvetka)
                {
                  switch (redaktirsimvol)
                  {
                    case 0:
                      lcd.setCursor(8, 0);
                      lcd.print("  ");
                      break;
                    case 1:
                      lcd.setCursor(11, 0);
                      lcd.print("  ");
                      break;
                    case 2:
                      lcd.setCursor(6, 1);
                      lcd.print("  ");
                      break;
                    case 3:
                      lcd.setCursor(9, 1);
                      lcd.print("  ");
                      break;
                  }
                  podsvetka = !podsvetka;
                } else {
                  podsvetka = !podsvetka;
                }
                delay(150);
                //Управление методом перехода влево вправо 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
                switch (position_joystik()) {
                  case 'R':
                    delay(500);
                    if (redaktirsimvol < 3) {
                      redaktirsimvol = redaktirsimvol + 1;
                    }
                    break;
                  case 'L':
                    delay(500);
                    if (redaktirsimvol > 0) {
                      redaktirsimvol = redaktirsimvol - 1;
                    }
                    break;
                  case 'U':
                    delay(500);
                    //Если редактируемый символ № то соответвенные параметры
                    switch (redaktirsimvol)
                    {
                      case 0: //Часы 0-23
                        tChasTimer = izmenitVPredelah(tChasTimer, true, 0, 23);
                        break;
                      case 1:  //Минуты 0-59
                        tMinutTimer = izmenitVPredelah(tMinutTimer, true, 0, 59);
                        break;
                      case 2:  //Часы2 0-23
                        tRabotaChasTimer = izmenitVPredelah(tRabotaChasTimer, true, 0, 23);
                        break;
                      case 3: //Минуты2 0-59
                        yRabotaMinutTimer = izmenitVPredelah(yRabotaMinutTimer, true, 0, 59);
                        break;
                    }
                    break;
                  case 'D':
                    delay(500);
                    //Если редактируемый символ № то соответвенные параметры
                    switch (redaktirsimvol)
                    {
                      case 0: //Часы 0-23
                        tChasTimer = izmenitVPredelah(tChasTimer, false, 0, 23);
                        break;
                      case 1:  //Минуты 0-59
                        tMinutTimer = izmenitVPredelah(tMinutTimer, false, 0, 59);
                        break;
                      case 2:  //Часы2 0-23
                        tRabotaChasTimer = izmenitVPredelah(tRabotaChasTimer, false, 0, 23);
                        break;
                      case 3: //Минуты2 0-59
                        yRabotaMinutTimer = izmenitVPredelah(yRabotaMinutTimer, false, 0, 59);
                        break;
                    }
                    break;
                  case 'B':
                    delay(500);
                    //Если есть изменения тогда записать
                    if (tChasTimer != ChasTimer || tMinutTimer != MinutTimer || tRabotaChasTimer != RabotaChasTimer || yRabotaMinutTimer != RabotaMinutTimer) {
                      lcd.clear();
                      lcd.home();
                      lcd.print("SAVE CONFIG");
                      delay(1000);
                      EEPROM.write(0, tChasTimer);
                      EEPROM.write(1, tMinutTimer);
                      EEPROM.write(2, tRabotaChasTimer);
                      EEPROM.write(3, yRabotaMinutTimer);
                      ProchitatTimer();
                    }
                    vuhod = false;
                    menuNaDisplay = 0; //Нужно обновить дисплей
                    break;
                  default:  //Проверка выхода по таймауту
                    vuhod = wottimeout();
                    break;
                }//Управление джойстиком
              }//Меню редактирования
              break;
            }//меню настройка таймера
          case 3:
            NumMenu = 0; //Выход
          default:  //Проверка выхода по таймауту
            vuhod = wottimeout();
            break;
        }
    }
  }
}

//Проверка таймаута по времени
bool wottimeout() {
  if (TimerPower > 3000) {
    return true;
  } else {
    return false;
  }
}

