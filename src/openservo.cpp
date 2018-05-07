#include "openservo.h"
#include "i2c.h"
#include "defines.h"
#include "debug.h"

#include <unistd.h>

namespace openservo {

Servo::Servo(ServoBus* bus, int address): bus(bus), locked(true) {

  for (int i = 0; i < SERVO_MAX_SPACE; i++) {
    local[i] = false;
    data[i] = 0;
  }

  data[TWI_ADDRESS] = address;

  update(true);

}

Servo::~Servo() {

}

bool Servo::reset() {

  return command(RESET);

}

bool Servo::enable() {

  return command(PWM_ENABLE);

}

bool Servo::disable() {

  return command(PWM_DISABLE);

}

void Servo::unlock() {

  locked = false;

}

bool Servo::registersCommit() {

  return command(REGISTERS_SAVE);

}

bool Servo::registersRestore() {

  return command(REGISTERS_RESTORE);

}

bool Servo::registersDefault() {

  return command(REGISTERS_DEFAULT);

}

int Servo::getType() {
  
  return read1B(DEVICE_TYPE);

}

int Servo::getSubType() {

  return read1B(DEVICE_SUBTYPE);
  
}

int Servo::getVersion() {

  return read2B(VERSION_MAJOR);
  
}

int Servo::getFlags() {

  return read2B(FLAGS_HI);
  
}

int Servo::getTimer() {

  return read2B(TIMER_HI);
  
}

int Servo::getPosition() {

  return read2B(POSITION_HI);
  
}

int Servo::getVelocity() {

  return read2B(VELOCITY_HI);
  
}

int Servo::getPower() {

  return read2B(POWER_HI);
  
}

int Servo::getPWM() {

  return read1B(PWM_CW);
  
}

int Servo::getSeekPosition() {

  return read2B(SEEK_HI);
  
}

int Servo::getSeekVelocity() {
  
  return read2B(SEEK_VELOCITY_HI);

}

int Servo::getCurveInVelocity() {

  return read2B(CURRENT_CUT_OFF_HI);
  
}

int Servo::getCurveOutVelocity(){
  
  return read2B(CURRENT_SOFT_CUT_OFF_HI);

}

int Servo::getAddress() {

  return read1B(TWI_ADDRESS);

}

int Servo::getPidDeadband()  {

  return read1B(PID_DEADBAND);

}

int Servo::getPidGainProportional()  {

  return read2B(PID_PGAIN_HI);

}

int Servo::getPidGainDerivative()  {

  return read2B(PID_DGAIN_HI);

}

int Servo::getPidGainIntegral()  {

  return read2B(PID_IGAIN_HI);

}

int Servo::getPwmFreqDivider()  {

  return read2B(CURRENT_SOFT_CUT_OFF_HI);

}

int Servo::getMinSeek()  {

  return read2B(MIN_SEEK_HI);

}

int Servo::getMaxSeek()  {

  return read2B(MAX_SEEK_HI);

}

int Servo::getReverseSeek()  {

  return read1B(REVERSE_SEEK);

}

int Servo::getReverseSeekTo()  {

  return read2B(CURRENT_SOFT_CUT_OFF_HI);

}

int Servo::getServoID()  {

  return read2B(SERVO_ID_HI);

}

int Servo::getCurrentCutOff()  {

  return read2B(CURRENT_SOFT_CUT_OFF_HI);

}

int Servo::getCurrentSoftCuttOff()  {

  return read2B(CURRENT_SOFT_CUT_OFF_HI);

}

void Servo::setSeekPossition(int value) {

  write2B(SEEK_HI, value);

}

void Servo::setSeekVelocity(int value) {

  write2B(SEEK_VELOCITY_HI, value);

}


void Servo::setCurveInVelocity(int value) {

  write2B(CURVE_IN_VELOCITY_HI, value);

}

void Servo::setCurveOutVelocity(int value) {

  write2B(CURVE_OUT_VELOCITY_HI, value);

}

void Servo::setAddress(int value) {

  if (locked) return;

  write1B(TWI_ADDRESS, value);

}

void Servo::setPidDeadband(int value) {

  if (locked) return;

  write1B(PID_DEADBAND, value);

}

void Servo::setPidGain(int proportional, int derivative, int integral) {

  if (locked) return;

}

void Servo::setPwmFreqDivider(int value) {

  if (locked) return;

  write2B(PWM_FREQ_DIVIDER_HI, value);

}

void Servo::setMinSeek(int value) {

  if (locked) return;

  write2B(MIN_SEEK_HI, value);

}

void Servo::setMaxSeek(int value) {

  if (locked) return;

  write2B(MAX_SEEK_HI, value);

}

void Servo::setReverseSeek(int value) {

  if (locked) return;

}

void Servo::setReverseSeekTo(int value) {

  if (locked) return;

  write1B(REVERSE_SEEK, value);

}

void Servo::setServoID(int value) {

  if (locked) return;

  write2B(SERVO_ID_HI, value);

}

void Servo::setCurrentCutOff(int value) {

  if (locked) return;

  write2B(CURRENT_CUT_OFF_HI, value);

}

void Servo::setCurrentSoftCuttOff(int value) {

  if (locked) return;

  write2B(CURRENT_SOFT_CUT_OFF_HI, value);

}

int Servo::read2B(int address) {
  int value = 0;

  value = ((int)data[address]) << 8;
  value |= (int)data[address+1];

  return value;
}

int Servo::read1B(int address) {

  int value = 0;

  value = ((int)data[address]);

  return value;

}

void Servo::write2B(int address, int value) {

  data[value+1] = (unsigned char)value;
  data[value] = (unsigned char)(value >> 8);

  local[value+1] = true;
  local[value] = true;

}

void Servo::write1B(int address, int value) {

  data[value] = (unsigned char)value;
  local[value] = true;

}

bool Servo::command(unsigned char cmd) {

  if (!bus) return false;

  return bus->send(getAddress(), cmd, NULL, 0);

}

bool Servo::update(bool full) {

  if (!bus) return false;

  int i = 0;

  int address = getAddress();

  if (!locked) {

      if (!command(WRITE_ENABLE)) return false;

  }

  while (i < SERVO_MAX_SPACE) {

    int start = i;

    while (local[i]) {
      local[i] = false;
      i++;
    }

    if (start < i) {
      if (!bus->send(address, start, &data[start], i - start)) return false;
    }

    i++;

  }

  if (!locked) {

      if (!command(WRITE_DISABLE)) return false;
      locked = true;

  }

  int from = full ? 0 : FLAGS_HI;
  int to = full ? CURRENT_SOFT_CUT_OFF_LO : VOLTAGE_LO;

  int data_length = to - from + 1;
  if (!bus->receive(address, from, &data[from], data_length)) return false;

  for (i = from; i < to + 1; i++) {
    local[i] = false;
  }

  return true;
}

ServoBus::ServoBus() {

  __debug_enable();

  handle = NULL;

}

ServoBus::~ServoBus() {

  close();

}


bool ServoBus::open(const string& port) {

  if (handle) close();

#ifdef _BUILD_MPSSE
  if (port.empty()) {
    if ((handle = i2c_open(NULL, I2C_MPSSE)) == NULL)
      return false;
  } else {
#endif

    if ((handle = i2c_open(port.c_str(), I2C_DIRECT)) == NULL)
      return false;

#ifdef _BUILD_MPSSE
  }
#endif

  return true;
}

bool ServoBus::close() {
  if (!handle) return false;

  return i2c_close((i2c_handle*) &handle) != 0;
}

int ServoBus::scan(bool force) {

  if (force)
    servos.clear();
  else
    cleanupServos();

  addServos();

  return servos.size();
}

bool ServoBus::update(bool full) {

  bool result = true;

  for (vector<ServoHandler>::iterator it = servos.begin(); it != servos.end(); it++) {

    result &= (*it)->update(full);

  }

  return result;
}

ServoHandler ServoBus::get(int i) {

  return servos[i];

}

int ServoBus::size() {
  return servos.size();
}

int ServoBus::cleanupServos()
{
  if (!handle)
    return -1;

  unsigned char tmp[128];
  int count = i2c_scan((i2c_handle)handle, tmp);

  int counter = 0;

  // odstrani neobstoječ servo
  if (servos.size() > 0)
  {
    if (count == 0)
    {
      counter = servos.size();
      servos.clear();
      return counter;
    }
    for (int w = 0; w < count; w++)
    {
      for (int q = 0; q < servos.size(); q++)
      {
        if (servos[q]->getAddress() == (unsigned int)tmp[w])
        {
          servos.erase(servos.begin() + q);
          counter++;
          break;
        }
      }
    }

  }
  return counter;
}

/*
  Scan for servos on the bus and add them if they are not registered yet.
*/
int ServoBus::addServos() {

  if (!handle)
    return -1;

  unsigned char tmp[128];
  int count = i2c_scan((i2c_handle)handle, tmp);
  int counter = 0;

  for (int q = 0; q < count; q++)
  {
    if (!exists((unsigned int)tmp[q]))
    {
      ServoHandler servo(new Servo(this, (unsigned int)tmp[q]));
      servos.push_back(servo);
      counter++;
    }
  }

  return counter;
}

bool ServoBus::exists(int address) {

  return find(address) != NULL;

}

// writing data to servo
bool ServoBus::send(unsigned char address, unsigned char data_addr, unsigned char* data, int data_len) {

  if (i2c_select((i2c_handle)handle, address) != 0)
    return false;

  // If data_len == 0 then we have a command
  if (data_len < 1) {
    // first, send data address (7th bit denotes command or address)
    data_addr |= 0x80;
  } else {
    // first, send data address (7th bit denotes command or address)
    data_addr &= 0x7F;
  }

  unsigned char tmp_ch[data_len + 1];
  tmp_ch[0] = data_addr;

  if (data_len > 0)
    memcpy(&tmp_ch[1], data, data_len);

  return i2c_write((i2c_handle)handle, tmp_ch, data_len + 1) != 0;

}

// read data from servo
bool ServoBus::receive(unsigned char address, unsigned char data_addr, unsigned char* data, int data_len)
{
  if (i2c_select((i2c_handle)handle, address) != 0)
    return false;

  // first, send data address (7th bit denotes command or address)
  data_addr &= 0x7F;
  if (i2c_write((i2c_handle)handle, &data_addr, 1) != 0)
    return false;

  // read the data
  return i2c_read((i2c_handle)handle, data, data_len) != 0;

}

ServoHandler ServoBus::find(int address) {
  for (int q = 0; q < servos.size(); q++) {
    if (servos[q]->getAddress() == address)
      return servos[q];
  }
  return NULL;
}

}