#include "config.h"
#include "BMSModule.h"
#include "Logger.h"


BMSModule::BMSModule()
{
  for (int i = 0; i < 16; i++)
  {
    cellVolt[i] = 0.0f;
    lowestCellVolt[i] = 5.0f;
    highestCellVolt[i] = 0.0f;
  }
  moduleVolt = 0.0f;
  temperatures[0] = 0.0f;
  temperatures[1] = 0.0f;
  temperatures[2] = 0.0f;
  temperatures[3] = 0.0f;
  lowestTemperature = 200.0f;
  highestTemperature = -100.0f;
  lowestModuleVolt = 200.0f;
  highestModuleVolt = 0.0f;
  exists = false;
  reset = false;
  moduleAddress = 0;
  timeout = 30000; //milliseconds before comms timeout;
}

void BMSModule::clearmodule()
{
  for (int i = 0; i < 16; i++)
  {
    cellVolt[i] = 0.0f;
  }
  moduleVolt = 0.0f;
  temperatures[0] = 0.0f;
  temperatures[1] = 0.0f;
  temperatures[2] = 0.0f;
  temperatures[3] = 0.0f;
  exists = false;
  reset = false;
  moduleAddress = 0;
}

void BMSModule::decodevwtemp(BMS_CAN_MESSAGE &msg)
{
  if (msg.buf[7] == 0xFD)
  {
    if (msg.buf[2] != 0xFD)
    {
      temperatures[0] = (msg.buf[2] * 0.5) - 40;
    }
  }
  else
  {
    temperatures[0] = (msg.buf[0] * 0.5) - 43;
    if (msg.buf[4] < 0xF0)
    {
      temperatures[1] = (msg.buf[4] * 0.5) - 43;
    }
    else
    {
      temperatures[1] = 0;
    }
    if (msg.buf[5] < 0xF0)
    {
      temperatures[2] = (msg.buf[5] * 0.5) - 43;
    }
    else
    {
      temperatures[2] = 0;
    }
  }
}

void BMSModuleManager::decodebmwtemp(BMS_CAN_MESSAGE &msg, int debug, int CSC)
{
  int CMU = (msg.id & 0x00F) + 1;
  modules[CMU].decodebmwtemp(msg, CSC);
  if (debug == 1 && CMU > 0)
  {
    Serial.println();
    Serial.print(CMU);
    Serial.print(" Temp Found");
  }
}

void BMSModule::decodevwcan(int Id, BMS_CAN_MESSAGE &msg)
{
  switch (Id)
  {
    case 0:
      cmuerror = 0;
      cellVolt[0] = (uint16_t(msg.buf[1] >> 4) + uint16_t(msg.buf[2] << 4) + 1000) * 0.001;
      cellVolt[2] = (uint16_t(msg.buf[5] << 4) + uint16_t(msg.buf[4] >> 4) + 1000) * 0.001;
      cellVolt[1] = (msg.buf[3] + uint16_t((msg.buf[4] & 0x0F) << 8) + 1000) * 0.001;
      cellVolt[3] = (msg.buf[6] + uint16_t((msg.buf[7] & 0x0F) << 8) + 1000) * 0.001;

      break;
    case 1:
      cmuerror = 0;
      cellVolt[4] = (uint16_t(msg.buf[1] >> 4) + uint16_t(msg.buf[2] << 4) + 1000) * 0.001;
      cellVolt[6] = (uint16_t(msg.buf[5] << 4) + uint16_t(msg.buf[4] >> 4) + 1000) * 0.001;
      cellVolt[5] = (msg.buf[3] + uint16_t((msg.buf[4] & 0x0F) << 8) + 1000) * 0.001;
      cellVolt[7] = (msg.buf[6] + uint16_t((msg.buf[7] & 0x0F) << 8) + 1000) * 0.001;

      break;

    case 2:
      cmuerror = 0;
      cellVolt[8] = (uint16_t(msg.buf[1] >> 4) + uint16_t(msg.buf[2] << 4) + 1000) * 0.001;
      cellVolt[10] = (uint16_t(msg.buf[5] << 4) + uint16_t(msg.buf[4] >> 4) + 1000) * 0.001;
      cellVolt[9] = (msg.buf[3] + uint16_t((msg.buf[4] & 0x0F) << 8) + 1000) * 0.001;
      cellVolt[11] = (msg.buf[6] + uint16_t((msg.buf[7] & 0x0F) << 8) + 1000) * 0.001;
      break;

    case 3:
      cmuerror = 0;
      cellVolt[12] = (uint16_t(msg.buf[1] >> 4) + uint16_t(msg.buf[2] << 4) + 1000) * 0.001;
      break;

    default:
      break;

  }
  if (getLowTemp() < lowestTemperature) lowestTemperature = getLowTemp();
  if (getHighTemp() > highestTemperature) highestTemperature = getHighTemp();

  for (int i = 0; i < 13; i++)
  {
    if (lowestCellVolt[i] > cellVolt[i] && cellVolt[i] >= IgnoreCell) lowestCellVolt[i] = cellVolt[i];
    if (highestCellVolt[i] < cellVolt[i] && cellVolt[i] > 5.0) highestCellVolt[i] = cellVolt[i];
  }

  if (cmuerror == 0)
  {
    lasterror = millis();
  }
  else
  {
    if (millis() - lasterror < timeout)
    {
      if (lasterror + timeout - millis() < 5000)
      {
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("Module");
        SERIALCONSOLE.print(moduleAddress);
        SERIALCONSOLE.print("Counter Till Can Error : ");
        SERIALCONSOLE.println(lasterror + timeout - millis() );
      }
    }
    else
    {
      for (int i = 0; i < 8; i++)
      {
        cellVolt[i] = 0.0f;
      }
      moduleVolt = 0.0f;
      temperatures[0] = 0.0f;
      temperatures[1] = 0.0f;
      temperatures[2] = 0.0f;
    }
  }
}

