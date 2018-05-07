#ifndef __OPENSERVO_LIB
#define __OPENSERVO_LIB

#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <memory>

using namespace std;

#define SERVO_MAX_SPACE 0x80

namespace openservo {

class ServoBus;

class Servo {
friend ServoBus;
public:

  virtual ~Servo();

  // commands
  bool reset();
  bool enable();
  bool disable();
  bool registersCommit();
  bool registersRestore();
  bool registersDefault();

  bool set(const string& name, int value);
  int get(const string& name) const;

  vector<string> list() const;
  bool isReadonly(const string& name) const;
  bool isProtected(const string& name) const;
  bool exists(const string& name) const;

  int getType();
  int getSubType();
  pair<int, int> getVersion();
  int getFlags();
  int getTimer();
  int getPosition();
  int getVelocity();
  int getPower();
  int getPWM();

  int getSeekPosition();
  int getSeekVelocity();
  int getCurveInVelocity();
  int getCurveOutVelocity();

  void setSeekPosition(int value);
  void setSeekVelocity(int value);

  int getAddress();
  int getMinSeek();
  int getMaxSeek();

  // Protected write methods

  void unlock();

  bool update(bool full = false);

protected:

  Servo(ServoBus* bus, int address);

private:

  string normalize(const string& name) const;

  int read2B(const int address) const;
  int read1B(const int address) const;

  void write2B(const int address, int value);
  void write1B(const int address, int value);

  bool command(unsigned char cmd);

  ServoBus* bus;

  bool responsive;  // TODO: not used at the moment
  bool updated;
  bool locked;

  unsigned char data[SERVO_MAX_SPACE];
  bool local[SERVO_MAX_SPACE];

};

typedef std::shared_ptr<Servo> ServoHandler;

class ServoBus {
friend Servo;
public:

  ServoBus();
  ~ServoBus();

  bool open(const string& port);
  bool close();
  bool update(bool full = false);

  int scan(bool force = false);

  ServoHandler get(int i);
  ServoHandler find(int address);
  bool exists(int address); 
  int size();

protected:

  bool send(unsigned char address, unsigned char data_address, unsigned char* data, int data_lenght); 
  bool receive(unsigned char address, unsigned char data_address, unsigned char* data, int data_lenght);

private:

  void* handle;
  vector<ServoHandler> servos;
 
  int cleanupServos();
  int addServos();

};

}

#endif
