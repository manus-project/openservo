#include "openservo.h"
#include "i2c.h"
#include "defines.h"
#include "debug.h"

#include <map>
#include <algorithm>
#include <string> 

#include <unistd.h>

namespace openservo {

#define REGISTER_READONLY 1
#define REGISTER_PROTECTED 2

class Register {
public:
  Register(int address, int length, int flags = 0): address(address),
   length(length), flags(flags) {

  }

  ~Register() {

  }

  int address;
  int length;
  int flags;

};

map<string, Register> _registers = {
  {"type", Register(DEVICE_TYPE, 1, REGISTER_READONLY)},
  {"subtype", Register(DEVICE_SUBTYPE, 1, REGISTER_READONLY)},
  {"version.major", Register(VERSION_MAJOR, 1, REGISTER_READONLY)},
  {"version.minor", Register(VERSION_MINOR, 1, REGISTER_READONLY)},
  {"flags", Register(FLAGS_HI, 2, REGISTER_READONLY)},
  {"timer", Register(TIMER_HI, 2, REGISTER_READONLY)},
  {"position", Register(POSITION_HI, 2, REGISTER_READONLY)},
  {"velocity", Register(VELOCITY_HI, 2, REGISTER_READONLY)},
  {"power", Register(POWER_HI, 2, REGISTER_READONLY)},
  {"pwm.cw", Register(PWM_CW, 1, REGISTER_READONLY)},
  {"pwm.ccw", Register(PWM_CCW, 1, REGISTER_READONLY)},

  {"seek", Register(SEEK_HI, 2)},
  {"seek.velocity", Register(SEEK_VELOCITY_HI, 2)},
  {"voltage", Register(VOLTAGE_HI, 2)},


  {"address", Register(TWI_ADDRESS, 1, REGISTER_PROTECTED)},
  {"pid.deadband", Register(PID_DEADBAND, 1, REGISTER_PROTECTED)},
  {"pid.proportional", Register(PID_PGAIN_HI, 2, REGISTER_PROTECTED)},
  {"pid.derivative", Register(PID_DGAIN_HI, 2, REGISTER_PROTECTED)},
  {"pid.integral", Register(PID_IGAIN_HI, 2, REGISTER_PROTECTED)},
  {"pwm.divider", Register(PWM_FREQ_DIVIDER_HI, 2, REGISTER_PROTECTED)},
  {"seek.min", Register(MIN_SEEK_HI, 2, REGISTER_PROTECTED)},
  {"seek.max", Register(MAX_SEEK_HI, 2, REGISTER_PROTECTED)},
  {"seek.reverse", Register(REVERSE_SEEK, 1, REGISTER_PROTECTED)},

  {"servo", Register(SERVO_ID_HI, 2, REGISTER_PROTECTED)},
  {"cutoff", Register(CURRENT_CUT_OFF_HI, 2, REGISTER_PROTECTED)},
  {"cutoff.soft", Register(CURRENT_SOFT_CUT_OFF_HI, 2, REGISTER_PROTECTED)},  

  };

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
  
  return get("type");

}

int Servo::getSubType() {

  return get("subtype");
  
}

pair<int, int> Servo::getVersion() {

  return pair<int, int>(get("version.major"), get("version.minor"));
  
}

int Servo::getFlags() {

  return get("flags");
  
}

int Servo::getTimer() {

  return get("timer");
  
}

int Servo::getPosition() {

  return get("position");
  
}

int Servo::getVelocity() {

  return get("velocity");
  
}

int Servo::getPower() {

  return get("power");
  
}

int Servo::getPWM() {

  return get("pwm");
  
}

int Servo::getSeekPosition() {

  return get("seek");
  
}

int Servo::getSeekVelocity() {
  
  return get("seek.velocity");

}

int Servo::getAddress() {

  return get("address");

}

int Servo::getMinSeek()  {

  return get("seek.min");

}

int Servo::getMaxSeek()  {

  return get("seek.max");

}

void Servo::setSeekPosition(int value) {

  set("seek", value);

}

void Servo::setSeekVelocity(int value) {

  set("seek.velocity", value);

}

char register_name(char in) {
  if(in <= 'Z' && in >= 'A')
    return in - ('Z' - 'z');
  if (in == '_') return '.';
  return in;
}

string Servo::normalize(const string& name) const {

  string nname(name);

  transform(nname.begin(), nname.end(), nname.begin(), register_name);

  return nname;

}

int Servo::get(const string& name) const {

  string nname = normalize(name);

  std::map<string, Register>::iterator reg;

  reg = _registers.find(nname);

  if (reg == _registers.end()) return 0;

  if (reg->second.length == 1) {
    return read1B(reg->second.address);
  } else {
    return read2B(reg->second.address);
  }

}

bool Servo::set(const string& name, int value) {

  string nname = normalize(name);

  std::map<string, Register>::iterator reg;

  reg = _registers.find(nname);

  if (reg == _registers.end()) return false;

  if (reg->second.flags == REGISTER_READONLY) return false;
  if (reg->second.flags == REGISTER_PROTECTED && locked) return false;

  if (reg->second.length == 1) {
    write1B(reg->second.address, value);
  } else {
    write2B(reg->second.address, value);
  }

  return true;
}

vector<string> Servo::list() const {
  vector<string> names;
  for(std::map<string, Register>::const_iterator it = _registers.begin();
    it != _registers.end(); ++it) {
    names.push_back(it->first);
  }
  return names;
}

bool Servo::exists(const string& name) const {

  string nname = normalize(name);
  
  std::map<string, Register>::iterator reg;

  reg = _registers.find(nname);

  return !(reg == _registers.end());

}

bool Servo::isReadonly(const string& name) const {

  string nname = normalize(name);
  
  std::map<string, Register>::iterator reg;

  reg = _registers.find(nname);

  if (reg == _registers.end()) return false;

  return reg->second.flags == REGISTER_READONLY;

}
bool Servo::isProtected(const string& name) const {

  string nname = normalize(name);
  
  std::map<string, Register>::iterator reg;

  reg = _registers.find(nname);

  if (reg == _registers.end()) return false;

  return reg->second.flags == REGISTER_PROTECTED;

}

int Servo::read2B(const int address) const {
  int value = 0;

  value = ((int)data[address]) << 8;
  value |= (int)data[address+1];

  return value;
}

int Servo::read1B(const int address) const {

  int value = 0;

  value = ((int)data[address]);

  return value;

}

void Servo::write2B(const int address, int value) {

  data[address+1] = (unsigned char)value;
  data[address] = (unsigned char)(value >> 8);

  local[address+1] = true;
  local[address] = true;

}

void Servo::write1B(const int address, int value) {

  data[address] = (unsigned char)value;
  local[address] = true;

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

  if (__is_debug_enabled())
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