void BMSModule::decodevwcan(int Id, BMS_CAN_MESSAGE &msg)
{
  switch (Id)
  {
    case 0:
      error = msg.buf[0] + (msg.buf[1] << 8) + (msg.buf[2] << 16) + (msg.buf[3] << 24);
      balstat = (msg.buf[5]<< 8) + msg.buf[4];
      break;

    case 1:
      if (balstat == 0 && Ign == 0)
      {
        cellVolt[0] = float(msg.buf[0] + (msg.buf[1] & 0x3F) * 256) / 1000;
        cellVolt[1] = float(msg.buf[2] + (msg.buf[3] & 0x3F) * 256) / 1000;
        cellVolt[2] = float(msg.buf[4] + (msg.buf[5] & 0x3F) * 256) / 1000;
      }
      break;

    case 2:
      if (balstat == 0 && Ign == 0)
      {
        cellVolt[3] = float(msg.buf[0] + (msg.buf[1] & 0x3F) * 256) / 1000;
        cellVolt[4] = float(msg.buf[2] + (msg.buf[3] & 0x3F) * 256) / 1000;
        cellVolt[5] = float(msg.buf[4] + (msg.buf[5] & 0x3F) * 256) / 1000;
      }
      break;

    case 3:
      if (balstat == 0 && Ign == 0)
      {
        cellVolt[6] = float(msg.buf[0] + (msg.buf[1] & 0x3F) * 256) / 1000;
        cellVolt[7] = float(msg.buf[2] + (msg.buf[3] & 0x3F) * 256) / 1000;
        cellVolt[8] = float(msg.buf[4] + (msg.buf[5] & 0x3F) * 256) / 1000;
      }
      break;

    case 4:
      if (balstat == 0 && Ign == 0)
      {
        cellVolt[9] = float(msg.buf[0] + (msg.buf[1] & 0x3F) * 256) / 1000;
        cellVolt[10] = float(msg.buf[2] + (msg.buf[3] & 0x3F) * 256) / 1000;
        cellVolt[11] = float(msg.buf[4] + (msg.buf[5] & 0x3F) * 256) / 1000;
      }
      break;

    case 5:
      if (balstat == 0 && Ign == 0)
      {
        cellVolt[12] = float(msg.buf[0] + (msg.buf[1] & 0x3F) * 256) / 1000;
        cellVolt[13] = float(msg.buf[2] + (msg.buf[3] & 0x3F) * 256) / 1000;
        cellVolt[14] = float(msg.buf[4] + (msg.buf[5] & 0x3F) * 256) / 1000;
      }
      break;


    case 6:
      if (balstat == 0 && Ign == 0)
      {
        cellVolt[15] = float(msg.buf[0] + (msg.buf[1] & 0x3F) * 256) / 1000;
      }
      break;

    default:

      break;
  }
  for (int i = 0; i < 16; i++)
  {
    if (lowestCellVolt[i] > cellVolt[i] && cellVolt[i] >= IgnoreCell) lowestCellVolt[i] = cellVolt[i];
    if (highestCellVolt[i] < cellVolt[i] && cellVolt[i] > 5.0) highestCellVolt[i] = cellVolt[i];
  }
}

/*
  Reading the status of the board to identify any flags, will be more useful when implementing a sleep cycle
*/

uint8_t BMSModule::getFaults()
{
  return faults;
}

uint8_t BMSModule::getAlerts()
{
  return alerts;
}

uint8_t BMSModule::getCOVCells()
{
  return COVFaults;
}

uint8_t BMSModule::getCUVCells()
{
  return CUVFaults;
}

float BMSModule::getCellVoltage(int cell)
{
  if (cell < 0 || cell > 13) return 0.0f;
  return cellVolt[cell];
}

float BMSModule::getLowCellV()
{
  float lowVal = 10.0f;
  for (int i = 0; i < 13; i++) if (cellVolt[i] < lowVal && cellVolt[i] > IgnoreCell) lowVal = cellVolt[i];
  return lowVal;
}

float BMSModule::getHighCellV()
{
  float hiVal = 0.0f;
  for (int i = 0; i < 13; i++)
    if (cellVolt[i] > IgnoreCell && cellVolt[i] < 5.0)
    {
      if (cellVolt[i] > hiVal) hiVal = cellVolt[i];
    }
  return hiVal;
}

float BMSModule::getAverageV()
{
  int x = 0;
  float avgVal = 0.0f;
  for (int i = 0; i < 13; i++)
  {
    if (cellVolt[i] > IgnoreCell && cellVolt[i] < 5.0)
    {
      x++;
      avgVal += cellVolt[i];
    }
  }

  scells = x;
  avgVal /= x;
  return avgVal;
}

int BMSModule::getscells()
{
  return scells;
}

float BMSModule::getHighestModuleVolt()
{
  return highestModuleVolt;
}

float BMSModule::getLowestModuleVolt()
{
  return lowestModuleVolt;
}

float BMSModule::getHighestCellVolt(int cell)
{
  if (cell < 0 || cell > 13) return 0.0f;
  return highestCellVolt[cell];
}

float BMSModule::getLowestCellVolt(int cell)
{
  if (cell < 0 || cell > 13) return 0.0f;
  return lowestCellVolt[cell];
}

float BMSModule::getHighestTemp()
{
  return highestTemperature;
}

float BMSModule::getLowestTemp()
{
  return lowestTemperature;
}

float BMSModule::getLowTemp()
{
  if (sensor == 0)
  {
    if (getAvgTemp() > 0.5)
    {
      if (temperatures[0] > 0.5)
      {
        if (temperatures[0] < temperatures[1] && temperatures[0] < temperatures[2])
        {
          return (temperatures[0]);
        }
      }
      if (temperatures[1] > 0.5)
      {
        if (temperatures[1] < temperatures[0] && temperatures[1] < temperatures[2])
        {
          return (temperatures[1]);
        }
      }
      if (temperatures[2] > 0.5)
      {
        if (temperatures[2] < temperatures[1] && temperatures[2] < temperatures[0])
        {
          return (temperatures[2]);
        }
      }
    }
  }
  else
  {
    return temperatures[sensor - 1];
  }
}

float BMSModule::getHighTemp()
{
  if (sensor == 0)
  {
    return (temperatures[0] < temperatures[1]) ? temperatures[1] : temperatures[0];
  }
  else
  {
    return temperatures[sensor - 1];
  }
}

float BMSModule::getAvgTemp()
{
  if (sensor == 0)
  {
    if ((temperatures[0] + temperatures[1] + temperatures[2]) / 3.0f > 0.5)
    {
      if (temperatures[0] > 0.5 && temperatures[1] > 0.5 && temperatures[2] > 0.5)
      {
        return (temperatures[0] + temperatures[1] + temperatures[2]) / 3.0f;
      }
      if (temperatures[0] < 0.5 && temperatures[1] > 0.5 && temperatures[2] > 0.5)
      {
        return (temperatures[1] + temperatures[2]) / 2.0f;
      }
      if (temperatures[0] > 0.5 && temperatures[1] < 0.5 && temperatures[2] > 0.5)
      {
        return (temperatures[0] + temperatures[2]) / 2.0f;
      }
      if (temperatures[0] > 0.5 && temperatures[1] > 0.5 && temperatures[2] < 0.5)
      {
        return (temperatures[0] + temperatures[1]) / 2.0f;
      }
      if (temperatures[0] > 0.5 && temperatures[1] < 0.5 && temperatures[2] < 0.5)
      {
        return (temperatures[0]);
      }
      if (temperatures[0] < 0.5 && temperatures[1] > 0.5 && temperatures[2] < 0.5)
      {
        return (temperatures[1]);
      }
      if (temperatures[0] < 0.5 && temperatures[1] < 0.5 && temperatures[2] > 0.5)
      {
        return (temperatures[2]);
      }
      if (temperatures[0] < 0.5 && temperatures[1] < 0.5 && temperatures[2] < 0.5)
      {
        return (-80);
      }
    }
  }
  else
  {
    return temperatures[sensor - 1];
  }
}

float BMSModule::getModuleVoltage()
{
  moduleVolt = 0;
  for (int I; I < 13; I++)
  {
    if (cellVolt[I] > IgnoreCell && cellVolt[I] < 5.0)
    {
      moduleVolt = moduleVolt + cellVolt[I];
    }
  }
  return moduleVolt;
}

float BMSModule::getTemperature(int temp)
{
  if (temp < 0 || temp > 2) return 0.0f;
  return temperatures[temp];
}

void BMSModule::setAddress(int newAddr)
{
  if (newAddr < 0 || newAddr > MAX_MODULE_ADDR) return;
  moduleAddress = newAddr;
}

int BMSModule::getAddress()
{
  return moduleAddress;
}

bool BMSModule::isExisting()
{
  return exists;
}

bool BMSModule::isReset()
{
  return reset;
}

void BMSModule::settempsensor(int tempsensor)
{
  sensor = tempsensor;
}

void BMSModule::setExists(bool ex)
{
  exists = ex;
}

void BMSModule::setDelta(float ex)
{
  VoltDelta = ex;
}

void BMSModule::setReset(bool ex)
{
  reset = ex;
}

void BMSModule::setIgnoreCell(float Ignore)
{
  IgnoreCell = Ignore;
  Serial.println();
  Serial.println();
  Serial.println(Ignore);
  Serial.println();

}
